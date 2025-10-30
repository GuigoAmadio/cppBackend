#include "TaskRepositoryImpl.hpp"
#include "../../../../core/utils/Logger.hpp"
#include <sstream>

namespace Utils = Core::Utils;

namespace Domain::Task {

TaskRepositoryImpl::TaskRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {
    Utils::Logger::info("[TaskRepositoryImpl] Initialized COMPLETE VERSION");
}

// =========================================================
// TASK CRUD METHODS
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

bool TaskRepositoryImpl::update(const Task& task) {
    Utils::Logger::info("[TaskRepositoryImpl] update() - ID: " + task.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            UPDATE tasks SET
                title = $3, description = NULLIF($4,''), task_type = $5, status = $6,
                priority = $7, assigned_to = NULLIF($8,''), reporter = NULLIF($9,''),
                estimated_hours = $10, actual_hours = $11, story_points = $12,
                start_date = NULLIF($13,'')::timestamp, due_date = NULLIF($14,'')::timestamp,
                completed_at = NULLIF($15,'')::timestamp, parent_task_id = NULLIF($16,''),
                depends_on = $17, blocks = $18, tags = $19, labels = $20,
                position = $21, is_archived = $22, workspace_id = NULLIF($23,''),
                project_id = NULLIF($24,'')
            WHERE id = $1 AND tenant_id = $2
        )";

        std::vector<std::string> params;
        params.push_back(task.getId());
        params.push_back(task.getTenantId());
        params.push_back(task.getTitle());
        params.push_back(task.getDescription().value_or(""));
        params.push_back(task.getTaskType().toString());
        params.push_back(task.getStatus().toString());
        params.push_back(task.getPriority().toString());
        params.push_back(task.getAssignedTo().value_or(""));
        params.push_back(task.getReporter().value_or(""));
        params.push_back(std::to_string(task.getEstimatedHours()));
        params.push_back(std::to_string(task.getActualHours()));
        params.push_back(std::to_string(task.getStoryPoints()));
        params.push_back(task.getStartDate().value_or(""));
        params.push_back(task.getDueDate().value_or(""));
        params.push_back(task.getCompletedAt().value_or(""));
        params.push_back(task.getParentTaskId().value_or(""));
        params.push_back(arrayToPostgresArray(task.getDependsOn()));
        params.push_back(arrayToPostgresArray(task.getBlocks()));
        params.push_back(arrayToPostgresArray(task.getTags()));
        params.push_back(arrayToPostgresArray(task.getLabels()));
        params.push_back(std::to_string(task.getPosition()));
        params.push_back(task.isArchived() ? "true" : "false");
        params.push_back(task.getWorkspaceId().value_or(""));
        params.push_back(task.getProjectId().value_or(""));

        auto result = conn->executeParams(query, params);
        
        if (result.isSuccess()) {
            Utils::Logger::info("[TaskRepositoryImpl] Task updated successfully");
            return true;
        }
        
        Utils::Logger::error("[TaskRepositoryImpl] Failed to update task");
        return false;

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] update() exception: " + std::string(e.what()));
        return false;
    }
}

bool TaskRepositoryImpl::remove(const std::string& taskId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] remove() - ID: " + taskId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "DELETE FROM tasks WHERE id = $1 AND tenant_id = $2";
        
        std::vector<std::string> params;
        params.push_back(taskId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return result.isSuccess();

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] remove() exception: " + std::string(e.what()));
        return false;
    }
}

std::optional<Task> TaskRepositoryImpl::findById(const std::string& taskId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] findById() - ID: " + taskId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM tasks WHERE id = $1 AND tenant_id = $2 AND is_archived = false";
        
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
// FIND METHODS (FILTERS)
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

