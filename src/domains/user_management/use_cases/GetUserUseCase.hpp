#pragma once

#include "../../user_management/entities/User.hpp"
#include "../../user_management/repositories/UserRepository.hpp"
#include <memory>
#include <string>
#include <optional>

namespace Domains::UserManagement::UseCases {

// using namespace Entities; // Removed - use full namespace
using namespace Repositories;

/**
 * @brief Use Case: Buscar usuário por ID.
 */
class GetUserUseCase {
public:
    explicit GetUserUseCase(std::shared_ptr<UserRepository> repository);
    
    /**
     * @brief Executa o caso de uso.
     * @return std::optional<User>
     */
    std::optional<User> execute(const std::string& userId);

private:
    std::shared_ptr<UserRepository> repository_;
};

} // namespace Domains::UserManagement::UseCases

