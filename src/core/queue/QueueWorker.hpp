#pragma once

#include "RedisMessageQueue.hpp"
#include <thread>
#include <atomic>
#include <functional>
#include <memory>

namespace Core::Queue {

/**
 * @brief Worker para processar mensagens de forma assíncrona
 * 
 * Features:
 * - Processa mensagens em thread separada
 * - Retry automático com exponential backoff
 * - Move para DLQ após máximo de tentativas
 * - Reclaim de mensagens pendentes
 * - Start/Stop controlável
 */
class QueueWorker {
public:
    /**
     * @brief Construtor
     * @param messageQueue Message queue compartilhada
     * @param config Configuração do consumer group
     * @param handler Callback para processar mensagens
     * @param errorHandler Callback para erros (opcional)
     */
    QueueWorker(std::shared_ptr<RedisMessageQueue> messageQueue,
                ConsumerGroupConfig config,
                MessageHandler handler,
                ErrorHandler errorHandler = nullptr);
    
    ~QueueWorker();
    
    /**
     * @brief Inicia worker em thread separada
     */
    void start();
    
    /**
     * @brief Para worker gracefully
     */
    void stop();
    
    /**
     * @brief Verifica se está rodando
     */
    bool isRunning() const { return running_; }
    
    /**
     * @brief Obtém estatísticas do worker
     */
    struct Stats {
        long long processed = 0;
        long long succeeded = 0;
        long long failed = 0;
        long long retried = 0;
        long long movedToDLQ = 0;
        
        std::string toString() const {
            return "Processed: " + std::to_string(processed) +
                   ", Succeeded: " + std::to_string(succeeded) +
                   ", Failed: " + std::to_string(failed) +
                   ", Retried: " + std::to_string(retried) +
                   ", DLQ: " + std::to_string(movedToDLQ);
        }
    };
    
    Stats getStats() const {
        return stats_;
    }
    
    void resetStats() {
        stats_ = Stats{};
    }

private:
    std::shared_ptr<RedisMessageQueue> messageQueue_;
    ConsumerGroupConfig config_;
    MessageHandler handler_;
    ErrorHandler errorHandler_;
    
    std::atomic<bool> running_;
    std::atomic<bool> stopRequested_;
    std::unique_ptr<std::thread> workerThread_;
    
    Stats stats_;
    
    /**
     * @brief Loop principal do worker
     */
    void workerLoop();
    
    /**
     * @brief Processa uma mensagem
     * @return true se sucesso, false para retry
     */
    bool processMessage(const Message& message);
    
    /**
     * @brief Processa mensagens pendentes para retry/DLQ
     */
    void processPendingMessages();
};

} // namespace Core::Queue