std::vector<Task> TaskRepositoryImpl::findByWorkspace(const std::string& workspaceId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] findByWorkspace() - workspace: " + workspaceId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM tasks WHERE workspace_id = $1 AND tenant_id = $2 AND is_archived = false ORDER BY position, created_at DESC";
        
        std::vector<std::string> params;
        params.push_back(workspaceId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToTasks(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] findByWorkspace() exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Task> TaskRepositoryImpl::findByProject(const std::string& projectId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] findByProject() - project: " + projectId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM tasks WHERE project_id = $1 AND tenant_id = $2 AND is_archived = false ORDER BY position, created_at DESC";
        
        std::vector<std::string> params;
        params.push_back(projectId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToTasks(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] findByProject() exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Task> TaskRepositoryImpl::findByAssignee(const std::string& assigneeId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] findByAssignee() - assignee: " + assigneeId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM tasks WHERE assigned_to = $1 AND tenant_id = $2 AND is_archived = false ORDER BY priority DESC, due_date ASC";
        
        std::vector<std::string> params;
        params.push_back(assigneeId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToTasks(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] findByAssignee() exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Task> TaskRepositoryImpl::findByCreator(const std::string& creatorId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] findByCreator() - creator: " + creatorId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM tasks WHERE created_by = $1 AND tenant_id = $2 AND is_archived = false ORDER BY created_at DESC";
        
        std::vector<std::string> params;
        params.push_back(creatorId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToTasks(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] findByCreator() exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Task> TaskRepositoryImpl::findByStatus(const std::string& status, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] findByStatus() - status: " + status);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM tasks WHERE status = $1 AND tenant_id = $2 AND is_archived = false ORDER BY priority DESC, created_at DESC";
        
        std::vector<std::string> params;
        params.push_back(status);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToTasks(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] findByStatus() exception: " + std::string(e.what()));
        return {};
    }
}

std::vector<Task> TaskRepositoryImpl::findByParent(const std::string& parentId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] findByParent() - parent: " + parentId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM tasks WHERE parent_task_id = $1 AND tenant_id = $2 AND is_archived = false ORDER BY position, created_at DESC";
        
        std::vector<std::string> params;
        params.push_back(parentId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToTasks(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] findByParent() exception: " + std::string(e.what()));
        return {};
    }
}

// =========================================================
// COMMENT METHODS
// =========================================================

bool TaskRepositoryImpl::saveComment(const TaskComment& comment) {
    Utils::Logger::info("[TaskRepositoryImpl] saveComment() - task: " + comment.getTaskId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            INSERT INTO task_comments (
                id, task_id, tenant_id, content, created_by, parent_comment_id
            ) VALUES (
                $1, $2, $3, $4, $5, NULLIF($6,'')
            )
        )";

        std::vector<std::string> params;
        params.push_back(comment.getId());           // $1
        params.push_back(comment.getTaskId());        // $2
        params.push_back(comment.getTenantId());      // $3
        params.push_back(comment.getContent());       // $4 (corrigido: content antes de created_by)
        params.push_back(comment.getCreatedBy());     // $5
        params.push_back(comment.getParentCommentId().value_or(""));  // $6

        auto result = conn->executeParams(query, params);
        
        if (result.isSuccess()) {
            Utils::Logger::info("[TaskRepositoryImpl] Comment saved successfully");
            return true;
        }
        
        return false;

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] saveComment() exception: " + std::string(e.what()));
        return false;
    }
}

bool TaskRepositoryImpl::updateComment(const TaskComment& comment) {
    Utils::Logger::info("[TaskRepositoryImpl] updateComment() - ID: " + comment.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            UPDATE task_comments SET
                content = $3, is_edited = true, edited_at = NOW()
            WHERE id = $1 AND tenant_id = $2
        )";

        std::vector<std::string> params;
        params.push_back(comment.getId());
        params.push_back(comment.getTenantId());
        params.push_back(comment.getContent());

        auto result = conn->executeParams(query, params);
        return result.isSuccess();

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] updateComment() exception: " + std::string(e.what()));
        return false;
    }
}

