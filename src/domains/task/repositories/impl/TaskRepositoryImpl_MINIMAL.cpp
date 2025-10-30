#include "TaskRepositoryImpl.hpp"
#include "../../../../core/utils/Logger.hpp"
#include <sstream>

namespace Utils = Core::Utils;

namespace Domain::Task {

TaskRepositoryImpl::TaskRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {
    Utils::Logger::info("[TaskRepositoryImpl] Initialized (MINIMAL VERSION - 3 methods)");
}

// =========================================================
// MÉTODO 1: SAVE - Salvar nova task
// =========================================================
bool TaskRepositoryImpl::save(const Task& task) {
    Utils::Logger::info("[TaskRepositoryImpl] save() - ID: " + task.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            INSERT INTO tasks (
                id, tenant_id, workspace_id, project_id, title, description,
                task_type, status, priority, created_by, assigned_to, reporter,
                estimated_hours, actual_hours, story_points, start_date, due_date,
                parent_task_id, depends_on, blocks, tags, labels, position
            ) VALUES (
                $1, $2, NULLIF($3,''), NULLIF($4,''), $5, NULLIF($6,''),
                $7, $8, $9, $10, NULLIF($11,''), NULLIF($12,''),
                $13, $14, $15, NULLIF($16,'')::timestamp, NULLIF($17,'')::timestamp,
                NULLIF($18,''), $19, $20, $21, $22, $23
            )
        )";

        std::vector<std::string> params;
        params.push_back(task.getId());
        params.push_back(task.getTenantId());
        params.push_back(task.getWorkspaceId().value_or(""));
        params.push_back(task.getProjectId().value_or(""));
        params.push_back(task.getTitle());
        params.push_back(task.getDescription().value_or(""));
        params.push_back(task.getTaskType().toString());
        params.push_back(task.getStatus().toString());
        params.push_back(task.getPriority().toString());
        params.push_back(task.getCreatedBy());
        params.push_back(task.getAssignedTo().value_or(""));
        params.push_back(task.getReporter().value_or(""));
        params.push_back(std::to_string(task.getEstimatedHours()));
        params.push_back(std::to_string(task.getActualHours()));
        params.push_back(std::to_string(task.getStoryPoints()));
        params.push_back(task.getStartDate().value_or(""));
        params.push_back(task.getDueDate().value_or(""));
        params.push_back(task.getParentTaskId().value_or(""));
        params.push_back(arrayToPostgresArray(task.getDependsOn()));
        params.push_back(arrayToPostgresArray(task.getBlocks()));
        params.push_back(arrayToPostgresArray(task.getTags()));
        params.push_back(arrayToPostgresArray(task.getLabels()));
        params.push_back(std::to_string(task.getPosition()));

        auto result = conn->executeParams(query, params);
        
        if (result.isSuccess()) {
            Utils::Logger::info("[TaskRepositoryImpl] Task saved successfully");
            return true;
        }
        
        Utils::Logger::error("[TaskRepositoryImpl] Failed to save task");
        return false;

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] save() exception: " + std::string(e.what()));
        return false;
    }
}

// =========================================================
// MÉTODO 2: FIND BY ID - Buscar task por ID
// =========================================================
std::optional<Task> TaskRepositoryImpl::findById(const std::string& taskId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] findById() - ID: " + taskId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM tasks WHERE id = $1 AND tenant_id = $2";
        
        std::vector<std::string> params;
        params.push_back(taskId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        
        if (result.isSuccess() && result.rowCount() > 0) {
            return mapToTask(result, 0);
        }
        
        return std::nullopt;

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] findById() exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

// =========================================================
// MÉTODO 3: FIND BY TENANT - Listar tasks do tenant
// =========================================================
std::vector<Task> TaskRepositoryImpl::findByTenant(const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] findByTenant() - tenant: " + tenantId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM tasks WHERE tenant_id = $1 AND is_archived = false ORDER BY position, created_at DESC";
        
        std::vector<std::string> params;
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToTasks(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] findByTenant() exception: " + std::string(e.what()));
        return {};
    }
}

// =========================================================
// STUBS - Métodos não implementados (retornam valores padrão)
// =========================================================
bool TaskRepositoryImpl::update(const Task&) {
    Utils::Logger::warning("[TaskRepositoryImpl] update() - NOT IMPLEMENTED YET");
    return false;
}

bool TaskRepositoryImpl::remove(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] remove() - NOT IMPLEMENTED YET");
    return false;
}

std::vector<Task> TaskRepositoryImpl::findByWorkspace(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] findByWorkspace() - NOT IMPLEMENTED YET");
    return {};
}

std::vector<Task> TaskRepositoryImpl::findByProject(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] findByProject() - NOT IMPLEMENTED YET");
    return {};
}

std::vector<Task> TaskRepositoryImpl::findByAssignee(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] findByAssignee() - NOT IMPLEMENTED YET");
    return {};
}

std::vector<Task> TaskRepositoryImpl::findByCreator(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] findByCreator() - NOT IMPLEMENTED YET");
    return {};
}

std::vector<Task> TaskRepositoryImpl::findByStatus(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] findByStatus() - NOT IMPLEMENTED YET");
    return {};
}

std::vector<Task> TaskRepositoryImpl::findByParent(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] findByParent() - NOT IMPLEMENTED YET");
    return {};
}

bool TaskRepositoryImpl::saveComment(const TaskComment&) {
    Utils::Logger::warning("[TaskRepositoryImpl] saveComment() - NOT IMPLEMENTED YET");
    return false;
}

