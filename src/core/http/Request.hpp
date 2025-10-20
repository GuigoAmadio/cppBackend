#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "../../core/json/Json.hpp"

// Espaço de nomes que contém toda a lógica HTTP do backend.
namespace Core::Http {

/**
 * Enumeração dos métodos HTTP suportados.
 * 
 * Cada enum representa um método HTTP padrão.
 * DEL é usado ao invés de DELETE para evitar conflitos com macros do Windows.
 */
enum class Method {
    GET,        // Método GET: usado para recuperar recursos.
    POST,       // Método POST: usado para criar/mandar dados ao servidor.
    PUT,        // Método PUT: usado para substituir um recurso.
    DEL,        // DELETE renomeado para DEL (evitando conflito no Windows)
    PATCH,      // Método PATCH: usado para atualizar parcialmente um recurso.
    OPTIONS,    // Método OPTIONS: descobrir métodos suportados por um recurso.
    HEAD,       // Método HEAD: igual ao GET mas sem o corpo.
    UNKNOWN     // Método desconhecido/ou não suportado.
};

/**
 * Representa uma requisição HTTP já interpretada (parseada).
 * 
 * Uma requisição HTTP possui linha de início, headers e, opcionalmente, um corpo.
 * 
 * Exemplo de requisição:
 * ---------------------------------------
 * GET /users/123 HTTP/1.1
 * Host: localhost:8080
 * Content-Type: application/json
 * 
 * { "name": "John" }
 * ---------------------------------------
 * 
 * Esta classe oferece métodos para acessar facilmente partes da requisição,
 * como headers, parâmetros de rota, query string, corpo (body) e método.
 */
class Request {
public:
    Request() = default;
    
    // === MÉTODOS DE LEITURA ===

    /**
     * @brief Retorna o método HTTP (GET, POST, etc) desta requisição.
     */
    Method getMethod() const { return method_; }

    /**
     * @brief Retorna o caminho/path acessado.
     * Exemplo: "/users/123"
     */
    const std::string& getPath() const { return path_; }

    /**
     * @brief Retorna a versão do protocolo HTTP utilizado (ex: "HTTP/1.1").
     */
    const std::string& getVersion() const { return version_; }

    /**
     * @brief Retorna o corpo da requisição (body), geralmente usado em POST e PUT.
     */
    const std::string& getBody() const { return body_; }
    
    /**
     * @brief Busca um header pelo nome (case-insensitive).
     * @param name Nome do header (ex: "Content-Type")
     * @return Valor do header, ou "" se não encontrado.
     * 
     * Exemplo: getHeader("Host") pode retornar "localhost:8080".
     */
    std::string getHeader(const std::string& name) const;
    
    /**
     * @brief Verifica se um header específico existe.
     * @param name Nome do header.
     * @return true se existe, false caso contrário.
     */
    bool hasHeader(const std::string& name) const;
    
    /**
     * @brief Obtém todos os headers em forma de mapa.
     * 
     * Chave = nome do header, valor = valor.
     */
    const std::unordered_map<std::string, std::string>& getHeaders() const {
        return headers_;
    }
    
    /**
     * @brief Obtém o valor de um parâmetro da query string (?key=valor).
     * @param key Nome do parâmetro.
     * @return Valor do parâmetro, ou "" se não existir.
     * 
     * Exemplo: Para /users?name=John, getQuery("name") retorna "John".
     */
    std::string getQuery(const std::string& key) const;
    
    /**
     * @brief Obtém o valor de um parâmetro de path (ex: /users/:id).
     * @param key Nome do parâmetro de rota.
     * @return Valor do parâmetro, ou "" se não existir.
     * 
     * Exemplo: Para rota /users/:id com chamada /users/123, getParam("id") retorna "123".
     */
    std::string getParam(const std::string& key) const;
    
    /**
     * @brief Interpreta o body como JSON e retorna um objeto JsonValue.
     *        Retorna nullptr se o corpo estiver vazio ou for inválido.
     */
    std::shared_ptr<Core::Json::JsonValue> getJson() const;
    
    /**
     * @brief Converte o método HTTP (enum) em string, ex: Method::GET -> "GET".
     */
    std::string methodToString() const;
    
    /**
     * @brief Transforma uma string ("GET", "POST", etc) no enum Method correspondente.
     * @param method String do método.
     * @return Valor do enum Method correspondente.
     */
    static Method parseMethod(const std::string& method);

    // === MÉTODOS DE ESCRITA / USADOS PELO PARSER ===

    /**
     * @brief Define o método HTTP.
     */
    void setMethod(Method method) { method_ = method; }

    /**
     * @brief Define o path da requisição.
     */
    void setPath(const std::string& path) { path_ = path; }

    /**
     * @brief Define a versão do HTTP.
     */
    void setVersion(const std::string& version) { version_ = version; }

    /**
     * @brief Define o corpo da requisição.
     */
    void setBody(const std::string& body) { body_ = body; }

    /**
     * @brief Adiciona um header (nome, valor) à requisição.
     */
    void addHeader(const std::string& name, const std::string& value);

    /**
     * @brief Adiciona um parâmetro de rota (nome, valor), exemplo: :id
     */
    void addParam(const std::string& key, const std::string& value);

    /**
     * @brief Adiciona um parâmetro de query (nome, valor).
     */
    void addQuery(const std::string& key, const std::string& value);
    
    // === CUSTOM DATA (para middlewares) ===
    
    /**
     * @brief Adiciona dados customizados ao request (ex: tenant_id por um middleware)
     */
    void setCustomData(const std::string& key, const std::string& value);
    
    /**
     * @brief Obtém dado customizado do request
     * @return Valor se existe, string vazia caso contrário
     */
    std::string getCustomData(const std::string& key) const;
    
    /**
     * @brief Verifica se existe um dado customizado
     */
    bool hasCustomData(const std::string& key) const;
    
private:
    // Armazena o método HTTP (GET, POST, etc.)
    Method method_ = Method::UNKNOWN;

    // Path da requisição (ex: "/users/123")
    std::string path_;

    // Versão do protocolo HTTP (ex: "HTTP/1.1")
    std::string version_;

    // Mapa de headers: nome -> valor
    std::unordered_map<std::string, std::string> headers_;

    // Mapa de parâmetros de rota extraídos (ex: ":id" em /users/:id)
    std::unordered_map<std::string, std::string> params_;

    // Mapa de parâmetros da query string (?chave=valor)
    std::unordered_map<std::string, std::string> query_;

    // Corpo da requisição (normalmente para métodos POST/PUT)
    std::string body_;
    
    // Dados customizados (usados por middlewares, ex: tenant_id, user_id)
    std::unordered_map<std::string, std::string> customData_;
};

} // namespace Core::Http

