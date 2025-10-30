#pragma once

#include <string>
#include <stdexcept>

namespace Domain::Invoicing {

/**
 * @brief Tipo de fatura
 * 
 * - STANDARD: Fatura padrão
 * - PROFORMA: Fatura proforma (não é fatura fiscal, apenas cotação)
 * - CREDIT_NOTE: Nota de crédito (devolução/estorno)
 * - DEBIT_NOTE: Nota de débito (cobrança adicional)
 */
class InvoiceType {
public:
    enum class Type {
        STANDARD,
        PROFORMA,
        CREDIT_NOTE,
        DEBIT_NOTE
    };

    explicit InvoiceType(Type type) : type_(type) {}

    explicit InvoiceType(const std::string& str) {
        if (str == "standard") type_ = Type::STANDARD;
        else if (str == "proforma") type_ = Type::PROFORMA;
        else if (str == "credit_note") type_ = Type::CREDIT_NOTE;
        else if (str == "debit_note") type_ = Type::DEBIT_NOTE;
        else throw std::invalid_argument("Invalid invoice type: " + str);
    }

    Type getType() const { return type_; }

    std::string toString() const {
        switch (type_) {
            case Type::STANDARD: return "standard";
            case Type::PROFORMA: return "proforma";
            case Type::CREDIT_NOTE: return "credit_note";
            case Type::DEBIT_NOTE: return "debit_note";
            default: return "unknown";
        }
    }

    bool isStandard() const { return type_ == Type::STANDARD; }
    bool isProforma() const { return type_ == Type::PROFORMA; }
    bool isCreditNote() const { return type_ == Type::CREDIT_NOTE; }
    bool isDebitNote() const { return type_ == Type::DEBIT_NOTE; }

    bool operator==(const InvoiceType& other) const {
        return type_ == other.type_;
    }

private:
    Type type_;
};

} // namespace Domain::Invoicing

