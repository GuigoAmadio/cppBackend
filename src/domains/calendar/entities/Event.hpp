#ifndef EVENT_HPP
#define EVENT_HPP

#include <string>
#include <optional>
#include <vector>
#include "../value_objects/EventType.hpp"
#include "../value_objects/EventStatus.hpp"

namespace Domain::Calendar {

class Event {
public:
    Event(const std::string& id, const std::string& tenantId, const std::string& title,
          const EventType& type, const EventStatus& status, const std::string& startTime,
          const std::string& endTime, const std::string& createdBy)
        : id_(id), tenantId_(tenantId), title_(title), type_(type), status_(status),
          startTime_(startTime), endTime_(endTime), createdBy_(createdBy), allDay_(false),
          isPrivate_(false), isRecurring_(false) {}

    // Getters
    std::string getId() const { return id_; }
    std::string getTenantId() const { return tenantId_; }
    std::string getTitle() const { return title_; }
    EventType getType() const { return type_; }
    EventStatus getStatus() const { return status_; }
    std::string getStartTime() const { return startTime_; }
    std::string getEndTime() const { return endTime_; }
    std::string getCreatedBy() const { return createdBy_; }
    bool isAllDay() const { return allDay_; }
    bool isPrivate() const { return isPrivate_; }
    bool isRecurring() const { return isRecurring_; }

    std::optional<std::string> getWorkspaceId() const { return workspaceId_; }
    std::optional<std::string> getDescription() const { return description_; }
    std::optional<std::string> getLocation() const { return location_; }
    std::optional<std::string> getTimezone() const { return timezone_; }
    std::optional<std::string> getOrganizerId() const { return organizerId_; }
    std::optional<std::string> getTaskId() const { return taskId_; }
    std::optional<std::string> getRecurringRuleId() const { return recurringRuleId_; }
    std::optional<std::string> getParentEventId() const { return parentEventId_; }
    std::optional<std::string> getColor() const { return color_; }
    std::vector<std::string> getTags() const { return tags_; }

    // Setters
    void setWorkspaceId(const std::string& workspaceId) { workspaceId_ = workspaceId; }
    void setDescription(const std::string& description) { description_ = description; }
    void setLocation(const std::string& location) { location_ = location; }
    void setTimezone(const std::string& timezone) { timezone_ = timezone; }
    void setOrganizerId(const std::string& organizerId) { organizerId_ = organizerId; }
    void setTaskId(const std::string& taskId) { taskId_ = taskId; }
    void setColor(const std::string& color) { color_ = color; }
    void setTags(const std::vector<std::string>& tags) { tags_ = tags; }
    void setAllDay(bool allDay) { allDay_ = allDay; }
    void setPrivate(bool isPrivate) { isPrivate_ = isPrivate; }
    void setRecurring(bool isRecurring) { isRecurring_ = isRecurring; }
    void setRecurringRuleId(const std::string& ruleId) { recurringRuleId_ = ruleId; }
    void setParentEventId(const std::string& parentId) { parentEventId_ = parentId; }

    // Business methods
    void updateTitle(const std::string& title) {
        if (title.empty()) throw std::invalid_argument("Title cannot be empty");
        title_ = title;
    }

    void updateTimes(const std::string& startTime, const std::string& endTime) {
        // In a real impl, would validate that end > start
        startTime_ = startTime;
        endTime_ = endTime;
    }

    void updateStatus(const EventStatus& status) {
        status_ = status;
    }

    void cancel() {
        status_ = EventStatus(EventStatus::Type::CANCELLED);
    }

private:
    std::string id_;
    std::string tenantId_;
    std::string title_;
    EventType type_;
    EventStatus status_;
    std::string startTime_;
    std::string endTime_;
    std::string createdBy_;
    bool allDay_;
    bool isPrivate_;
    bool isRecurring_;

    std::optional<std::string> workspaceId_;
    std::optional<std::string> description_;
    std::optional<std::string> location_;
    std::optional<std::string> timezone_;
    std::optional<std::string> organizerId_;
    std::optional<std::string> taskId_;
    std::optional<std::string> recurringRuleId_;
    std::optional<std::string> parentEventId_;
    std::optional<std::string> color_;
    std::vector<std::string> tags_;
};

} // namespace Domain::Calendar

#endif // EVENT_HPP

