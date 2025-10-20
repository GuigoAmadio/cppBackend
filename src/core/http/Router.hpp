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
 * ================================================
 *              O QUE É ESTE ARQUIVO?
 * ================================================
 * 
 * Este arquivo define o sistema de roteamento do backend HTTP em C++. 
 * Ele é responsável por receber cada requisição, identificar qual "rota" (endpoint)
 * ela corresponde (ex: /health, /api/users/123), extrair os parâmetros (ex: id),
 * e disparar a função correta que responde a essa URL/método.
 * 
 * ================================================
 *             CONCEITOS E ESTRUTURAS
 * ================================================
 *
 * --- Handler ---
 * Um Handler é uma função que recebe um objeto Request e retorna um Response.
 * Ou seja, é um "delegado" do endpoint. 
 * Exemplo: 
 *   Response helloWorld(const Request& req) { ... }
 *   Router r; r.get("/hello", helloWorld);
 *
 * --- Route ---
 * Uma Rota representa uma relação entre:
 *   - O método HTTP (GET/POST...)
 *   - Um padrão de URL (ex: "/users/:id")
 *   - Um Handler responsável por atender essa rota
 * 
 * Ao cadastrar uma rota, o padrão (com possíveis parâmetros :id, :name) é convertido
 * para uma expressão regular (regex) para identificar e extrair automaticamente os
 * parâmetros dinâmicos da URL.
 * Exemplo:
 *   pattern: "/users/:id/profile"
 *   regex:   "/users/([^/]+)/profile"
 *   paramNames: ["id"]
 *
 * --- Router ---
 * O Router é o "receptor principal" das requisições HTTP. Ele:
 *  1. Armazena todas as rotas declaradas (vetor de Route)
 *  2. Recebe cada Request
 *  3. Procura uma Route compatível (por método e path), usando regex
 *  4. Extrai parâmetros (se existirem) e chama o Handler correspondente
 *  5. Se nenhuma rota casar, devolve 404
 *  6. Oferece suporte para middlewares (funções executadas antes do handler principal)
 *
 * ================================================
 *        DETALHANDO OS TIPOS E FUNÇÕES
 * ================================================
 */

// --- Handler ---
// Alias para função que recebe Request (const-ref) e retorna Response.
// Exemplo de um handler possível:
//   Response minhaFuncao(const Request& req) { ... }
using Handler = std::function<Response(const Request&)>;

// --- Estrutura de uma Rota ---
struct Route {
    Method method;                // Método HTTP (GET, POST, etc.)
    std::string pattern;          // Padrão da rota ex: "/users/:id"
    std::regex regex;             // Regex compilada para identificar match de path
    Handler handler;              // Função a ser executada se a rota casar
    std::vector<std::string> paramNames;  // Nomes dos parâmetros (extraídos do pattern)
    
    // Construtor: recebe método, pattern e handler
    Route(Method m, const std::string& p, Handler h);

    /**
     * matches: Verifica se esta rota "casa" com a request (método + path)
     *          Se casar, extrai e preenche o mapa de parâmetros (ex: { "id": "123" })
     * - req: a requisição a ser verificada
     * - params: mapa de saída para armazenar parâmetros extraídos da url
     * Return: true (=match); false (=não caso)
     */
    bool matches(const Request& req, std::unordered_map<std::string, std::string>& params) const;
};

// --- Router principal ---
class Router {
public:
    Router() = default;

    // =========== REGISTRO DE ROTAS ===========
    // Cada método HTTP tem seu registrador
    // Exemplo de uso:
    //    router.get("/users", handler1);
    //    router.post("/users", handler2);
    //    router.get("/users/:id", handler3);
    void get(const std::string& pattern, Handler handler);
    void post(const std::string& pattern, Handler handler);
    void put(const std::string& pattern, Handler handler);
    void del(const std::string& pattern, Handler handler);    // Para DELETE
    void patch(const std::string& pattern, Handler handler);

    // Registrar rota genérica para qualquer método
    void route(Method method, const std::string& pattern, Handler handler);

    // =========== PROCESSAMENTO DE REQUEST ===========
    /**
     * Recebe uma Request, processa todos os middlewares e 
     * executa o handler da rota correspondente (caso exista), produzindo um Response.
     * Caso não seja encontrada uma rota, retorna 404.
     */
    Response handle(const Request& request);

    // =========== MIDDLEWARES ===========
    /**
     * Middleware: função que pode interceptar/modificar Request/Response antes do handler
     * Retorna true (continua fluxo normalmente) ou false (bloqueia, devolve resposta customizada)
     * Exemplo: autenticação, log, modificação de header, etc.
     * Uso: 
     *   router.use([](Request& req, Response& res) { ... });
     */
    using Middleware = std::function<bool(Request&, Response&)>;
    void use(Middleware middleware);

private:
    // Vetor com todas as rotas registradas
    std::vector<Route> routes_;

    // Vetor de middlewares a serem executados antes de qualquer handler
    std::vector<Middleware> middlewares_;

    // Busca uma rota que "case" com o request
    // Retorna ponteiro (caso match), ou nullptr se nenhuma corresponder
    const Route* findRoute(const Request& request, 
                           std::unordered_map<std::string, std::string>& params) const;

    // Handler default para caso nenhuma rota seja encontrada (404 Not Found)
    Response notFoundHandler(const Request& request);

    // Handler para erros internos (500 Internal Server Error), usado para exceptions no handler
    Response errorHandler(const std::exception& e);
};

} // namespace Core::Http

