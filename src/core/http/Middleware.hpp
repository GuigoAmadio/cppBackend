#pragma once

#include "Request.hpp"
#include "Response.hpp"
#include <functional>
#include <vector>
#include <memory>
#include <string>

namespace Core::Http {

/**
 * @brief Type definition para funções Next.
 * 
 * Middlewares chamam next() para passar controle para o próximo middleware.
 */
using NextFunction = std::function<void()>;

/**
 * @brief Type definition para middleware functions.
 * 
 * Middleware recebe:
 * - Request& - requisição HTTP (pode modificar)
 * - Response& - resposta HTTP (pode modificar)
 * - NextFunction - callback para próximo middleware
 * 
 * @example
 *   // Middleware de logging
 *   [](Request& req, Response& res, NextFunction next) {
 *       Logger::info("Request: " + req.getPath());
 *       next();  // Chama próximo middleware
 *       Logger::info("Response: " + std::to_string(res.getStatusCode()));
 *   }
 */
using MiddlewareFunction = std::function<void(Request&, Response&, NextFunction)>;

/**
 * @brief Representa um middleware individual.
 */
class Middleware {
public:
    /**
     * @brief Cria middleware sem filtro de path.
     * 
     * Será executado para todas as rotas.
     */
    explicit Middleware(MiddlewareFunction fn);
    
    /**
     * @brief Cria middleware com filtro de path.
     * 
     * Será executado apenas para paths que começam com pathPrefix.
     * 
     * @example
     *   Middleware("/api", authMiddleware);  // Apenas rotas /api/*
     */
    Middleware(const std::string& pathPrefix, MiddlewareFunction fn);
    
    /**
     * @brief Verifica se middleware deve ser executado para este path.
     */
    bool shouldExecute(const std::string& path) const;
    
    /**
     * @brief Executa o middleware.
     */
    void execute(Request& req, Response& res, NextFunction next) const;

private:
    std::string pathPrefix_;
    MiddlewareFunction function_;
    bool hasPathFilter_;
};

/**
 * @brief Gerencia cadeia de middlewares.
 * 
 * Middlewares são executados na ordem em que foram adicionados.
 * 
 * @example
 *   MiddlewareChain chain;
 *   
 *   // Middleware global de CORS
 *   chain.use([](Request& req, Response& res, NextFunction next) {
 *       res.setHeader("Access-Control-Allow-Origin", "*");
 *       next();
 *   });
 *   
 *   // Middleware de autenticação apenas em /api
 *   chain.use("/api", authMiddleware);
 *   
 *   // Executar chain
 *   Response finalResponse;
 *   chain.execute(request, finalResponse, [&]() {
 *       // Handler final (rota)
 *       finalResponse = routeHandler(request);
 *   });
 */
class MiddlewareChain {
public:
    /**
     * @brief Adiciona middleware global (todas as rotas).
     */
    void use(MiddlewareFunction middleware);
    
    /**
     * @brief Adiciona middleware com filtro de path.
     */
    void use(const std::string& pathPrefix, MiddlewareFunction middleware);
    
    /**
     * @brief Executa toda a cadeia de middlewares.
     * 
     * @param req Requisição HTTP
     * @param res Resposta HTTP (será modificada pelos middlewares)
     * @param finalHandler Handler final (normalmente a rota)
     */
    void execute(Request& req, Response& res, std::function<void()> finalHandler);
    
    /**
     * @brief Número de middlewares registrados.
     */
    size_t size() const { return middlewares_.size(); }
    
    /**
     * @brief Remove todos os middlewares.
     */
    void clear() { middlewares_.clear(); }

private:
    std::vector<Middleware> middlewares_;
    
    // Executa middleware na posição index
    void executeAt(size_t index, Request& req, Response& res, std::function<void()> finalHandler);
};

// ==================== BUILT-IN MIDDLEWARES ====================

/**
 * @namespace Core::Http::Middlewares
 * @brief Middlewares prontos para uso.
 */
namespace Middlewares {

/**
 * @brief Middleware de CORS.
 * 
 * Adiciona headers de CORS para permitir requisições cross-origin.
 * 
 * @example
 *   chain.use(Middlewares::cors());
 */
MiddlewareFunction cors(
    const std::string& origin = "*",
    const std::string& methods = "GET,POST,PUT,DELETE,PATCH,OPTIONS",
    const std::string& headers = "Content-Type,Authorization"
);

/**
 * @brief Middleware de logging de requisições.
 * 
 * Loga método, path e tempo de resposta.
 * 
 * @example
 *   chain.use(Middlewares::requestLogger());
 */
MiddlewareFunction requestLogger();

/**
 * @brief Middleware de medição de tempo.
 * 
 * Adiciona header X-Response-Time com tempo em ms.
 * 
 * @example
 *   chain.use(Middlewares::timing());
 */
MiddlewareFunction timing();

/**
 * @brief Middleware de tratamento de erros.
 * 
 * Captura exceções lançadas em middlewares/handlers posteriores.
 * 
 * @example
 *   chain.use(Middlewares::errorHandler());
 */
MiddlewareFunction errorHandler();

/**
 * @brief Middleware de validação de JSON body.
 * 
 * Valida que body é JSON válido para métodos POST/PUT/PATCH.
 * 
 * @example
 *   chain.use("/api", Middlewares::jsonBodyParser());
 */
MiddlewareFunction jsonBodyParser();

} // namespace Middlewares

} // namespace Core::Http