bool TaskRepositoryImpl::removeComment(const std::string& commentId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] removeComment() - ID: " + commentId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "UPDATE task_comments SET is_deleted = true, deleted_at = NOW() WHERE id = $1 AND tenant_id = $2";
        
        std::vector<std::string> params;
        params.push_back(commentId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return result.isSuccess();

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] removeComment() exception: " + std::string(e.what()));
        return false;
    }
}

std::optional<TaskComment> TaskRepositoryImpl::findCommentById(const std::string& commentId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] findCommentById() - ID: " + commentId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM task_comments WHERE id = $1 AND tenant_id = $2 AND is_deleted = false";
        
        std::vector<std::string> params;
        params.push_back(commentId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        
        if (result.isSuccess() && result.rowCount() > 0) {
            return mapToComment(result, 0);
        }
        
        return std::nullopt;

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] findCommentById() exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<TaskComment> TaskRepositoryImpl::findCommentsByTask(const std::string& taskId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] findCommentsByTask() - task: " + taskId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM task_comments WHERE task_id = $1 AND tenant_id = $2 AND is_deleted = false ORDER BY created_at ASC";
        
        std::vector<std::string> params;
        params.push_back(taskId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToComments(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] findCommentsByTask() exception: " + std::string(e.what()));
        return {};
    }
}

// =========================================================
// CHECKLIST METHODS
// =========================================================

bool TaskRepositoryImpl::saveChecklist(const Checklist& checklist) {
    Utils::Logger::info("[TaskRepositoryImpl] saveChecklist() - task: " + checklist.getTaskId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            INSERT INTO task_checklists (
                id, task_id, tenant_id, title, position
            ) VALUES (
                $1, $2, $3, $4, $5
            )
        )";

        std::vector<std::string> params;
        params.push_back(checklist.getId());
        params.push_back(checklist.getTaskId());
        params.push_back(checklist.getTenantId());
        params.push_back(checklist.getTitle());
        params.push_back(std::to_string(checklist.getPosition()));

        auto result = conn->executeParams(query, params);
        
        if (result.isSuccess()) {
            Utils::Logger::info("[TaskRepositoryImpl] Checklist saved successfully");
            return true;
        }
        
        return false;

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] saveChecklist() exception: " + std::string(e.what()));
        return false;
    }
}

bool TaskRepositoryImpl::updateChecklist(const Checklist& checklist) {
    Utils::Logger::info("[TaskRepositoryImpl] updateChecklist() - ID: " + checklist.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            UPDATE task_checklists SET
                title = $3, position = $4
            WHERE id = $1 AND tenant_id = $2
        )";

        std::vector<std::string> params;
        params.push_back(checklist.getId());
        params.push_back(checklist.getTenantId());
        params.push_back(checklist.getTitle());
        params.push_back(std::to_string(checklist.getPosition()));

        auto result = conn->executeParams(query, params);
        return result.isSuccess();

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] updateChecklist() exception: " + std::string(e.what()));
        return false;
    }
}

bool TaskRepositoryImpl::removeChecklist(const std::string& checklistId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] removeChecklist() - ID: " + checklistId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "DELETE FROM task_checklists WHERE id = $1 AND tenant_id = $2";
        
        std::vector<std::string> params;
        params.push_back(checklistId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return result.isSuccess();

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] removeChecklist() exception: " + std::string(e.what()));
        return false;
    }
}

std::vector<Checklist> TaskRepositoryImpl::findChecklistsByTask(const std::string& taskId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] findChecklistsByTask() - task: " + taskId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM task_checklists WHERE task_id = $1 AND tenant_id = $2 ORDER BY position ASC";
        
        std::vector<std::string> params;
        params.push_back(taskId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToChecklists(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] findChecklistsByTask() exception: " + std::string(e.what()));
        return {};
    }
}

// =========================================================
// CHECKLIST ITEM METHODS
// =========================================================

