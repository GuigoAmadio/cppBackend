#include "QueryValidator.hpp"
#include <sstream>
#include <cctype>

namespace Domain::Database {

// Comandos perigosos que devem ser sempre bloqueados
const std::vector<std::string> QueryValidator::DANGEROUS_COMMANDS = {
    "DROP", "DELETE", "TRUNCATE", "ALTER", "CREATE", "RENAME",
    "INSERT", "UPDATE", "REPLACE", "MERGE"
};

// Comandos DDL (Data Definition Language)
const std::vector<std::string> QueryValidator::DDL_COMMANDS = {
    "CREATE", "ALTER", "DROP", "TRUNCATE", "RENAME"
};

// Comandos DCL (Data Control Language)
const std::vector<std::string> QueryValidator::DCL_COMMANDS = {
    "GRANT", "REVOKE", "DENY"
};

std::string QueryValidator::toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return result;
}

std::string QueryValidator::normalizeQuery(const std::string& query) {
    std::string normalized = query;
    
    // Remove leading/trailing whitespace
    auto start = normalized.find_first_not_of(" \t\n\r");
    auto end = normalized.find_last_not_of(" \t\n\r");
    
    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }
    
    normalized = normalized.substr(start, end - start + 1);
    
    // Remove comments (-- style)
    size_t commentPos = normalized.find("--");
    if (commentPos != std::string::npos) {
        normalized = normalized.substr(0, commentPos);
    }
    
    // Remove /* */ style comments
    size_t blockCommentStart = normalized.find("/*");
    while (blockCommentStart != std::string::npos) {
        size_t blockCommentEnd = normalized.find("*/", blockCommentStart);
        if (blockCommentEnd != std::string::npos) {
            normalized.erase(blockCommentStart, blockCommentEnd - blockCommentStart + 2);
        } else {
            break; // Unclosed comment, will be caught by validation
        }
        blockCommentStart = normalized.find("/*");
    }
    
    return normalized;
}

bool QueryValidator::containsMultipleStatements(const std::string& query) {
    // Check for semicolons (indicating multiple statements)
    size_t semicolonCount = 0;
    bool inString = false;
    
    for (char c : query) {
        if (c == '\'') {
            inString = !inString;
        } else if (c == ';' && !inString) {
            semicolonCount++;
        }
    }
    
    return semicolonCount > 1; // Allow one trailing semicolon
}

bool QueryValidator::containsDangerousCommand(const std::string& query) {
    std::string lowerQuery = toLowerCase(query);
    
    // Check for dangerous keywords
    for (const auto& cmd : DANGEROUS_COMMANDS) {
        std::string lowerCmd = toLowerCase(cmd);
        
        // Check if command appears as a separate word (not part of another word)
        size_t pos = lowerQuery.find(lowerCmd);
        while (pos != std::string::npos) {
            bool isWordStart = (pos == 0 || !std::isalnum(lowerQuery[pos - 1]));
            bool isWordEnd = (pos + lowerCmd.length() >= lowerQuery.length() || 
                             !std::isalnum(lowerQuery[pos + lowerCmd.length()]));
            
            if (isWordStart && isWordEnd) {
                return true;
            }
            
            pos = lowerQuery.find(lowerCmd, pos + 1);
        }
    }
    
    // Check for DCL commands
    for (const auto& cmd : DCL_COMMANDS) {
        std::string lowerCmd = toLowerCase(cmd);
        if (lowerQuery.find(lowerCmd) != std::string::npos) {
            return true;
        }
    }
    
    // Check for common SQL injection patterns
    if (lowerQuery.find("union") != std::string::npos ||
        lowerQuery.find("exec") != std::string::npos ||
        lowerQuery.find("execute") != std::string::npos ||
        lowerQuery.find("sp_") != std::string::npos ||
        lowerQuery.find("xp_") != std::string::npos ||
        lowerQuery.find("into outfile") != std::string::npos ||
        lowerQuery.find("into dumpfile") != std::string::npos) {
        return true;
    }
    
    return false;
}

bool QueryValidator::isValidTableName(const std::string& tableName) {
    if (tableName.empty()) return false;
    
    // Table name should only contain alphanumeric, underscore, or dot (for schema.table)
    std::regex validPattern("^[a-zA-Z_][a-zA-Z0-9_.]*$");
    return std::regex_match(tableName, validPattern);
}

std::vector<std::string> QueryValidator::extractTableNames(const std::string& query) {
    std::vector<std::string> tables;
    std::string lowerQuery = toLowerCase(query);
    
    // Simple extraction: look for "FROM table_name" or "JOIN table_name"
    std::regex fromPattern("from\\s+([a-z_][a-z0-9_]*)", std::regex::icase);
    std::regex joinPattern("join\\s+([a-z_][a-z0-9_]*)", std::regex::icase);
    
    std::smatch match;
    std::string::const_iterator searchStart(lowerQuery.cbegin());
    
    while (std::regex_search(searchStart, lowerQuery.cend(), match, fromPattern)) {
        tables.push_back(match[1]);
        searchStart = match.suffix().first;
    }
    
    searchStart = lowerQuery.cbegin();
    while (std::regex_search(searchStart, lowerQuery.cend(), match, joinPattern)) {
        tables.push_back(match[1]);
        searchStart = match.suffix().first;
    }
    
    return tables;
}

QueryValidator::ValidationResult QueryValidator::validate(
    const std::string& query,
    const std::vector<std::string>& allowedTables,
    bool readOnly
) {
    ValidationResult result;
    result.valid = false;
    
    // 1. Check if query is empty
    std::string normalized = normalizeQuery(query);
    if (normalized.empty()) {
        result.error = "Query is empty";
        return result;
    }
    
    result.sanitizedQuery = normalized;
    
    // 2. Check for multiple statements
    if (containsMultipleStatements(normalized)) {
        result.error = "Multiple statements are not allowed";
        return result;
    }
    
    // 3. Check for dangerous commands
    if (containsDangerousCommand(normalized)) {
        result.error = "Query contains dangerous or forbidden commands";
        return result;
    }
    
    // 4. If read-only mode, ensure query is SELECT
    std::string lowerQuery = toLowerCase(normalized);
    if (readOnly && lowerQuery.find("select") != 0) {
        result.error = "Only SELECT queries are allowed in read-only mode";
        return result;
    }
    
    // 5. Check table whitelist (if provided)
    if (!allowedTables.empty()) {
        std::vector<std::string> usedTables = extractTableNames(normalized);
        
        for (const auto& table : usedTables) {
            bool found = false;
            for (const auto& allowed : allowedTables) {
                if (toLowerCase(table) == toLowerCase(allowed)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                result.error = "Table '" + table + "' is not in the allowed list";
                return result;
            }
        }
    }
    
    // 6. Basic syntax validation
    if (normalized.length() > 10000) {
        result.error = "Query is too long (max 10000 characters)";
        return result;
    }
    
    // 7. Check for unclosed strings
    int singleQuoteCount = 0;
    int doubleQuoteCount = 0;
    for (char c : normalized) {
        if (c == '\'') singleQuoteCount++;
        if (c == '"') doubleQuoteCount++;
    }
    
    if (singleQuoteCount % 2 != 0 || doubleQuoteCount % 2 != 0) {
        result.error = "Unclosed string literal in query";
        return result;
    }
    
    // All checks passed
    result.valid = true;
    result.error = "";
    return result;
}

} // namespace Domain::Database


