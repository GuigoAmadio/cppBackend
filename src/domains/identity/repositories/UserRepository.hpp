#pragma once

#include "../entities/User.hpp"
#include "../../../core/database/ConnectionPool.hpp"
#include <memory>
#include <optional>
#include <vector>

namespace Domains::Identity::Repositories {

using namespace Entities;

/**
 * @brief Repository para User Entity.
 * 
 * Implementa padrão Repository do DDD.
 * Abstrai acesso ao banco de dados PostgreSQL.
 */
class UserRepository {
public:
    explicit UserRepository(std::shared_ptr<Core::Database::ConnectionPool> pool);
    
    /**
     * @brief Busca usuário por ID.
     */
    std::optional<User> findById(const std::string& id);
    
    /**
     * @brief Busca usuário por email.
     */
    std::optional<User> findByEmail(const std::string& email);
    
    /**
     * @brief Lista todos os usuários com paginação.
     */
    std::vector<User> findAll(int limit = 50, int offset = 0);
    
    /**
     * @brief Salva novo usuário no banco.
     * @return User salvo (com ID gerado)
     */
    User save(const User& user);
    
    /**
     * @brief Atualiza usuário existente.
     */
    void update(const User& user);
    
    /**
     * @brief Deleta usuário por ID.
     */
    void deleteById(const std::string& id);
    
    /**
     * @brief Verifica se email já existe.
     */
    bool existsByEmail(const std::string& email);
    
    /**
     * @brief Conta total de usuários.
     */
    int count();
    
    /**
     * @brief Conta usuários ativos.
     */
    int countActive();

private:
    std::shared_ptr<Core::Database::ConnectionPool> pool_;
    
    // Helpers
    User mapToUser(const Core::Database::QueryResult& result, int row);
    std::string formatTimestamp(std::chrono::system_clock::time_point tp);
    std::chrono::system_clock::time_point parseTimestamp(const std::string& str);
};

} // namespace Domains::Identity::Repositories

