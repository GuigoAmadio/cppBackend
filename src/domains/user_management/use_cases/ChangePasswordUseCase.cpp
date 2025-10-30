#include "ChangePasswordUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::UserManagement::UseCases {

ChangePasswordUseCase::ChangePasswordUseCase(
    std::shared_ptr<UserRepository> repository,
    std::shared_ptr<Domains::IAM::Services::BcryptService> bcryptService,
    std::shared_ptr<Domains::Audit::Repositories::AuditLogRepository> auditLogRepository
) : repository_(repository), bcryptService_(bcryptService), auditLogRepository_(auditLogRepository) {}

void ChangePasswordUseCase::execute(const ChangePasswordDto& dto) {
    // 1. Buscar usuário
    auto userOpt = repository_->findById(dto.userId);
    
    if (!userOpt.has_value()) {
        LOG_WARNING("ChangePassword failed: user not found - " + dto.userId);
        throw std::runtime_error("User not found");
    }
    
    User user = userOpt.value();
    
    // 2. Verificar autorização (usuário só pode trocar sua própria senha)
    if (dto.userId != dto.requestingUserId) {
        LOG_WARNING("ChangePassword failed: unauthorized attempt by " + dto.requestingUserId + 
                   " to change password of " + dto.userId);
        throw std::runtime_error("You can only change your own password");
    }
    
    // 3. Validar senha antiga
    if (!bcryptService_->verify(dto.oldPassword, user.getPasswordHash())) {
        LOG_WARNING("ChangePassword failed: invalid old password - " + dto.userId);
        throw std::runtime_error("Current password is incorrect");
    }
    
    // 4. Validar nova senha (será validado pelo Password value object)
    Password newPassword(dto.newPassword);
    
    // 5. Hash da nova senha
    std::string hashedPassword = bcryptService_->hash(dto.newPassword);
    Password hashedPasswordObj(hashedPassword, true); // true = já está hasheado
    
    // 6. Atualizar senha
    user.changePassword(hashedPasswordObj);
    
    // 7. Salvar
    repository_->update(user);
    
    LOG_INFO("Password changed successfully for user: " + user.getId());
    
    // 8. Registrar no Audit Log
    try {
        // Criar JSON com detalhes da mudança de senha
        std::string detailsJson = "{\"changed_by\": \"" + dto.requestingUserId + "\", ";
        detailsJson += "\"is_self_change\": " + std::string(dto.userId == dto.requestingUserId ? "true" : "false");
        detailsJson += "}";
        
        auditLogRepository_->log(
            user.getId(),                           // user_id
            std::nullopt,                           // tenant_id (não temos contexto de tenant aqui)
            "PASSWORD_CHANGED",                     // action
            "user",                                 // resource
            user.getId(),                           // resource_id
            detailsJson,                            // details (JSON com info útil)
            "127.0.0.1",                           // ip_address (TODO: get real IP)
            "UserAgent/1.0"                        // user_agent (TODO: get real User-Agent)
        );
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create audit log: " + std::string(e.what()));
        // Não falhar a mudança de senha por causa de erro no audit log
    }
    
    // TODO: Implementar invalidação de tokens (logout forçado)
    // Isso requerá um sistema de blacklist de tokens ou versão de senha no token
}

} // namespace Domains::UserManagement::UseCases

