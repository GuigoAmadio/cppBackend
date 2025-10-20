/*
 * UserRepository.cpp
 * 
 * Este arquivo implementa os métodos do repositório UserRepository para manipulação dos objetos User
 * na base de dados. Ele faz parte do backend, escrito em C++. 
 * 
 * ---------------- Sintaxe C++ importante neste arquivo ----------------
 * 
 * - STL significa Standard Template Library. É a biblioteca padrão do C++ e fornece containers genéricos e utilidades como
 *   std::string (string), std::vector (vetor), std::optional (valor que pode existir ou não), std::shared_ptr (ponteiro inteligente).
 * - std::vector<T>: vetor dinâmico (array flexível, que cresce e diminui). Ex: std::vector<User>
 * - std::optional<T>: representa um valor que pode estar presente ou ausente, útil para buscas em bancos de dados. Ex: std::optional<User>
 * - std::string: a string padrão do C++. Sempre preferível ao char*.
 * - std::shared_ptr<T>: ponteiro inteligente, gerencia automaticamente a vida útil do objeto (libera memória ao final).
 * - auto: pede ao compilador deduzir o tipo (igual ao var em outras linguagens).
 * - namespace: usado para organizar código em módulos, evitando conflitos de nome.
 * - :: (escopo): acessa membros de namespace/classe.
 * - "const T&": passagem de argumentos por referência constante (evita cópia e não permite alterar).
 * - "throw std::runtime_error(...)": lança uma exceção, abortando a execução normal e informando um erro.
 * - Funções privadas (ex: mapToUser, formatTimestamp) só são usadas internamente.
 * - R"( ... )": define string multilinha.
 * 
 * ---------------- pool_ e conn->executeParams ----------------
 * 
 * O atributo "pool_" na classe UserRepository é um ponteiro inteligente (shared_ptr) para um objeto ConnectionPool.
 * O ConnectionPool armazena várias conexões prontas com o banco de dados para evitar criar/destruir toda hora,
 * melhorando a eficiência.
 * 
 * pool_->acquire() devolve uma conexão do pool para uso temporário. 
 * Exemplo linha 52: 
 *      UserRepository::UserRepository(std::shared_ptr<Core::Database::ConnectionPool> pool)
 *          : pool_(pool) {}
 * Aqui, pool_ recebe o ponteiro passado no construtor. Assim, todos métodos do repositório poderão usar pool_->acquire().
 * 
 * Já conn->executeParams(sql, {arg1, arg2, ...}) executa uma query SQL, 
 * substituindo $1, $2... pelos argumentos do vector passado. 
 * Retorna um resultado que pode ser usado para ler linhas/colunas retornadas do banco.
 */


#include "UserRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>

namespace Domains::Identity::Repositories {

// ----------------------
// Linha 52 detalhada:
//
// UserRepository::UserRepository(std::shared_ptr<Core::Database::ConnectionPool> pool)
//     : pool_(pool) {}
//
// Isto é o construtor da classe UserRepository. Ele recebe como argumento
// um ponteiro inteligente (std::shared_ptr) para um pool de conexões com o banco (ConnectionPool).
// Esse ponteiro é usado para inicializar o atributo privado pool_.
// Ou seja, pool_ vai guardar o pool para uso posterior pelos métodos da classe.
// 
// A sintaxe ": pool_(pool)" chama-se lista de inicialização. Aqui ela faz a associação entre o argumento
// "pool" e o atributo (membro) "pool_" da classe. Assim, depois da construção, pool_ passa a apontar para o pool passado.
// 
// Exemplo reduzido do uso:
//
// class Teste {
// public:
//     Teste(int valor) : val_(valor) {}
// private:
//     int val_;
// };
//
// Aqui, val_ recebe valor vindo do construtor.)

UserRepository::UserRepository(std::shared_ptr<Core::Database::ConnectionPool> pool)
    : pool_(pool) {}

// Busca usuário por ID
std::optional<User> UserRepository::findById(const std::string& id) {
    auto conn = pool_->acquire(); // pega conexão do pool
    std::string sql = R"(
        SELECT id, email, password_hash, name, is_active, 
               email_verified, created_at, updated_at, last_login_at
        FROM users
        WHERE id = $1
    )";
    auto result = conn->executeParams(sql, {id}); // executa query parametrizada

    if (result.isSuccess() && result.rowCount() > 0) {
        return mapToUser(result, 0); // retorna objeto User
    }
    return std::nullopt;
}

// Busca usuário pelo email
std::optional<User> UserRepository::findByEmail(const std::string& email) {
    auto conn = pool_->acquire();
    std::string sql = R"(
        SELECT id, email, password_hash, name, is_active, 
               email_verified, created_at, updated_at, last_login_at
        FROM users
        WHERE email = $1
    )";
    auto result = conn->executeParams(sql, {email});
    if (result.isSuccess() && result.rowCount() > 0) {
        return mapToUser(result, 0);
    }
    return std::nullopt;
}

