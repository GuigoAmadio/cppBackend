#include "GetUserUseCase.hpp"

namespace Domains::UserManagement::UseCases {

GetUserUseCase::GetUserUseCase(std::shared_ptr<UserRepository> repository)
    : repository_(repository) {}

std::optional<User> GetUserUseCase::execute(const std::string& userId) {
    return repository_->findById(userId);
}

} // namespace Domains::UserManagement::UseCases

