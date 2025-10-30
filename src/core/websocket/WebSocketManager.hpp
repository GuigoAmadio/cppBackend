#pragma once

#include "WebSocketConnection.hpp"
#include <map>
#include <mutex>
#include <thread>
#include <vector>

namespace Core::WebSocket {

/**
 * @brief Gerenciador de conexões WebSocket
 * 
 * Gerencia múltiplas conexões WebSocket ativas:
 * - Adiciona/remove conexões
 * - Broadcasting (enviar para todos)
 * - Rooms/Groups (opcional)
 * - Thread pool para processar frames
 */
class WebSocketManager {
public:
    WebSocketManager();
    ~WebSocketManager();
    
    /**
     * @brief Adiciona nova conexão
     * @param connection Conexão estabelecida
     * @return ID da conexão
     */
    std::string addConnection(std::shared_ptr<WebSocketConnection> connection);
    
    /**
     * @brief Remove conexão
     */
    void removeConnection(const std::string& id);
    
    /**
     * @brief Envia mensagem para uma conexão específica
     */
    bool sendTo(const std::string& id, const std::string& message);
    
    /**
     * @brief Broadcasting - envia para todas as conexões ativas
     */
    void broadcast(const std::string& message);
    
    /**
     * @brief Broadcasting exceto para uma conexão
     */
    void broadcastExcept(const std::string& exceptId, const std::string& message);
    
    /**
     * @brief Retorna número de conexões ativas
     */
    size_t getConnectionCount() const;
    
    /**
     * @brief Retorna IDs de todas as conexões ativas
     */
    std::vector<std::string> getConnectionIds() const;
    
    /**
     * @brief Para todas as conexões
     */
    void shutdown();

private:
    mutable std::mutex mutex_;
    std::map<std::string, std::shared_ptr<WebSocketConnection>> connections_;
    std::map<std::string, std::thread> threads_; // Thread por conexão para processar frames
    std::atomic<bool> running_;
    
    /**
     * @brief Loop de processamento de frames para uma conexão
     */
    void connectionLoop(const std::string& id);
};

} // namespace Core::WebSocket

