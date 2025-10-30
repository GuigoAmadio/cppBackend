#include "TaskController.hpp"
#include "../../../core/utils/Logger.hpp"
#include "../../../core/json/Json.hpp"
#include <sstream>

using namespace Core::Http;
namespace Utils = Core::Utils;

namespace Domain::Task {

TaskController::TaskController(std::shared_ptr<TaskRepository> repository)
    : repository_(repository) {
    Utils::Logger::info("[TaskController] Initialized");
}

// =========================================================
// CREATE TASK
// =========================================================

Response TaskController::createTask(const Request& req) {
    Utils::Logger::info("[TaskController] createTask()");
    
    auto json = Core::Json::makeObject();
    
    try {
        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();

        std::string tenantId = req.getCustomData("user_tenant_id");
        std::string userId = req.getCustomData("user_id");

        if (tenantId.empty()) {
            json->asObject()["error"] = Core::Json::makeString("Tenant ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        CreateTaskDTO dto;
        dto.tenantId = tenantId;
        dto.createdBy = userId;

        // Required fields
        if (bodyObj.count("title") && bodyObj["title"]->isString()) {
            dto.title = bodyObj["title"]->asString();
        } else {
            json->asObject()["error"] = Core::Json::makeString("Title is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        // Optional fields
        if (bodyObj.count("description") && bodyObj["description"]->isString()) {
            dto.description = bodyObj["description"]->asString();
        }

        if (bodyObj.count("task_type") && bodyObj["task_type"]->isString()) {
            dto.taskType = bodyObj["task_type"]->asString();
        }

        if (bodyObj.count("priority") && bodyObj["priority"]->isString()) {
            dto.priority = bodyObj["priority"]->asString();
        }

        if (bodyObj.count("workspace_id") && bodyObj["workspace_id"]->isString()) {
            dto.workspaceId = bodyObj["workspace_id"]->asString();
        }

        if (bodyObj.count("project_id") && bodyObj["project_id"]->isString()) {
            dto.projectId = bodyObj["project_id"]->asString();
        }

        if (bodyObj.count("assigned_to") && bodyObj["assigned_to"]->isString()) {
            dto.assignedTo = bodyObj["assigned_to"]->asString();
        }

        if (bodyObj.count("parent_task_id") && bodyObj["parent_task_id"]->isString()) {
            dto.parentTaskId = bodyObj["parent_task_id"]->asString();
        }

        if (bodyObj.count("estimated_hours") && bodyObj["estimated_hours"]->isNumber()) {
            dto.estimatedHours = bodyObj["estimated_hours"]->asNumber();
        }

        if (bodyObj.count("story_points") && bodyObj["story_points"]->isNumber()) {
            dto.storyPoints = static_cast<int>(bodyObj["story_points"]->asNumber());
        }

        if (bodyObj.count("due_date") && bodyObj["due_date"]->isString()) {
            dto.dueDate = bodyObj["due_date"]->asString();
        }

        // Execute use case
        CreateTaskUseCase useCase(repository_);
        auto result = useCase.execute(dto);

        if (result.success) {
            json->asObject()["id"] = Core::Json::makeString(result.taskId);
            json->asObject()["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::Created).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskController] createTask() exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// GET TASK
// =========================================================

Response TaskController::getTask(const Request& req) {
    Utils::Logger::info("[TaskController] getTask()");
    
    auto json = Core::Json::makeObject();
    
    try {
        std::string taskId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");

        if (taskId.empty()) {
            json->asObject()["error"] = Core::Json::makeString("Task ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        GetTaskDTO dto{taskId, tenantId};
        GetTaskUseCase useCase(repository_);
        auto task = useCase.execute(dto);

        if (task.has_value()) {
            // Retornar task como objeto JSON (não como string aninhada)
            auto taskJson = taskToJsonObject(*task);
            return Response(StatusCode::OK).json(*taskJson);
        } else {
            json->asObject()["error"] = Core::Json::makeString("Task not found");
            return Response(StatusCode::NotFound).json(*json);
        }

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskController] getTask() exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// LIST TASKS
// =========================================================

Response TaskController::listTasks(const Request& req) {
    Utils::Logger::info("[TaskController] listTasks()");
    
    auto json = Core::Json::makeObject();
    
    try {
        std::string tenantId = req.getCustomData("user_tenant_id");

        ListTasksDTO dto;
        dto.tenantId = tenantId;

        // Parse query parameters
        std::string workspaceId = req.getQuery("workspace_id");
        if (!workspaceId.empty()) dto.workspaceId = workspaceId;

        std::string projectId = req.getQuery("project_id");
        if (!projectId.empty()) dto.projectId = projectId;

        std::string assignedTo = req.getQuery("assigned_to");
        if (!assignedTo.empty()) dto.assignedTo = assignedTo;

        std::string status = req.getQuery("status");
        if (!status.empty()) dto.status = status;

        ListTasksUseCase useCase(repository_);
        auto tasks = useCase.execute(dto);

        auto tasksArray = Core::Json::makeArray();
        for (const auto& task : tasks) {
            tasksArray->asArray().push_back(taskToJsonObject(task));  // Usar taskToJsonObject em vez de string
        }

        json->asObject()["tasks"] = tasksArray;
        json->asObject()["count"] = Core::Json::makeNumber(tasks.size());
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskController] listTasks() exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// UPDATE TASK
// =========================================================

Response TaskController::updateTask(const Request& req) {
    Utils::Logger::info("[TaskController] updateTask()");
    
    auto json = Core::Json::makeObject();
    
    try {
        std::string taskId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");

        if (taskId.empty()) {
            json->asObject()["error"] = Core::Json::makeString("Task ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();

        UpdateTaskDTO dto;
        dto.taskId = taskId;
        dto.tenantId = tenantId;

        // Parse optional update fields
        if (bodyObj.count("title") && bodyObj["title"]->isString()) {
            dto.title = bodyObj["title"]->asString();
        }

        if (bodyObj.count("description") && bodyObj["description"]->isString()) {
            dto.description = bodyObj["description"]->asString();
        }

        if (bodyObj.count("priority") && bodyObj["priority"]->isString()) {
            dto.priority = bodyObj["priority"]->asString();
        }

        if (bodyObj.count("assigned_to") && bodyObj["assigned_to"]->isString()) {
            dto.assignedTo = bodyObj["assigned_to"]->asString();
        }

        if (bodyObj.count("estimated_hours") && bodyObj["estimated_hours"]->isNumber()) {
            dto.estimatedHours = bodyObj["estimated_hours"]->asNumber();
        }

        if (bodyObj.count("story_points") && bodyObj["story_points"]->isNumber()) {
            dto.storyPoints = static_cast<int>(bodyObj["story_points"]->asNumber());
        }

        if (bodyObj.count("due_date") && bodyObj["due_date"]->isString()) {
            dto.dueDate = bodyObj["due_date"]->asString();
        }

        UpdateTaskUseCase useCase(repository_);
        auto result = useCase.execute(dto);

        if (result.success) {
            json->asObject()["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskController] updateTask() exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// DELETE TASK
// =========================================================

Response TaskController::deleteTask(const Request& req) {
    Utils::Logger::info("[TaskController] deleteTask()");
    
    auto json = Core::Json::makeObject();
    
    try {
        std::string taskId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");

        if (taskId.empty()) {
            json->asObject()["error"] = Core::Json::makeString("Task ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        DeleteTaskDTO dto{taskId, tenantId};
        DeleteTaskUseCase useCase(repository_);
        auto result = useCase.execute(dto);

        if (result.success) {
            json->asObject()["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskController] deleteTask() exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// UPDATE STATUS
// =========================================================

Response TaskController::updateStatus(const Request& req) {
    Utils::Logger::info("[TaskController] updateStatus()");
    
    auto json = Core::Json::makeObject();
    
    try {
        std::string taskId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");

        if (taskId.empty()) {
            json->asObject()["error"] = Core::Json::makeString("Task ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();

        if (!bodyObj.count("status") || !bodyObj["status"]->isString()) {
            json->asObject()["error"] = Core::Json::makeString("Status is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        UpdateStatusDTO dto{taskId, tenantId, bodyObj["status"]->asString()};
        UpdateStatusUseCase useCase(repository_);
        auto result = useCase.execute(dto);

        if (result.success) {
            json->asObject()["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::OK).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskController] updateStatus() exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// ADD COMMENT
// =========================================================

Response TaskController::addComment(const Request& req) {
    Utils::Logger::info("[TaskController] addComment()");
    
    auto json = Core::Json::makeObject();
    
    try {
        std::string taskId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        std::string userId = req.getCustomData("user_id");

        if (taskId.empty()) {
            json->asObject()["error"] = Core::Json::makeString("Task ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();

        if (!bodyObj.count("content") || !bodyObj["content"]->isString()) {
            json->asObject()["error"] = Core::Json::makeString("Content is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        AddCommentDTO dto;
        dto.taskId = taskId;
        dto.tenantId = tenantId;
        dto.content = bodyObj["content"]->asString();
        dto.createdBy = userId;

        AddCommentUseCase useCase(repository_);
        auto result = useCase.execute(dto);

        if (result.success) {
            json->asObject()["id"] = Core::Json::makeString(result.commentId);
            json->asObject()["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::Created).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskController] addComment() exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// GET COMMENTS
// =========================================================

Response TaskController::getComments(const Request& req) {
    Utils::Logger::info("[TaskController] getComments()");
    
    auto json = Core::Json::makeObject();
    
    try {
        std::string taskId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");

        if (taskId.empty()) {
            json->asObject()["error"] = Core::Json::makeString("Task ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto comments = repository_->findCommentsByTask(taskId, tenantId);

        auto commentsArray = Core::Json::makeArray();
        for (const auto& comment : comments) {
            commentsArray->asArray().push_back(commentToJsonObject(comment));  // Usar objeto JSON
        }

        json->asObject()["comments"] = commentsArray;
        json->asObject()["count"] = Core::Json::makeNumber(comments.size());
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskController] getComments() exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// ADD CHECKLIST
// =========================================================

Response TaskController::addChecklist(const Request& req) {
    Utils::Logger::info("[TaskController] addChecklist()");
    
    auto json = Core::Json::makeObject();
    
    try {
        std::string taskId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");

        if (taskId.empty()) {
            json->asObject()["error"] = Core::Json::makeString("Task ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto bodyJson = req.getJson();
        if (!bodyJson || !bodyJson->isObject()) {
            json->asObject()["error"] = Core::Json::makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto& bodyObj = bodyJson->asObject();

        if (!bodyObj.count("title") || !bodyObj["title"]->isString()) {
            json->asObject()["error"] = Core::Json::makeString("Title is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        AddChecklistDTO dto;
        dto.taskId = taskId;
        dto.tenantId = tenantId;
        dto.title = bodyObj["title"]->asString();

        // Parse items array
        if (bodyObj.count("items") && bodyObj["items"]->isArray()) {
            for (const auto& itemJson : bodyObj["items"]->asArray()) {
                if (itemJson->isObject() && itemJson->asObject().count("content")) {
                    ChecklistItemData item;
                    item.content = itemJson->asObject()["content"]->asString();
                    dto.items.push_back(item);
                }
            }
        }

        AddChecklistUseCase useCase(repository_);
        auto result = useCase.execute(dto);

        if (result.success) {
            json->asObject()["id"] = Core::Json::makeString(result.checklistId);
            json->asObject()["message"] = Core::Json::makeString(result.message);
            return Response(StatusCode::Created).json(*json);
        } else {
            json->asObject()["error"] = Core::Json::makeString(result.message);
            return Response(StatusCode::BadRequest).json(*json);
        }

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskController] addChecklist() exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// GET CHECKLISTS
// =========================================================

Response TaskController::getChecklists(const Request& req) {
    Utils::Logger::info("[TaskController] getChecklists()");
    
    auto json = Core::Json::makeObject();
    
    try {
        std::string taskId = req.getParam("id");
        std::string tenantId = req.getCustomData("user_tenant_id");

        if (taskId.empty()) {
            json->asObject()["error"] = Core::Json::makeString("Task ID is required");
            return Response(StatusCode::BadRequest).json(*json);
        }

        auto checklists = repository_->findChecklistsByTask(taskId, tenantId);

        auto checklistsArray = Core::Json::makeArray();
        for (const auto& checklist : checklists) {
            checklistsArray->asArray().push_back(checklistToJsonObject(checklist));  // Usar objeto JSON
        }

        json->asObject()["checklists"] = checklistsArray;
        json->asObject()["count"] = Core::Json::makeNumber(checklists.size());
        return Response(StatusCode::OK).json(*json);

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskController] getChecklists() exception: " + std::string(e.what()));
        json->asObject()["error"] = Core::Json::makeString(std::string(e.what()));
        return Response(StatusCode::InternalServerError).json(*json);
    }
}

// =========================================================
// HELPER METHODS
// =========================================================

std::shared_ptr<Core::Json::JsonValue> TaskController::taskToJsonObject(const Task& task) {
    auto json = Core::Json::makeObject();
    
    json->asObject()["id"] = Core::Json::makeString(task.getId());
    json->asObject()["title"] = Core::Json::makeString(task.getTitle());
    json->asObject()["task_type"] = Core::Json::makeString(task.getTaskType().toString());
    json->asObject()["status"] = Core::Json::makeString(task.getStatus().toString());
    json->asObject()["priority"] = Core::Json::makeString(task.getPriority().toString());
    json->asObject()["created_by"] = Core::Json::makeString(task.getCreatedBy());
    
    if (task.getDescription().has_value()) {
        json->asObject()["description"] = Core::Json::makeString(*task.getDescription());
    }
    if (task.getAssignedTo().has_value()) {
        json->asObject()["assigned_to"] = Core::Json::makeString(*task.getAssignedTo());
    }
    if (task.getWorkspaceId().has_value()) {
        json->asObject()["workspace_id"] = Core::Json::makeString(*task.getWorkspaceId());
    }
    if (task.getProjectId().has_value()) {
        json->asObject()["project_id"] = Core::Json::makeString(*task.getProjectId());
    }
    if (task.getReporter().has_value()) {
        json->asObject()["reporter"] = Core::Json::makeString(*task.getReporter());
    }
    
    return json;
}

std::shared_ptr<Core::Json::JsonValue> TaskController::commentToJsonObject(const TaskComment& comment) {
    auto json = Core::Json::makeObject();
    json->asObject()["id"] = Core::Json::makeString(comment.getId());
    json->asObject()["task_id"] = Core::Json::makeString(comment.getTaskId());
    json->asObject()["content"] = Core::Json::makeString(comment.getContent());
    json->asObject()["created_by"] = Core::Json::makeString(comment.getCreatedBy());
    if (comment.getParentCommentId().has_value()) {
        json->asObject()["parent_comment_id"] = Core::Json::makeString(*comment.getParentCommentId());
    }
    return json;
}

std::shared_ptr<Core::Json::JsonValue> TaskController::checklistToJsonObject(const Checklist& checklist) {
    auto json = Core::Json::makeObject();
    json->asObject()["id"] = Core::Json::makeString(checklist.getId());
    json->asObject()["task_id"] = Core::Json::makeString(checklist.getTaskId());
    json->asObject()["title"] = Core::Json::makeString(checklist.getTitle());
    json->asObject()["position"] = Core::Json::makeNumber(checklist.getPosition());
    return json;
}

std::string TaskController::taskToJson(const Task& task) {
    std::ostringstream json;
    json << "{"
         << "\"id\":\"" << task.getId() << "\""
         << ",\"title\":\"" << task.getTitle() << "\""
         << ",\"task_type\":\"" << task.getTaskType().toString() << "\""
         << ",\"status\":\"" << task.getStatus().toString() << "\""
         << ",\"priority\":\"" << task.getPriority().toString() << "\""
         << ",\"created_by\":\"" << task.getCreatedBy() << "\"";

    if (task.getDescription().has_value()) {
        json << ",\"description\":\"" << *task.getDescription() << "\"";
    }

    if (task.getAssignedTo().has_value()) {
        json << ",\"assigned_to\":\"" << *task.getAssignedTo() << "\"";
    }

    if (task.getWorkspaceId().has_value()) {
        json << ",\"workspace_id\":\"" << *task.getWorkspaceId() << "\"";
    }

    if (task.getProjectId().has_value()) {
        json << ",\"project_id\":\"" << *task.getProjectId() << "\"";
    }

    json << ",\"estimated_hours\":" << task.getEstimatedHours()
         << ",\"actual_hours\":" << task.getActualHours()
         << ",\"story_points\":" << task.getStoryPoints();

    if (task.getDueDate().has_value()) {
        json << ",\"due_date\":\"" << *task.getDueDate() << "\"";
    }

    if (task.getCreatedAt().has_value()) {
        json << ",\"created_at\":\"" << *task.getCreatedAt() << "\"";
    }

    json << "}";
    return json.str();
}

std::string TaskController::commentToJson(const TaskComment& comment) {
    std::ostringstream json;
    json << "{"
         << "\"id\":\"" << comment.getId() << "\""
         << ",\"content\":\"" << comment.getContent() << "\""
         << ",\"created_by\":\"" << comment.getCreatedBy() << "\"";

    if (comment.getCreatedAt().has_value()) {
        json << ",\"created_at\":\"" << *comment.getCreatedAt() << "\"";
    }

    json << "}";
    return json.str();
}

std::string TaskController::checklistToJson(const Checklist& checklist) {
    std::ostringstream json;
    json << "{"
         << "\"id\":\"" << checklist.getId() << "\""
         << ",\"title\":\"" << checklist.getTitle() << "\"";

    if (checklist.getDescription().has_value()) {
        json << ",\"description\":\"" << *checklist.getDescription() << "\"";
    }

    json << "}";
    return json.str();
}

} // namespace Domain::Task

