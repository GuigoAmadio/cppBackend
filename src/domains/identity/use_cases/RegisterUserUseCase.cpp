#include "RegisterUserUseCase.hpp"
#include "../../../core/utils/LoggerNew.hpp"

namespace Domains::Identity::UseCases {

RegisterUserUseCase::RegisterUserUseCase(
    std::shared_ptr<UserRepository> repository,
    std::shared_ptr<Services::BcryptService> bcryptService
) : repository_(repository), bcryptService_(bcryptService) {}

User RegisterUserUseCase::execute(const RegisterUserDto& dto) {
    // 1. Validar email
    Email email(dto.email);
    
    // 2. Verificar se email já existe
    if (repository_->existsByEmail(email.value())) {
        throw std::runtime_error("Email already registered: " + email.value());
    }
    
    // 3. Validar e hash senha
    Password password(dto.password);
    std::string hashedPassword = bcryptService_->hash(dto.password);
    Password hashedPasswordObj(hashedPassword, true); // true = já está hasheado
    
    // 4. Criar usuário com senha hasheada (role é atribuído por tenant depois)
    User user = User::create(email, hashedPasswordObj, dto.name);
    
    // 5. Salvar no banco
    User savedUser = repository_->save(user);
    
    LOG_INFO("User registered successfully: " + savedUser.getEmail().value());
    
    return savedUser;
}

} // namespace Domains::Identity::UseCases

