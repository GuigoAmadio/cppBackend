#include "WebSocketManager.hpp"
#include "../utils/LoggerNew.hpp"
#include <sstream>
#include <chrono>

namespace Core::WebSocket {

WebSocketManager::WebSocketManager() : running_(true) {
    LOG_INFO("WebSocketManager: Initialized");
}

WebSocketManager::~WebSocketManager() {
    shutdown();
}

std::string WebSocketManager::addConnection(std::shared_ptr<WebSocketConnection> connection) {
    if (!connection) {
        return "";
    }
    
    const std::string& id = connection->getId();
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_[id] = connection;
    }
    
    // Criar thread para processar frames desta conexão
    threads_[id] = std::thread(&WebSocketManager::connectionLoop, this, id);
    
    LOG_INFO("WebSocketManager: Connection added - " + id);
    
    return id;
}

void WebSocketManager::removeConnection(const std::string& id) {
    std::shared_ptr<WebSocketConnection> connection;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = connections_.find(id);
        if (it != connections_.end()) {
            connection = it->second;
            connections_.erase(it);
        }
    }
    
    if (connection) {
        connection->forceClose();
    }
    
    // ✅ Aguardar thread terminar (APENAS se não for a própria thread)
    auto threadIt = threads_.find(id);
    if (threadIt != threads_.end()) {
        if (threadIt->second.joinable()) {
            // ✅ VERIFICAR se não estamos tentando join na própria thread (deadlock!)
            if (threadIt->second.get_id() != std::this_thread::get_id()) {
                threadIt->second.join();
                LOG_DEBUG("WebSocketManager: Thread joined - " + id);
            } else {
                // Thread está se removendo a si mesma - detach para evitar deadlock
                threadIt->second.detach();
                LOG_DEBUG("WebSocketManager: Thread detached (self-removal) - " + id);
            }
        }
        threads_.erase(threadIt);
    }
    
    LOG_INFO("WebSocketManager: Connection removed - " + id);
}

bool WebSocketManager::sendTo(const std::string& id, const std::string& message) {
    std::shared_ptr<WebSocketConnection> connection;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = connections_.find(id);
        if (it != connections_.end()) {
            connection = it->second;
        }
    }
    
    if (connection && connection->isOpen()) {
        return connection->sendText(message);
    }
    
    return false;
}

void WebSocketManager::broadcast(const std::string& message) {
    std::vector<std::shared_ptr<WebSocketConnection>> activeConnections;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& pair : connections_) {
            if (pair.second->isOpen()) {
                activeConnections.push_back(pair.second);
            }
        }
    }
    
    for (const auto& conn : activeConnections) {
        conn->sendText(message);
    }
    
    LOG_DEBUG("WebSocketManager: Broadcast to " + std::to_string(activeConnections.size()) + " connections");
}

void WebSocketManager::broadcastExcept(const std::string& exceptId, const std::string& message) {
    std::vector<std::shared_ptr<WebSocketConnection>> activeConnections;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& pair : connections_) {
            if (pair.first != exceptId && pair.second->isOpen()) {
                activeConnections.push_back(pair.second);
            }
        }
    }
    
    for (const auto& conn : activeConnections) {
        conn->sendText(message);
    }
    
    LOG_DEBUG("WebSocketManager: Broadcast to " + std::to_string(activeConnections.size()) + " connections (except " + exceptId + ")");
}

size_t WebSocketManager::getConnectionCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connections_.size();
}

std::vector<std::string> WebSocketManager::getConnectionIds() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> ids;
    ids.reserve(connections_.size());
    
    for (const auto& pair : connections_) {
        ids.push_back(pair.first);
    }
    
    return ids;
}

void WebSocketManager::shutdown() {
    running_ = false;
    
    std::vector<std::string> ids;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& pair : connections_) {
            ids.push_back(pair.first);
        }
    }
    
    for (const auto& id : ids) {
        removeConnection(id);
    }
    
    LOG_INFO("WebSocketManager: Shutdown complete");
}

void WebSocketManager::connectionLoop(const std::string& id) {
    LOG_DEBUG("WebSocketManager: Connection loop started - " + id);
    
    std::shared_ptr<WebSocketConnection> connection;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = connections_.find(id);
        if (it != connections_.end()) {
            connection = it->second;
        }
    }
    
    if (!connection) {
        LOG_ERROR("WebSocketManager: Connection not found - " + id);
        return;
    }
    
    while (running_ && connection->isOpen()) {
        try {
            if (!connection->receiveFrame()) {
                // Conexão fechada ou erro
                break;
            }
        } catch (const std::exception& e) {
            LOG_ERROR("WebSocketManager: Error processing frame - " + std::string(e.what()));
            break;
        }
    }
    
    // Remover conexão ao sair do loop
    removeConnection(id);
    
    LOG_DEBUG("WebSocketManager: Connection loop ended - " + id);
}

} // namespace Core::WebSocket

