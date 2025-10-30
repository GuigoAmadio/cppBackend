#pragma once

#include <string>
#include <memory>
#include <optional>
#include "../repositories/WorkspaceRepository.hpp"
#include "../value_objects/WorkspaceType.hpp"

namespace Domain {
namespace Workspace {

/**
 * @brief Use Case: Criar novo workspace
 */
class CreateWorkspaceUseCase {
private:
    std::shared_ptr<WorkspaceRepository> repository_;

public:
    struct Input {
        std::string tenantId;
        std::string name;
        std::string slug;
        std::string type;  // "personal", "team", "enterprise"
        std::string createdBy;
        std::optional<std::string> description;
    };

    struct Output {
        bool success;
        std::string workspaceId;
        std::string message;
    };

    explicit CreateWorkspaceUseCase(std::shared_ptr<WorkspaceRepository> repository)
        : repository_(repository) {}

    Output execute(const Input& input) {
        try {
            // Validar slug único
            auto existing = repository_->findBySlug(input.tenantId, input.slug);
            if (existing.has_value()) {
                return Output{false, "", "Workspace with this slug already exists"};
            }

            // Criar workspace
            WorkspaceType type(input.type);
            std::string id = repository_->create(
                input.tenantId,
                input.name,
                input.slug,
                type,
                input.createdBy,
                input.description
            );

            // Adicionar criador como owner
            MemberRole ownerRole(MemberRole::Role::OWNER);
            repository_->addMember(id, input.createdBy, ownerRole);

            return Output{true, id, "Workspace created successfully"};

        } catch (const std::exception& e) {
            return Output{false, "", std::string("Failed to create workspace: ") + e.what()};
        }
    }
};

} // namespace Workspace
} // namespace Domain

