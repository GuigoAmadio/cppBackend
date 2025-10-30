#include "Router.hpp"
#include "../utils/Logger.hpp"
#include <sstream>

// Macro LOG_DEBUG
#define LOG_DEBUG(msg) Utils::Logger::debug(msg)

namespace Core::Http {

Route::Route(Method m, const std::string& p, Handler h)
    : method(m), pattern(p), handler(h) {
    
    // Converter pattern para regex
    // Ex: "/users/:id" -> "/users/([^/]+)"
    std::string regexPattern = pattern;
    
    // Encontrar parâmetros (:id, :name, etc)
    size_t pos = 0;
    while ((pos = regexPattern.find(':', pos)) != std::string::npos) {
        size_t endPos = regexPattern.find('/', pos);
        if (endPos == std::string::npos) {
            endPos = regexPattern.length();
        }
        
        // Extrair nome do parâmetro
        std::string paramName = regexPattern.substr(pos + 1, endPos - pos - 1);
        paramNames.push_back(paramName);
        
        // Substituir :param por regex
        regexPattern.replace(pos, endPos - pos, "([^/]+)");
        pos = endPos;
    }
    
    // Compilar regex
    regex = std::regex(regexPattern);
}

bool Route::matches(const Request& req, 
                   std::unordered_map<std::string, std::string>& params) const {
    // Verificar method
    if (req.getMethod() != method) {
        return false;
    }
    
    // Verificar path com regex
    std::smatch matches;
    std::string path = req.getPath();
    
    if (!std::regex_match(path, matches, regex)) {
        return false;
    }
    
    // Extrair parâmetros
    for (size_t i = 0; i < paramNames.size() && i + 1 < matches.size(); i++) {
        params[paramNames[i]] = matches[i + 1].str();
    }
    
    return true;
}

void Router::get(const std::string& pattern, Handler handler) {
    route(Method::GET, pattern, handler);
}

void Router::post(const std::string& pattern, Handler handler) {
    route(Method::POST, pattern, handler);
}

void Router::put(const std::string& pattern, Handler handler) {
    route(Method::PUT, pattern, handler);
}

void Router::del(const std::string& pattern, Handler handler) {
    route(Method::DEL, pattern, handler);
}

void Router::patch(const std::string& pattern, Handler handler) {
    route(Method::PATCH, pattern, handler);
}

void Router::route(Method method, const std::string& pattern, Handler handler) {
    routes_.emplace_back(method, pattern, handler);
    
    std::string methodStr = "UNKNOWN";
    if (method == Method::GET) methodStr = "GET";
    else if (method == Method::POST) methodStr = "POST";
    else if (method == Method::PUT) methodStr = "PUT";
    else if (method == Method::DEL) methodStr = "DELETE";
    else if (method == Method::PATCH) methodStr = "PATCH";
    
    Utils::Logger::info("[Router::route] this=" + std::to_string(reinterpret_cast<uintptr_t>(this)) + 
                       " | Rota #" + std::to_string(routes_.size()) + " registrada: " + methodStr + " " + pattern);
}

Response Router::handle(const Request& request) {
    // LOG IMEDIATO no início
    // Utils::Logger::info("🔵 [Router::handle] ========== CHAMADO! ==========");
    
    try {
        Response response;
        auto& req = const_cast<Request&>(request);
        
        // Utils::Logger::info("🔵 [Router] handle() called for: " + req.methodToString() + " " + req.getPath());
        // Utils::Logger::info("🔵 [Router] middlewareChain_ is " + std::string(middlewareChain_ ? "NOT NULL" : "NULL"));
        
        // Se tem MiddlewareChain v2.0, usar ela
        // Utils::Logger::info("🔵 [Router] Checking if middlewareChain_...");
        if (middlewareChain_) {
            // Utils::Logger::info("🔵🔵🔵 [Router] ENTRANDO NO IF! Using MiddlewareChain v2.0");
            // Utils::Logger::info("🔵 [Router] MiddlewareChain size: " + std::to_string(middlewareChain_->size()));
            middlewareChain_->execute(req, response, [&]() {
                // Utils::Logger::info("🔵 [Router] Inside MiddlewareChain finalHandler");
                
                // SALVAR headers setados pelos middlewares (especialmente CORS!)
                auto middlewareHeaders = response.getHeaders();
                // Utils::Logger::info("🔵 [Router] Salvando " + std::to_string(middlewareHeaders.size()) + " headers dos middlewares");
                
                // Executar middlewares legados
                for (auto& middleware : middlewares_) {
                    Response tempResponse;
                    if (!middleware(req, tempResponse)) {
                        // Middleware bloqueou o request
                        response = tempResponse;
                        return;
                    }
                }
                
                // Encontrar rota
                std::unordered_map<std::string, std::string> params;
                const Route* route = findRoute(request, params);
                
                if (!route) {
                    response = notFoundHandler(request);
                    return;
                }
                
                // Adicionar parâmetros ao request
                for (const auto& [key, value] : params) {
                    req.addParam(key, value);
                }
                
                    // Executar handler (retorna NOVA response, perdendo headers do middleware!)
                    response = route->handler(request);
                    
                    // RESTAURAR headers dos middlewares (CORS, etc)
                    for (const auto& [name, value] : middlewareHeaders) {
                        // Só restaurar se o handler não sobrescreveu
                        if (response.getHeaders().find(name) == response.getHeaders().end()) {
                            response.setHeader(name, value);
                        }
                    }
                });
                
                return response;
            } else {
            // Executar middlewares
            for (auto& middleware : middlewares_) {
                if (!middleware(req, response)) {
                    // Middleware bloqueou o request
                    return response;
                }
            }
            
            // Encontrar rota
            std::unordered_map<std::string, std::string> params;
            const Route* route = findRoute(request, params);
            
            if (!route) {
                return notFoundHandler(request);
            }
            
            // Adicionar parâmetros ao request
            for (const auto& [key, value] : params) {
                req.addParam(key, value);
            }
            
            // Executar handler
            return route->handler(request);
        }
        
    } catch (const std::exception& e) {
        return errorHandler(e);
    }
}

void Router::use(Middleware middleware) {
    middlewares_.push_back(middleware);
}

const Route* Router::findRoute(const Request& request,
                               std::unordered_map<std::string, std::string>& params) const {
    // Utils::Logger::info("[Router::findRoute] Procurando rota para: " + 
    //                    const_cast<Request&>(request).methodToString() + " " + request.getPath() + 
    //                    " | Total de rotas: " + std::to_string(routes_.size()));
    
    int count = 0;
    for (const auto& route : routes_) {
        count++;
        if (route.matches(request, params)) {
            // Utils::Logger::info("[Router::findRoute] MATCH encontrado na rota #" + std::to_string(count) + ": " + route.pattern);
            return &route;
        }
    }
    
    Utils::Logger::info("[Router::findRoute] ❌ Nenhuma rota encontrada para: " + request.getPath());
    return nullptr;
}

Response Router::notFoundHandler(const Request& request) {
    std::string body = "404 Not Found: " + request.getPath();
    return Response(StatusCode::NotFound).text(body);
}

Response Router::errorHandler(const std::exception& e) {
    std::string body = std::string("500 Internal Server Error: ") + e.what();
    return Response(StatusCode::InternalServerError).text(body);
}

void Router::setMiddlewares(std::shared_ptr<MiddlewareChain> chain) {
    middlewareChain_ = chain;
}

std::shared_ptr<MiddlewareChain> Router::getMiddlewares() const {
    return middlewareChain_;
}

} // namespace Core::Http

