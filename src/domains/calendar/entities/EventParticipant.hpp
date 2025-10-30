#ifndef EVENT_PARTICIPANT_HPP
#define EVENT_PARTICIPANT_HPP

#include <string>
#include <optional>

namespace Domain::Calendar {

class EventParticipant {
public:
    enum class Role {
        ORGANIZER,
        REQUIRED_ATTENDEE,
        OPTIONAL_ATTENDEE,
        ATTENDEE
    };

    enum class Status {
        PENDING,
        ACCEPTED,
        DECLINED,
        TENTATIVE
    };

    EventParticipant(const std::string& id, const std::string& eventId, const std::string& tenantId,
                     const std::string& userId, Role role, Status status)
        : id_(id), eventId_(eventId), tenantId_(tenantId), userId_(userId),
          role_(role), status_(status) {}

    // Getters
    std::string getId() const { return id_; }
    std::string getEventId() const { return eventId_; }
    std::string getTenantId() const { return tenantId_; }
    std::string getUserId() const { return userId_; }
    Role getRole() const { return role_; }
    Status getStatus() const { return status_; }
    std::optional<std::string> getResponseTime() const { return responseTime_; }

    // Setters
    void setResponseTime(const std::string& time) { responseTime_ = time; }

    // Business methods
    void accept() {
        status_ = Status::ACCEPTED;
    }

    void decline() {
        status_ = Status::DECLINED;
    }

    void markTentative() {
        status_ = Status::TENTATIVE;
    }

    // Helper methods
    static std::string roleToString(Role role) {
        switch (role) {
            case Role::ORGANIZER: return "organizer";
            case Role::REQUIRED_ATTENDEE: return "required";
            case Role::OPTIONAL_ATTENDEE: return "optional";
            case Role::ATTENDEE: return "attendee";
            default: return "attendee";
        }
    }

    static Role stringToRole(const std::string& str) {
        if (str == "organizer") return Role::ORGANIZER;
        if (str == "required") return Role::REQUIRED_ATTENDEE;
        if (str == "optional") return Role::OPTIONAL_ATTENDEE;
        return Role::ATTENDEE;
    }

    static std::string statusToString(Status status) {
        switch (status) {
            case Status::PENDING: return "pending";
            case Status::ACCEPTED: return "accepted";
            case Status::DECLINED: return "declined";
            case Status::TENTATIVE: return "tentative";
            default: return "pending";
        }
    }

    static Status stringToStatus(const std::string& str) {
        if (str == "accepted") return Status::ACCEPTED;
        if (str == "declined") return Status::DECLINED;
        if (str == "tentative") return Status::TENTATIVE;
        return Status::PENDING;
    }

private:
    std::string id_;
    std::string eventId_;
    std::string tenantId_;
    std::string userId_;
    Role role_;
    Status status_;
    std::optional<std::string> responseTime_;
};

} // namespace Domain::Calendar

#endif // EVENT_PARTICIPANT_HPP

