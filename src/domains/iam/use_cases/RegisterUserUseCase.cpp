#include "RegisterUserUseCase.hpp"
#include "../../audit/entities/AuditLog.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <random>

namespace Domains::IAM::UseCases {

RegisterUserUseCase::RegisterUserUseCase(
    std::shared_ptr<Domains::UserManagement::Repositories::UserRepository> repository,
    std::shared_ptr<Domains::IAM::Services::BcryptService> bcryptService,
    std::shared_ptr<Domains::Audit::Repositories::AuditLogRepository> auditLogRepository,
    std::shared_ptr<Domains::TenantManagement::Repositories::TenantRepository> tenantRepository
) : repository_(repository), 
    bcryptService_(bcryptService), 
    auditLogRepository_(auditLogRepository),
    tenantRepository_(tenantRepository) {}

User RegisterUserUseCase::execute(const RegisterUserDto& dto) {
    // 1. Validar email
    Email email(dto.email);
    
    // 2. Validar nome
    if (dto.name.empty()) {
        throw std::invalid_argument("Name cannot be empty");
    }
    if (dto.name.length() > 100) {
        throw std::invalid_argument("Name is too long (max 100 characters)");
    }
    
    // Verificar caracteres inválidos (emojis, HTML, scripts)
    for (size_t i = 0; i < dto.name.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(dto.name[i]);
        
        // Bloquear caracteres de controle
        if (c < 32 || c == 127) {
            throw std::invalid_argument("Name contains invalid characters");
        }
        
        // Bloquear HTML/XSS
        if (c == '<' || c == '>' || c == '&' || c == '"' || c == '\'') {
            throw std::invalid_argument("Name contains invalid characters");
        }
        
        // Detectar emojis e caracteres não-latinos (UTF-8 multibyte com bytes > 0xEF)
        // Emojis geralmente começam com 0xF0-0xF4
        if (c >= 0xF0) {
            throw std::invalid_argument("Name contains invalid characters (emojis not allowed)");
        }
    }
    
    // 3. Verificar se email já existe
    if (repository_->existsByEmail(email.value())) {
        throw std::runtime_error("Email already registered: " + email.value());
    }
    
    // 4. Validar senha (mas NÃO hashear ainda - operação custosa)
    Password password(dto.password);
    
    // ========== VALIDAR TENANT ANTES DE CRIAR USER ==========
    // ⚠️ CRÍTICO: Validar tenant ANTES de criar user no banco
    //    Se tenant não existir, falha aqui sem criar user órfão
    
    std::string tenantId;
    std::string tenantSubdomain;
    std::string userRole;
    bool shouldCreateNewTenant = false;
    std::string newTenantName;
    
    if (!dto.tenant_subdomain.empty()) {
        // CASO 1: Usuário forneceu tenant_subdomain → Validar se existe
        LOG_INFO("User provided tenant_subdomain: " + dto.tenant_subdomain);
        
        auto existingTenant = tenantRepository_->findBySubdomain(dto.tenant_subdomain);
        if (!existingTenant.has_value()) {
            // ⚠️ FAIL FAST: Tenant não existe, abortar ANTES de criar user
            throw std::runtime_error("Tenant not found: " + dto.tenant_subdomain + ". Please contact your organization admin or register without tenant to create a new organization.");
        }
        
        tenantId = existingTenant->getId();
        tenantSubdomain = existingTenant->getSubdomain();
        userRole = "member"; // Usuário entrando em tenant existente = member
        
        LOG_INFO("✅ Tenant validated: " + tenantId + " (user will be added as " + userRole + ")");
        
    } else {
        // CASO 2: Usuário NÃO forneceu tenant_subdomain → Preparar criação de novo tenant
        LOG_INFO("No tenant_subdomain provided, will create new tenant for user");
        
        // Gerar subdomain a partir do nome ou email
        std::string baseName = dto.name.empty() ? email.value().substr(0, email.value().find('@')) : dto.name;
        std::string slug = generateSlug(baseName);
        
        // Garantir que o slug é único
        tenantSubdomain = ensureUniqueSlug(slug);
        tenantId = generateTenantId(tenantSubdomain);
        newTenantName = dto.name + "'s Organization";
        userRole = "admin"; // Primeiro usuário do tenant = admin
        shouldCreateNewTenant = true;
        
        LOG_INFO("✅ Prepared new tenant: " + tenantId + " (" + tenantSubdomain + "), user will be admin");
    }
    
    // ========== TUDO VALIDADO → AGORA SIM CRIAR USER ==========
    
    // 5. Hashear senha (operação custosa, só fazer agora que tudo foi validado)
    std::string hashedPassword = bcryptService_->hash(dto.password);
    Password hashedPasswordObj(hashedPassword, true); // true = já está hasheado
    
    // 6. Criar usuário com senha hasheada
    User user = User::create(email, hashedPasswordObj, dto.name);
    
    // 7. Salvar no banco
    User savedUser = repository_->save(user);
    
    LOG_INFO("✅ User created in database: " + savedUser.getEmail().value());
    
    // ========== CRIAR/ASSOCIAR TENANT ==========
    
    if (shouldCreateNewTenant) {
        // Criar novo tenant
        bool tenantCreated = tenantRepository_->createTenant(tenantId, newTenantName, tenantSubdomain);
        if (!tenantCreated) {
            LOG_ERROR("❌ Failed to create tenant for user: " + savedUser.getId());
            throw std::runtime_error("Failed to create organization. User created but not associated with tenant.");
        }
        
        LOG_INFO("✅ Created new tenant: " + tenantId + " (" + tenantSubdomain + ")");
    }
    
    // 8. Adicionar relacionamento user_tenants
    try {
        tenantRepository_->addUserToTenant(savedUser.getId(), tenantId, userRole);
        LOG_INFO("User-Tenant relationship created: user_id=" + savedUser.getId() + ", tenant_id=" + tenantId + ", role=" + userRole);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to add user to tenant: " + std::string(e.what()));
        // Continuar mesmo se falhar (usuário já foi criado)
    }
    
    // 9. Registrar no Audit Log
    try {
        std::string details = "{\"email\": \"" + savedUser.getEmail().value() + 
                              "\", \"tenant_id\": \"" + tenantId + 
                              "\", \"role\": \"" + userRole + "\"}";
        
        auditLogRepository_->log(
            savedUser.getId(),                      // user_id
            tenantId,                               // tenant_id
            "USER_REGISTERED",                      // action
            "user",                                 // resource
            savedUser.getId(),                      // resource_id
            details,                                // details (JSON)
            "127.0.0.1",                           // ip_address (TODO: get real IP)
            "UserAgent/1.0"                        // user_agent (TODO: get real User-Agent)
        );
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create audit log: " + std::string(e.what()));
        // Não falhar o registro por causa de erro no audit log
    }
    
    return savedUser;
}

// Helper: Gerar slug a partir de string
std::string RegisterUserUseCase::generateSlug(const std::string& input) {
    std::string slug = input;
    
    // Converter para lowercase
    std::transform(slug.begin(), slug.end(), slug.begin(), ::tolower);
    
    // Remover espaços e caracteres especiais
    slug.erase(std::remove_if(slug.begin(), slug.end(), [](unsigned char c) {
        return !std::isalnum(c) && c != '-' && c != '_';
    }), slug.end());
    
    // Limitar a 30 caracteres
    if (slug.length() > 30) {
        slug = slug.substr(0, 30);
    }
    
    return slug.empty() ? "org" : slug;
}

// Helper: Garantir slug único
std::string RegisterUserUseCase::ensureUniqueSlug(const std::string& baseSlug) {
    std::string slug = baseSlug;
    int attempt = 1;
    
    while (tenantRepository_->findBySubdomain(slug).has_value()) {
        slug = baseSlug + "-" + std::to_string(attempt);
        attempt++;
        
        if (attempt > 100) {
            // Fallback: adicionar timestamp
            auto now = std::chrono::system_clock::now().time_since_epoch().count();
            slug = baseSlug + "-" + std::to_string(now % 10000);
            break;
        }
    }
    
    return slug;
}

// Helper: Gerar ID do tenant
std::string RegisterUserUseCase::generateTenantId(const std::string& slug) {
    // Formato: {slug}-tenant-{random}
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    return slug + "-tenant-" + std::to_string(dis(gen));
}

} // namespace Domains::IAM::UseCases

