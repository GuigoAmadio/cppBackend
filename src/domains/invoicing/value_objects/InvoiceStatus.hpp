#pragma once

#include <string>
#include <stdexcept>

namespace Domain::Invoicing {

/**
 * @brief Status da fatura
 * 
 * Estados possíveis:
 * - DRAFT: Rascunho, ainda não enviada
 * - SENT: Enviada ao cliente
 * - PAID: Totalmente paga
 * - PARTIAL: Parcialmente paga
 * - OVERDUE: Vencida (não paga após due_date)
 * - CANCELLED: Cancelada
 * - VOID: Anulada (sem efeito)
 */
class InvoiceStatus {
public:
    enum class Type {
        DRAFT,
        SENT,
        PAID,
        PARTIAL,
        OVERDUE,
        CANCELLED,
        VOIDED
    };

    explicit InvoiceStatus(Type type) : type_(type) {}

    explicit InvoiceStatus(const std::string& str) {
        if (str == "draft") type_ = Type::DRAFT;
        else if (str == "sent") type_ = Type::SENT;
        else if (str == "paid") type_ = Type::PAID;
        else if (str == "partial") type_ = Type::PARTIAL;
        else if (str == "overdue") type_ = Type::OVERDUE;
        else if (str == "cancelled") type_ = Type::CANCELLED;
        else if (str == "void") type_ = Type::VOIDED;
        else throw std::invalid_argument("Invalid invoice status: " + str);
    }

    Type getType() const { return type_; }

    std::string toString() const {
        switch (type_) {
            case Type::DRAFT: return "draft";
            case Type::SENT: return "sent";
            case Type::PAID: return "paid";
            case Type::PARTIAL: return "partial";
            case Type::OVERDUE: return "overdue";
            case Type::CANCELLED: return "cancelled";
            case Type::VOIDED: return "void";
            default: return "unknown";
        }
    }

    // Helpers
    bool isDraft() const { return type_ == Type::DRAFT; }
    bool isSent() const { return type_ == Type::SENT; }
    bool isPaid() const { return type_ == Type::PAID; }
    bool isPartial() const { return type_ == Type::PARTIAL; }
    bool isOverdue() const { return type_ == Type::OVERDUE; }
    bool isCancelled() const { return type_ == Type::CANCELLED; }
    bool isVoid() const { return type_ == Type::VOIDED; }
    
    bool canBeEdited() const { return isDraft(); }
    bool canBeSent() const { return isDraft(); }
    bool canBePaid() const { return isSent() || isPartial() || isOverdue(); }
    bool canBeCancelled() const { return !isPaid() && !isCancelled() && !isVoid(); }

    bool operator==(const InvoiceStatus& other) const {
        return type_ == other.type_;
    }

private:
    Type type_;
};

} // namespace Domain::Invoicing

