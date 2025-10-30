#include "EventRepositoryImpl.hpp"
#include "../../../../core/utils/Logger.hpp"

namespace Domain::Calendar {

namespace Utils = Core::Utils;

EventRepositoryImpl::EventRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {
    Utils::Logger::info("[EventRepositoryImpl] Initialized");
}

// =========================================================
// EVENT CRUD METHODS
// =========================================================

bool EventRepositoryImpl::save(const Event& event) {
    Utils::Logger::info("[EventRepositoryImpl] save() - ID: " + event.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            INSERT INTO events (
                id, tenant_id, workspace_id, title, description, location,
                event_type, status, start_time, end_time, all_day, timezone,
                created_by, organizer_id, task_id, is_recurring, recurring_rule_id,
                parent_event_id, is_private, color, tags
            ) VALUES (
                $1, $2, NULLIF($3,''), $4, NULLIF($5,''), NULLIF($6,''),
                $7, $8, $9, $10, $11, $12,
                $13, NULLIF($14,''), NULLIF($15,''), $16, NULLIF($17,''),
                NULLIF($18,''), $19, NULLIF($20,''), $21
            )
        )";

        std::vector<std::string> params;
        params.push_back(event.getId());
        params.push_back(event.getTenantId());
        params.push_back(event.getWorkspaceId().value_or(""));
        params.push_back(event.getTitle());
        params.push_back(event.getDescription().value_or(""));
        params.push_back(event.getLocation().value_or(""));
        params.push_back(event.getType().toString());
        params.push_back(event.getStatus().toString());
        params.push_back(event.getStartTime());
        params.push_back(event.getEndTime());
        params.push_back(event.isAllDay() ? "true" : "false");
        params.push_back(event.getTimezone().value_or("UTC"));
        params.push_back(event.getCreatedBy());
        params.push_back(event.getOrganizerId().value_or(""));
        params.push_back(event.getTaskId().value_or(""));
        params.push_back(event.isRecurring() ? "true" : "false");
        params.push_back(event.getRecurringRuleId().value_or(""));
        params.push_back(event.getParentEventId().value_or(""));
        params.push_back(event.isPrivate() ? "true" : "false");
        params.push_back(event.getColor().value_or(""));
        
        // Convert tags to PostgreSQL array format
        std::string tagsArray = "{";
        auto tags = event.getTags();
        for (size_t i = 0; i < tags.size(); ++i) {
            if (i > 0) tagsArray += ",";
            tagsArray += "\"" + tags[i] + "\"";
        }
        tagsArray += "}";
        params.push_back(tagsArray);

        Utils::Logger::debug("[EventRepositoryImpl] Executing INSERT with " + std::to_string(params.size()) + " params");
        Utils::Logger::debug("[EventRepositoryImpl] start_time=$9: " + params[8]);
        Utils::Logger::debug("[EventRepositoryImpl] end_time=$10: " + params[9]);
        Utils::Logger::debug("[EventRepositoryImpl] all_day=$11: " + params[10]);
        
        auto result = conn->executeParams(query, params);
        
        if (result.isSuccess()) {
            Utils::Logger::info("[EventRepositoryImpl] Event saved successfully");
            return true;
        }
        
        Utils::Logger::error("[EventRepositoryImpl] Failed to save event - SQL Error: " + result.getError());
        return false;

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] save() exception: " + std::string(e.what()));
        return false;
    }
}

