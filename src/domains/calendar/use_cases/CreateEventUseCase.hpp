#ifndef CREATE_EVENT_USE_CASE_HPP
#define CREATE_EVENT_USE_CASE_HPP

#include <string>
#include <optional>
#include <vector>
#include "../repositories/EventRepository.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Calendar {

namespace Utils = Core::Utils;

struct CreateEventDTO {
    std::string tenantId;
    std::string title;
    std::string eventType;
    std::string startTime;
    std::string endTime;
    std::string createdBy;
    
    std::optional<std::string> workspaceId;
    std::optional<std::string> description;
    std::optional<std::string> location;
    std::optional<std::string> timezone;
    std::optional<std::string> organizerId;
    std::optional<std::string> taskId;
    std::optional<std::string> color;
    std::optional<bool> allDay;
    std::optional<bool> isPrivate;
    std::vector<std::string> tags;
};

class CreateEventUseCase {
public:
    explicit CreateEventUseCase(std::shared_ptr<EventRepository> repository)
        : repository_(repository) {}

    std::string execute(const CreateEventDTO& dto) {
        Utils::Logger::info("[CreateEventUseCase] Creating event: " + dto.title);

        // Generate unique ID
        std::string id = generateSimpleUUID();

        // Create event entity
        Event event(
            id, dto.tenantId, dto.title,
            EventType(dto.eventType),
            EventStatus(EventStatus::Type::CONFIRMED),
            dto.startTime, dto.endTime, dto.createdBy
        );

        // Set optional fields
        if (dto.workspaceId) event.setWorkspaceId(*dto.workspaceId);
        if (dto.description) event.setDescription(*dto.description);
        if (dto.location) event.setLocation(*dto.location);
        if (dto.timezone) event.setTimezone(*dto.timezone);
        if (dto.organizerId) event.setOrganizerId(*dto.organizerId);
        if (dto.taskId) event.setTaskId(*dto.taskId);
        if (dto.color) event.setColor(*dto.color);
        if (dto.allDay) event.setAllDay(*dto.allDay);
        if (dto.isPrivate) event.setPrivate(*dto.isPrivate);
        if (!dto.tags.empty()) event.setTags(dto.tags);

        // Save to repository
        if (!repository_->save(event)) {
            throw std::runtime_error("Failed to save event");
        }

        Utils::Logger::info("[CreateEventUseCase] Event created successfully: " + id);
        return id;
    }

private:
    std::shared_ptr<EventRepository> repository_;

    std::string generateSimpleUUID() {
        static int counter = 0;
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        return "evt-" + std::to_string(timestamp) + "-" + std::to_string(++counter);
    }
};

} // namespace Domain::Calendar

#endif // CREATE_EVENT_USE_CASE_HPP