bool TaskRepositoryImpl::saveChecklistItem(const ChecklistItem& item) {
    Utils::Logger::info("[TaskRepositoryImpl] saveChecklistItem() - checklist: " + item.getChecklistId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            INSERT INTO checklist_items (
                id, checklist_id, tenant_id, content, is_completed, position
            ) VALUES (
                $1, $2, $3, $4, $5, $6
            )
        )";

        std::vector<std::string> params;
        params.push_back(item.getId());
        params.push_back(item.getChecklistId());
        params.push_back(item.getTenantId());
        params.push_back(item.getContent());
        params.push_back(item.isCompleted() ? "true" : "false");
        params.push_back(std::to_string(item.getPosition()));

        auto result = conn->executeParams(query, params);
        
        if (result.isSuccess()) {
            Utils::Logger::info("[TaskRepositoryImpl] Checklist item saved successfully");
            return true;
        }
        
        return false;

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] saveChecklistItem() exception: " + std::string(e.what()));
        return false;
    }
}

bool TaskRepositoryImpl::updateChecklistItem(const ChecklistItem& item) {
    Utils::Logger::info("[TaskRepositoryImpl] updateChecklistItem() - ID: " + item.getId());
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = R"(
            UPDATE checklist_items SET
                content = $3, is_completed = $4, position = $5,
                completed_by = NULLIF($6,''), completed_at = NULLIF($7,'')::timestamp
            WHERE id = $1 AND tenant_id = $2
        )";

        std::vector<std::string> params;
        params.push_back(item.getId());
        params.push_back(item.getTenantId());
        params.push_back(item.getContent());
        params.push_back(item.isCompleted() ? "true" : "false");
        params.push_back(std::to_string(item.getPosition()));
        params.push_back(item.getCompletedBy().value_or(""));
        params.push_back(item.getCompletedAt().value_or(""));

        auto result = conn->executeParams(query, params);
        return result.isSuccess();

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] updateChecklistItem() exception: " + std::string(e.what()));
        return false;
    }
}

bool TaskRepositoryImpl::removeChecklistItem(const std::string& itemId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] removeChecklistItem() - ID: " + itemId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "DELETE FROM checklist_items WHERE id = $1 AND tenant_id = $2";
        
        std::vector<std::string> params;
        params.push_back(itemId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return result.isSuccess();

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] removeChecklistItem() exception: " + std::string(e.what()));
        return false;
    }
}

std::vector<ChecklistItem> TaskRepositoryImpl::findChecklistItems(const std::string& checklistId, const std::string& tenantId) {
    Utils::Logger::info("[TaskRepositoryImpl] findChecklistItems() - checklist: " + checklistId);
    
    try {
        auto conn = pool_->acquire();
        
        std::string query = "SELECT * FROM checklist_items WHERE checklist_id = $1 AND tenant_id = $2 ORDER BY position ASC";
        
        std::vector<std::string> params;
        params.push_back(checklistId);
        params.push_back(tenantId);
        
        auto result = conn->executeParams(query, params);
        return mapToChecklistItems(result);

    } catch (const std::exception& e) {
        Utils::Logger::error("[TaskRepositoryImpl] findChecklistItems() exception: " + std::string(e.what()));
        return {};
    }
}

// =========================================================
// MAPPING METHODS
// =========================================================

