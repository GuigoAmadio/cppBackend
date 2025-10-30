#pragma once

#include <string>
#include <memory>
#include <vector>
#include "../repositories/WorkspaceRepository.hpp"
#include "../entities/Workspace.hpp"

namespace Domain {
namespace Workspace {

/**
 * @brief Use Case: Listar workspaces
 */
class ListWorkspacesUseCase {
private:
    std::shared_ptr<WorkspaceRepository> repository_;

public:
    struct Input {
        std::string tenantId;
        std::optional<std::string> userId;  // Se fornecido, lista só do usuário
        int limit = 100;
        int offset = 0;
    };

    struct Output {
        bool success;
        std::vector<Workspace> workspaces;
        int total;
        std::string message;
    };

    explicit ListWorkspacesUseCase(std::shared_ptr<WorkspaceRepository> repository)
        : repository_(repository) {}

    Output execute(const Input& input) {
        try {
            std::vector<Workspace> workspaces;

            if (input.userId.has_value()) {
                // Listar workspaces do usuário
                workspaces = repository_->findByUser(*input.userId, input.tenantId);
            } else {
                // Listar todos do tenant
                workspaces = repository_->findByTenant(input.tenantId, input.limit, input.offset);
            }

            return Output{
                true,
                workspaces,
                static_cast<int>(workspaces.size()),
                "Workspaces listed successfully"
            };

        } catch (const std::exception& e) {
            return Output{false, {}, 0, std::string("Error: ") + e.what()};
        }
    }
};

} // namespace Workspace
} // namespace Domain

