#ifndef GET_EVENT_USE_CASE_HPP
#define GET_EVENT_USE_CASE_HPP

#include "../repositories/EventRepository.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Calendar {

namespace Utils = Core::Utils;

struct GetEventDTO {
    std::string eventId;
    std::string tenantId;
};

class GetEventUseCase {
public:
    explicit GetEventUseCase(std::shared_ptr<EventRepository> repository)
        : repository_(repository) {}

    std::optional<Event> execute(const GetEventDTO& dto) {
        Utils::Logger::info("[GetEventUseCase] Getting event: " + dto.eventId);
        return repository_->findById(dto.eventId, dto.tenantId);
    }

private:
    std::shared_ptr<EventRepository> repository_;
};

} // namespace Domain::Calendar

#endif // GET_EVENT_USE_CASE_HPP

