#pragma once

#include <string>
#include <stdexcept>

namespace Domain {
namespace Customer {

/**
 * @brief Value Object: CustomerStatus
 * Status do cliente no sistema
 */
class CustomerStatus {
public:
    enum class Status {
        ACTIVE,       // Ativo
        INACTIVE,     // Inativo
        BLOCKED,      // Bloqueado
        PROSPECT      // Prospect (potencial cliente)
    };

    explicit CustomerStatus(Status status) : status_(status) {}
    
    explicit CustomerStatus(const std::string& statusStr) {
        if (statusStr == "active") {
            status_ = Status::ACTIVE;
        } else if (statusStr == "inactive") {
            status_ = Status::INACTIVE;
        } else if (statusStr == "blocked") {
            status_ = Status::BLOCKED;
        } else if (statusStr == "prospect") {
            status_ = Status::PROSPECT;
        } else {
            throw std::invalid_argument("Invalid customer status: " + statusStr);
        }
    }

    Status value() const { return status_; }
    
    std::string toString() const {
        switch (status_) {
            case Status::ACTIVE: return "active";
            case Status::INACTIVE: return "inactive";
            case Status::BLOCKED: return "blocked";
            case Status::PROSPECT: return "prospect";
            default: return "unknown";
        }
    }

    bool isActive() const { return status_ == Status::ACTIVE; }
    bool isBlocked() const { return status_ == Status::BLOCKED; }

    bool operator==(const CustomerStatus& other) const {
        return status_ == other.status_;
    }

private:
    Status status_;
};

} // namespace Customer
} // namespace Domain