bool TaskRepositoryImpl::updateComment(const TaskComment&) {
    Utils::Logger::warning("[TaskRepositoryImpl] updateComment() - NOT IMPLEMENTED YET");
    return false;
}

bool TaskRepositoryImpl::removeComment(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] removeComment() - NOT IMPLEMENTED YET");
    return false;
}

std::optional<TaskComment> TaskRepositoryImpl::findCommentById(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] findCommentById() - NOT IMPLEMENTED YET");
    return std::nullopt;
}

std::vector<TaskComment> TaskRepositoryImpl::findCommentsByTask(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] findCommentsByTask() - NOT IMPLEMENTED YET");
    return {};
}

bool TaskRepositoryImpl::saveChecklist(const Checklist&) {
    Utils::Logger::warning("[TaskRepositoryImpl] saveChecklist() - NOT IMPLEMENTED YET");
    return false;
}

bool TaskRepositoryImpl::updateChecklist(const Checklist&) {
    Utils::Logger::warning("[TaskRepositoryImpl] updateChecklist() - NOT IMPLEMENTED YET");
    return false;
}

bool TaskRepositoryImpl::removeChecklist(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] removeChecklist() - NOT IMPLEMENTED YET");
    return false;
}

std::vector<Checklist> TaskRepositoryImpl::findChecklistsByTask(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] findChecklistsByTask() - NOT IMPLEMENTED YET");
    return {};
}

bool TaskRepositoryImpl::saveChecklistItem(const ChecklistItem&) {
    Utils::Logger::warning("[TaskRepositoryImpl] saveChecklistItem() - NOT IMPLEMENTED YET");
    return false;
}

bool TaskRepositoryImpl::updateChecklistItem(const ChecklistItem&) {
    Utils::Logger::warning("[TaskRepositoryImpl] updateChecklistItem() - NOT IMPLEMENTED YET");
    return false;
}

bool TaskRepositoryImpl::removeChecklistItem(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] removeChecklistItem() - NOT IMPLEMENTED YET");
    return false;
}

std::vector<ChecklistItem> TaskRepositoryImpl::findChecklistItems(const std::string&, const std::string&) {
    Utils::Logger::warning("[TaskRepositoryImpl] findChecklistItems() - NOT IMPLEMENTED YET");
    return {};
}

// =========================================================
// HELPERS - Mapeamento e conversão
// =========================================================
Task TaskRepositoryImpl::mapToTask(const Core::Database::QueryResult& result, int row) {
    std::string id = result.getValue(row, 0);
    std::string tenantId = result.getValue(row, 1);
    std::string title = result.getValue(row, 4);
    std::string taskTypeStr = result.getValue(row, 6);
    std::string statusStr = result.getValue(row, 7);
    std::string priorityStr = result.getValue(row, 8);
    std::string createdBy = result.getValue(row, 9);

    Task task(id, tenantId, title, TaskType(taskTypeStr), TaskStatus(statusStr), TaskPriority(priorityStr), createdBy);

    // Set optional fields if present
    std::string description = result.getValue(row, 5);
    if (!description.empty()) task.setDescription(description);

    std::string assignedTo = result.getValue(row, 10);
    if (!assignedTo.empty()) task.setAssignedTo(assignedTo);

    return task;
}

std::vector<Task> TaskRepositoryImpl::mapToTasks(const Core::Database::QueryResult& result) {
    std::vector<Task> tasks;
    
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); i++) {
            tasks.push_back(mapToTask(result, i));
        }
    }
    
    return tasks;
}

TaskComment TaskRepositoryImpl::mapToComment(const Core::Database::QueryResult&, int) {
    throw std::runtime_error("mapToComment() not implemented");
}

std::vector<TaskComment> TaskRepositoryImpl::mapToComments(const Core::Database::QueryResult&) {
    return {};
}

Checklist TaskRepositoryImpl::mapToChecklist(const Core::Database::QueryResult&, int) {
    throw std::runtime_error("mapToChecklist() not implemented");
}

std::vector<Checklist> TaskRepositoryImpl::mapToChecklists(const Core::Database::QueryResult&) {
    return {};
}

ChecklistItem TaskRepositoryImpl::mapToChecklistItem(const Core::Database::QueryResult&, int) {
    throw std::runtime_error("mapToChecklistItem() not implemented");
}

std::vector<ChecklistItem> TaskRepositoryImpl::mapToChecklistItems(const Core::Database::QueryResult&) {
    return {};
}

std::string TaskRepositoryImpl::arrayToPostgresArray(const std::vector<std::string>& vec) {
    if (vec.empty()) {
        return "{}";
    }
    
    std::ostringstream oss;
    oss << "{";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "\"" << vec[i] << "\"";
    }
    oss << "}";
    return oss.str();
}

std::vector<std::string> TaskRepositoryImpl::postgresArrayToVector(const std::string& pgArray) {
    std::vector<std::string> result;
    
    if (pgArray.empty() || pgArray == "{}") {
        return result;
    }
    
    std::string cleaned = pgArray;
    if (cleaned[0] == '{') cleaned = cleaned.substr(1);
    if (cleaned.back() == '}') cleaned.pop_back();
    
    std::istringstream iss(cleaned);
    std::string item;
    
    while (std::getline(iss, item, ',')) {
        if (!item.empty() && item[0] == '"') item = item.substr(1);
        if (!item.empty() && item.back() == '"') item.pop_back();
        
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    
    return result;
}

} // namespace Domain::Task

