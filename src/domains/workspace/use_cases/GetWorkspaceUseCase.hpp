#pragma once

#include <string>
#include <memory>
#include <optional>
#include "../repositories/WorkspaceRepository.hpp"
#include "../entities/Workspace.hpp"

namespace Domain {
namespace Workspace {

/**
 * @brief Use Case: Buscar workspace por ID
 */
class GetWorkspaceUseCase {
private:
    std::shared_ptr<WorkspaceRepository> repository_;

public:
    struct Input {
        std::string workspaceId;
        std::optional<std::string> userId;  // Para validar acesso
    };

    struct Output {
        bool success;
        std::optional<Workspace> workspace;
        std::string message;
    };

    explicit GetWorkspaceUseCase(std::shared_ptr<WorkspaceRepository> repository)
        : repository_(repository) {}

    Output execute(const Input& input) {
        try {
            // Buscar workspace
            auto workspace = repository_->findById(input.workspaceId);
            if (!workspace.has_value()) {
                return Output{false, std::nullopt, "Workspace not found"};
            }

            // Validar acesso se userId fornecido
            if (input.userId.has_value()) {
                if (!repository_->isMember(input.workspaceId, *input.userId)) {
                    return Output{false, std::nullopt, "Access denied"};
                }
            }

            return Output{true, workspace, "Workspace found"};

        } catch (const std::exception& e) {
            return Output{false, std::nullopt, std::string("Error: ") + e.what()};
        }
    }
};

} // namespace Workspace
} // namespace Domain

