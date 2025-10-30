#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../../../core/database/ConnectionPool.hpp"
#include "../validators/QueryValidator.hpp"
#include "../../../core/utils/Logger.hpp"

namespace Utils {
    using Core::Utils::Logger;
}

namespace Domain::Database {

struct QueryExecutionDTO {
    std::string query;
    std::string userId;          // Who is executing
    std::string tenantId;        // Which tenant
    bool readOnly = true;        // Default: only SELECT
    std::vector<std::string> allowedTables;  // Empty = all tables
};

struct QueryExecutionResult {
    bool success;
    std::string error;
    int rowCount;
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> rows;
    double executionTimeMs;
    std::string validatedQuery;
};

class ExecuteQueryUseCase {
public:
    explicit ExecuteQueryUseCase(std::shared_ptr<Core::Database::ConnectionPool> pool)
        : pool_(pool) {}
    
    QueryExecutionResult execute(const QueryExecutionDTO& dto) {
        QueryExecutionResult result;
        result.success = false;
        result.rowCount = 0;
        result.executionTimeMs = 0;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        try {
            // 1. Validate query
            Utils::Logger::info("[ExecuteQueryUseCase] Validating query for user: " + dto.userId);
            
            auto validation = QueryValidator::validate(dto.query, dto.allowedTables, dto.readOnly);
            
            if (!validation.valid) {
                result.error = "Query validation failed: " + validation.error;
                Utils::Logger::error("[ExecuteQueryUseCase] " + result.error);
                return result;
            }
            
            result.validatedQuery = validation.sanitizedQuery;
            
            // 2. Get connection from pool
            auto conn = pool_->acquire();
            
            // 3. Execute query
            Utils::Logger::info("[ExecuteQueryUseCase] Executing query: " + result.validatedQuery.substr(0, 100) + "...");
            
            auto queryResult = conn->execute(result.validatedQuery);
            
            if (!queryResult.isSuccess()) {
                result.error = "Query execution failed";
                Utils::Logger::error("[ExecuteQueryUseCase] " + result.error);
                return result;
            }
            
            // 4. Extract results
            result.rowCount = queryResult.rowCount();
            
            // Get column names
            int colCount = queryResult.columnCount();
            for (int i = 0; i < colCount; i++) {
                result.columns.push_back(queryResult.getColumnName(i));
            }
            
            // Get rows
            for (int row = 0; row < result.rowCount && row < 1000; row++) { // Limit to 1000 rows
                std::vector<std::string> rowData;
                for (int col = 0; col < colCount; col++) {
                    rowData.push_back(queryResult.getValue(row, col));
                }
                result.rows.push_back(rowData);
            }
            
            // 5. Calculate execution time
            auto endTime = std::chrono::high_resolution_clock::now();
            result.executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
            
            result.success = true;
            Utils::Logger::info("[ExecuteQueryUseCase] Query executed successfully. Rows: " + 
                               std::to_string(result.rowCount) + ", Time: " + 
                               std::to_string(result.executionTimeMs) + "ms");
            
        } catch (const std::exception& e) {
            result.error = std::string("Exception: ") + e.what();
            Utils::Logger::error("[ExecuteQueryUseCase] " + result.error);
        }
        
        return result;
    }
    
private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
};

} // namespace Domain::Database


