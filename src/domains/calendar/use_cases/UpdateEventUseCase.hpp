#ifndef UPDATE_EVENT_USE_CASE_HPP
#define UPDATE_EVENT_USE_CASE_HPP

#include "../repositories/EventRepository.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Calendar {

namespace Utils = Core::Utils;

struct UpdateEventDTO {
    std::string eventId;
    std::string tenantId;
    std::optional<std::string> title;
    std::optional<std::string> description;
    std::optional<std::string> location;
    std::optional<std::string> startTime;
    std::optional<std::string> endTime;
    std::optional<std::string> color;
    std::optional<bool> allDay;
    std::optional<bool> isPrivate;
};

class UpdateEventUseCase {
public:
    explicit UpdateEventUseCase(std::shared_ptr<EventRepository> repository)
        : repository_(repository) {}

    void execute(const UpdateEventDTO& dto) {
        Utils::Logger::info("[UpdateEventUseCase] Updating event: " + dto.eventId);

        // Find existing event
        auto eventOpt = repository_->findById(dto.eventId, dto.tenantId);
        if (!eventOpt) {
            throw std::runtime_error("Event not found");
        }

        Event event = *eventOpt;

        // Update fields if provided
        if (dto.title) event.updateTitle(*dto.title);
        if (dto.description) event.setDescription(*dto.description);
        if (dto.location) event.setLocation(*dto.location);
        if (dto.color) event.setColor(*dto.color);
        if (dto.allDay) event.setAllDay(*dto.allDay);
        if (dto.isPrivate) event.setPrivate(*dto.isPrivate);
        
        if (dto.startTime && dto.endTime) {
            event.updateTimes(*dto.startTime, *dto.endTime);
        }

        // Save updated event
        if (!repository_->update(event)) {
            throw std::runtime_error("Failed to update event");
        }

        Utils::Logger::info("[UpdateEventUseCase] Event updated successfully");
    }

private:
    std::shared_ptr<EventRepository> repository_;
};

} // namespace Domain::Calendar

#endif // UPDATE_EVENT_USE_CASE_HPP

