/**
 * 🚀 C++ Backend do ZERO
 * 
 * Este é o ponto de entrada da aplicação.
 * Aqui vamos:
 * 1. Criar o servidor HTTP
 * 2. Configurar rotas
 * 3. Iniciar o servidor
 */

#include "../core/http/Server.hpp"
#include "../core/http/Router.hpp"
#include "../core/http/Request.hpp"
#include "../core/http/Response.hpp"
#include "../core/utils/Logger.hpp"

#include <iostream>
#include <memory>
#include <csignal>

using namespace Core::Http;
using namespace Core::Utils;

// Global server pointer para signal handler
std::unique_ptr<Server> globalServer;

/**
 * Signal handler para Ctrl+C
 */
void signalHandler(int signal) {
    std::cout << "\n🛑 Recebido sinal " << signal << ", parando servidor..." << std::endl;
    if (globalServer) {
        globalServer->stop();
    }
    exit(0);
}

/**
 * Configurar rotas da aplicação
 */
void setupRoutes(Router& router) {
    
    // ==================== HEALTH CHECK ====================
    router.get("/health", [](const Request& req) {
        return Response(StatusCode::OK)
            .text("OK - Server is running!");
    });
    
    // ==================== ROOT ====================
    router.get("/", [](const Request& req) {
        std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>C++ Backend do ZERO</title>
    <style>
        body {
            font-family: 'Segoe UI', Arial, sans-serif;
            max-width: 800px;
            margin: 50px auto;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        .container {
            background: rgba(255,255,255,0.1);
            padding: 30px;
            border-radius: 10px;
            backdrop-filter: blur(10px);
        }
        h1 { font-size: 3em; margin: 0; }
        h2 { color: #ffd700; }
        code {
            background: rgba(0,0,0,0.3);
            padding: 2px 8px;
            border-radius: 4px;
            font-family: 'Courier New', monospace;
        }
        .endpoint {
            background: rgba(0,0,0,0.2);
            padding: 15px;
            margin: 10px 0;
            border-radius: 5px;
            border-left: 4px solid #ffd700;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 C++ Backend</h1>
        <p><strong>100% C++ Puro - Sem Frameworks!</strong></p>
        
        <h2>📡 Endpoints Disponíveis</h2>
        
        <div class="endpoint">
            <strong>GET /health</strong><br>
            Health check do servidor
        </div>
        
        <div class="endpoint">
            <strong>GET /api/hello</strong><br>
            Teste simples de API
        </div>
        
        <div class="endpoint">
            <strong>GET /api/users/:id</strong><br>
            Exemplo de rota com parâmetros
        </div>
        
        <h2>🎯 Como Testar</h2>
        <pre>
curl http://localhost:8080/health
curl http://localhost:8080/api/hello
curl http://localhost:8080/api/users/123
        </pre>
        
        <h2>✨ Features Implementadas</h2>
        <ul>
            <li>✅ HTTP Server do zero (sockets TCP)</li>
            <li>✅ Request parsing manual</li>
            <li>✅ Response building</li>
            <li>✅ URL routing com regex</li>
            <li>✅ Path parameters</li>
            <li>✅ Logging</li>
        </ul>
        
        <p style="margin-top: 40px; opacity: 0.8;">
            <small>Feito com ❤️ em C++ puro</small>
        </p>
    </div>
</body>
</html>
        )";
        
        return Response(StatusCode::OK).html(html);
    });
    
    // ==================== API ====================
    router.get("/api/hello", [](const Request& req) {
        return Response(StatusCode::OK)
            .text("Hello from C++ Backend!");
    });
    
    // ==================== API COM PARAMETROS ====================
    router.get("/api/users/:id", [](const Request& req) {
        std::string userId = req.getParam("id");
        
        std::string response = "User ID: " + userId + "\n";
        response += "Name: John Doe\n";
        response += "Email: john@example.com\n";
        
        return Response(StatusCode::OK).text(response);
    });
    
    // ==================== 404 CUSTOM ====================
    // (Já tratado automaticamente pelo router)
}

/**
 * Entry point
 */
int main(int argc, char* argv[]) {
    try {
        // Configurar logging
        Logger::setLevel(Logger::Level::INFO);
        
        Logger::info("═══════════════════════════════════════");
        Logger::info("  🚀 C++ BACKEND DO ZERO");
        Logger::info("  Sem frameworks, apenas C++ puro!");
        Logger::info("═══════════════════════════════════════");
        
        // Porta (pode ser passada via argumento)
        int port = 8080;
        if (argc > 1) {
            port = std::atoi(argv[1]);
        }
        
        // Criar servidor
        globalServer = std::make_unique<Server>(port);
        
        // Criar e configurar router
        auto router = std::make_shared<Router>();
        setupRoutes(*router);
        
        // Registrar router no servidor
        globalServer->setRouter(router);
        
        // Configurar signal handler para Ctrl+C
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
        
        // Iniciar servidor (bloqueia aqui)
        globalServer->start();
        
    } catch (const std::exception& e) {
        Logger::error(std::string("❌ Erro fatal: ") + e.what());
        return 1;
    }
    
    return 0;
}

