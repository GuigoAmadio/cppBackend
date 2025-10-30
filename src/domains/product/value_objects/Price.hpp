#pragma once

#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace Domain {
namespace Product {

/**
 * @brief Value Object: Price
 * Representa um valor monetário com validação (sempre >= 0)
 */
class Price {
public:
    // Construtor principal (valor em centavos para evitar erros de ponto flutuante)
    explicit Price(long cents, const std::string& currency = "BRL") 
        : cents_(cents), currency_(currency) {
        if (cents < 0) {
            throw std::invalid_argument("Price cannot be negative");
        }
        if (currency.length() != 3) {
            throw std::invalid_argument("Currency must be 3-letter ISO code (e.g., BRL, USD)");
        }
    }

    // Construtor alternativo (valor decimal)
    static Price fromDecimal(double value, const std::string& currency = "BRL") {
        if (value < 0) {
            throw std::invalid_argument("Price cannot be negative");
        }
        long cents = static_cast<long>(std::round(value * 100));
        return Price(cents, currency);
    }

    // Construtor de string "12.50"
    static Price fromString(const std::string& valueStr, const std::string& currency = "BRL") {
        try {
            double value = std::stod(valueStr);
            return fromDecimal(value, currency);
        } catch (const std::exception&) {
            throw std::invalid_argument("Invalid price format: " + valueStr);
        }
    }

    long cents() const { return cents_; }
    const std::string& currency() const { return currency_; }

    double toDecimal() const {
        return cents_ / 100.0;
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << toDecimal();
        return oss.str();
    }

    std::string toStringWithCurrency() const {
        return currency_ + " " + toString();
    }

    bool operator==(const Price& other) const {
        return cents_ == other.cents_ && currency_ == other.currency_;
    }

    bool operator<(const Price& other) const {
        if (currency_ != other.currency_) {
            throw std::logic_error("Cannot compare prices with different currencies");
        }
        return cents_ < other.cents_;
    }

    bool operator>(const Price& other) const {
        return other < *this;
    }

    bool operator<=(const Price& other) const {
        return !(*this > other);
    }

    bool operator>=(const Price& other) const {
        return !(*this < other);
    }

private:
    long cents_;
    std::string currency_;
};

} // namespace Product
} // namespace Domain

