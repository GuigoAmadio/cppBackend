#pragma once

#include <string>
#include <stdexcept>

namespace Domain::Subscription {

class PlanType {
public:
    enum class Type {
        BASIC,      // Plano básico
        PRO,        // Plano profissional
        ENTERPRISE, // Plano empresarial
        CUSTOM      // Plano customizado
    };

    PlanType() : type_(Type::BASIC) {}
    explicit PlanType(Type type) : type_(type) {}
    explicit PlanType(const std::string& str) {
        if (str == "basic") type_ = Type::BASIC;
        else if (str == "pro") type_ = Type::PRO;
        else if (str == "enterprise") type_ = Type::ENTERPRISE;
        else if (str == "custom") type_ = Type::CUSTOM;
        else throw std::invalid_argument("Invalid plan type: " + str);
    }

    std::string toString() const {
        switch (type_) {
            case Type::BASIC: return "basic";
            case Type::PRO: return "pro";
            case Type::ENTERPRISE: return "enterprise";
            case Type::CUSTOM: return "custom";
            default: return "basic";
        }
    }

    Type getType() const { return type_; }

    bool isBasic() const { return type_ == Type::BASIC; }
    bool isPro() const { return type_ == Type::PRO; }
    bool isEnterprise() const { return type_ == Type::ENTERPRISE; }
    bool isCustom() const { return type_ == Type::CUSTOM; }

private:
    Type type_;
};

} // namespace Domain::Subscription

