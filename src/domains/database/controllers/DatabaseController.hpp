#pragma once

#include <memory>
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../use_cases/ExecuteQueryUseCase.hpp"
#include "../../../core/database/ConnectionPool.hpp"

namespace Domain::Database {

class DatabaseController {
public:
    explicit DatabaseController(
        std::shared_ptr<ExecuteQueryUseCase> executeQueryUseCase,
        std::shared_ptr<Core::Database::ConnectionPool> pool
    ) : executeQueryUseCase_(executeQueryUseCase), pool_(pool) {}
    
    // POST /api/admin/database/query - Execute SQL query
    Core::Http::Response executeQuery(const Core::Http::Request& req);
    
    // GET /api/admin/database/tables - List all tables
    Core::Http::Response listTables(const Core::Http::Request& req);
    
    // GET /api/admin/database/tables/:name/schema - Get table schema
    Core::Http::Response getTableSchema(const Core::Http::Request& req);
    
    // GET /api/admin/database/tables/:name/count - Get row count
    Core::Http::Response getTableCount(const Core::Http::Request& req);
    
    // GET /api/admin/database/stats - Get database statistics
    Core::Http::Response getDatabaseStats(const Core::Http::Request& req);
    
private:
    std::shared_ptr<ExecuteQueryUseCase> executeQueryUseCase_;
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
};

} // namespace Domain::Database


