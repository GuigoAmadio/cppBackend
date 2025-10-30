#ifndef EVENT_REPOSITORY_HPP
#define EVENT_REPOSITORY_HPP

#include <vector>
#include <optional>
#include <memory>
#include "../entities/Event.hpp"
#include "../entities/EventParticipant.hpp"
#include "../entities/EventReminder.hpp"

namespace Domain::Calendar {

class EventRepository {
public:
    virtual ~EventRepository() = default;

    // Event CRUD
    virtual bool save(const Event& event) = 0;
    virtual std::optional<Event> findById(const std::string& id, const std::string& tenantId) = 0;
    virtual std::vector<Event> findByTenant(const std::string& tenantId) = 0;
    virtual std::vector<Event> findByWorkspace(const std::string& workspaceId, const std::string& tenantId) = 0;
    virtual std::vector<Event> findByDateRange(const std::string& tenantId, const std::string& startDate, const std::string& endDate) = 0;
    virtual std::vector<Event> findByUser(const std::string& userId, const std::string& tenantId) = 0;
    virtual bool update(const Event& event) = 0;
    virtual bool remove(const std::string& id, const std::string& tenantId) = 0;

    // Participant management
    virtual bool saveParticipant(const EventParticipant& participant) = 0;
    virtual std::vector<EventParticipant> findParticipantsByEvent(const std::string& eventId, const std::string& tenantId) = 0;
    virtual std::optional<EventParticipant> findParticipant(const std::string& eventId, const std::string& userId, const std::string& tenantId) = 0;
    virtual bool updateParticipantStatus(const std::string& id, EventParticipant::Status status) = 0;
    virtual bool removeParticipant(const std::string& eventId, const std::string& userId, const std::string& tenantId) = 0;

    // Reminder management
    virtual bool saveReminder(const EventReminder& reminder) = 0;
    virtual std::vector<EventReminder> findRemindersByEvent(const std::string& eventId, const std::string& tenantId) = 0;
    virtual std::vector<EventReminder> findRemindersByUser(const std::string& userId, const std::string& tenantId) = 0;
    virtual bool removeReminder(const std::string& id) = 0;
};

} // namespace Domain::Calendar

#endif // EVENT_REPOSITORY_HPP

