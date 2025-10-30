#pragma once

#include "../TaskRepository.hpp"
#include "../../../../core/database/ConnectionPool.hpp"
#include <memory>

namespace Domain::Task {

class TaskRepositoryImpl : public TaskRepository {
public:
    explicit TaskRepositoryImpl(std::shared_ptr<Core::Database::ConnectionPool> pool);
    virtual ~TaskRepositoryImpl() = default;

    // Task CRUD
    bool save(const Task& task) override;
    bool update(const Task& task) override;
    bool remove(const std::string& taskId, const std::string& tenantId) override;
    std::optional<Task> findById(const std::string& taskId, const std::string& tenantId) override;
    std::vector<Task> findByTenant(const std::string& tenantId) override;
    std::vector<Task> findByWorkspace(const std::string& workspaceId, const std::string& tenantId) override;
    std::vector<Task> findByProject(const std::string& projectId, const std::string& tenantId) override;
    std::vector<Task> findByAssignee(const std::string& userId, const std::string& tenantId) override;
    std::vector<Task> findByCreator(const std::string& userId, const std::string& tenantId) override;
    std::vector<Task> findByStatus(const std::string& status, const std::string& tenantId) override;
    std::vector<Task> findByParent(const std::string& parentId, const std::string& tenantId) override;
    
    // Comments
    bool saveComment(const TaskComment& comment) override;
    bool updateComment(const TaskComment& comment) override;
    bool removeComment(const std::string& commentId, const std::string& tenantId) override;
    std::optional<TaskComment> findCommentById(const std::string& commentId, const std::string& tenantId) override;
    std::vector<TaskComment> findCommentsByTask(const std::string& taskId, const std::string& tenantId) override;
    
    // Checklists
    bool saveChecklist(const Checklist& checklist) override;
    bool updateChecklist(const Checklist& checklist) override;
    bool removeChecklist(const std::string& checklistId, const std::string& tenantId) override;
    std::vector<Checklist> findChecklistsByTask(const std::string& taskId, const std::string& tenantId) override;
    
    // Checklist Items
    bool saveChecklistItem(const ChecklistItem& item) override;
    bool updateChecklistItem(const ChecklistItem& item) override;
    bool removeChecklistItem(const std::string& itemId, const std::string& tenantId) override;
    std::vector<ChecklistItem> findChecklistItems(const std::string& checklistId, const std::string& tenantId) override;

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;

    // Helper methods
    Task mapToTask(const Core::Database::QueryResult& result, int row);
    std::vector<Task> mapToTasks(const Core::Database::QueryResult& result);
    TaskComment mapToComment(const Core::Database::QueryResult& result, int row);
    std::vector<TaskComment> mapToComments(const Core::Database::QueryResult& result);
    Checklist mapToChecklist(const Core::Database::QueryResult& result, int row);
    std::vector<Checklist> mapToChecklists(const Core::Database::QueryResult& result);
    ChecklistItem mapToChecklistItem(const Core::Database::QueryResult& result, int row);
    std::vector<ChecklistItem> mapToChecklistItems(const Core::Database::QueryResult& result);
    
    std::string arrayToPostgresArray(const std::vector<std::string>& vec);
    std::vector<std::string> postgresArrayToVector(const std::string& pgArray);
};

} // namespace Domain::Task

