#pragma once

#include <string>
#include <memory>
#include <optional>
#include "../repositories/WorkspaceRepository.hpp"

namespace Domain {
namespace Workspace {

/**
 * @brief Use Case: Atualizar workspace
 */
class UpdateWorkspaceUseCase {
private:
    std::shared_ptr<WorkspaceRepository> repository_;

public:
    struct Input {
        std::string workspaceId;
        std::string userId;  // Quem está atualizando
        std::optional<std::string> name;
        std::optional<std::string> description;
        std::optional<std::string> settings;
    };

    struct Output {
        bool success;
        std::string message;
    };

    explicit UpdateWorkspaceUseCase(std::shared_ptr<WorkspaceRepository> repository)
        : repository_(repository) {}

    Output execute(const Input& input) {
        try {
            // Verificar se usuário tem permissão
            auto member = repository_->findMember(input.workspaceId, input.userId);
            if (!member.has_value() || !member->canManageSettings()) {
                return Output{false, "Access denied: insufficient permissions"};
            }

            // Atualizar
            bool success = repository_->update(
                input.workspaceId,
                input.name,
                input.description,
                input.settings
            );

            if (success) {
                return Output{true, "Workspace updated successfully"};
            } else {
                return Output{false, "Failed to update workspace"};
            }

        } catch (const std::exception& e) {
            return Output{false, std::string("Error: ") + e.what()};
        }
    }
};

} // namespace Workspace
} // namespace Domain

