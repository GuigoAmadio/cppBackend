#ifndef DELETE_EVENT_USE_CASE_HPP
#define DELETE_EVENT_USE_CASE_HPP

#include "../repositories/EventRepository.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Calendar {

namespace Utils = Core::Utils;

struct DeleteEventDTO {
    std::string eventId;
    std::string tenantId;
};

class DeleteEventUseCase {
public:
    explicit DeleteEventUseCase(std::shared_ptr<EventRepository> repository)
        : repository_(repository) {}

    void execute(const DeleteEventDTO& dto) {
        Utils::Logger::info("[DeleteEventUseCase] Deleting event: " + dto.eventId);

        if (!repository_->remove(dto.eventId, dto.tenantId)) {
            throw std::runtime_error("Failed to delete event");
        }

        Utils::Logger::info("[DeleteEventUseCase] Event deleted successfully");
    }

private:
    std::shared_ptr<EventRepository> repository_;
};

} // namespace Domain::Calendar

#endif // DELETE_EVENT_USE_CASE_HPP

