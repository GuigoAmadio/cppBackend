#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "../../core/json/Json.hpp"

namespace Core::Http {

/**
 * HTTP Status Codes
 */
enum class StatusCode {
    OK = 200,
    Created = 201,
    NoContent = 204,
    
    BadRequest = 400,
    Unauthorized = 401,
    Forbidden = 403,
    NotFound = 404,
    Conflict = 409,
    
    InternalServerError = 500,
    NotImplemented = 501,
    ServiceUnavailable = 503
};

/**
 * HTTP Response
 * 
 * Representa uma resposta HTTP
 * 
 * Exemplo:
 * HTTP/1.1 200 OK
 * Content-Type: application/json
 * Content-Length: 27
 * 
 * { "message": "Success!" }
 */
class Response {
public:
    Response(StatusCode status = StatusCode::OK);
    
    // Setters
    Response& setStatus(StatusCode status);
    Response& setBody(const std::string& body);
    Response& setHeader(const std::string& name, const std::string& value);
    
    /**
     * Configurar body como JSON
     */
    Response& json(const Core::Json::JsonValue& value);
    
    /**
     * Configurar body como texto
     */
    Response& text(const std::string& text);
    
    /**
     * Configurar body como HTML
     */
    Response& html(const std::string& html);
    
    /**
     * Serializar response completo para string
     * (pronto para enviar via socket)
     */
    std::string toString() const;
    
    /**
     * Converter status code para string
     */
    static std::string statusToString(StatusCode status);
    
    // Getters
    StatusCode getStatus() const { return status_; }
    const std::string& getBody() const { return body_; }
    const std::unordered_map<std::string, std::string>& getHeaders() const {
        return headers_;
    }
    
private:
    StatusCode status_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
};

} // namespace Core::Http

