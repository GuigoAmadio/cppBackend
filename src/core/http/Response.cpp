#include "Response.hpp"
#include "../json/Json.hpp"
#include <sstream>

namespace Core::Http {

Response::Response(StatusCode status) 
    : status_(status) {
    // Headers padrão
    setHeader("Server", "cppBackend/1.0");
    setHeader("Content-Type", "text/plain");
}

Response& Response::setStatus(StatusCode status) {
    status_ = status;
    return *this;
}

Response& Response::setBody(const std::string& body) {
    body_ = body;
    setHeader("Content-Length", std::to_string(body_.length()));
    return *this;
}

Response& Response::setHeader(const std::string& name, const std::string& value) {
    headers_[name] = value;
    return *this;
}

Response& Response::json(const Core::Json::JsonValue& value) {
    setHeader("Content-Type", "application/json");
    
    // Serializar JsonValue para string
    // Note: precisamos de um shared_ptr, então criamos um temporário
    // (não ideal, mas funciona por enquanto)
    std::string jsonStr = value.toString(0);
    setBody(jsonStr);
    
    return *this;
}

Response& Response::text(const std::string& text) {
    setHeader("Content-Type", "text/plain; charset=utf-8");
    setBody(text);
    return *this;
}

Response& Response::html(const std::string& html) {
    setHeader("Content-Type", "text/html; charset=utf-8");
    setBody(html);
    return *this;
}

std::string Response::toString() const {
    std::ostringstream oss;
    
    // Status line: "HTTP/1.1 200 OK\r\n"
    oss << "HTTP/1.1 " << (int)status_ << " " << statusToString(status_) << "\r\n";
    
    // Headers
    for (const auto& [name, value] : headers_) {
        oss << name << ": " << value << "\r\n";
    }
    
    // Blank line separating headers from body
    oss << "\r\n";
    
    // Body
    oss << body_;
    
    return oss.str();
}

std::string Response::statusToString(StatusCode status) {
    switch (status) {
        case StatusCode::OK: return "OK";
        case StatusCode::Created: return "Created";
        case StatusCode::NoContent: return "No Content";
        
        case StatusCode::BadRequest: return "Bad Request";
        case StatusCode::Unauthorized: return "Unauthorized";
        case StatusCode::Forbidden: return "Forbidden";
        case StatusCode::NotFound: return "Not Found";
        case StatusCode::Conflict: return "Conflict";
        
        case StatusCode::InternalServerError: return "Internal Server Error";
        case StatusCode::NotImplemented: return "Not Implemented";
        case StatusCode::ServiceUnavailable: return "Service Unavailable";
        
        default: return "Unknown";
    }
}

} // namespace Core::Http

