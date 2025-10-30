#pragma once

#include <memory>
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../use_cases/CreateTaskUseCase.hpp"
#include "../use_cases/GetTaskUseCase.hpp"
#include "../use_cases/UpdateTaskUseCase.hpp"
#include "../use_cases/DeleteTaskUseCase.hpp"
#include "../use_cases/ListTasksUseCase.hpp"
#include "../use_cases/UpdateStatusUseCase.hpp"
#include "../use_cases/AddCommentUseCase.hpp"
#include "../use_cases/AddChecklistUseCase.hpp"
#include "../repositories/TaskRepository.hpp"

namespace Domain::Task {

class TaskController {
public:
    explicit TaskController(std::shared_ptr<TaskRepository> repository);

    // Task CRUD
    Core::Http::Response createTask(const Core::Http::Request& req);
    Core::Http::Response getTask(const Core::Http::Request& req);
    Core::Http::Response listTasks(const Core::Http::Request& req);
    Core::Http::Response updateTask(const Core::Http::Request& req);
    Core::Http::Response deleteTask(const Core::Http::Request& req);
    
    // Task Operations
    Core::Http::Response updateStatus(const Core::Http::Request& req);
    Core::Http::Response addComment(const Core::Http::Request& req);
    Core::Http::Response getComments(const Core::Http::Request& req);
    Core::Http::Response addChecklist(const Core::Http::Request& req);
    Core::Http::Response getChecklists(const Core::Http::Request& req);

private:
    std::shared_ptr<TaskRepository> repository_;
    
    // Helper methods
    std::shared_ptr<Core::Json::JsonValue> taskToJsonObject(const Task& task);
    std::shared_ptr<Core::Json::JsonValue> commentToJsonObject(const TaskComment& comment);
    std::shared_ptr<Core::Json::JsonValue> checklistToJsonObject(const Checklist& checklist);
    std::string taskToJson(const Task& task);
    std::string commentToJson(const TaskComment& comment);
    std::string checklistToJson(const Checklist& checklist);
};

} // namespace Domain::Task