std::optional<Event> EventRepositoryImpl::findById(const std::string& id, const std::string& tenantId) {
    Utils::Logger::info("[EventRepositoryImpl] findById() - ID: " + id);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM events WHERE id = $1 AND tenant_id = $2";
        
        std::vector<std::string> params;
        params.push_back(id);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        
        if (result.isSuccess() && result.rowCount() > 0) {
            return mapToEvent(result, 0);
        }
        
        return std::nullopt;

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] findById() exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<Event> EventRepositoryImpl::findByTenant(const std::string& tenantId) {
    Utils::Logger::info("[EventRepositoryImpl] findByTenant() - tenant: " + tenantId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM events WHERE tenant_id = $1 AND status != 'cancelled' ORDER BY start_time ASC";
        
        std::vector<std::string> params;
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToEvents(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] findByTenant() exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Event> EventRepositoryImpl::findByWorkspace(const std::string& workspaceId, const std::string& tenantId) {
    Utils::Logger::info("[EventRepositoryImpl] findByWorkspace() - workspace: " + workspaceId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM events WHERE workspace_id = $1 AND tenant_id = $2 AND status != 'cancelled' ORDER BY start_time ASC";
        
        std::vector<std::string> params;
        params.push_back(workspaceId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToEvents(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] findByWorkspace() exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Event> EventRepositoryImpl::findByDateRange(const std::string& tenantId, const std::string& startDate, const std::string& endDate) {
    Utils::Logger::info("[EventRepositoryImpl] findByDateRange() - " + startDate + " to " + endDate);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT * FROM events 
            WHERE tenant_id = $1 
            AND start_time >= $2 
            AND start_time <= $3 
            AND status != 'cancelled'
            ORDER BY start_time ASC
        )";
        
        std::vector<std::string> params;
        params.push_back(tenantId);
        params.push_back(startDate);
        params.push_back(endDate);
        
        auto result = conn->executeParams(query, params);
        return mapToEvents(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] findByDateRange() exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Event> EventRepositoryImpl::findByUser(const std::string& userId, const std::string& tenantId) {
    Utils::Logger::info("[EventRepositoryImpl] findByUser() - user: " + userId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            SELECT DISTINCT e.* FROM events e
            LEFT JOIN event_participants ep ON e.id = ep.event_id
            WHERE e.tenant_id = $1
            AND (e.created_by = $2 OR ep.user_id = $2)
            AND e.status != 'cancelled'
            ORDER BY e.start_time ASC
        )";
        
        std::vector<std::string> params;
        params.push_back(tenantId);
        params.push_back(userId);
        
        auto result = conn->executeParams(query, params);
        return mapToEvents(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] findByUser() exception: " + std::string(e.what()));
        return {};
    }
}

bool EventRepositoryImpl::update(const Event& event) {
    Utils::Logger::info("[EventRepositoryImpl] update() - ID: " + event.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            UPDATE events SET
                title = $1,
                description = NULLIF($2,''),
                location = NULLIF($3,''),
                event_type = $4,
                status = $5,
                start_time = $6,
                end_time = $7,
                all_day = $8,
                timezone = $9,
                organizer_id = NULLIF($10,''),
                task_id = NULLIF($11,''),
                is_private = $12,
                color = NULLIF($13,''),
                tags = $14,
                updated_at = CURRENT_TIMESTAMP
            WHERE id = $15 AND tenant_id = $16
        )";

        std::vector<std::string> params;
        params.push_back(event.getTitle());
        params.push_back(event.getDescription().value_or(""));
        params.push_back(event.getLocation().value_or(""));
        params.push_back(event.getType().toString());
        params.push_back(event.getStatus().toString());
        params.push_back(event.getStartTime());
        params.push_back(event.getEndTime());
        params.push_back(event.isAllDay() ? "true" : "false");
        params.push_back(event.getTimezone().value_or("UTC"));
        params.push_back(event.getOrganizerId().value_or(""));
        params.push_back(event.getTaskId().value_or(""));
        params.push_back(event.isPrivate() ? "true" : "false");
        params.push_back(event.getColor().value_or(""));
        
        // Convert tags to PostgreSQL array format
        std::string tagsArray = "{";
        auto tags = event.getTags();
        for (size_t i = 0; i < tags.size(); ++i) {
            if (i > 0) tagsArray += ",";
            tagsArray += "\"" + tags[i] + "\"";
        }
        tagsArray += "}";
        params.push_back(tagsArray);
        
        params.push_back(event.getId());
        params.push_back(event.getTenantId());

        auto result = conn->executeParams(query, params);
        
        if (result.isSuccess()) {
            Utils::Logger::info("[EventRepositoryImpl] Event updated successfully");
            return true;
        }
        
        Utils::Logger::error("[EventRepositoryImpl] Failed to update event");
        return false;

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] update() exception: " + std::string(e.what()));
        return false;
    }
}

bool EventRepositoryImpl::remove(const std::string& id, const std::string& tenantId) {
    Utils::Logger::info("[EventRepositoryImpl] remove() - ID: " + id);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "DELETE FROM events WHERE id = $1 AND tenant_id = $2";
        
        std::vector<std::string> params;
        params.push_back(id);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return result.isSuccess();

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] remove() exception: " + std::string(e.what()));
        return false;
    }
}

