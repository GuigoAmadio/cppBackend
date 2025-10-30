#pragma once

#include <string>
#include <optional>
#include "../value_objects/PlanType.hpp"
#include "../value_objects/BillingCycle.hpp"

namespace Domain::Subscription {

class Plan {
public:
    Plan(
        const std::string& id,
        const std::string& tenantId,
        const std::string& name,
        double price,
        const BillingCycle& billingCycle
    ) : id_(id), tenantId_(tenantId), name_(name), 
        price_(price), currency_("BRL"), billingCycle_(billingCycle),
        planType_(PlanType::Type::BASIC), trialDays_(0),
        isActive_(true), isPublic_(true), displayOrder_(0) {}

    // Getters
    std::string getId() const { return id_; }
    std::string getTenantId() const { return tenantId_; }
    std::string getName() const { return name_; }
    std::optional<std::string> getDescription() const { return description_; }
    PlanType getPlanType() const { return planType_; }
    double getPrice() const { return price_; }
    std::string getCurrency() const { return currency_; }
    BillingCycle getBillingCycle() const { return billingCycle_; }
    int getTrialDays() const { return trialDays_; }
    std::optional<std::string> getFeatures() const { return features_; }
    std::optional<std::string> getLimits() const { return limits_; }
    bool isActive() const { return isActive_; }
    bool isPublic() const { return isPublic_; }
    int getDisplayOrder() const { return displayOrder_; }
    std::optional<std::string> getCreatedBy() const { return createdBy_; }
    std::optional<std::string> getUpdatedBy() const { return updatedBy_; }
    std::optional<std::string> getCreatedAt() const { return createdAt_; }
    std::optional<std::string> getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setName(const std::string& name) { name_ = name; }
    void setDescription(const std::string& description) { description_ = description; }
    void setPlanType(const PlanType& type) { planType_ = type; }
    void setPrice(double price) { 
        if (price < 0) throw std::invalid_argument("Price cannot be negative");
        price_ = price; 
    }
    void setCurrency(const std::string& currency) { currency_ = currency; }
    void setBillingCycle(const BillingCycle& cycle) { billingCycle_ = cycle; }
    void setTrialDays(int days) { 
        if (days < 0) throw std::invalid_argument("Trial days cannot be negative");
        trialDays_ = days; 
    }
    void setFeatures(const std::string& features) { features_ = features; }
    void setLimits(const std::string& limits) { limits_ = limits; }
    void setIsActive(bool active) { isActive_ = active; }
    void setIsPublic(bool isPublic) { isPublic_ = isPublic; }
    void setDisplayOrder(int order) { displayOrder_ = order; }
    void setCreatedBy(const std::string& createdBy) { createdBy_ = createdBy; }
    void setUpdatedBy(const std::string& updatedBy) { updatedBy_ = updatedBy; }
    void setCreatedAt(const std::string& createdAt) { createdAt_ = createdAt; }
    void setUpdatedAt(const std::string& updatedAt) { updatedAt_ = updatedAt; }

    // Business logic
    bool hasTrialPeriod() const { return trialDays_ > 0; }
    bool isRecurring() const { return billingCycle_.isRecurring(); }

private:
    std::string id_;
    std::string tenantId_;
    std::string name_;
    std::optional<std::string> description_;
    PlanType planType_;
    double price_;
    std::string currency_;
    BillingCycle billingCycle_;
    int trialDays_;
    std::optional<std::string> features_;
    std::optional<std::string> limits_;
    bool isActive_;
    bool isPublic_;
    int displayOrder_;
    std::optional<std::string> createdBy_;
    std::optional<std::string> updatedBy_;
    std::optional<std::string> createdAt_;
    std::optional<std::string> updatedAt_;
};

} // namespace Domain::Subscription

