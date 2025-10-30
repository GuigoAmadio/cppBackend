#ifndef EVENT_CONTROLLER_HPP
#define EVENT_CONTROLLER_HPP

#include "../repositories/EventRepository.hpp"
#include "../use_cases/CreateEventUseCase.hpp"
#include "../use_cases/GetEventUseCase.hpp"
#include "../use_cases/ListEventsUseCase.hpp"
#include "../use_cases/UpdateEventUseCase.hpp"
#include "../use_cases/DeleteEventUseCase.hpp"
#include "../use_cases/AddParticipantUseCase.hpp"
#include "../use_cases/AddReminderUseCase.hpp"
#include "../use_cases/UpdateParticipantStatusUseCase.hpp"
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include <memory>

namespace Domain::Calendar {

class EventController {
public:
    explicit EventController(std::shared_ptr<EventRepository> repository);

    // Event endpoints
    Core::Http::Response createEvent(const Core::Http::Request& req);
    Core::Http::Response getEvent(const Core::Http::Request& req);
    Core::Http::Response listEvents(const Core::Http::Request& req);
    Core::Http::Response updateEvent(const Core::Http::Request& req);
    Core::Http::Response deleteEvent(const Core::Http::Request& req);

    // Participant endpoints
    Core::Http::Response addParticipant(const Core::Http::Request& req);
    Core::Http::Response getParticipants(const Core::Http::Request& req);
    Core::Http::Response updateParticipantStatus(const Core::Http::Request& req);

    // Reminder endpoints
    Core::Http::Response addReminder(const Core::Http::Request& req);
    Core::Http::Response getReminders(const Core::Http::Request& req);

private:
    std::shared_ptr<EventRepository> repository_;

    // Helper methods
    std::shared_ptr<Core::Json::JsonValue> eventToJsonObject(const Event& event);
    std::shared_ptr<Core::Json::JsonValue> participantToJsonObject(const EventParticipant& participant);
    std::shared_ptr<Core::Json::JsonValue> reminderToJsonObject(const EventReminder& reminder);
};

} // namespace Domain::Calendar

#endif // EVENT_CONTROLLER_HPP

