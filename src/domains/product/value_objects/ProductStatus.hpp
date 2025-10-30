#pragma once

#include <string>
#include <stdexcept>

namespace Domain {
namespace Product {

/**
 * @brief Value Object: ProductStatus
 * Status do produto (active, inactive, archived)
 */
class ProductStatus {
public:
    enum class Status {
        ACTIVE,    // Produto ativo (disponível para venda)
        INACTIVE,  // Produto inativo (temporariamente indisponível)
        ARCHIVED   // Produto arquivado (não pode ser ativado novamente)
    };

    explicit ProductStatus(Status status) : status_(status) {}

    explicit ProductStatus(const std::string& statusStr) {
        if (statusStr == "active") {
            status_ = Status::ACTIVE;
        } else if (statusStr == "inactive") {
            status_ = Status::INACTIVE;
        } else if (statusStr == "archived") {
            status_ = Status::ARCHIVED;
        } else {
            throw std::invalid_argument("Invalid product status: " + statusStr);
        }
    }

    Status value() const { return status_; }

    std::string toString() const {
        switch (status_) {
            case Status::ACTIVE: return "active";
            case Status::INACTIVE: return "inactive";
            case Status::ARCHIVED: return "archived";
            default: return "active";
        }
    }

    bool isActive() const { return status_ == Status::ACTIVE; }
    bool isInactive() const { return status_ == Status::INACTIVE; }
    bool isArchived() const { return status_ == Status::ARCHIVED; }

    bool operator==(const ProductStatus& other) const {
        return status_ == other.status_;
    }

private:
    Status status_;
};

} // namespace Product
} // namespace Domain

