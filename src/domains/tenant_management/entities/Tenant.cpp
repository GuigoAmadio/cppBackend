#include "Tenant.hpp"
#include <regex>
#include <stdexcept>
#include <random>
#include <sstream>
#include <iomanip>

namespace Domains::TenantManagement::Entities {

Tenant::Tenant(
    const std::string& id,
    const std::string& name,
    const std::string& subdomain,
    Plan plan,
    int maxUsers,
    bool isActive,
    const std::string& settings,
    std::chrono::system_clock::time_point createdAt,
    std::chrono::system_clock::time_point updatedAt
) : id_(id),
    name_(name),
    subdomain_(subdomain),
    plan_(plan),
    maxUsers_(maxUsers),
    isActive_(isActive),
    settings_(settings),
    createdAt_(createdAt),
    updatedAt_(updatedAt) {
}

Tenant Tenant::create(
    const std::string& name,
    const std::string& subdomain,
    Plan plan,
    int maxUsers
) {
    if (name.empty()) {
        throw std::invalid_argument("Tenant name cannot be empty");
    }

    validateSubdomain(subdomain);

    if (maxUsers < 1 || maxUsers > 10000) {
        throw std::invalid_argument("Max users must be between 1 and 10000");
    }

    auto now = std::chrono::system_clock::now();

    return Tenant(
        generateId(),
        name,
        subdomain,
        plan,
        maxUsers,
        true,  // isActive
        "{}",  // settings (JSON vazio)
        now,
        now
    );
}

void Tenant::activate() {
    isActive_ = true;
    updateTimestamp();
}

void Tenant::deactivate() {
    isActive_ = false;
    updateTimestamp();
}

void Tenant::changePlan(Plan newPlan) {
    plan_ = newPlan;
    updateTimestamp();
}

void Tenant::setMaxUsers(int maxUsers) {
    if (maxUsers < 1 || maxUsers > 10000) {
        throw std::invalid_argument("Max users must be between 1 and 10000");
    }
    maxUsers_ = maxUsers;
    updateTimestamp();
}

void Tenant::updateSettings(const std::string& settings) {
    settings_ = settings;
    updateTimestamp();
}

std::shared_ptr<Core::Json::JsonValue> Tenant::toJson() const {
    auto obj = Core::Json::makeObject();
    
    obj->asObject()["id"] = Core::Json::makeString(id_);
    obj->asObject()["name"] = Core::Json::makeString(name_);
    obj->asObject()["subdomain"] = Core::Json::makeString(subdomain_);
    obj->asObject()["plan"] = Core::Json::makeString(planToString(plan_));
    obj->asObject()["max_users"] = Core::Json::makeNumber(maxUsers_);
    obj->asObject()["is_active"] = Core::Json::makeBool(isActive_);
    
    // Tentar parsear settings como JSON
    try {
        auto settingsJson = Core::Json::Json::parse(settings_);
        obj->asObject()["settings"] = settingsJson;
    } catch (...) {
        obj->asObject()["settings"] = Core::Json::makeObject();
    }
    
    // Timestamps (converter para epoch)
    auto createdEpoch = std::chrono::duration_cast<std::chrono::seconds>(
        createdAt_.time_since_epoch()
    ).count();
    obj->asObject()["created_at"] = Core::Json::makeNumber(createdEpoch);
    
    auto updatedEpoch = std::chrono::duration_cast<std::chrono::seconds>(
        updatedAt_.time_since_epoch()
    ).count();
    obj->asObject()["updated_at"] = Core::Json::makeNumber(updatedEpoch);
    
    return obj;
}

std::string Tenant::planToString(Plan plan) {
    switch (plan) {
        case Plan::FREE: return "free";
        case Plan::PRO: return "pro";
        case Plan::ENTERPRISE: return "enterprise";
        default: return "free";
    }
}

Tenant::Plan Tenant::stringToPlan(const std::string& str) {
    if (str == "free") return Plan::FREE;
    if (str == "pro") return Plan::PRO;
    if (str == "enterprise") return Plan::ENTERPRISE;
    return Plan::FREE;
}

std::string Tenant::generateId() {
    // Gerar UUID v4 simples (compatível com Windows/MinGW)
    // Em produção, use biblioteca UUID real (boost::uuid ou similar)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex;
    
    for (int i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4"; // UUID v4
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 12; i++) {
        ss << dis(gen);
    }
    
    return ss.str();
}

void Tenant::updateTimestamp() {
    updatedAt_ = std::chrono::system_clock::now();
}

void Tenant::validateSubdomain(const std::string& subdomain) {
    if (subdomain.empty()) {
        throw std::invalid_argument("Subdomain cannot be empty");
    }

    if (subdomain.length() < 3 || subdomain.length() > 63) {
        throw std::invalid_argument("Subdomain must be between 3 and 63 characters");
    }

    // Verificar formato: apenas letras minúsculas, números e hífens
    std::regex subdomainRegex("^[a-z0-9-]+$");
    if (!std::regex_match(subdomain, subdomainRegex)) {
        throw std::invalid_argument(
            "Subdomain can only contain lowercase letters, numbers, and hyphens"
        );
    }

    // Não pode começar ou terminar com hífen
    if (subdomain[0] == '-' || subdomain[subdomain.length() - 1] == '-') {
        throw std::invalid_argument("Subdomain cannot start or end with a hyphen");
    }

    // Subdomínios reservados
    std::vector<std::string> reserved = {
        "www", "api", "admin", "app", "mail", "ftp", "smtp", "localhost"
    };
    
    for (const auto& r : reserved) {
        if (subdomain == r) {
            throw std::invalid_argument("Subdomain '" + subdomain + "' is reserved");
        }
    }
}

} // namespace Domains::TenantManagement::Entities