// =========================================================
// PARTICIPANT METHODS
// =========================================================

bool EventRepositoryImpl::saveParticipant(const EventParticipant& participant) {
    Utils::Logger::info("[EventRepositoryImpl] saveParticipant() - event: " + participant.getEventId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            INSERT INTO event_participants (
                id, event_id, tenant_id, user_id, role, status
            ) VALUES (
                $1, $2, $3, $4, $5, $6
            )
            ON CONFLICT (event_id, user_id) DO UPDATE SET
                role = EXCLUDED.role,
                status = EXCLUDED.status
        )";

        std::vector<std::string> params;
        params.push_back(participant.getId());
        params.push_back(participant.getEventId());
        params.push_back(participant.getTenantId());
        params.push_back(participant.getUserId());
        params.push_back(EventParticipant::roleToString(participant.getRole()));
        params.push_back(EventParticipant::statusToString(participant.getStatus()));

        auto result = conn->executeParams(query, params);
        
        if (result.isSuccess()) {
            Utils::Logger::info("[EventRepositoryImpl] Participant saved successfully");
            return true;
        }
        
        return false;

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] saveParticipant() exception: " + std::string(e.what()));
        return false;
    }
}

std::vector<EventParticipant> EventRepositoryImpl::findParticipantsByEvent(const std::string& eventId, const std::string& tenantId) {
    Utils::Logger::info("[EventRepositoryImpl] findParticipantsByEvent() - event: " + eventId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM event_participants WHERE event_id = $1 AND tenant_id = $2 ORDER BY created_at";
        
        std::vector<std::string> params;
        params.push_back(eventId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToParticipants(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] findParticipantsByEvent() exception: " + std::string(e.what()));
        return {};
    }
}

std::optional<EventParticipant> EventRepositoryImpl::findParticipant(const std::string& eventId, const std::string& userId, const std::string& tenantId) {
    Utils::Logger::info("[EventRepositoryImpl] findParticipant() - event: " + eventId + ", user: " + userId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM event_participants WHERE event_id = $1 AND user_id = $2 AND tenant_id = $3";
        
        std::vector<std::string> params;
        params.push_back(eventId);
        params.push_back(userId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        
        if (result.isSuccess() && result.rowCount() > 0) {
            return mapToParticipant(result, 0);
        }
        
        return std::nullopt;

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] findParticipant() exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool EventRepositoryImpl::updateParticipantStatus(const std::string& id, EventParticipant::Status status) {
    Utils::Logger::info("[EventRepositoryImpl] updateParticipantStatus() - ID: " + id);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            UPDATE event_participants SET
                status = $1,
                response_time = CURRENT_TIMESTAMP,
                updated_at = CURRENT_TIMESTAMP
            WHERE id = $2
        )";

        std::vector<std::string> params;
        params.push_back(EventParticipant::statusToString(status));
        params.push_back(id);

        auto result = conn->executeParams(query, params);
        return result.isSuccess();

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] updateParticipantStatus() exception: " + std::string(e.what()));
        return false;
    }
}

bool EventRepositoryImpl::removeParticipant(const std::string& eventId, const std::string& userId, const std::string& tenantId) {
    Utils::Logger::info("[EventRepositoryImpl] removeParticipant() - event: " + eventId + ", user: " + userId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "DELETE FROM event_participants WHERE event_id = $1 AND user_id = $2 AND tenant_id = $3";
        
        std::vector<std::string> params;
        params.push_back(eventId);
        params.push_back(userId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return result.isSuccess();

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] removeParticipant() exception: " + std::string(e.what()));
        return false;
    }
}

