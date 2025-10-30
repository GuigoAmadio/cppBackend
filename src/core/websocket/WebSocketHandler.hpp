#pragma once

#include "WebSocketManager.hpp"
#include "../http/Request.hpp"
#include "../http/Response.hpp"
#include <functional>
#include <memory>

// Use Request/Response from Http namespace
using Core::Http::Request;
using Core::Http::Response;

namespace Core::WebSocket {

/**
 * @brief Event handlers para WebSocket
 */
struct WebSocketEventHandlers {
    std::function<void(const std::string& clientId)> onConnect;
    std::function<void(const std::string& clientId, const std::string& message)> onMessage;
    std::function<void(const std::string& clientId, uint16_t code, const std::string& reason)> onDisconnect;
    std::function<void(const std::string& clientId, const std::string& error)> onError;
};

/**
 * @brief Handler para WebSocket integration com HTTP Server
 * 
 * Detecta HTTP Upgrade requests e promove para WebSocket
 */
class WebSocketHandler {
public:
    /**
     * @brief Construtor
     * @param manager WebSocketManager compartilhado
     */
    explicit WebSocketHandler(std::shared_ptr<WebSocketManager> manager);
    
    /**
     * @brief Define event handlers
     */
    void setEventHandlers(const WebSocketEventHandlers& handlers);
    
    /**
     * @brief Processa request HTTP
     * @param socket Socket do cliente
     * @param request Request HTTP
     * @return true se foi upgrade para WebSocket, false caso contrário
     */
    bool handleUpgrade(socket_t socket, const Request& request);
    
    /**
     * @brief Envia mensagem para um cliente específico
     */
    bool sendTo(const std::string& clientId, const std::string& message);
    
    /**
     * @brief Broadcasting
     */
    void broadcast(const std::string& message);
    
    /**
     * @brief Broadcasting exceto um cliente
     */
    void broadcastExcept(const std::string& exceptId, const std::string& message);
    
    /**
     * @brief Retorna número de conexões ativas
     */
    size_t getConnectionCount() const;

private:
    std::shared_ptr<WebSocketManager> manager_;
    WebSocketEventHandlers handlers_;
    std::atomic<uint64_t> nextClientId_;
    
    /**
     * @brief Gera ID único para cliente
     */
    std::string generateClientId();
    
    /**
     * @brief Verifica se request é upgrade para WebSocket
     */
    bool isWebSocketUpgrade(const Request& request);
    
    /**
     * @brief Extrai HTTP request raw do socket
     */
    std::string readHttpRequest(socket_t socket);
};

} // namespace Core::WebSocket

