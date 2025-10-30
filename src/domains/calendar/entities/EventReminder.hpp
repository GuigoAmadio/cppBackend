#ifndef EVENT_REMINDER_HPP
#define EVENT_REMINDER_HPP

#include <string>
#include <optional>
#include "../value_objects/ReminderType.hpp"

namespace Domain::Calendar {

class EventReminder {
public:
    EventReminder(const std::string& id, const std::string& eventId, const std::string& tenantId,
                  const std::string& userId, const ReminderType& type, int minutesBefore)
        : id_(id), eventId_(eventId), tenantId_(tenantId), userId_(userId),
          type_(type), minutesBefore_(minutesBefore), isSent_(false) {}

    // Getters
    std::string getId() const { return id_; }
    std::string getEventId() const { return eventId_; }
    std::string getTenantId() const { return tenantId_; }
    std::string getUserId() const { return userId_; }
    ReminderType getType() const { return type_; }
    int getMinutesBefore() const { return minutesBefore_; }
    bool isSent() const { return isSent_; }
    std::optional<std::string> getSentAt() const { return sentAt_; }

    // Business methods
    void markAsSent() {
        isSent_ = true;
    }

    void setSentAt(const std::string& time) {
        sentAt_ = time;
    }

private:
    std::string id_;
    std::string eventId_;
    std::string tenantId_;
    std::string userId_;
    ReminderType type_;
    int minutesBefore_;
    bool isSent_;
    std::optional<std::string> sentAt_;
};

} // namespace Domain::Calendar

#endif // EVENT_REMINDER_HPP

