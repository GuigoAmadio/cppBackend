#pragma once

#include <string>
#include <map>
#include <chrono>
#include <optional>
#include <functional>
#include <cmath>

namespace Core::Queue {

/**
 * @brief Representa uma mensagem na fila
 */
struct Message {
    std::string id;                               // ID único da mensagem (ex: "1234567890-0")
    std::string stream;                           // Nome do stream/queue
    std::map<std::string, std::string> data;      // Dados da mensagem (chave-valor)
    std::chrono::system_clock::time_point timestamp; // Timestamp de criação
    int deliveryCount = 0;                        // Número de tentativas de entrega
    long long idleTimeMs = 0;                     // Tempo em ms que ficou pendente
    
    /**
     * @brief Construtor padrão
     */
    Message() = default;
    
    /**
     * @brief Construtor com ID e dados
     */
    Message(const std::string& msgId, 
            const std::string& streamName,
            const std::map<std::string, std::string>& msgData)
        : id(msgId),
          stream(streamName),
          data(msgData),
          timestamp(std::chrono::system_clock::now()),
          deliveryCount(0),
          idleTimeMs(0) {}
    
    /**
     * @brief Obtém valor de um campo
     */
    std::optional<std::string> get(const std::string& key) const {
        auto it = data.find(key);
        if (it != data.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    /**
     * @brief Define valor de um campo
     */
    void set(const std::string& key, const std::string& value) {
        data[key] = value;
    }
    
    /**
     * @brief Verifica se contém um campo
     */
    bool has(const std::string& key) const {
        return data.find(key) != data.end();
    }
};

/**
 * @brief Configurações de retry para mensagens
 */
struct RetryConfig {
    int maxRetries = 3;                           // Número máximo de tentativas
    int initialDelayMs = 1000;                    // Delay inicial (1s)
    int maxDelayMs = 30000;                       // Delay máximo (30s)
    double backoffMultiplier = 2.0;               // Multiplicador exponencial
    
    /**
     * @brief Calcula delay para uma tentativa específica
     */
    int getDelayMs(int attemptNumber) const {
        if (attemptNumber <= 0) return initialDelayMs;
        
        int delay = static_cast<int>(initialDelayMs * 
                                     std::pow(backoffMultiplier, attemptNumber - 1));
        return std::min(delay, maxDelayMs);
    }
};

/**
 * @brief Configurações de um Consumer Group
 */
struct ConsumerGroupConfig {
    std::string streamName;                       // Nome do stream
    std::string groupName;                        // Nome do consumer group
    std::string consumerName;                     // Nome do consumer
    int blockTimeMs = 5000;                       // Tempo de bloqueio no XREADGROUP (5s)
    int batchSize = 10;                           // Número de mensagens por batch
    bool autoCreateGroup = true;                  // Criar group automaticamente
    std::string startId = "$";                    // ID inicial ("$" = novos, "0" = desde início)
    
    ConsumerGroupConfig() = default;
    
    ConsumerGroupConfig(const std::string& stream, 
                       const std::string& group,
                       const std::string& consumer = "worker-1")
        : streamName(stream),
          groupName(group),
          consumerName(consumer) {}
};

/**
 * @brief Status de processamento de uma mensagem
 */
enum class MessageStatus {
    Pending,      // Aguardando processamento
    Processing,   // Sendo processada
    Completed,    // Processada com sucesso
    Failed,       // Falhou após todas as tentativas
    Retrying      // Aguardando retry
};

/**
 * @brief Callback para processar mensagem
 * @return true se processou com sucesso, false para retry
 */
using MessageHandler = std::function<bool(const Message& message)>;

/**
 * @brief Callback para erros
 */
using ErrorHandler = std::function<void(const Message& message, const std::string& error)>;

} // namespace Core::Queue

