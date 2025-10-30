#ifndef EVENT_REPOSITORY_IMPL_HPP
#define EVENT_REPOSITORY_IMPL_HPP

#include "../EventRepository.hpp"
#include "../../../../core/database/ConnectionPool.hpp"
#include <memory>

namespace Domain::Calendar {

class EventRepositoryImpl : public EventRepository {
public:
    explicit EventRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool);

    // Event CRUD
    bool save(const Event& event) override;
    std::optional<Event> findById(const std::string& id, const std::string& tenantId) override;
    std::vector<Event> findByTenant(const std::string& tenantId) override;
    std::vector<Event> findByWorkspace(const std::string& workspaceId, const std::string& tenantId) override;
    std::vector<Event> findByDateRange(const std::string& tenantId, const std::string& startDate, const std::string& endDate) override;
    std::vector<Event> findByUser(const std::string& userId, const std::string& tenantId) override;
    bool update(const Event& event) override;
    bool remove(const std::string& id, const std::string& tenantId) override;

    // Participant management
    bool saveParticipant(const EventParticipant& participant) override;
    std::vector<EventParticipant> findParticipantsByEvent(const std::string& eventId, const std::string& tenantId) override;
    std::optional<EventParticipant> findParticipant(const std::string& eventId, const std::string& userId, const std::string& tenantId) override;
    bool updateParticipantStatus(const std::string& id, EventParticipant::Status status) override;
    bool removeParticipant(const std::string& eventId, const std::string& userId, const std::string& tenantId) override;

    // Reminder management
    bool saveReminder(const EventReminder& reminder) override;
    std::vector<EventReminder> findRemindersByEvent(const std::string& eventId, const std::string& tenantId) override;
    std::vector<EventReminder> findRemindersByUser(const std::string& userId, const std::string& tenantId) override;
    bool removeReminder(const std::string& id) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;

    // Helper methods for mapping
    Event mapToEvent(const Core::Database::QueryResult& result, int row);
    std::vector<Event> mapToEvents(const Core::Database::QueryResult& result);
    
    EventParticipant mapToParticipant(const Core::Database::QueryResult& result, int row);
    std::vector<EventParticipant> mapToParticipants(const Core::Database::QueryResult& result);
    
    EventReminder mapToReminder(const Core::Database::QueryResult& result, int row);
    std::vector<EventReminder> mapToReminders(const Core::Database::QueryResult& result);
};

} // namespace Domain::Calendar

#endif // EVENT_REPOSITORY_IMPL_HPP

