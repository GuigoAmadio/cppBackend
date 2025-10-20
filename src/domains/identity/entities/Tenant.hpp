#pragma once

#include <string>
#include <chrono>
#include <optional>
#include "../../../core/json/Json.hpp"

namespace Domains::Identity::Entities {

/**
 * @brief Entidade Tenant (Organização/Empresa)
 * 
 * Representa uma organização no sistema multitenant.
 * Cada tenant tem seu próprio subdomínio e configurações.
 * 
 * @example
 *   auto tenant = Tenant::create("Demo Company", "demo", "pro", 10);
 *   bool isActive = tenant.isActive();
 */
class Tenant {
public:
    /**
     * @brief Planos de assinatura disponíveis
     */
    enum class Plan {
        FREE,
        PRO,
        ENTERPRISE
    };

    // Construtor completo (usado pelo Repository ao carregar do banco)
    Tenant(
        const std::string& id,
        const std::string& name,
        const std::string& subdomain,
        Plan plan,
        int maxUsers,
        bool isActive,
        const std::string& settings,
        std::chrono::system_clock::time_point createdAt,
        std::chrono::system_clock::time_point updatedAt
    );

    // Factory method para criar novo tenant
    static Tenant create(
        const std::string& name,
        const std::string& subdomain,
        Plan plan = Plan::FREE,
        int maxUsers = 5
    );

    // Getters
    std::string getId() const { return id_; }
    std::string getName() const { return name_; }
    std::string getSubdomain() const { return subdomain_; }
    Plan getPlan() const { return plan_; }
    int getMaxUsers() const { return maxUsers_; }
    bool isActive() const { return isActive_; }
    std::string getSettings() const { return settings_; }
    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt_; }
    std::chrono::system_clock::time_point getUpdatedAt() const { return updatedAt_; }

    // Business logic
    void activate();
    void deactivate();
    void changePlan(Plan newPlan);
    void setMaxUsers(int maxUsers);
    void updateSettings(const std::string& settings);

    // Conversão para JSON
    std::shared_ptr<Core::Json::JsonValue> toJson() const;

    // Helpers para Plan
    static std::string planToString(Plan plan);
    static Plan stringToPlan(const std::string& str);

private:
    std::string id_;
    std::string name_;
    std::string subdomain_;
    Plan plan_;
    int maxUsers_;
    bool isActive_;
    std::string settings_;  // JSON string
    std::chrono::system_clock::time_point createdAt_;
    std::chrono::system_clock::time_point updatedAt_;

    // Helper
    static std::string generateId();
    void updateTimestamp();

    // Validações
    static void validateSubdomain(const std::string& subdomain);
};

} // namespace Domains::Identity::Entities

