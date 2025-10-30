#include "Middleware.hpp"
#include "../utils/Logger.hpp"

// Macro LOG_DEBUG
#define LOG_DEBUG(msg) Utils::Logger::debug(msg)
#include <chrono>
#include <sstream>

namespace Core::Http {

// ==================== Middleware ====================

Middleware::Middleware(MiddlewareFunction fn)
    : pathPrefix_(""), function_(std::move(fn)), hasPathFilter_(false) {}

Middleware::Middleware(const std::string& pathPrefix, MiddlewareFunction fn)
    : pathPrefix_(pathPrefix), function_(std::move(fn)), hasPathFilter_(true) {}

bool Middleware::shouldExecute(const std::string& path) const {
    if (!hasPathFilter_) {
        return true;  // Sem filtro = executa sempre
    }
    
    // Verifica se path começa com pathPrefix_
    if (path.length() < pathPrefix_.length()) {
        return false;
    }
    
    return path.compare(0, pathPrefix_.length(), pathPrefix_) == 0;
}

void Middleware::execute(Request& req, Response& res, NextFunction next) const {
    function_(req, res, next);
}

// ==================== MiddlewareChain ====================

void MiddlewareChain::use(MiddlewareFunction middleware) {
    middlewares_.emplace_back(std::move(middleware));
}

void MiddlewareChain::use(const std::string& pathPrefix, MiddlewareFunction middleware) {
    middlewares_.emplace_back(pathPrefix, std::move(middleware));
}

void MiddlewareChain::execute(Request& req, Response& res, std::function<void()> finalHandler) {
    executeAt(0, req, res, finalHandler);
}

void MiddlewareChain::executeAt(size_t index, Request& req, Response& res, std::function<void()> finalHandler) {
    // Se chegou no fim da chain, executar handler final
    if (index >= middlewares_.size()) {
        finalHandler();
        return;
    }
    
    const auto& middleware = middlewares_[index];
    
    // Verificar se middleware deve ser executado para este path
    if (!middleware.shouldExecute(req.getPath())) {
        // Pular para próximo middleware
        executeAt(index + 1, req, res, finalHandler);
        return;
    }
    
    // Executar middleware
    try {
        middleware.execute(req, res, [this, index, &req, &res, finalHandler]() {
            // Next function: executa próximo middleware
            executeAt(index + 1, req, res, finalHandler);
        });
    } catch (const std::exception& e) {
        // Se middleware lançar exceção, parar chain
        Utils::Logger::error("Middleware exception: " + std::string(e.what()));
        
        // Retornar erro 500 se ainda não enviou resposta
        if (res.getStatus() == StatusCode::OK || res.getStatus() == static_cast<StatusCode>(0)) {
            res = Response(StatusCode::InternalServerError)
                .text("Internal Server Error: " + std::string(e.what()));
        }
    }
}

// ==================== BUILT-IN MIDDLEWARES ====================

namespace Middlewares {

MiddlewareFunction cors(
    const std::string& origin,
    const std::string& methods,
    const std::string& headers
) {
    return [origin, methods, headers](Request& req, Response& res, NextFunction next) {
        // Refletir o Origin da requisição (permite file://, localhost, null, etc)
        std::string requestOrigin = req.getHeader("Origin");
        
        if (!requestOrigin.empty()) {
            // Se tem Origin na requisição, refletir ele de volta
            res.setHeader("Access-Control-Allow-Origin", requestOrigin);
        } else if (origin == "*") {
            // Se não tem Origin mas está configurado como *, usar *
            res.setHeader("Access-Control-Allow-Origin", "*");
        } else {
            // Usar o origin configurado
            res.setHeader("Access-Control-Allow-Origin", origin);
        }
        
        // Headers CORS adicionais
        res.setHeader("Access-Control-Allow-Methods", methods);
        res.setHeader("Access-Control-Allow-Headers", headers);
        res.setHeader("Access-Control-Allow-Credentials", "true");
        res.setHeader("Access-Control-Max-Age", "86400");  // 24 horas
        
        // Se é preflight (OPTIONS), retornar 204 imediatamente
        if (req.getMethod() == Method::OPTIONS) {
            res.setStatus(StatusCode::NoContent);
            return;  // NÃO chama next() - para chain aqui
        }
        next();
    };
}

MiddlewareFunction requestLogger() {
    return [](Request& req, Response& res, NextFunction next) {
        auto start = std::chrono::steady_clock::now();
        
        // Log antes de processar
        Utils::Logger::debug("→ " + req.methodToString() + " " + req.getPath());
        
        // Processar requisição
        next();
        
        // Log depois de processar
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start
        ).count();
        
        std::ostringstream oss;
        oss << "← " << req.methodToString() << " " << req.getPath() 
            << " - " << static_cast<int>(res.getStatus())
            << " (" << duration << "ms)";
        
        Utils::Logger::info(oss.str());
    };
}

MiddlewareFunction timing() {
    return [](Request& req, Response& res, NextFunction next) {
        (void)req;  // Unused
        
        auto start = std::chrono::steady_clock::now();
        
        next();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start
        ).count();
        
        res.setHeader("X-Response-Time", std::to_string(duration) + "ms");
    };
}

MiddlewareFunction errorHandler() {
    return [](Request& req, Response& res, NextFunction next) {
        try {
            next();
        } catch (const std::exception& e) {
            Utils::Logger::error("Request handler exception: " + std::string(e.what()));
            
            // Criar resposta de erro
            auto json = Core::Json::makeObject();
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString(e.what());
            json->asObject()["path"] = Core::Json::makeString(req.getPath());
            
            res = Response(StatusCode::InternalServerError).json(*json);
        }
    };
}

MiddlewareFunction jsonBodyParser() {
    return [](Request& req, Response& res, NextFunction next) {
        // Apenas para métodos que esperam body
        if (req.getMethod() == Method::POST || 
            req.getMethod() == Method::PUT || 
            req.getMethod() == Method::PATCH) {
            
            // Verificar se tem body
            if (!req.getBody().empty()) {
                // Tentar parsear JSON
                try {
                    auto json = req.getJson();
                    if (!json) {
                        // Body existe mas não é JSON válido
                        auto errorJson = Core::Json::makeObject();
                        errorJson->asObject()["status"] = Core::Json::makeString("error");
                        errorJson->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
                        
                        res = Response(StatusCode::BadRequest).json(*errorJson);
                        return;  // NÃO chama next()
                    }
                } catch (const std::exception& e) {
                    auto errorJson = Core::Json::makeObject();
                    errorJson->asObject()["status"] = Core::Json::makeString("error");
                    errorJson->asObject()["message"] = Core::Json::makeString("JSON parse error: " + std::string(e.what()));
                    
                    res = Response(StatusCode::BadRequest).json(*errorJson);
                    return;  // NÃO chama next()
                }
            }
        }
        
        next();
    };
}

} // namespace Middlewares

} // namespace Core::Http