Task TaskRepositoryImpl::mapToTask(const Core::Database::QueryResult& result, int row) {
    // Column positions (0-indexed):
    // 0=id, 1=tenant_id, 2=workspace_id, 3=project_id, 4=title, 5=description
    // 6=task_type, 7=status, 8=priority, 9=created_by, 10=assigned_to, 11=reporter
    
    std::string id = result.getValue(row, 0);
    std::string tenantId = result.getValue(row, 1);
    std::string title = result.getValue(row, 4);           // Corrigido: 6 -> 4
    std::string taskTypeStr = result.getValue(row, 6);      // Corrigido: 8 -> 6
    std::string statusStr = result.getValue(row, 7);        // Corrigido: 9 -> 7
    std::string priorityStr = result.getValue(row, 8);      // Corrigido: 10 -> 8
    std::string createdBy = result.getValue(row, 9);        // Corrigido: 11 -> 9

    Task task(id, tenantId, title, TaskType(taskTypeStr), TaskStatus(statusStr), TaskPriority(priorityStr), createdBy);

    // Set optional fields if present
    std::string workspaceId = result.getValue(row, 2);
    if (!workspaceId.empty()) task.setWorkspaceId(workspaceId);

    std::string projectId = result.getValue(row, 3);
    if (!projectId.empty()) task.setProjectId(projectId);

    std::string description = result.getValue(row, 5);      // Corrigido: 7 -> 5
    if (!description.empty()) task.setDescription(description);

    std::string assignedTo = result.getValue(row, 10);      // Corrigido: 12 -> 10
    if (!assignedTo.empty()) task.setAssignedTo(assignedTo);

    std::string reporter = result.getValue(row, 11);        // Corrigido: 13 -> 11
    if (!reporter.empty()) task.setReporter(reporter);

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

TaskComment TaskRepositoryImpl::mapToComment(const Core::Database::QueryResult& result, int row) {
    // Column positions: 0=id, 1=task_id, 2=tenant_id, 3=content, 4=created_by, 5=parent_comment_id
    std::string id = result.getValue(row, 0);
    std::string taskId = result.getValue(row, 1);
    std::string tenantId = result.getValue(row, 2);
    std::string content = result.getValue(row, 3);       // Corrigido: 4 -> 3
    std::string createdBy = result.getValue(row, 4);     // Corrigido: 3 -> 4

    TaskComment comment(id, taskId, tenantId, content, createdBy);

    std::string parentCommentId = result.getValue(row, 5);
    if (!parentCommentId.empty()) {
        comment.setParentCommentId(parentCommentId);
    }

    return comment;
}

std::vector<TaskComment> TaskRepositoryImpl::mapToComments(const Core::Database::QueryResult& result) {
    std::vector<TaskComment> comments;
    
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); i++) {
            comments.push_back(mapToComment(result, i));
        }
    }
    
    return comments;
}

Checklist TaskRepositoryImpl::mapToChecklist(const Core::Database::QueryResult& result, int row) {
    // Column positions: 0=id, 1=task_id, 2=tenant_id, 3=title, 4=description, 5=position
    std::string id = result.getValue(row, 0);
    std::string taskId = result.getValue(row, 1);
    std::string tenantId = result.getValue(row, 2);
    std::string title = result.getValue(row, 3);
    int position = std::stoi(result.getValue(row, 5));  // Corrigido: 4 -> 5

    Checklist checklist(id, taskId, tenantId, title);
    checklist.setPosition(position);
    return checklist;
}

std::vector<Checklist> TaskRepositoryImpl::mapToChecklists(const Core::Database::QueryResult& result) {
    std::vector<Checklist> checklists;
    
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); i++) {
            checklists.push_back(mapToChecklist(result, i));
        }
    }
    
    return checklists;
}

ChecklistItem TaskRepositoryImpl::mapToChecklistItem(const Core::Database::QueryResult& result, int row) {
    std::string id = result.getValue(row, 0);
    std::string checklistId = result.getValue(row, 1);
    std::string tenantId = result.getValue(row, 2);
    std::string content = result.getValue(row, 3);
    bool isCompleted = result.getValue(row, 4) == "t" || result.getValue(row, 4) == "true";
    int position = std::stoi(result.getValue(row, 5));

    ChecklistItem item(id, checklistId, tenantId, content);
    item.setPosition(position);
    
    if (isCompleted) {
        std::string completedBy = result.getValue(row, 6);
        std::string completedAt = result.getValue(row, 7);
        if (!completedBy.empty()) {
            item.complete(completedBy, completedAt);
        }
    }

    return item;
}

std::vector<ChecklistItem> TaskRepositoryImpl::mapToChecklistItems(const Core::Database::QueryResult& result) {
    std::vector<ChecklistItem> items;
    
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); i++) {
            items.push_back(mapToChecklistItem(result, i));
        }
    }
    
    return items;
}

// =========================================================
// HELPER METHODS
// =========================================================

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
