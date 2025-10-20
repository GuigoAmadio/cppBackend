#include "Connection.hpp"
#include "../utils/Logger.hpp"
#include <stdexcept>
#include <cstring>

namespace Core::Database {

// ==================== QueryResult ====================

QueryResult::QueryResult(PGresult* result) 
    : result_(result) {}

QueryResult::~QueryResult() {
    if (result_) {
        PQclear(result_);
    }
}

QueryResult::QueryResult(QueryResult&& other) noexcept
    : result_(other.result_) {
    other.result_ = nullptr;
}

QueryResult& QueryResult::operator=(QueryResult&& other) noexcept {
    if (this != &other) {
        if (result_) {
            PQclear(result_);
        }
        result_ = other.result_;
        other.result_ = nullptr;
    }
    return *this;
}

int QueryResult::rowCount() const {
    if (!result_) return 0;
    return PQntuples(result_);
}

int QueryResult::columnCount() const {
    if (!result_) return 0;
    return PQnfields(result_);
}

std::string QueryResult::getValue(int row, int col) const {
    if (!result_ || isNull(row, col)) {
        return "";
    }
    const char* value = PQgetvalue(result_, row, col);
    return value ? std::string(value) : "";
}

std::string QueryResult::getColumnName(int col) const {
    if (!result_) return "";
    const char* name = PQfname(result_, col);
    return name ? std::string(name) : "";
}

bool QueryResult::isNull(int row, int col) const {
    if (!result_) return true;
    return PQgetisnull(result_, row, col) == 1;
}

bool QueryResult::isSuccess() const {
    if (!result_) return false;
    ExecStatusType status = PQresultStatus(result_);
    return (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK);
}

std::string QueryResult::getError() const {
    if (!result_) return "No result";
    const char* error = PQresultErrorMessage(result_);
    return error ? std::string(error) : "";
}

// ==================== Connection ====================

Connection::Connection(const std::string& connectionString)
    : connection_(nullptr), connectionString_(connectionString) {
    
    connection_ = PQconnectdb(connectionString.c_str());
    
    if (PQstatus(connection_) != CONNECTION_OK) {
        std::string error = PQerrorMessage(connection_);
        PQfinish(connection_);
        connection_ = nullptr;
        throw std::runtime_error("Failed to connect to database: " + error);
    }
    
    Utils::Logger::info("🐘 PostgreSQL conectado com sucesso");
}

Connection::~Connection() {
    if (connection_) {
        PQfinish(connection_);
        connection_ = nullptr;
    }
}

bool Connection::isConnected() const {
    return connection_ && (PQstatus(connection_) == CONNECTION_OK);
}

QueryResult Connection::execute(const std::string& sql) {
    if (!isConnected()) {
        throw std::runtime_error("Database connection is not active");
    }
    
    PGresult* result = PQexec(connection_, sql.c_str());
    return QueryResult(result);
}

QueryResult Connection::executeParams(const std::string& sql, const std::vector<std::string>& params) {
    if (!isConnected()) {
        throw std::runtime_error("Database connection is not active");
    }
    
    // Preparar parâmetros para libpq
    std::vector<const char*> paramValues;
    paramValues.reserve(params.size());
    
    for (const auto& param : params) {
        paramValues.push_back(param.c_str());
    }
    
    PGresult* result = PQexecParams(
        connection_,
        sql.c_str(),
        static_cast<int>(params.size()),
        nullptr,  // paramTypes (NULL = infer types)
        paramValues.data(),
        nullptr,  // paramLengths (NULL = text format)
        nullptr,  // paramFormats (NULL = text format)
        0         // resultFormat (0 = text)
    );
    
    return QueryResult(result);
}

bool Connection::beginTransaction() {
    auto result = execute("BEGIN");
    return result.isSuccess();
}

bool Connection::commit() {
    auto result = execute("COMMIT");
    return result.isSuccess();
}

bool Connection::rollback() {
    auto result = execute("ROLLBACK");
    return result.isSuccess();
}

std::string Connection::escape(const std::string& str) const {
    if (!connection_) return str;
    
    // Alocar buffer (precisa de 2x tamanho + 1)
    std::vector<char> buffer(str.length() * 2 + 1);
    
    int error = 0;
    size_t len = PQescapeStringConn(
        connection_,
        buffer.data(),
        str.c_str(),
        str.length(),
        &error
    );
    
    if (error) {
        throw std::runtime_error("Failed to escape string");
    }
    
    return std::string(buffer.data(), len);
}

} // namespace Core::Database
