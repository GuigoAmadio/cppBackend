#pragma once

#include "Message.hpp"
#include <string>
#include <vector>
#include <optional>
#include <functional>

namespace Core::Queue {

/**
 * @brief Interface abstrata para Message Queue
 * 
 * Permite implementações diferentes (Redis Streams, RabbitMQ, Kafka, etc)
 * sem alterar o código que usa a fila
 */
class MessageQueue {
public:
    virtual ~MessageQueue() = default;
    
    // ========================================
    // PRODUCER OPERATIONS
    // ========================================
    
    /**
     * @brief Publica uma mensagem na fila
     * @param stream Nome do stream/queue
     * @param data Dados da mensagem (chave-valor)
     * @return ID da mensagem publicada ou empty se erro
     */
    virtual std::string publish(const std::string& stream, 
                                const std::map<std::string, std::string>& data) = 0;
    
    /**
     * @brief Publica mensagem com payload JSON simples
     * @param stream Nome do stream/queue
     * @param payload Payload da mensagem (string)
     * @return ID da mensagem publicada
     */
    virtual std::string publishSimple(const std::string& stream, 
                                      const std::string& payload) {
        return publish(stream, {{"payload", payload}});
    }
    
    // ========================================
    // CONSUMER GROUP OPERATIONS
    // ========================================
    
    /**
     * @brief Cria ou verifica consumer group
     * @param stream Nome do stream
     * @param group Nome do group
     * @param startId ID inicial ("$" = novos, "0" = desde início)
     * @return true se criado ou já existe
     */
    virtual bool createConsumerGroup(const std::string& stream,
                                    const std::string& group,
                                    const std::string& startId = "$") = 0;
    
    /**
     * @brief Consome mensagens do consumer group
     * @param config Configuração do consumer group
     * @return Lista de mensagens
     */
    virtual std::vector<Message> consume(const ConsumerGroupConfig& config) = 0;
    
    /**
     * @brief Confirma processamento de mensagens (ACK)
     * @param stream Nome do stream
     * @param group Nome do consumer group
     * @param messageIds Lista de IDs de mensagens
     * @return Número de mensagens confirmadas
     */
    virtual int acknowledge(const std::string& stream,
                           const std::string& group,
                           const std::vector<std::string>& messageIds) = 0;
    
    /**
     * @brief Confirma uma única mensagem
     */
    virtual int acknowledge(const std::string& stream,
                           const std::string& group,
                           const std::string& messageId) {
        return acknowledge(stream, group, {messageId});
    }
    
    // ========================================
    // MONITORING & MANAGEMENT
    // ========================================
    
    /**
     * @brief Lista mensagens pendentes (não confirmadas)
     * @param stream Nome do stream
     * @param group Nome do consumer group
     * @param count Número máximo de mensagens
     * @return Lista de mensagens pendentes
     */
    virtual std::vector<Message> getPendingMessages(const std::string& stream,
                                                    const std::string& group,
                                                    int count = 10) = 0;
    
    /**
     * @brief Reclama mensagens pendentes de outros consumers (para retry)
     * @param stream Nome do stream
     * @param group Nome do consumer group
     * @param consumer Novo consumer
     * @param minIdleMs Tempo mínimo de idle
     * @param messageIds IDs das mensagens a reclamar
     * @return Mensagens reclamadas
     */
    virtual std::vector<Message> claimPendingMessages(const std::string& stream,
                                                      const std::string& group,
                                                      const std::string& consumer,
                                                      long long minIdleMs,
                                                      const std::vector<std::string>& messageIds) = 0;
    
    /**
     * @brief Obtém tamanho do stream/queue
     * @param stream Nome do stream
     * @return Número de mensagens
     */
    virtual long long getQueueLength(const std::string& stream) = 0;
    
    /**
     * @brief Remove mensagens do stream
     * @param stream Nome do stream
     * @param messageIds IDs das mensagens a remover
     * @return Número de mensagens removidas
     */
    virtual int deleteMessages(const std::string& stream,
                              const std::vector<std::string>& messageIds) = 0;
    
    // ========================================
    // UTILITY
    // ========================================
    
    /**
     * @brief Verifica se a conexão está ativa
     */
    virtual bool isConnected() const = 0;
    
    /**
     * @brief Obtém estatísticas da fila
     */
    virtual std::map<std::string, long long> getStats(const std::string& stream) {
        return {
            {"length", getQueueLength(stream)}
        };
    }
};

} // namespace Core::Queue