// =========================================================
// REMINDER METHODS
// =========================================================

bool EventRepositoryImpl::saveReminder(const EventReminder& reminder) {
    Utils::Logger::info("[EventRepositoryImpl] saveReminder() - event: " + reminder.getEventId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            INSERT INTO event_reminders (
                id, event_id, tenant_id, user_id, reminder_type, minutes_before
            ) VALUES (
                $1, $2, $3, $4, $5, $6
            )
        )";

        std::vector<std::string> params;
        params.push_back(reminder.getId());
        params.push_back(reminder.getEventId());
        params.push_back(reminder.getTenantId());
        params.push_back(reminder.getUserId());
        params.push_back(reminder.getType().toString());
        params.push_back(std::to_string(reminder.getMinutesBefore()));

        auto result = conn->executeParams(query, params);
        
        if (result.isSuccess()) {
            Utils::Logger::info("[EventRepositoryImpl] Reminder saved successfully");
            return true;
        }
        
        return false;

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] saveReminder() exception: " + std::string(e.what()));
        return false;
    }
}

std::vector<EventReminder> EventRepositoryImpl::findRemindersByEvent(const std::string& eventId, const std::string& tenantId) {
    Utils::Logger::info("[EventRepositoryImpl] findRemindersByEvent() - event: " + eventId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM event_reminders WHERE event_id = $1 AND tenant_id = $2 ORDER BY minutes_before";
        
        std::vector<std::string> params;
        params.push_back(eventId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToReminders(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] findRemindersByEvent() exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<EventReminder> EventRepositoryImpl::findRemindersByUser(const std::string& userId, const std::string& tenantId) {
    Utils::Logger::info("[EventRepositoryImpl] findRemindersByUser() - user: " + userId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM event_reminders WHERE user_id = $1 AND tenant_id = $2 AND is_sent = false ORDER BY created_at";
        
        std::vector<std::string> params;
        params.push_back(userId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToReminders(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] findRemindersByUser() exception: " + std::string(e.what()));
        return {};
    }
}

bool EventRepositoryImpl::removeReminder(const std::string& id) {
    Utils::Logger::info("[EventRepositoryImpl] removeReminder() - ID: " + id);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "DELETE FROM event_reminders WHERE id = $1";
        
        std::vector<std::string> params;
        params.push_back(id);
        
        auto result = conn->executeParams(query, params);
        return result.isSuccess();

    } catch (const std::exception& e) {
        Utils::Logger::error("[EventRepositoryImpl] removeReminder() exception: " + std::string(e.what()));
        return false;
    }
}

// =========================================================
// MAPPING METHODS
// =========================================================

Event EventRepositoryImpl::mapToEvent(const Core::Database::QueryResult& result, int row) {
    // Column positions (based on events table schema):
    // 0=id, 1=tenant_id, 2=workspace_id, 3=title, 4=description, 5=location
    // 6=event_type, 7=status, 8=start_time, 9=end_time, 10=all_day, 11=timezone
    // 12=created_by, 13=organizer_id, 14=task_id, 15=is_recurring, 16=recurring_rule_id
    // 17=parent_event_id, 18=is_private, 19=color, 20=tags
    
    std::string id = result.getValue(row, 0);
    std::string tenantId = result.getValue(row, 1);
    std::string title = result.getValue(row, 3);
    std::string eventTypeStr = result.getValue(row, 6);
    std::string statusStr = result.getValue(row, 7);
    std::string startTime = result.getValue(row, 8);
    std::string endTime = result.getValue(row, 9);
    std::string createdBy = result.getValue(row, 12);

    Event event(id, tenantId, title, EventType(eventTypeStr), EventStatus(statusStr),
                startTime, endTime, createdBy);

    // Set optional fields
    std::string workspaceId = result.getValue(row, 2);
    if (!workspaceId.empty()) event.setWorkspaceId(workspaceId);

    std::string description = result.getValue(row, 4);
    if (!description.empty()) event.setDescription(description);

    std::string location = result.getValue(row, 5);
    if (!location.empty()) event.setLocation(location);

    std::string timezone = result.getValue(row, 11);
    if (!timezone.empty()) event.setTimezone(timezone);

    std::string organizerId = result.getValue(row, 13);
    if (!organizerId.empty()) event.setOrganizerId(organizerId);

    std::string taskId = result.getValue(row, 14);
    if (!taskId.empty()) event.setTaskId(taskId);

    std::string recurringRuleId = result.getValue(row, 16);
    if (!recurringRuleId.empty()) event.setRecurringRuleId(recurringRuleId);

    std::string parentEventId = result.getValue(row, 17);
    if (!parentEventId.empty()) event.setParentEventId(parentEventId);

    std::string color = result.getValue(row, 19);
    if (!color.empty()) event.setColor(color);

    // Parse boolean fields
    std::string allDay = result.getValue(row, 10);
    event.setAllDay(allDay == "t" || allDay == "true");

    std::string isRecurring = result.getValue(row, 15);
    event.setRecurring(isRecurring == "t" || isRecurring == "true");

    std::string isPrivate = result.getValue(row, 18);
    event.setPrivate(isPrivate == "t" || isPrivate == "true");

    return event;
}

std::vector<Event> EventRepositoryImpl::mapToEvents(const Core::Database::QueryResult& result) {
    std::vector<Event> events;
    
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); i++) {
            events.push_back(mapToEvent(result, i));
        }
    }
    
    return events;
}

