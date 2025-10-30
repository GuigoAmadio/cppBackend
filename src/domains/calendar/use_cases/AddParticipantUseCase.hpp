#ifndef ADD_PARTICIPANT_USE_CASE_HPP
#define ADD_PARTICIPANT_USE_CASE_HPP

#include "../repositories/EventRepository.hpp"
#include "../entities/EventParticipant.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Calendar {

namespace Utils = Core::Utils;

struct AddParticipantDTO {
    std::string eventId;
    std::string tenantId;
    std::string userId;
    std::string role;
};

class AddParticipantUseCase {
public:
    explicit AddParticipantUseCase(std::shared_ptr<EventRepository> repository)
        : repository_(repository) {}

    std::string execute(const AddParticipantDTO& dto) {
        Utils::Logger::info("[AddParticipantUseCase] Adding participant to event: " + dto.eventId);

        // Generate unique ID
        std::string id = generateSimpleUUID();

        // Create participant entity
        EventParticipant participant(
            id, dto.eventId, dto.tenantId, dto.userId,
            EventParticipant::stringToRole(dto.role),
            EventParticipant::Status::PENDING
        );

        // Save to repository
        if (!repository_->saveParticipant(participant)) {
            throw std::runtime_error("Failed to add participant");
        }

        Utils::Logger::info("[AddParticipantUseCase] Participant added successfully");
        return id;
    }

private:
    std::shared_ptr<EventRepository> repository_;

    std::string generateSimpleUUID() {
        static int counter = 0;
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        return "part-" + std::to_string(timestamp) + "-" + std::to_string(++counter);
    }
};

} // namespace Domain::Calendar

#endif // ADD_PARTICIPANT_USE_CASE_HPP

