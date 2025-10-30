#pragma once

#include <stdexcept>
#include <string>

namespace Domain {
namespace Product {

/**
 * @brief Value Object: Stock
 * Representa quantidade em estoque (sempre >= 0)
 */
class Stock {
public:
    explicit Stock(int quantity = 0, int lowStockThreshold = 10) 
        : quantity_(quantity), lowStockThreshold_(lowStockThreshold) {
        if (quantity < 0) {
            throw std::invalid_argument("Stock quantity cannot be negative");
        }
        if (lowStockThreshold < 0) {
            throw std::invalid_argument("Low stock threshold cannot be negative");
        }
    }

    int quantity() const { return quantity_; }
    int lowStockThreshold() const { return lowStockThreshold_; }

    bool isAvailable() const { return quantity_ > 0; }
    bool isLowStock() const { return quantity_ <= lowStockThreshold_ && quantity_ > 0; }
    bool isOutOfStock() const { return quantity_ == 0; }

    void add(int amount) {
        if (amount < 0) {
            throw std::invalid_argument("Cannot add negative amount to stock");
        }
        quantity_ += amount;
    }

    void subtract(int amount) {
        if (amount < 0) {
            throw std::invalid_argument("Cannot subtract negative amount from stock");
        }
        if (amount > quantity_) {
            throw std::logic_error("Insufficient stock: trying to subtract " + 
                                 std::to_string(amount) + " but only " + 
                                 std::to_string(quantity_) + " available");
        }
        quantity_ -= amount;
    }

    void set(int quantity) {
        if (quantity < 0) {
            throw std::invalid_argument("Stock quantity cannot be negative");
        }
        quantity_ = quantity;
    }

    std::string toString() const {
        return std::to_string(quantity_);
    }

    bool operator==(const Stock& other) const {
        return quantity_ == other.quantity_ && 
               lowStockThreshold_ == other.lowStockThreshold_;
    }

private:
    int quantity_;
    int lowStockThreshold_;
};

} // namespace Product
} // namespace Domain