EventParticipant EventRepositoryImpl::mapToParticipant(const Core::Database::QueryResult& result, int row) {
    // Column positions: 0=id, 1=event_id, 2=tenant_id, 3=user_id, 4=role, 5=status, 6=response_time
    
    std::string id = result.getValue(row, 0);
    std::string eventId = result.getValue(row, 1);
    std::string tenantId = result.getValue(row, 2);
    std::string userId = result.getValue(row, 3);
    std::string roleStr = result.getValue(row, 4);
    std::string statusStr = result.getValue(row, 5);

    EventParticipant participant(id, eventId, tenantId, userId,
                                 EventParticipant::stringToRole(roleStr),
                                 EventParticipant::stringToStatus(statusStr));

    std::string responseTime = result.getValue(row, 6);
    if (!responseTime.empty()) {
        participant.setResponseTime(responseTime);
    }

    return participant;
}

std::vector<EventParticipant> EventRepositoryImpl::mapToParticipants(const Core::Database::QueryResult& result) {
    std::vector<EventParticipant> participants;
    
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); i++) {
            participants.push_back(mapToParticipant(result, i));
        }
    }
    
    return participants;
}

EventReminder EventRepositoryImpl::mapToReminder(const Core::Database::QueryResult& result, int row) {
    // Column positions: 0=id, 1=event_id, 2=tenant_id, 3=user_id, 4=reminder_type, 5=minutes_before, 6=is_sent, 7=sent_at
    
    std::string id = result.getValue(row, 0);
    std::string eventId = result.getValue(row, 1);
    std::string tenantId = result.getValue(row, 2);
    std::string userId = result.getValue(row, 3);
    std::string typeStr = result.getValue(row, 4);
    int minutesBefore = std::stoi(result.getValue(row, 5));

    EventReminder reminder(id, eventId, tenantId, userId, ReminderType(typeStr), minutesBefore);

    std::string isSent = result.getValue(row, 6);
    if (isSent == "t" || isSent == "true") {
        reminder.markAsSent();
    }

    std::string sentAt = result.getValue(row, 7);
    if (!sentAt.empty()) {
        reminder.setSentAt(sentAt);
    }

    return reminder;
}

std::vector<EventReminder> EventRepositoryImpl::mapToReminders(const Core::Database::QueryResult& result) {
    std::vector<EventReminder> reminders;
    
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); i++) {
            reminders.push_back(mapToReminder(result, i));
        }
    }
    
    return reminders;
}

} // namespace Domain::Calendar

