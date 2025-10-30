#pragma once

#include <string>
#include <stdexcept>

namespace Domain {
namespace Workspace {

/**
 * @brief Value Object: WorkspaceType
 * Tipos de workspace disponíveis
 */
class WorkspaceType {
public:
    enum class Type {
        PERSONAL,      // Workspace pessoal
        TEAM,          // Workspace de equipe
        ENTERPRISE     // Workspace empresarial
    };

    explicit WorkspaceType(Type type) : type_(type) {}
    explicit WorkspaceType(const std::string& typeStr) {
        if (typeStr == "personal") type_ = Type::PERSONAL;
        else if (typeStr == "team") type_ = Type::TEAM;
        else if (typeStr == "enterprise") type_ = Type::ENTERPRISE;
        else throw std::invalid_argument("Invalid workspace type: " + typeStr);
    }

    Type getType() const { return type_; }
    std::string toString() const {
        switch (type_) {
            case Type::PERSONAL: return "personal";
            case Type::TEAM: return "team";
            case Type::ENTERPRISE: return "enterprise";
            default: return "unknown";
        }
    }

    bool operator==(const WorkspaceType& other) const {
        return type_ == other.type_;
    }

private:
    Type type_;
};

} // namespace Workspace
} // namespace Domain

