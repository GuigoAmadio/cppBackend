#include "DatabaseController.hpp"
#include "../../../core/utils/Logger.hpp"
#include "../../../core/json/Json.hpp"

namespace Utils {
    using Core::Utils::Logger;
}

using namespace Core::Http;
using namespace Core::Json;

namespace Domain::Database {

Response DatabaseController::executeQuery(const Request& req) {
    try {
        // Parse request
        auto body = req.getJson();
        if (!body || !body->isObject()) {
            auto errJson = makeObject();
            errJson->asObject()["error"] = makeString("Invalid JSON body");
            return Response(StatusCode::BadRequest).json(*errJson);
        }
        
        auto obj = body->asObject();
        
        // Extract fields
        std::string query = obj.count("query") && obj["query"]->isString() 
            ? obj["query"]->asString() : "";
        
        bool readOnly = true;
        if (obj.count("read_only") && obj["read_only"]->isBool()) {
            readOnly = obj["read_only"]->asBool();
        }
        
        // Get user info from middleware
        std::string userId = req.getCustomData("user_id");
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        if (userId.empty()) {
            auto errJson = makeObject();
            errJson->asObject()["error"] = makeString("User ID is required");
            return Response(StatusCode::Unauthorized).json(*errJson);
        }
        
        // Build DTO
        QueryExecutionDTO dto;
        dto.query = query;
        dto.userId = userId;
        dto.tenantId = tenantId;
        dto.readOnly = readOnly;
        
        // Extract allowed tables (if provided)
        if (obj.count("allowed_tables") && obj["allowed_tables"]->isArray()) {
            auto tablesArray = obj["allowed_tables"]->asArray();
            for (const auto& table : tablesArray) {
                if (table->isString()) {
                    dto.allowedTables.push_back(table->asString());
                }
            }
        }
        
        // Execute query
        auto result = executeQueryUseCase_->execute(dto);
        
        if (!result.success) {
            auto errJson = makeObject();
            errJson->asObject()["error"] = makeString(result.error);
            errJson->asObject()["validated_query"] = makeString(result.validatedQuery);
            return Response(StatusCode::BadRequest).json(*errJson);
        }
        
        // Build response
        auto columnsArray = makeArray();
        for (const auto& col : result.columns) {
            columnsArray->asArray().push_back(makeString(col));
        }
        
        auto rowsArray = makeArray();
        for (const auto& row : result.rows) {
            auto rowArray = makeArray();
            for (const auto& cell : row) {
                rowArray->asArray().push_back(makeString(cell));
            }
            rowsArray->asArray().push_back(std::move(rowArray));
        }
        
        auto respJson = makeObject();
        respJson->asObject()["success"] = makeBool(true);
        respJson->asObject()["row_count"] = makeNumber(result.rowCount);
        respJson->asObject()["execution_time_ms"] = makeNumber(result.executionTimeMs);
        respJson->asObject()["columns"] = std::move(columnsArray);
        respJson->asObject()["rows"] = std::move(rowsArray);
        respJson->asObject()["validated_query"] = makeString(result.validatedQuery);
        
        return Response(StatusCode::OK).json(*respJson);
        
    } catch (const std::exception& e) {
        Utils::Logger::error("[DatabaseController::executeQuery] Exception: " + std::string(e.what()));
        auto errJson = makeObject();
        errJson->asObject()["error"] = makeString(std::string("Internal error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*errJson);
    }
}

Response DatabaseController::listTables(const Request& req) {
    try {
        std::string tenantId = req.getCustomData("user_tenant_id");
        
        auto conn = pool_->acquire();
        // PostgreSQL query to list all tables
        std::string query = 
            "SELECT table_name, table_type "
            "FROM information_schema.tables "
            "WHERE table_schema = 'public' "
            "ORDER BY table_name";
        
        auto result = conn->execute(query);
        
        if (!result.isSuccess()) {
            auto errJson = makeObject();
            errJson->asObject()["error"] = makeString("Failed to list tables");
            return Response(StatusCode::InternalServerError).json(*errJson);
        }
        
        auto tablesArray = makeArray();
        for (int i = 0; i < result.rowCount(); i++) {
            auto tableObj = makeObject();
            tableObj->asObject()["name"] = makeString(result.getValue(i, 0));
            tableObj->asObject()["type"] = makeString(result.getValue(i, 1));
            tablesArray->asArray().push_back(std::move(tableObj));
        }
        
        auto respJson = makeObject();
        respJson->asObject()["tables"] = std::move(tablesArray);
        respJson->asObject()["count"] = makeNumber(result.rowCount());
        
        return Response(StatusCode::OK).json(*respJson);
        
    } catch (const std::exception& e) {
        Utils::Logger::error("[DatabaseController::listTables] Exception: " + std::string(e.what()));
        auto errJson = makeObject();
        errJson->asObject()["error"] = makeString(std::string("Internal error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*errJson);
    }
}

Response DatabaseController::getTableSchema(const Request& req) {
    try {
        std::string tableName = req.getParam("name");
        
        if (tableName.empty()) {
            auto errJson = makeObject();
            errJson->asObject()["error"] = makeString("Table name is required");
            return Response(StatusCode::BadRequest).json(*errJson);
        }
        
        auto conn = pool_->acquire();
        // Get table columns
        std::string query = 
            "SELECT column_name, data_type, is_nullable, column_default "
            "FROM information_schema.columns "
            "WHERE table_schema = 'public' AND table_name = '" + tableName + "' "
            "ORDER BY ordinal_position";
        
        auto result = conn->execute(query);
        
        if (!result.isSuccess()) {
            auto errJson = makeObject();
            errJson->asObject()["error"] = makeString("Table not found or failed to get schema");
            return Response(StatusCode::NotFound).json(*errJson);
        }
        
        auto columnsArray = makeArray();
        for (int i = 0; i < result.rowCount(); i++) {
            auto colObj = makeObject();
            colObj->asObject()["name"] = makeString(result.getValue(i, 0));
            colObj->asObject()["type"] = makeString(result.getValue(i, 1));
            colObj->asObject()["nullable"] = makeString(result.getValue(i, 2));
            colObj->asObject()["default"] = makeString(result.getValue(i, 3));
            columnsArray->asArray().push_back(std::move(colObj));
        }
        
        auto respJson = makeObject();
        respJson->asObject()["table"] = makeString(tableName);
        respJson->asObject()["columns"] = std::move(columnsArray);
        respJson->asObject()["column_count"] = makeNumber(result.rowCount());
        
        return Response(StatusCode::OK).json(*respJson);
        
    } catch (const std::exception& e) {
        Utils::Logger::error("[DatabaseController::getTableSchema] Exception: " + std::string(e.what()));
        auto errJson = makeObject();
        errJson->asObject()["error"] = makeString(std::string("Internal error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*errJson);
    }
}

Response DatabaseController::getTableCount(const Request& req) {
    try {
        std::string tableName = req.getParam("name");
        
        if (tableName.empty()) {
            auto errJson = makeObject();
            errJson->asObject()["error"] = makeString("Table name is required");
            return Response(StatusCode::BadRequest).json(*errJson);
        }
        
        auto conn = pool_->acquire();
        // Escapar nome da tabela com aspas duplas para preservar case sensitivity
        std::string query = "SELECT COUNT(*) FROM \"" + tableName + "\"";
        auto result = conn->execute(query);
        
        if (!result.isSuccess() || result.rowCount() == 0) {
            auto errJson = makeObject();
            errJson->asObject()["error"] = makeString("Failed to get row count");
            return Response(StatusCode::InternalServerError).json(*errJson);
        }
        
        int count = std::stoi(result.getValue(0, 0));
        
        auto respJson = makeObject();
        respJson->asObject()["table"] = makeString(tableName);
        respJson->asObject()["row_count"] = makeNumber(count);
        
        return Response(StatusCode::OK).json(*respJson);
        
    } catch (const std::exception& e) {
        Utils::Logger::error("[DatabaseController::getTableCount] Exception: " + std::string(e.what()));
        auto errJson = makeObject();
        errJson->asObject()["error"] = makeString(std::string("Internal error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*errJson);
    }
}

Response DatabaseController::getDatabaseStats(const Request& req) {
    try {
        auto conn = pool_->acquire();
        // Get database size
        std::string dbSizeQuery = "SELECT pg_database_size(current_database())";
        auto dbSizeResult = conn->execute(dbSizeQuery);
        
        long long dbSize = 0;
        if (dbSizeResult.isSuccess() && dbSizeResult.rowCount() > 0) {
            dbSize = std::stoll(dbSizeResult.getValue(0, 0));
        }
        
        // Get table count
        std::string tableCountQuery = 
            "SELECT COUNT(*) FROM information_schema.tables "
            "WHERE table_schema = 'public' AND table_type = 'BASE TABLE'";
        auto tableCountResult = conn->execute(tableCountQuery);
        
        int tableCount = 0;
        if (tableCountResult.isSuccess() && tableCountResult.rowCount() > 0) {
            tableCount = std::stoi(tableCountResult.getValue(0, 0));
        }
        
        // Get total row count (sum of all tables)
        std::string totalRowsQuery = 
            "SELECT SUM(n_live_tup) FROM pg_stat_user_tables";
        auto totalRowsResult = conn->execute(totalRowsQuery);
        
        long long totalRows = 0;
        if (totalRowsResult.isSuccess() && totalRowsResult.rowCount() > 0) {
            std::string val = totalRowsResult.getValue(0, 0);
            if (!val.empty()) {
                totalRows = std::stoll(val);
            }
        }
        
        auto respJson = makeObject();
        respJson->asObject()["database_size_bytes"] = makeNumber(dbSize);
        respJson->asObject()["database_size_mb"] = makeNumber(dbSize / (1024.0 * 1024.0));
        respJson->asObject()["table_count"] = makeNumber(tableCount);
        respJson->asObject()["total_rows"] = makeNumber(totalRows);
        
        return Response(StatusCode::OK).json(*respJson);
        
    } catch (const std::exception& e) {
        Utils::Logger::error("[DatabaseController::getDatabaseStats] Exception: " + std::string(e.what()));
        auto errJson = makeObject();
        errJson->asObject()["error"] = makeString(std::string("Internal error: ") + e.what());
        return Response(StatusCode::InternalServerError).json(*errJson);
    }
}

} // namespace Domain::Database


