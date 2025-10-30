#pragma once

#include <string>
#include <stdexcept>

namespace Domain {
namespace Category {

/**
 * @brief Value Object: CategoryStatus
 * Status de uma categoria (ativa, inativa, arquivada)
 */
class CategoryStatus {
public:
    enum class Status {
        ACTIVE,      // Ativa e visível
        INACTIVE,    // Inativa (não visível)
        ARCHIVED     // Arquivada (mantida para histórico)
    };

    explicit CategoryStatus(Status status) : status_(status) {}
    
    explicit CategoryStatus(const std::string& statusStr) {
        if (statusStr == "active") {
            status_ = Status::ACTIVE;
        } else if (statusStr == "inactive") {
            status_ = Status::INACTIVE;
        } else if (statusStr == "archived") {
            status_ = Status::ARCHIVED;
        } else {
            throw std::invalid_argument("Invalid category status: " + statusStr);
        }
    }

    Status value() const { return status_; }
    
    std::string toString() const {
        switch (status_) {
            case Status::ACTIVE: return "active";
            case Status::INACTIVE: return "inactive";
            case Status::ARCHIVED: return "archived";
            default: return "unknown";
        }
    }

    bool isActive() const { return status_ == Status::ACTIVE; }
    bool isInactive() const { return status_ == Status::INACTIVE; }
    bool isArchived() const { return status_ == Status::ARCHIVED; }

    bool operator==(const CategoryStatus& other) const {
        return status_ == other.status_;
    }

private:
    Status status_;
};

} // namespace Category
} // namespace Domain

