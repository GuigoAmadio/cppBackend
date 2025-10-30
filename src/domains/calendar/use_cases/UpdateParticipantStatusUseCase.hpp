#ifndef UPDATE_PARTICIPANT_STATUS_USE_CASE_HPP
#define UPDATE_PARTICIPANT_STATUS_USE_CASE_HPP

#include "../repositories/EventRepository.hpp"
#include "../entities/EventParticipant.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Calendar {

namespace Utils = Core::Utils;

struct UpdateParticipantStatusDTO {
    std::string participantId;
    std::string status;
};

class UpdateParticipantStatusUseCase {
public:
    explicit UpdateParticipantStatusUseCase(std::shared_ptr<EventRepository> repository)
        : repository_(repository) {}

    void execute(const UpdateParticipantStatusDTO& dto) {
        Utils::Logger::info("[UpdateParticipantStatusUseCase] Updating participant status: " + dto.participantId);

        EventParticipant::Status status = EventParticipant::stringToStatus(dto.status);

        if (!repository_->updateParticipantStatus(dto.participantId, status)) {
            throw std::runtime_error("Failed to update participant status");
        }

        Utils::Logger::info("[UpdateParticipantStatusUseCase] Participant status updated successfully");
    }

private:
    std::shared_ptr<EventRepository> repository_;
};

} // namespace Domain::Calendar

#endif // UPDATE_PARTICIPANT_STATUS_USE_CASE_HPP

