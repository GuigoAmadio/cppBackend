#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <libpq-fe.h>

namespace Core::Database {

/**
 * @brief Representa o resultado de uma query SQL.
 */
class QueryResult {
public:
    explicit QueryResult(PGresult* result);
    ~QueryResult();
    
    // Desabilitar cópia
    QueryResult(const QueryResult&) = delete;
    QueryResult& operator=(const QueryResult&) = delete;
    
    // Permitir movimentação
    QueryResult(QueryResult&& other) noexcept;
    QueryResult& operator=(QueryResult&& other) noexcept;
    
    int rowCount() const;
    int columnCount() const;
    std::string getValue(int row, int col) const;
    std::string getColumnName(int col) const;
    bool isNull(int row, int col) const;
    bool isSuccess() const;
    std::string getError() const;

private:
    PGresult* result_;
};

/**
 * @brief Representa uma conexão com o banco PostgreSQL.
 *
 * OBS: Para conectar na porta correta do seu banco de dados,
 * adicione o parâmetro "port" na conexão, por exemplo:
 *
 *   "host=localhost port=5432 dbname=mydb user=postgres password=pass"
 *
 * Troque 5432 para a porta que seu banco está rodando!
 */
class Connection {
public:
    /**
     * @brief Conecta ao banco de dados.
     * @param connectionString String de conexão PostgreSQL 
     *        (ex: "host=localhost port=5432 dbname=mydb user=postgres password=pass")
     * 
     * IMPORTANTE: Para definir a porta, inclua "port=PORTA" na string.
     */
    explicit Connection(const std::string& connectionString);
    ~Connection();
    
    // Desabilitar cópia
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    
    bool isConnected() const;
    QueryResult execute(const std::string& sql);
    QueryResult executeParams(const std::string& sql, const std::vector<std::string>& params);
    bool beginTransaction();
    bool commit();
    bool rollback();
    std::string escape(const std::string& str) const;

private:
    PGconn* connection_;
    std::string connectionString_;
};

} // namespace Core::Database

