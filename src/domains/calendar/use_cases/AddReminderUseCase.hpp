#ifndef ADD_REMINDER_USE_CASE_HPP
#define ADD_REMINDER_USE_CASE_HPP

#include "../repositories/EventRepository.hpp"
#include "../entities/EventReminder.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Domain::Calendar {

namespace Utils = Core::Utils;

struct AddReminderDTO {
    std::string eventId;
    std::string tenantId;
    std::string userId;
    std::string reminderType;
    int minutesBefore;
};

class AddReminderUseCase {
public:
    explicit AddReminderUseCase(std::shared_ptr<EventRepository> repository)
        : repository_(repository) {}

    std::string execute(const AddReminderDTO& dto) {
        Utils::Logger::info("[AddReminderUseCase] Adding reminder to event: " + dto.eventId);

        // Generate unique ID
        std::string id = generateSimpleUUID();

        // Create reminder entity
        EventReminder reminder(
            id, dto.eventId, dto.tenantId, dto.userId,
            ReminderType(dto.reminderType),
            dto.minutesBefore
        );

        // Save to repository
        if (!repository_->saveReminder(reminder)) {
            throw std::runtime_error("Failed to add reminder");
        }

        Utils::Logger::info("[AddReminderUseCase] Reminder added successfully");
        return id;
    }

private:
    std::shared_ptr<EventRepository> repository_;

    std::string generateSimpleUUID() {
        static int counter = 0;
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        return "rem-" + std::to_string(timestamp) + "-" + std::to_string(++counter);
    }
};

} // namespace Domain::Calendar

#endif // ADD_REMINDER_USE_CASE_HPP