// Busca todos os usuários, com paginação
std::vector<User> UserRepository::findAll(int limit, int offset) {
    auto conn = pool_->acquire();
    std::string sql = R"(
        SELECT id, email, password_hash, name, is_active, 
               email_verified, created_at, updated_at, last_login_at
        FROM users
        ORDER BY created_at DESC
        LIMIT $1 OFFSET $2
    )";
    auto result = conn->executeParams(sql, {
        std::to_string(limit),
        std::to_string(offset)
    });
    std::vector<User> users;
    if (result.isSuccess()) {
        for (int i = 0; i < result.rowCount(); ++i) {
            users.push_back(mapToUser(result, i));
        }
    }
    return users;
}

// Salva novo usuário
User UserRepository::save(const User& user) {
    auto conn = pool_->acquire();
    std::string sql = R"(
        INSERT INTO users (
            id, email, password_hash, name, is_active,
            email_verified, created_at, updated_at
        ) VALUES (
            $1, $2, $3, $4, $5, $6, $7, $8
        )
        RETURNING id, email, password_hash, name, is_active,
                  email_verified, created_at, updated_at, last_login_at
    )";
    auto result = conn->executeParams(sql, {
        user.getId(),
        user.getEmail().value(),
        user.getPasswordHash(),
        user.getName(),
        user.isActive() ? "true" : "false",
        user.isEmailVerified() ? "true" : "false",
        formatTimestamp(user.getCreatedAt()),
        formatTimestamp(user.getUpdatedAt())
    });
    if (result.isSuccess() && result.rowCount() > 0) {
        LOG_INFO("User created: " + user.getEmail().value());
        return mapToUser(result, 0);
    }
    throw std::runtime_error("Failed to save user");
}

// Atualiza dados de usuário
void UserRepository::update(const User& user) {
    auto conn = pool_->acquire();
    std::string sql = R"(
        UPDATE users SET
            email = $2,
            password_hash = $3,
            name = $4,
            is_active = $5,
            email_verified = $6,
            updated_at = $7
        WHERE id = $1
    )";
    auto result = conn->executeParams(sql, {
        user.getId(),
        user.getEmail().value(),
        user.getPasswordHash(),
        user.getName(),
        user.isActive() ? "true" : "false",
        user.isEmailVerified() ? "true" : "false",
        formatTimestamp(user.getUpdatedAt())
    });
    if (result.isSuccess()) {
        LOG_INFO("User updated: " + user.getId());
    } else {
        throw std::runtime_error("Failed to update user: " + result.getError());
    }
}

// Remove usuário pelo ID
void UserRepository::deleteById(const std::string& id) {
    auto conn = pool_->acquire();
    std::string sql = "DELETE FROM users WHERE id = $1";
    auto result = conn->executeParams(sql, {id});
    if (result.isSuccess()) {
        LOG_INFO("User deleted: " + id);
    } else {
        throw std::runtime_error("Failed to delete user: " + result.getError());
    }
}

// Verifica se já existe usuário com o email
bool UserRepository::existsByEmail(const std::string& email) {
    auto conn = pool_->acquire();
    std::string sql = "SELECT COUNT(*) FROM users WHERE email = $1";
    auto result = conn->executeParams(sql, {email});
    if (result.isSuccess() && result.rowCount() > 0) {
        int count = std::stoi(result.getValue(0, 0));
        return count > 0;
    }
    return false;
}

// Conta total de usuários
int UserRepository::count() {
    auto conn = pool_->acquire();
    auto result = conn->execute("SELECT COUNT(*) FROM users");
    if (result.isSuccess() && result.rowCount() > 0) {
        return std::stoi(result.getValue(0, 0));
    }
    return 0;
}

// Conta usuários ativos
int UserRepository::countActive() {
    auto conn = pool_->acquire();
    auto result = conn->execute("SELECT COUNT(*) FROM users WHERE is_active = true");
    if (result.isSuccess() && result.rowCount() > 0) {
        return std::stoi(result.getValue(0, 0));
    }
    return 0;
}

// =========== Métodos utilitários internos ============

// Converte resultado do banco para objeto User
User UserRepository::mapToUser(const Core::Database::QueryResult& result, int row) {
    std::string id = result.getValue(row, 0);
    Email email(result.getValue(row, 1));
    std::string passwordHash = result.getValue(row, 2);
    std::string name = result.getValue(row, 3);
    bool isActive = result.getValue(row, 4) == "t" || result.getValue(row, 4) == "true";
    bool emailVerified = result.getValue(row, 5) == "t" || result.getValue(row, 5) == "true";
    auto createdAt = parseTimestamp(result.getValue(row, 6));
    auto updatedAt = parseTimestamp(result.getValue(row, 7));
    std::optional<std::chrono::system_clock::time_point> lastLoginAt;
    if (!result.isNull(row, 8)) {
        lastLoginAt = parseTimestamp(result.getValue(row, 8));
    }
    return User(id, email, passwordHash, name, isActive, 
                emailVerified, createdAt, updatedAt, lastLoginAt);
}

// Formata timestamp C++ para string no formato SQL
std::string UserRepository::formatTimestamp(std::chrono::system_clock::time_point tp) {
    auto time = std::chrono::system_clock::to_time_t(tp);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Converte string timestamp SQL para time_point C++
std::chrono::system_clock::time_point UserRepository::parseTimestamp(const std::string& str) {
    std::tm tm = {};
    std::stringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto time = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(time);
}

} // namespace Domains::Identity::Repositories

