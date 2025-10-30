#include "QueueWorker.hpp"
#include "../utils/LoggerNew.hpp"
#include <chrono>
#include <thread>

namespace Core::Queue {

QueueWorker::QueueWorker(std::shared_ptr<RedisMessageQueue> messageQueue,
                         ConsumerGroupConfig config,
                         MessageHandler handler,
                         ErrorHandler errorHandler)
    : messageQueue_(messageQueue),
      config_(std::move(config)),
      handler_(std::move(handler)),
      errorHandler_(std::move(errorHandler)),
      running_(false),
      stopRequested_(false) {
    
    if (!messageQueue_) {
        throw std::invalid_argument("MessageQueue cannot be null");
    }
    
    if (!handler_) {
        throw std::invalid_argument("MessageHandler cannot be null");
    }
    
    // Garantir que consumer group existe
    messageQueue_->createConsumerGroup(config_.streamName, config_.groupName, config_.startId);
    
    LOG_INFO("[QueueWorker] Initialized for stream: " + config_.streamName + 
             ", group: " + config_.groupName + ", consumer: " + config_.consumerName);
}

QueueWorker::~QueueWorker() {
    stop();
}

void QueueWorker::start() {
    if (running_) {
        LOG_WARNING("[QueueWorker] Already running");
        return;
    }
    
    stopRequested_ = false;
    running_ = true;
    
    workerThread_ = std::make_unique<std::thread>([this]() {
        workerLoop();
    });
    
    LOG_INFO("[QueueWorker] Started worker thread for " + config_.streamName);
}

void QueueWorker::stop() {
    if (!running_) {
        return;
    }
    
    LOG_INFO("[QueueWorker] Stopping worker for " + config_.streamName + "...");
    
    stopRequested_ = true;
    
    if (workerThread_ && workerThread_->joinable()) {
        workerThread_->join();
    }
    
    running_ = false;
    
    LOG_INFO("[QueueWorker] Worker stopped. Final stats: " + stats_.toString());
}

void QueueWorker::workerLoop() {
    LOG_INFO("[QueueWorker] Worker loop started");
    
    int consecutiveEmptyPolls = 0;
    const int maxEmptyPollsBeforeRetryCheck = 5; // Após 5 polls vazios, verifica pending
    
    while (!stopRequested_) {
        try {
            // 1. Consumir novas mensagens
            auto messages = messageQueue_->consume(config_);
            
            if (messages.empty()) {
                consecutiveEmptyPolls++;
                
                // Após vários polls vazios, processar mensagens pendentes
                if (consecutiveEmptyPolls >= maxEmptyPollsBeforeRetryCheck) {
                    processPendingMessages();
                    consecutiveEmptyPolls = 0;
                }
                
                // Pequeno sleep se não há mensagens (evita busy-wait)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            
            consecutiveEmptyPolls = 0;
            
            // 2. Processar cada mensagem
            for (const auto& message : messages) {
                stats_.processed++;
                
                bool success = processMessage(message);
                
                if (success) {
                    // ACK - mensagem processada com sucesso
                    messageQueue_->acknowledge(config_.streamName, config_.groupName, {message.id});
                    stats_.succeeded++;
                    
                    LOG_DEBUG("[QueueWorker] Message processed successfully: " + message.id);
                } else {
                    // NACK - falha, será retentada automaticamente
                    // (não damos ACK, ela fica pending)
                    stats_.failed++;
                    
                    LOG_WARNING("[QueueWorker] Message processing failed (will retry): " + message.id);
                }
            }
            
        } catch (const std::exception& e) {
            LOG_ERROR("[QueueWorker] Exception in worker loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Backoff em caso de erro
        }
    }
    
    LOG_INFO("[QueueWorker] Worker loop exited");
}

bool QueueWorker::processMessage(const Message& message) {
    try {
        // Chamar handler do usuário
        bool success = handler_(message);
        return success;
        
    } catch (const std::exception& e) {
        std::string error = std::string("Exception: ") + e.what();
        LOG_ERROR("[QueueWorker] Error processing message " + message.id + ": " + error);
        
        // Chamar error handler se fornecido
        if (errorHandler_) {
            try {
                errorHandler_(message, error);
            } catch (...) {
                LOG_ERROR("[QueueWorker] Error handler itself threw exception");
            }
        }
        
        return false; // Falha
    } catch (...) {
        LOG_ERROR("[QueueWorker] Unknown exception processing message " + message.id);
        return false;
    }
}

void QueueWorker::processPendingMessages() {
    try {
        // Reclamar mensagens pendentes há mais de 5 minutos (300000 ms)
        // e mover para DLQ se excederam tentativas máximas
        auto retriedMessages = messageQueue_->processPendingForRetry(
            config_.streamName,
            config_.groupName,
            config_.consumerName,
            300000 // 5 minutos
        );
        
        if (!retriedMessages.empty()) {
            LOG_INFO("[QueueWorker] Processing " + std::to_string(retriedMessages.size()) + 
                    " pending messages for retry");
            
            stats_.retried += retriedMessages.size();
            
            // Processar mensagens reclamadas
            for (const auto& message : retriedMessages) {
                bool success = processMessage(message);
                
                if (success) {
                    messageQueue_->acknowledge(config_.streamName, config_.groupName, {message.id});
                    stats_.succeeded++;
                } else {
                    stats_.failed++;
                    
                    // Verificar se deve mover para DLQ
                    if (message.deliveryCount >= messageQueue_->getRetryConfig().maxRetries - 1) {
                        messageQueue_->moveToDLQ(message, "max_retries_exceeded_after_reclaim");
                        messageQueue_->acknowledge(config_.streamName, config_.groupName, {message.id});
                        stats_.movedToDLQ++;
                        
                        LOG_WARNING("[QueueWorker] Message moved to DLQ after max retries: " + message.id);
                    }
                }
            }
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("[QueueWorker] Error processing pending messages: " + std::string(e.what()));
    }
}

} // namespace Core::Queue

