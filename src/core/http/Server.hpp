#pragma once

#include <string>
#include <functional>
#include <memory>
#include <atomic>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

namespace Core::Http {

class Request;
class Response;
class Router;

/**
 * HTTP Server - Implementação do zero!
 * 
 * Este é um servidor TCP que:
 * 1. Escuta em uma porta
 * 2. Aceita conexões
 * 3. Parseia requests HTTP
 * 4. Roteia para handlers
 * 5. Envia responses
 */
class Server {
public:
    Server(int port = 8080);
    ~Server();
    
    // Não copiável, não movível (tem resources)
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    
    /**
     * Iniciar servidor
     * Bloqueia até stop() ser chamado
     */
    void start();
    
    /**
     * Parar servidor gracefully
     */
    void stop();
    
    /**
     * Registrar router
     */
    void setRouter(std::shared_ptr<Router> router);
    
    /**
     * Configurar número de threads
     */
    void setThreadCount(int count);
    
private:
    int port_;
    socket_t serverSocket_;
    std::atomic<bool> running_;
    std::shared_ptr<Router> router_;
    int threadCount_;
    
    /**
     * Criar socket e bind
     */
    void createSocket();
    
    /**
     * Loop principal - aceita conexões
     */
    void acceptLoop();
    
    /**
     * Processar uma conexão
     */
    void handleConnection(socket_t clientSocket);
    
    /**
     * Parsear request HTTP
     */
    std::unique_ptr<Request> parseRequest(socket_t socket);
    
    /**
     * Enviar response HTTP
     */
    void sendResponse(socket_t socket, const Response& response);
    
    /**
     * Fechar socket safely
     */
    void closeSocket(socket_t socket);
    
    /**
     * Inicializar Winsock (Windows)
     */
    void initWinsock();
    
    /**
     * Cleanup Winsock (Windows)
     */
    void cleanupWinsock();
};

} // namespace Core::Http

