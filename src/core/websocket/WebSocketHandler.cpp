#include "WebSocketHandler.hpp"
#include "../utils/LoggerNew.hpp"
#include <sstream>
#include <chrono>
#include <algorithm>

namespace Core::WebSocket {

WebSocketHandler::WebSocketHandler(std::shared_ptr<WebSocketManager> manager)
    : manager_(manager), nextClientId_(1) {
    if (!manager_) {
        throw std::invalid_argument("WebSocketManager cannot be null");
    }
    LOG_INFO("WebSocketHandler: Initialized");
}

void WebSocketHandler::setEventHandlers(const WebSocketEventHandlers& handlers) {
    handlers_ = handlers;
}

bool WebSocketHandler::handleUpgrade(socket_t socket, const Request& request) {
    // Verificar se é upgrade request
    if (!isWebSocketUpgrade(request)) {
        return false;
    }
    
    // ✅ RECONSTRUIR HTTP request a partir do objeto Request já parseado
    // (Server.cpp já leu o socket, não podemos ler novamente!)
    std::ostringstream httpRequestStream;
    
    // Request line: GET /ws HTTP/1.1
    httpRequestStream << request.methodToString() << " " 
                     << request.getPath() << " HTTP/1.1\r\n";
    
    // Headers
    auto headers = request.getHeaders();
    for (const auto& [key, value] : headers) {
        httpRequestStream << key << ": " << value << "\r\n";
    }
    
    // Empty line (fim dos headers)
    httpRequestStream << "\r\n";
    
    std::string httpRequest = httpRequestStream.str();
    
    LOG_DEBUG("WebSocketHandler: Reconstructed HTTP request:\n" + httpRequest);
    
    // Criar conexão WebSocket
    std::string clientId = generateClientId();
    auto connection = std::make_shared<WebSocketConnection>(socket, clientId);
    
    // Configurar callbacks
    connection->setOnMessage([this, clientId](const std::string& message) {
        if (handlers_.onMessage) {
            handlers_.onMessage(clientId, message);
        }
    });
    
    connection->setOnClose([this, clientId](uint16_t code, const std::string& reason) {
        if (handlers_.onDisconnect) {
            handlers_.onDisconnect(clientId, code, reason);
        }
        // ✅ NÃO remover aqui - connectionLoop já remove automaticamente ao terminar
        // Evita deadlock: callback é chamado dentro de receiveFrame() que está no connectionLoop
    });
    
    connection->setOnError([this, clientId](const std::string& error) {
        if (handlers_.onError) {
            handlers_.onError(clientId, error);
        }
    });
    
    // Realizar handshake
    if (!connection->performHandshake(httpRequest)) {
        LOG_ERROR("WebSocketHandler: Handshake failed for client " + clientId);
        return false;
    }
    
    // Adicionar ao manager
    manager_->addConnection(connection);
    
    // Callback de conexão
    if (handlers_.onConnect) {
        handlers_.onConnect(clientId);
    }
    
    LOG_INFO("WebSocketHandler: Client " + clientId + " upgraded to WebSocket");
    
    return true;
}

bool WebSocketHandler::sendTo(const std::string& clientId, const std::string& message) {
    return manager_->sendTo(clientId, message);
}

void WebSocketHandler::broadcast(const std::string& message) {
    manager_->broadcast(message);
}

void WebSocketHandler::broadcastExcept(const std::string& exceptId, const std::string& message) {
    manager_->broadcastExcept(exceptId, message);
}

size_t WebSocketHandler::getConnectionCount() const {
    return manager_->getConnectionCount();
}

std::string WebSocketHandler::generateClientId() {
    auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    uint64_t id = nextClientId_.fetch_add(1);
    
    std::ostringstream oss;
    oss << "ws_" << timestamp << "_" << id;
    return oss.str();
}

bool WebSocketHandler::isWebSocketUpgrade(const Request& request) {
    // Verificar headers necessários para WebSocket upgrade
    std::string upgrade = request.getHeader("Upgrade");
    std::string connection = request.getHeader("Connection");
    std::string wsKey = request.getHeader("Sec-WebSocket-Key");
    
    // Case-insensitive check
    std::transform(upgrade.begin(), upgrade.end(), upgrade.begin(), ::tolower);
    std::transform(connection.begin(), connection.end(), connection.begin(), ::tolower);
    
    return (upgrade == "websocket" && 
            connection.find("upgrade") != std::string::npos &&
            !wsKey.empty());
}

std::string WebSocketHandler::readHttpRequest(socket_t socket) {
    char buffer[4096];
    std::string request;
    
    // Ler até encontrar \r\n\r\n (fim dos headers HTTP)
    while (true) {
        ssize_t bytesRead = recv(socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead <= 0) {
            return "";
        }
        
        buffer[bytesRead] = '\0';
        request += buffer;
        
        // Verificar se temos headers completos
        if (request.find("\r\n\r\n") != std::string::npos) {
            break;
        }
        
        // Timeout safety
        if (request.size() > 8192) {
            LOG_WARNING("WebSocketHandler: HTTP request too large");
            break;
        }
    }
    
    return request;
}

} // namespace Core::WebSocket

