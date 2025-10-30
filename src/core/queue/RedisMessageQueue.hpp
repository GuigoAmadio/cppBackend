#pragma once

#include "MessageQueue.hpp"
#include "../cache/RedisPool.hpp"
#include <memory>
#include <mutex>

namespace Core::Queue {

/**
 * @brief Implementação de MessageQueue usando Redis Streams
 * 
 * Features:
 * - Consumer Groups para processamento distribuído
 * - Retry automático com exponential backoff
 * - Dead Letter Queue (DLQ) para mensagens com falha
 * - Reclaim de mensagens pendentes
 * - Thread-safe
 */
class RedisMessageQueue : public MessageQueue {
public:
    /**
     * @brief Construtor
     * @param redisPool Pool de conexões Redis
     * @param retryConfig Configuração de retry (opcional)
     */
    explicit RedisMessageQueue(std::shared_ptr<Core::Cache::RedisPool> redisPool,
                               RetryConfig retryConfig = RetryConfig{});
    
    ~RedisMessageQueue() override = default;
    
    // ========================================
    // PRODUCER OPERATIONS
    // ========================================
    
    std::string publish(const std::string& stream, 
                       const std::map<std::string, std::string>& data) override;
    
    // ========================================
    // CONSUMER GROUP OPERATIONS
    // ========================================
    
    bool createConsumerGroup(const std::string& stream,
                            const std::string& group,
                            const std::string& startId = "$") override;
    
    std::vector<Message> consume(const ConsumerGroupConfig& config) override;
    
    int acknowledge(const std::string& stream,
                   const std::string& group,
                   const std::vector<std::string>& messageIds) override;
    
    // ========================================
    // MONITORING & MANAGEMENT
    // ========================================
    
    std::vector<Message> getPendingMessages(const std::string& stream,
                                           const std::string& group,
                                           int count = 10) override;
    
    std::vector<Message> claimPendingMessages(const std::string& stream,
                                              const std::string& group,
                                              const std::string& consumer,
                                              long long minIdleMs,
                                              const std::vector<std::string>& messageIds) override;
    
    long long getQueueLength(const std::string& stream) override;
    
    int deleteMessages(const std::string& stream,
                      const std::vector<std::string>& messageIds) override;
    
    bool isConnected() const override;
    
    std::map<std::string, long long> getStats(const std::string& stream) override;
    
    // ========================================
    // RETRY & DLQ
    // ========================================
    
    /**
     * @brief Move mensagem para Dead Letter Queue
     * @param message Mensagem original
     * @param reason Razão da falha
     * @return ID da mensagem no DLQ
     */
    std::string moveToDLQ(const Message& message, const std::string& reason = "max_retries_exceeded");
    
    /**
     * @brief Processa mensagens pendentes para retry
     * @param stream Nome do stream
     * @param group Nome do consumer group
     * @param consumer Nome do consumer atual
     * @param minIdleMs Tempo mínimo de idle (5 minutos = 300000ms)
     * @return Mensagens reclamadas para retry
     */
    std::vector<Message> processPendingForRetry(const std::string& stream,
                                                const std::string& group,
                                                const std::string& consumer,
                                                long long minIdleMs = 300000);
    
    /**
     * @brief Obtém nome do DLQ stream para um stream
     */
    static std::string getDLQStreamName(const std::string& stream) {
        return stream + "-dlq";
    }
    
    /**
     * @brief Obtém configuração de retry
     */
    const RetryConfig& getRetryConfig() const { return retryConfig_; }
    
private:
    std::shared_ptr<Core::Cache::RedisPool> redisPool_;
    RetryConfig retryConfig_;
    mutable std::mutex mutex_;
    
    /**
     * @brief Converte dados Redis para Message
     */
    Message parseMessage(const std::string& stream,
                        const std::string& id,
                        const std::map<std::string, std::string>& data) const;
    
    /**
     * @brief Adiciona metadata de retry à mensagem
     */
    std::map<std::string, std::string> addRetryMetadata(
        const std::map<std::string, std::string>& data,
        int retryCount) const;
    
    /**
     * @brief Extrai retry count da mensagem
     */
    int getRetryCount(const std::map<std::string, std::string>& data) const;
};

} // namespace Core::Queue

