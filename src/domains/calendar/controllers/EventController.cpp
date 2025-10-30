#include "EventController.hpp"
#include "../../../core/json/Json.hpp"
#include "../../../core/utils/Logger.hpp"

using namespace Core::Http;
using namespace Core::Json;

namespace Utils = Core::Utils;

namespace Domain::Calendar {

EventController::EventController(std::shared_ptr<EventRepository> repository)
    : repository_(repository) {
    Utils::Logger::info("[EventController] Initialized");
}

// =========================================================
// CREATE EVENT
// =========================================================

Response EventController::createEvent(const Request& req) {
    Utils::Logger::info("[EventController] createEvent()");
    
    auto json = makeObject();
    
    try {
        std::string tenantId = req.getCustomData("user_tenant_id");
        std::string userId = req.getCustomData("user_id");

        if (tenantId.empty()) {
            json->asObject()["error"] = makeString("Tenant ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Parse JSON body
        auto body = req.getJson();
        if (!body || !body->isObject()) {
            json->asObject()["error"] = makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& obj = body->asObject();

        // Validate required fields
        if (!obj.count("title") || !obj.count("event_type") || !obj.count("start_time") || !obj.count("end_time")) {
            json->asObject()["error"] = makeString("Missing required fields");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Create DTO
        CreateEventDTO dto;
        dto.tenantId = tenantId;
        dto.title = obj["title"]->asString();
        dto.eventType = obj["event_type"]->asString();
        dto.startTime = obj["start_time"]->asString();
        dto.endTime = obj["end_time"]->asString();
        dto.createdBy = userId;

        // Optional fields
        if (obj.count("workspace_id")) dto.workspaceId = obj["workspace_id"]->asString();
        if (obj.count("description")) dto.description = obj["description"]->asString();
        if (obj.count("location")) dto.location = obj["location"]->asString();
        if (obj.count("timezone")) dto.timezone = obj["timezone"]->asString();
        if (obj.count("organizer_id")) dto.organizerId = obj["organizer_id"]->asString();
        if (obj.count("task_id")) dto.taskId = obj["task_id"]->asString();
        if (obj.count("color")) dto.color = obj["color"]->asString();
        if (obj.count("all_day")) dto.allDay = (obj["all_day"]->asString() == "true");
        if (obj.count("is_private")) dto.isPrivate = (obj["is_private"]->asString() == "true");

        // Execute use case
        CreateEventUseCase useCase(repository_);
        std::string eventId = useCase.execute(dto);

        json->asObject()["id"] = makeString(eventId);
        json->asObject()["message"] = makeString("Event created successfully");
        return Response(StatusCode::Created).json(*json);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventController] createEvent() exception: " + std::string(e.what()));
        json->asObject()["error"] = makeString(std::string(e.what()));
        return Response(StatusCode::BadRequest).json(*json);
    }
}

// =========================================================
// GET EVENT
// =========================================================

Response EventController::getEvent(const Request& req) {
    Utils::Logger::info("[EventController] getEvent()");
    
    auto json = makeObject();
    
    try {
        std::string eventId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");

        if (eventId.empty()) {
            json->asObject()["error"] = makeString("Event ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        GetEventDTO dto{eventId, tenantId};
        GetEventUseCase useCase(repository_);
        auto event = useCase.execute(dto);

        if (event.has_value()) {
            return Response(StatusCode::OK).json(*eventToJsonObject(*event));
        } else {
            json->asObject()["error"] = makeString("Event not found");
            return Response(StatusCode::NotFound).json(*json);
        }

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventController] getEvent() exception: " + std::string(e.what()));
        json->asObject()["error"] = makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// LIST EVENTS
// =========================================================

Response EventController::listEvents(const Request& req) {
    Utils::Logger::info("[EventController] listEvents()");
    
    auto json = makeObject();
    
    try {
        std::string tenantId = req.getCustomData("user_tenant_id");

        ListEventsDTO dto;
        dto.tenantId = tenantId;

        // Parse query parameters
        std::string workspaceId = req.getQuery("workspace_id");
        if (!workspaceId.empty()) dto.workspaceId = workspaceId;

        std::string userId = req.getQuery("user_id");
        if (!userId.empty()) dto.userId = userId;

        std::string startDate = req.getQuery("start_date");
        if (!startDate.empty()) dto.startDate = startDate;

        std::string endDate = req.getQuery("end_date");
        if (!endDate.empty()) dto.endDate = endDate;

        // Execute use case
        ListEventsUseCase useCase(repository_);
        auto events = useCase.execute(dto);

        auto eventsArray = makeArray();
        for (const auto& event : events) {
            eventsArray->asArray().push_back(eventToJsonObject(event));
        }

        json->asObject()["events"] = eventsArray;
        json->asObject()["count"] = makeNumber(events.size());
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventController] listEvents() exception: " + std::string(e.what()));
        json->asObject()["error"] = makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// UPDATE EVENT
// =========================================================

Response EventController::updateEvent(const Request& req) {
    Utils::Logger::info("[EventController] updateEvent()");
    
    auto json = makeObject();
    
    try {
        std::string eventId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");

        if (eventId.empty()) {
            json->asObject()["error"] = makeString("Event ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Parse JSON body
        auto body = req.getJson();
        if (!body || !body->isObject()) {
            json->asObject()["error"] = makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& obj = body->asObject();

        // Create DTO
        UpdateEventDTO dto;
        dto.eventId = eventId;
        dto.tenantId = tenantId;

        // Optional fields
        if (obj.count("title")) dto.title = obj["title"]->asString();
        if (obj.count("description")) dto.description = obj["description"]->asString();
        if (obj.count("location")) dto.location = obj["location"]->asString();
        if (obj.count("start_time")) dto.startTime = obj["start_time"]->asString();
        if (obj.count("end_time")) dto.endTime = obj["end_time"]->asString();
        if (obj.count("color")) dto.color = obj["color"]->asString();
        if (obj.count("all_day")) dto.allDay = (obj["all_day"]->asString() == "true");
        if (obj.count("is_private")) dto.isPrivate = (obj["is_private"]->asString() == "true");

        // Execute use case
        UpdateEventUseCase useCase(repository_);
        useCase.execute(dto);

        json->asObject()["message"] = makeString("Event updated successfully");
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventController] updateEvent() exception: " + std::string(e.what()));
        json->asObject()["error"] = makeString(std::string(e.what()));
        return Response(StatusCode::BadRequest).json(*json);
    }
}

// =========================================================
// DELETE EVENT
// =========================================================

Response EventController::deleteEvent(const Request& req) {
    Utils::Logger::info("[EventController] deleteEvent()");
    
    auto json = makeObject();
    
    try {
        std::string eventId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");

        if (eventId.empty()) {
            json->asObject()["error"] = makeString("Event ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        DeleteEventDTO dto{eventId, tenantId};
        DeleteEventUseCase useCase(repository_);
        useCase.execute(dto);

        json->asObject()["message"] = makeString("Event deleted successfully");
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventController] deleteEvent() exception: " + std::string(e.what()));
        json->asObject()["error"] = makeString(std::string(e.what()));
        return Response(StatusCode::BadRequest).json(*json);
    }
}

// =========================================================
// ADD PARTICIPANT
// =========================================================

Response EventController::addParticipant(const Request& req) {
    Utils::Logger::info("[EventController] addParticipant()");
    
    auto json = makeObject();
    
    try {
        std::string eventId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");

        if (eventId.empty()) {
            json->asObject()["error"] = makeString("Event ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Parse JSON body
        auto body = req.getJson();
        if (!body || !body->isObject()) {
            json->asObject()["error"] = makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& obj = body->asObject();

        // Validate required fields
        if (!obj.count("user_id") || !obj.count("role")) {
            json->asObject()["error"] = makeString("Missing required fields");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Create DTO
        AddParticipantDTO dto;
        dto.eventId = eventId;
        dto.tenantId = tenantId;
        dto.userId = obj["user_id"]->asString();
        dto.role = obj["role"]->asString();

        // Execute use case
        AddParticipantUseCase useCase(repository_);
        std::string participantId = useCase.execute(dto);

        json->asObject()["id"] = makeString(participantId);
        json->asObject()["message"] = makeString("Participant added successfully");
        return Response(StatusCode::Created).json(*json);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventController] addParticipant() exception: " + std::string(e.what()));
        json->asObject()["error"] = makeString(std::string(e.what()));
        return Response(StatusCode::BadRequest).json(*json);
    }
}

// =========================================================
// GET PARTICIPANTS
// =========================================================

Response EventController::getParticipants(const Request& req) {
    Utils::Logger::info("[EventController] getParticipants()");
    
    auto json = makeObject();
    
    try {
        std::string eventId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");

        if (eventId.empty()) {
            json->asObject()["error"] = makeString("Event ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Get participants
        auto participants = repository_->findParticipantsByEvent(eventId, tenantId);

        auto participantsArray = makeArray();
        for (const auto& participant : participants) {
            participantsArray->asArray().push_back(participantToJsonObject(participant));
        }

        json->asObject()["participants"] = participantsArray;
        json->asObject()["count"] = makeNumber(participants.size());
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventController] getParticipants() exception: " + std::string(e.what()));
        json->asObject()["error"] = makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// UPDATE PARTICIPANT STATUS
// =========================================================

Response EventController::updateParticipantStatus(const Request& req) {
    Utils::Logger::info("[EventController] updateParticipantStatus()");
    
    auto json = makeObject();
    
    try {
        std::string participantId = req.getParam("id");

        if (participantId.empty()) {
            json->asObject()["error"] = makeString("Participant ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Parse JSON body
        auto body = req.getJson();
        if (!body || !body->isObject()) {
            json->asObject()["error"] = makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& obj = body->asObject();

        // Validate required fields
        if (!obj.count("status")) {
            json->asObject()["error"] = makeString("Status is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Create DTO
        UpdateParticipantStatusDTO dto;
        dto.participantId = participantId;
        dto.status = obj["status"]->asString();

        // Execute use case
        UpdateParticipantStatusUseCase useCase(repository_);
        useCase.execute(dto);

        json->asObject()["message"] = makeString("Participant status updated successfully");
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventController] updateParticipantStatus() exception: " + std::string(e.what()));
        json->asObject()["error"] = makeString(std::string(e.what()));
        return Response(StatusCode::BadRequest).json(*json);
    }
}

// =========================================================
// ADD REMINDER
// =========================================================

Response EventController::addReminder(const Request& req) {
    Utils::Logger::info("[EventController] addReminder()");
    
    auto json = makeObject();
    
    try {
        std::string eventId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        std::string userId = req.getCustomData("user_id");

        if (eventId.empty()) {
            json->asObject()["error"] = makeString("Event ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Parse JSON body
        auto body = req.getJson();
        if (!body || !body->isObject()) {
            json->asObject()["error"] = makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& obj = body->asObject();

        // Validate required fields
        if (!obj.count("reminder_type") || !obj.count("minutes_before")) {
            json->asObject()["error"] = makeString("Missing required fields");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Create DTO
        AddReminderDTO dto;
        dto.eventId = eventId;
        dto.tenantId = tenantId;
        dto.userId = userId;
        dto.reminderType = obj["reminder_type"]->asString();
        dto.minutesBefore = static_cast<int>(obj["minutes_before"]->asNumber());

        // Execute use case
        AddReminderUseCase useCase(repository_);
        std::string reminderId = useCase.execute(dto);

        json->asObject()["id"] = makeString(reminderId);
        json->asObject()["message"] = makeString("Reminder added successfully");
        return Response(StatusCode::Created).json(*json);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventController] addReminder() exception: " + std::string(e.what()));
        json->asObject()["error"] = makeString(std::string(e.what()));
        return Response(StatusCode::BadRequest).json(*json);
    }
}

// =========================================================
// GET REMINDERS
// =========================================================

Response EventController::getReminders(const Request& req) {
    Utils::Logger::info("[EventController] getReminders()");
    
    auto json = makeObject();
    
    try {
        std::string eventId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");

        if (eventId.empty()) {
            json->asObject()["error"] = makeString("Event ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Get reminders
        auto reminders = repository_->findRemindersByEvent(eventId, tenantId);

        auto remindersArray = makeArray();
        for (const auto& reminder : reminders) {
            remindersArray->asArray().push_back(reminderToJsonObject(reminder));
        }

        json->asObject()["reminders"] = remindersArray;
        json->asObject()["count"] = makeNumber(reminders.size());
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventController] getReminders() exception: " + std::string(e.what()));
        json->asObject()["error"] = makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// HELPER METHODS
// =========================================================

std::shared_ptr<JsonValue> EventController::eventToJsonObject(const Event& event) {
    auto json = makeObject();
    
    json->asObject()["id"] = makeString(event.getId());
    json->asObject()["tenant_id"] = makeString(event.getTenantId());
    json->asObject()["title"] = makeString(event.getTitle());
    json->asObject()["event_type"] = makeString(event.getType().toString());
    json->asObject()["status"] = makeString(event.getStatus().toString());
    json->asObject()["start_time"] = makeString(event.getStartTime());
    json->asObject()["end_time"] = makeString(event.getEndTime());
    json->asObject()["created_by"] = makeString(event.getCreatedBy());
    json->asObject()["all_day"] = makeString(event.isAllDay() ? "true" : "false");
    json->asObject()["is_private"] = makeString(event.isPrivate() ? "true" : "false");
    json->asObject()["is_recurring"] = makeString(event.isRecurring() ? "true" : "false");
    
    if (event.getWorkspaceId()) json->asObject()["workspace_id"] = makeString(*event.getWorkspaceId());
    if (event.getDescription()) json->asObject()["description"] = makeString(*event.getDescription());
    if (event.getLocation()) json->asObject()["location"] = makeString(*event.getLocation());
    if (event.getTimezone()) json->asObject()["timezone"] = makeString(*event.getTimezone());
    if (event.getOrganizerId()) json->asObject()["organizer_id"] = makeString(*event.getOrganizerId());
    if (event.getTaskId()) json->asObject()["task_id"] = makeString(*event.getTaskId());
    if (event.getColor()) json->asObject()["color"] = makeString(*event.getColor());
    if (event.getRecurringRuleId()) json->asObject()["recurring_rule_id"] = makeString(*event.getRecurringRuleId());
    if (event.getParentEventId()) json->asObject()["parent_event_id"] = makeString(*event.getParentEventId());
    
    return json;
}

std::shared_ptr<JsonValue> EventController::participantToJsonObject(const EventParticipant& participant) {
    auto json = makeObject();
    json->asObject()["id"] = makeString(participant.getId());
    json->asObject()["event_id"] = makeString(participant.getEventId());
    json->asObject()["user_id"] = makeString(participant.getUserId());
    json->asObject()["role"] = makeString(EventParticipant::roleToString(participant.getRole()));
    json->asObject()["status"] = makeString(EventParticipant::statusToString(participant.getStatus()));
    if (participant.getResponseTime()) {
        json->asObject()["response_time"] = makeString(*participant.getResponseTime());
    }
    return json;
}

std::shared_ptr<JsonValue> EventController::reminderToJsonObject(const EventReminder& reminder) {
    auto json = makeObject();
    json->asObject()["id"] = makeString(reminder.getId());
    json->asObject()["event_id"] = makeString(reminder.getEventId());
    json->asObject()["user_id"] = makeString(reminder.getUserId());
    json->asObject()["reminder_type"] = makeString(reminder.getType().toString());
    json->asObject()["minutes_before"] = makeNumber(reminder.getMinutesBefore());
    json->asObject()["is_sent"] = makeString(reminder.isSent() ? "true" : "false");
    if (reminder.getSentAt()) {
        json->asObject()["sent_at"] = makeString(*reminder.getSentAt());
    }
    return json;
}

} // namespace Domain::Calendar

