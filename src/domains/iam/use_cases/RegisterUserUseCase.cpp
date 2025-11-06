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
    std::shared_ptr<Domains::IAM::Services::JwtService> jwtService,
    std::shared_ptr<Domains::Audit::Repositories::AuditLogRepository> auditLogRepository,
    std::shared_ptr<Domains::TenantManagement::Repositories::TenantRepository> tenantRepository
) : repository_(repository), 
    bcryptService_(bcryptService),
    jwtService_(jwtService),
    auditLogRepository_(auditLogRepository),
    tenantRepository_(tenantRepository) {}

RegisterResult RegisterUserUseCase::execute(const RegisterUserDto& dto) {
    LOG_DEBUG("🔵 [REGISTER] Starting user registration");
    LOG_DEBUG("   Email: " + dto.email);
    LOG_DEBUG("   Name: " + dto.name);
    LOG_DEBUG("   Tenant Subdomain provided: " + std::string(dto.tenant_subdomain.empty() ? "NO (will create new)" : "YES (" + dto.tenant_subdomain + ")"));
    
    // 1. Validar email
    LOG_DEBUG("🔵 [REGISTER] Validating email...");
    Email email(dto.email);
    
    // 2. Validar nome
    LOG_DEBUG("🔵 [REGISTER] Validating name...");
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
    LOG_DEBUG("🔵 [REGISTER] Checking if email already exists...");
    if (repository_->existsByEmail(email.value())) {
        LOG_ERROR("❌ [REGISTER] Email already registered: " + email.value());
        throw std::runtime_error("Email already registered: " + email.value());
    }
    LOG_DEBUG("✅ [REGISTER] Email is unique");
    
    // 4. Validar senha (mas NÃO hashear ainda - operação custosa)
    LOG_DEBUG("🔵 [REGISTER] Validating password...");
    Password password(dto.password);
    
    // ========== VALIDAR TENANT ANTES DE CRIAR USER ==========
    LOG_DEBUG("🔵 [REGISTER] Validating tenant...");
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
    LOG_DEBUG("✅ [REGISTER] All validations passed, proceeding to create user");
    
    // 5. Hashear senha (operação custosa, só fazer agora que tudo foi validado)
    LOG_DEBUG("🔵 [REGISTER] Hashing password...");
    std::string hashedPassword = bcryptService_->hash(dto.password);
    Password hashedPasswordObj(hashedPassword, true); // true = já está hasheado
    
    // 6. Criar usuário com senha hasheada
    LOG_DEBUG("🔵 [REGISTER] Creating User entity...");
    User user = User::create(email, hashedPasswordObj, dto.name);
    
    // 7. Salvar no banco
    LOG_DEBUG("🔵 [REGISTER] Saving user to database...");
    User savedUser = repository_->save(user);
    
    LOG_INFO("✅ User created in database: " + savedUser.getEmail().value());
    LOG_DEBUG("   User ID: " + savedUser.getId());
    
    // ========== CRIAR/ASSOCIAR TENANT ==========
    LOG_DEBUG("🔵 [REGISTER] Creating/associating tenant...");
    
    if (shouldCreateNewTenant) {
        // Criar novo tenant
        LOG_DEBUG("   Creating new tenant with ID: " + tenantId);
        bool tenantCreated = tenantRepository_->createTenant(tenantId, newTenantName, tenantSubdomain);
        if (!tenantCreated) {
            LOG_ERROR("❌ Failed to create tenant for user: " + savedUser.getId());
            throw std::runtime_error("Failed to create organization. User created but not associated with tenant.");
        }
        
        LOG_INFO("✅ Created new tenant: " + tenantId + " (" + tenantSubdomain + ")");
    } else {
        LOG_DEBUG("   Using existing tenant: " + tenantId + " (" + tenantSubdomain + ")");
    }
    
    // 8. Adicionar relacionamento user_tenants
    LOG_DEBUG("🔵 [REGISTER] Adding user to tenant with role: " + userRole);
    try {
        tenantRepository_->addUserToTenant(savedUser.getId(), tenantId, userRole);
        LOG_INFO("✅ User-Tenant relationship created: user_id=" + savedUser.getId() + ", tenant_id=" + tenantId + ", role=" + userRole);
    } catch (const std::exception& e) {
        LOG_ERROR("❌ Failed to add user to tenant: " + std::string(e.what()));
        // Continuar mesmo se falhar (usuário já foi criado)
    }
    
    // 9. Gerar tokens JWT (similar ao login)
    LOG_DEBUG("🔑 [REGISTER] Generating JWT tokens for user: " + savedUser.getId());
    LOG_DEBUG("   Parameters: user_id=" + savedUser.getId() + 
              ", email=" + savedUser.getEmail().value() +
              ", tenant_id=" + tenantId +
              ", tenant_subdomain=" + tenantSubdomain +
              ", role=" + userRole);
    
    std::string token = jwtService_->generateTokenForTenant(
        savedUser.getId(),
        savedUser.getEmail().value(),
        tenantId,
        tenantSubdomain,
        userRole
    );
    LOG_DEBUG("   Access token generated: " + std::string(token.empty() ? "FAILED ❌" : "OK ✅ (length: " + std::to_string(token.length()) + ")"));
    
    std::string refreshToken = jwtService_->generateRefreshToken(savedUser.getId());
    LOG_DEBUG("   Refresh token generated: " + std::string(refreshToken.empty() ? "FAILED ❌" : "OK ✅ (length: " + std::to_string(refreshToken.length()) + ")"));
    
    LOG_INFO("✅ User registered and authenticated: " + savedUser.getEmail().value() + 
             " @ " + tenantSubdomain + " (tenant_id: " + tenantId + ", role: " + userRole + ")");
    
    // 10. Registrar no Audit Log
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
        LOG_DEBUG("✅ [REGISTER] Audit log created successfully");
    } catch (const std::exception& e) {
        LOG_ERROR("❌ [REGISTER] Failed to create audit log: " + std::string(e.what()));
        // Não falhar o registro por causa de erro no audit log
    }
    
    // 11. Retornar resultado com tokens
    LOG_DEBUG("🔵 [REGISTER] Preparing RegisterResult to return");
    LOG_DEBUG("   user.id: " + savedUser.getId());
    LOG_DEBUG("   token length: " + std::to_string(token.length()));
    LOG_DEBUG("   refreshToken length: " + std::to_string(refreshToken.length()));
    LOG_DEBUG("   tenant_id: " + tenantId);
    LOG_DEBUG("   tenant_subdomain: " + tenantSubdomain);
    LOG_DEBUG("   role: " + userRole);
    
    return RegisterResult{
        savedUser,
        token,
        refreshToken,
        tenantId,
        tenantSubdomain,
        userRole
    };
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

