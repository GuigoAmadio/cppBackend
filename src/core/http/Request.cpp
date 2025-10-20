#include "Request.hpp"
#include "../json/Json.hpp"
#include <algorithm>
#include <cctype>

namespace Core::Http {

std::string Request::getHeader(const std::string& name) const {
    // Case-insensitive search
    for (const auto& [key, value] : headers_) {
        if (std::equal(key.begin(), key.end(), name.begin(), name.end(),
                      [](char a, char b) {
                          return std::tolower(a) == std::tolower(b);
                      })) {
            return value;
        }
    }
    return "";
}

bool Request::hasHeader(const std::string& name) const {
    return !getHeader(name).empty();
}

std::string Request::getQuery(const std::string& key) const {
    auto it = query_.find(key);
    return (it != query_.end()) ? it->second : "";
}

std::string Request::getParam(const std::string& key) const {
    auto it = params_.find(key);
    return (it != params_.end()) ? it->second : "";
}

std::shared_ptr<Core::Json::JsonValue> Request::getJson() const {
    if (body_.empty()) {
        return nullptr;
    }
    
    try {
        // Retorna diretamente o shared_ptr (muito mais simples!)
        return Core::Json::Json::parse(body_);
    } catch (const std::exception& e) {
        // Se o body não for um JSON válido, evitamos crash e retornamos nullptr
        return nullptr;
    }
}

std::string Request::methodToString() const {
    switch (method_) {
        case Method::GET: return "GET";
        case Method::POST: return "POST";
        case Method::PUT: return "PUT";
        case Method::DEL: return "DELETE";
        case Method::PATCH: return "PATCH";
        case Method::OPTIONS: return "OPTIONS";
        case Method::HEAD: return "HEAD";
        default: return "UNKNOWN";
    }
}

Method Request::parseMethod(const std::string& method) {
    if (method == "GET") return Method::GET;
    if (method == "POST") return Method::POST;
    if (method == "PUT") return Method::PUT;
    if (method == "DELETE") return Method::DEL;
    if (method == "PATCH") return Method::PATCH;
    if (method == "OPTIONS") return Method::OPTIONS;
    if (method == "HEAD") return Method::HEAD;
    return Method::UNKNOWN;
}

void Request::addHeader(const std::string& name, const std::string& value) {
    headers_[name] = value;
}

void Request::addParam(const std::string& key, const std::string& value) {
    params_[key] = value;
}

void Request::addQuery(const std::string& key, const std::string& value) {
    query_[key] = value;
}

void Request::setCustomData(const std::string& key, const std::string& value) {
    customData_[key] = value;
}

std::string Request::getCustomData(const std::string& key) const {
    auto it = customData_.find(key);
    if (it != customData_.end()) {
        return it->second;
    }
    return "";
}

bool Request::hasCustomData(const std::string& key) const {
    return customData_.find(key) != customData_.end();
}

} // namespace Core::Http
