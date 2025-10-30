#pragma once

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <hiredis/hiredis.h>

namespace Core::Cache {

/**
 * @brief Callback quando uma mensagem é recebida
 * @param channel Canal que recebeu a mensagem
 * @param message Conteúdo da mensagem
 */
using PubSubMessageHandler = std::function<void(const std::string& channel, const std::string& message)>;

/**
 * @brief Gerencia Redis Pub/Sub em background thread
 * 
 * Permite subscrever a múltiplos canais e receber mensagens via callbacks.
 * Roda em thread separada para não bloquear a aplicação.
 */
class RedisPubSub {
public:
    /**
     * @brief Construtor
     * @param host Host do Redis
     * @param port Porta do Redis
     * @param timeoutSeconds Timeout de conexão
     */
    RedisPubSub(const std::string& host = "127.0.0.1", int port = 6379, int timeoutSeconds = 5);
    
    /**
     * @brief Destrutor - para thread e desconecta
     */
    ~RedisPubSub();
    
    /**
     * @brief Subscreve a um canal
     * @param channel Nome do canal
     * @param handler Callback para mensagens deste canal
     * @return true se sucesso
     */
    bool subscribe(const std::string& channel, PubSubMessageHandler handler);
    
    /**
     * @brief Remove subscrição de um canal
     * @param channel Nome do canal
     * @return true se sucesso
     */
    bool unsubscribe(const std::string& channel);
    
    /**
     * @brief Publica mensagem em um canal (usa conexão separada)
     * @param channel Nome do canal
     * @param message Mensagem
     * @return Número de subscribers que receberam
     */
    int publish(const std::string& channel, const std::string& message);
    
    /**
     * @brief Inicia thread de listening
     * @return true se sucesso
     */
    bool start();
    
    /**
     * @brief Para thread de listening
     */
    void stop();
    
    /**
     * @brief Verifica se está rodando
     */
    bool isRunning() const { return running_; }
    
    /**
     * @brief Verifica se está conectado
     */
    bool isConnected() const { return subscribeContext_ != nullptr; }
    
    /**
     * @brief Retorna lista de canais subscritos
     */
    std::vector<std::string> getSubscribedChannels() const;

private:
    std::string host_;
    int port_;
    int timeoutSeconds_;
    
    // Contextos Redis
    redisContext* subscribeContext_;  // Para SUBSCRIBE (blocking)
    redisContext* publishContext_;    // Para PUBLISH (non-blocking)
    
    // Thread management
    std::atomic<bool> running_;
    std::atomic<bool> stopRequested_;
    std::thread listenerThread_;
    
    // Handlers por canal
    std::map<std::string, PubSubMessageHandler> handlers_;
    mutable std::mutex handlersMutex_;
    
    /**
     * @brief Loop principal da thread de listening
     */
    void listenLoop();
    
    /**
     * @brief Conecta contexto de subscribe
     */
    bool connectSubscribe();
    
    /**
     * @brief Conecta contexto de publish
     */
    bool connectPublish();
    
    /**
     * @brief Desconecta todos os contextos
     */
    void disconnect();
    
    /**
     * @brief Processa reply de subscribe
     */
    void handleSubscribeReply(redisReply* reply);
};

} // namespace Core::Cache

