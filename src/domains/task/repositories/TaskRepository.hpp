#pragma once

#include <memory>
#include <vector>
#include <optional>
#include "../entities/Task.hpp"
#include "../entities/TaskComment.hpp"
#include "../entities/Checklist.hpp"
#include "../entities/ChecklistItem.hpp"

namespace Domain::Task {

class TaskRepository {
public:
    virtual ~TaskRepository() = default;

    // Task CRUD
    virtual bool save(const Task& task) = 0;
    virtual bool update(const Task& task) = 0;
    virtual bool remove(const std::string& taskId, const std::string& tenantId) = 0;
    virtual std::optional<Task> findById(const std::string& taskId, const std::string& tenantId) = 0;
    virtual std::vector<Task> findByTenant(const std::string& tenantId) = 0;
    virtual std::vector<Task> findByWorkspace(const std::string& workspaceId, const std::string& tenantId) = 0;
    virtual std::vector<Task> findByProject(const std::string& projectId, const std::string& tenantId) = 0;
    virtual std::vector<Task> findByAssignee(const std::string& userId, const std::string& tenantId) = 0;
    virtual std::vector<Task> findByCreator(const std::string& userId, const std::string& tenantId) = 0;
    virtual std::vector<Task> findByStatus(const std::string& status, const std::string& tenantId) = 0;
    virtual std::vector<Task> findByParent(const std::string& parentId, const std::string& tenantId) = 0;
    
    // Comments
    virtual bool saveComment(const TaskComment& comment) = 0;
    virtual bool updateComment(const TaskComment& comment) = 0;
    virtual bool removeComment(const std::string& commentId, const std::string& tenantId) = 0;
    virtual std::optional<TaskComment> findCommentById(const std::string& commentId, const std::string& tenantId) = 0;
    virtual std::vector<TaskComment> findCommentsByTask(const std::string& taskId, const std::string& tenantId) = 0;
    
    // Checklists
    virtual bool saveChecklist(const Checklist& checklist) = 0;
    virtual bool updateChecklist(const Checklist& checklist) = 0;
    virtual bool removeChecklist(const std::string& checklistId, const std::string& tenantId) = 0;
    virtual std::vector<Checklist> findChecklistsByTask(const std::string& taskId, const std::string& tenantId) = 0;
    
    // Checklist Items
    virtual bool saveChecklistItem(const ChecklistItem& item) = 0;
    virtual bool updateChecklistItem(const ChecklistItem& item) = 0;
    virtual bool removeChecklistItem(const std::string& itemId, const std::string& tenantId) = 0;
    virtual std::vector<ChecklistItem> findChecklistItems(const std::string& checklistId, const std::string& tenantId) = 0;
};

} // namespace Domain::Task

