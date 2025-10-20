#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "../../core/json/Json.hpp"

namespace Core::Http {

/**
 * HTTP Methods
 */
enum class Method {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    OPTIONS,
    HEAD,
    UNKNOWN
};

/**
 * HTTP Request
 * 
 * Representa um request HTTP parseado
 * 
 * Exemplo de request:
 * GET /users/123 HTTP/1.1
 * Host: localhost:8080
 * Content-Type: application/json
 * 
 * { "name": "John" }
 */
class Request {
public:
    Request() = default;
    
    // Getters
    Method getMethod() const { return method_; }
    const std::string& getPath() const { return path_; }
    const std::string& getVersion() const { return version_; }
    const std::string& getBody() const { return body_; }
    
    /**
     * Obter header (case-insensitive)
     */
    std::string getHeader(const std::string& name) const;
    
    /**
     * Verificar se header existe
     */
    bool hasHeader(const std::string& name) const;
    
    /**
     * Obter todos os headers
     */
    const std::unordered_map<std::string, std::string>& getHeaders() const {
        return headers_;
    }
    
    /**
     * Obter query parameter
     * Ex: /users?name=John -> getQuery("name") = "John"
     */
    std::string getQuery(const std::string& key) const;
    
    /**
     * Obter path parameter
     * Ex: /users/:id -> getParam("id")
     */
    std::string getParam(const std::string& key) const;
    
    /**
     * Parsear body como JSON
     */
    std::unique_ptr<Core::Json::JsonValue> getJson() const;
    
    /**
     * Converter method para string
     */
    std::string methodToString() const;
    
    /**
     * Parsear method de string
     */
    static Method parseMethod(const std::string& method);
    
    // Setters (usado pelo parser)
    void setMethod(Method method) { method_ = method; }
    void setPath(const std::string& path) { path_ = path; }
    void setVersion(const std::string& version) { version_ = version; }
    void setBody(const std::string& body) { body_ = body; }
    void addHeader(const std::string& name, const std::string& value);
    void addParam(const std::string& key, const std::string& value);
    void addQuery(const std::string& key, const std::string& value);
    
private:
    Method method_ = Method::UNKNOWN;
    std::string path_;
    std::string version_;
    std::unordered_map<std::string, std::string> headers_;
    std::unordered_map<std::string, std::string> params_;
    std::unordered_map<std::string, std::string> query_;
    std::string body_;
};

} // namespace Core::Http

