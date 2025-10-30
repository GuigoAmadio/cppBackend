#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <regex>

namespace Domain::Database {

/**
 * @brief Validador de queries SQL para prevenir SQL Injection e comandos perigosos
 * 
 * Características:
 * - Bloqueia comandos DDL destrutivos (DROP, TRUNCATE, ALTER)
 * - Bloqueia comandos de controle (GRANT, REVOKE)
 * - Limita queries apenas a SELECT e comandos seguros
 * - Valida sintaxe básica
 * - Permite whitelisting de tabelas
 */
class QueryValidator {
public:
    struct ValidationResult {
        bool valid;
        std::string error;
        std::string sanitizedQuery;
    };
    
    /**
     * @brief Valida uma query SQL
     * @param query Query a ser validada
     * @param allowedTables Lista de tabelas permitidas (vazio = todas)
     * @param readOnly Se true, apenas SELECT é permitido
     * @return Resultado da validação
     */
    static ValidationResult validate(
        const std::string& query,
        const std::vector<std::string>& allowedTables = {},
        bool readOnly = true
    );
    
private:
    // Comandos bloqueados
    static const std::vector<std::string> DANGEROUS_COMMANDS;
    static const std::vector<std::string> DDL_COMMANDS;
    static const std::vector<std::string> DCL_COMMANDS;
    
    // Helpers
    static bool containsDangerousCommand(const std::string& query);
    static bool containsMultipleStatements(const std::string& query);
    static bool isValidTableName(const std::string& tableName);
    static std::vector<std::string> extractTableNames(const std::string& query);
    static std::string normalizeQuery(const std::string& query);
    static std::string toLowerCase(const std::string& str);
};

} // namespace Domain::Database


