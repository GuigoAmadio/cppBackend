#ifndef LIST_EVENTS_USE_CASE_HPP
#define LIST_EVENTS_USE_CASE_HPP

#include "../repositories/EventRepository.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Calendar {

namespace Utils = Core::Utils;

struct ListEventsDTO {
    std::string tenantId;
    std::optional<std::string> workspaceId;
    std::optional<std::string> userId;
    std::optional<std::string> startDate;
    std::optional<std::string> endDate;
};

class ListEventsUseCase {
public:
    explicit ListEventsUseCase(std::shared_ptr<EventRepository> repository)
        : repository_(repository) {}

    std::vector<Event> execute(const ListEventsDTO& dto) {
        Utils::Logger::info("[ListEventsUseCase] Listing events for tenant: " + dto.tenantId);

        // If date range is specified
        if (dto.startDate && dto.endDate) {
            return repository_->findByDateRange(dto.tenantId, *dto.startDate, *dto.endDate);
        }

        // If workspace is specified
        if (dto.workspaceId) {
            return repository_->findByWorkspace(*dto.workspaceId, dto.tenantId);
        }

        // If user is specified
        if (dto.userId) {
            return repository_->findByUser(*dto.userId, dto.tenantId);
        }

        // Default: all events for tenant
        return repository_->findByTenant(dto.tenantId);
    }

private:
    std::shared_ptr<EventRepository> repository_;
};

} // namespace Domain::Calendar

#endif // LIST_EVENTS_USE_CASE_HPP

