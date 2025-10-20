#pragma once

#include <string>
#include <functional>
#include <vector>
#include <regex>
#include <memory>
#include "Request.hpp"
#include "Response.hpp"

namespace Core::Http {

/**
 * Handler function type
 * Recebe request, retorna response
 */
using Handler = std::function<Response(const Request&)>;

/**
 * Route
 * Representa uma rota (pattern + handler)
 */
struct Route {
    Method method;
    std::string pattern;          // Ex: "/users/:id"
    std::regex regex;             // Regex compilado
    Handler handler;
    std::vector<std::string> paramNames;  // Ex: ["id"]
    
    Route(Method m, const std::string& p, Handler h);
    
    /**
     * Verificar se request matches esta rota
     */
    bool matches(const Request& req, std::unordered_map<std::string, std::string>& params) const;
};

/**
 * Router
 * 
 * Sistema de roteamento que mapeia URLs para handlers
 * 
 * Exemplo:
 * router.get("/users", listUsersHandler);
 * router.post("/users", createUserHandler);
 * router.get("/users/:id", getUserHandler);
 */
class Router {
public:
    Router() = default;
    
    /**
     * Registrar rotas
     */
    void get(const std::string& pattern, Handler handler);
    void post(const std::string& pattern, Handler handler);
    void put(const std::string& pattern, Handler handler);
    void del(const std::string& pattern, Handler handler);
    void patch(const std::string& pattern, Handler handler);
    
    /**
     * Rota genérica
     */
    void route(Method method, const std::string& pattern, Handler handler);
    
    /**
     * Processar request e retornar response
     */
    Response handle(const Request& request);
    
    /**
     * Middleware (executa antes de todos os handlers)
     */
    using Middleware = std::function<bool(Request&, Response&)>;
    void use(Middleware middleware);
    
private:
    std::vector<Route> routes_;
    std::vector<Middleware> middlewares_;
    
    /**
     * Encontrar rota que matches o request
     */
    const Route* findRoute(const Request& request, 
                           std::unordered_map<std::string, std::string>& params) const;
    
    /**
     * Handler para 404 Not Found
     */
    Response notFoundHandler(const Request& request);
    
    /**
     * Handler para 500 Internal Server Error
     */
    Response errorHandler(const std::exception& e);
};

} // namespace Core::Http

