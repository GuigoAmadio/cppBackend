#ifndef RECURRING_RULE_HPP
#define RECURRING_RULE_HPP

#include <string>
#include <optional>
#include <vector>
#include "../value_objects/RecurrenceFrequency.hpp"

namespace Domain::Calendar {

class RecurringRule {
public:
    RecurringRule(const std::string& id, const std::string& tenantId,
                  const RecurrenceFrequency& frequency, int interval)
        : id_(id), tenantId_(tenantId), frequency_(frequency), interval_(interval) {}

    // Getters
    std::string getId() const { return id_; }
    std::string getTenantId() const { return tenantId_; }
    RecurrenceFrequency getFrequency() const { return frequency_; }
    int getInterval() const { return interval_; }
    std::vector<std::string> getByDay() const { return byDay_; }
    std::vector<int> getByMonthDay() const { return byMonthDay_; }
    std::vector<int> getByMonth() const { return byMonth_; }
    std::optional<int> getCount() const { return count_; }
    std::optional<std::string> getUntilDate() const { return untilDate_; }

    // Setters
    void setByDay(const std::vector<std::string>& days) { byDay_ = days; }
    void setByMonthDay(const std::vector<int>& days) { byMonthDay_ = days; }
    void setByMonth(const std::vector<int>& months) { byMonth_ = months; }
    void setCount(int count) { count_ = count; }
    void setUntilDate(const std::string& date) { untilDate_ = date; }

private:
    std::string id_;
    std::string tenantId_;
    RecurrenceFrequency frequency_;
    int interval_;
    std::vector<std::string> byDay_;
    std::vector<int> byMonthDay_;
    std::vector<int> byMonth_;
    std::optional<int> count_;
    std::optional<std::string> untilDate_;
};

} // namespace Domain::Calendar

#endif // RECURRING_RULE_HPP

