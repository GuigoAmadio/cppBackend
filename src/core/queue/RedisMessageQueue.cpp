#include "RedisMessageQueue.hpp"
#include "../utils/LoggerNew.hpp"
#include <sstream>
#include <chrono>

namespace Core::Queue {

RedisMessageQueue::RedisMessageQueue(std::shared_ptr<Core::Cache::RedisPool> redisPool,
                                     RetryConfig retryConfig)
    : redisPool_(redisPool), retryConfig_(retryConfig) {
    if (!redisPool_) {
        throw std::invalid_argument("RedisPool cannot be null");
    }
    LOG_INFO("[RedisMessageQueue] Initialized with retry config: " +
             std::to_string(retryConfig_.maxRetries) + " retries");
}

// ========================================
// PRODUCER OPERATIONS
// ========================================

std::string RedisMessageQueue::publish(const std::string& stream, 
                                       const std::map<std::string, std::string>& data) {
    auto client = redisPool_->acquire();
    if (!client) {
        LOG_ERROR("[RedisMessageQueue] Failed to acquire Redis client");
        return "";
    }
    
    std::string messageId = client->xadd(stream, data);
    redisPool_->release(std::move(client));
    
    if (!messageId.empty()) {
        LOG_DEBUG("[RedisMessageQueue] Published message to " + stream + ": " + messageId);
    }
    
    return messageId;
}

// ========================================
// CONSUMER GROUP OPERATIONS
// ========================================

bool RedisMessageQueue::createConsumerGroup(const std::string& stream,
                                           const std::string& group,
                                           const std::string& startId) {
    auto client = redisPool_->acquire();
    if (!client) {
        LOG_ERROR("[RedisMessageQueue] Failed to acquire Redis client");
        return false;
    }
    
    bool success = client->xgroupCreate(stream, group, startId, true);
    redisPool_->release(std::move(client));
    
    if (success) {
        LOG_INFO("[RedisMessageQueue] Consumer group created: " + stream + " -> " + group);
    }
    
    return success;
}

std::vector<Message> RedisMessageQueue::consume(const ConsumerGroupConfig& config) {
    std::vector<Message> messages;
    
    auto client = redisPool_->acquire();
    if (!client) {
        LOG_ERROR("[RedisMessageQueue] Failed to acquire Redis client");
        return messages;
    }
    
    // Garantir que consumer group existe
    if (config.autoCreateGroup) {
        client->xgroupCreate(config.streamName, config.groupName, config.startId, true);
    }
    
    // XREADGROUP GROUP group consumer BLOCK ms COUNT n STREAMS stream >
    auto result = client->xreadgroup(
        config.groupName,
        config.consumerName,
        {{config.streamName, ">"}},  // ">" = apenas novas mensagens pendentes
        config.blockTimeMs,
        config.batchSize
    );
    
    redisPool_->release(std::move(client));
    
    // Parse resultados
    for (const auto& [streamName, streamMessages] : result) {
        for (const auto& [msgId, data] : streamMessages) {
            Message msg = parseMessage(streamName, msgId, data);
            messages.push_back(msg);
            
            LOG_DEBUG("[RedisMessageQueue] Consumed message: " + msgId + 
                     " from " + streamName + " (group: " + config.groupName + ")");
        }
    }
    
    return messages;
}

int RedisMessageQueue::acknowledge(const std::string& stream,
                                  const std::string& group,
                                  const std::vector<std::string>& messageIds) {
    if (messageIds.empty()) {
        return 0;
    }
    
    auto client = redisPool_->acquire();
    if (!client) {
        LOG_ERROR("[RedisMessageQueue] Failed to acquire Redis client");
        return 0;
    }
    
    int acked = client->xack(stream, group, messageIds);
    redisPool_->release(std::move(client));
    
    LOG_DEBUG("[RedisMessageQueue] Acknowledged " + std::to_string(acked) + 
             " messages in " + stream);
    
    return acked;
}

// ========================================
// MONITORING & MANAGEMENT
// ========================================

std::vector<Message> RedisMessageQueue::getPendingMessages(const std::string& stream,
                                                          const std::string& group,
                                                          int count) {
    std::vector<Message> messages;
    
    auto client = redisPool_->acquire();
    if (!client) {
        return messages;
    }
    
    auto pending = client->xpending(stream, group, count);
    redisPool_->release(std::move(client));
    
    for (const auto& [id, consumer, idleTime, deliveryCount] : pending) {
        Message msg;
        msg.id = id;
        msg.stream = stream;
        msg.deliveryCount = deliveryCount;
        msg.idleTimeMs = idleTime;
        messages.push_back(msg);
    }
    
    return messages;
}

std::vector<Message> RedisMessageQueue::claimPendingMessages(const std::string& stream,
                                                             const std::string& group,
                                                             const std::string& consumer,
                                                             long long minIdleMs,
                                                             const std::vector<std::string>& messageIds) {
    std::vector<Message> messages;
    
    if (messageIds.empty()) {
        return messages;
    }
    
    auto client = redisPool_->acquire();
    if (!client) {
        return messages;
    }
    
    auto claimed = client->xclaim(stream, group, consumer, minIdleMs, messageIds);
    redisPool_->release(std::move(client));
    
    for (const auto& [msgId, data] : claimed) {
        Message msg = parseMessage(stream, msgId, data);
        messages.push_back(msg);
        
        LOG_INFO("[RedisMessageQueue] Claimed message " + msgId + " for retry");
    }
    
    return messages;
}

long long RedisMessageQueue::getQueueLength(const std::string& stream) {
    auto client = redisPool_->acquire();
    if (!client) {
        return 0;
    }
    
    long long length = client->xlen(stream);
    redisPool_->release(std::move(client));
    
    return length;
}

int RedisMessageQueue::deleteMessages(const std::string& stream,
                                     const std::vector<std::string>& messageIds) {
    if (messageIds.empty()) {
        return 0;
    }
    
    auto client = redisPool_->acquire();
    if (!client) {
        return 0;
    }
    
    int deleted = client->xdel(stream, messageIds);
    redisPool_->release(std::move(client));
    
    return deleted;
}

bool RedisMessageQueue::isConnected() const {
    auto client = redisPool_->acquire();
    if (!client) {
        return false;
    }
    
    bool connected = client->isConnected();
    redisPool_->release(std::move(client));
    
    return connected;
}

std::map<std::string, long long> RedisMessageQueue::getStats(const std::string& stream) {
    std::map<std::string, long long> stats;
    
    auto client = redisPool_->acquire();
    if (!client) {
        return stats;
    }
    
    stats["length"] = client->xlen(stream);
    stats["dlq_length"] = client->xlen(getDLQStreamName(stream));
    
    redisPool_->release(std::move(client));
    
    return stats;
}

// ========================================
// RETRY & DLQ
// ========================================

std::string RedisMessageQueue::moveToDLQ(const Message& message, const std::string& reason) {
    std::string dlqStream = getDLQStreamName(message.stream);
    
    // Adicionar metadata do DLQ
    auto dlqData = message.data;
    dlqData["_dlq_reason"] = reason;
    dlqData["_dlq_original_id"] = message.id;
    dlqData["_dlq_original_stream"] = message.stream;
    dlqData["_dlq_timestamp"] = std::to_string(
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
    );
    dlqData["_dlq_delivery_count"] = std::to_string(message.deliveryCount);
    
    std::string dlqId = publish(dlqStream, dlqData);
    
    if (!dlqId.empty()) {
        LOG_WARNING("[RedisMessageQueue] Message moved to DLQ: " + message.id + 
                   " -> " + dlqStream + " (reason: " + reason + ")");
    }
    
    return dlqId;
}

std::vector<Message> RedisMessageQueue::processPendingForRetry(const std::string& stream,
                                                               const std::string& group,
                                                               const std::string& consumer,
                                                               long long minIdleMs) {
    std::vector<Message> retriedMessages;
    
    // 1. Obter mensagens pendentes
    auto pendingList = getPendingMessages(stream, group, 100);
    
    if (pendingList.empty()) {
        return retriedMessages;
    }
    
    LOG_DEBUG("[RedisMessageQueue] Found " + std::to_string(pendingList.size()) + 
             " pending messages in " + stream);
    
    // 2. Filtrar mensagens que devem ser retentadas
    std::vector<std::string> idsToRetry;
    std::vector<std::string> idsToDLQ;
    
    for (const auto& pending : pendingList) {
        // Ignorar mensagens recentemente processadas
        if (pending.idleTimeMs < minIdleMs) {
            continue;
        }
        
        // Verificar se excedeu tentativas máximas
        if (pending.deliveryCount >= retryConfig_.maxRetries) {
            idsToDLQ.push_back(pending.id);
        } else {
            idsToRetry.push_back(pending.id);
        }
    }
    
    // 3. Mover mensagens com muitas falhas para DLQ
    if (!idsToDLQ.empty()) {
        // Precisamos buscar os dados completos das mensagens
        auto client = redisPool_->acquire();
        if (client) {
            for (const auto& id : idsToDLQ) {
                // Reclamar a mensagem para obter seus dados
                auto claimed = client->xclaim(stream, group, consumer, minIdleMs, {id});
                if (!claimed.empty()) {
                    Message msg = parseMessage(stream, claimed[0].first, claimed[0].second);
                    msg.deliveryCount = retryConfig_.maxRetries;
                    
                    // Mover para DLQ
                    moveToDLQ(msg, "max_retries_exceeded");
                    
                    // Confirmar (remover do pending)
                    acknowledge(stream, group, {id});
                }
            }
            redisPool_->release(std::move(client));
        }
        
        LOG_INFO("[RedisMessageQueue] Moved " + std::to_string(idsToDLQ.size()) + 
                " messages to DLQ (max retries exceeded)");
    }
    
    // 4. Reclamar mensagens para retry
    if (!idsToRetry.empty()) {
        retriedMessages = claimPendingMessages(stream, group, consumer, minIdleMs, idsToRetry);
        
        LOG_INFO("[RedisMessageQueue] Claimed " + std::to_string(retriedMessages.size()) + 
                " messages for retry");
    }
    
    return retriedMessages;
}

// ========================================
// PRIVATE METHODS
// ========================================

Message RedisMessageQueue::parseMessage(const std::string& stream,
                                       const std::string& id,
                                       const std::map<std::string, std::string>& data) const {
    Message msg(id, stream, data);
    
    // Extrair retry count se presente
    msg.deliveryCount = getRetryCount(data);
    
    return msg;
}

std::map<std::string, std::string> RedisMessageQueue::addRetryMetadata(
    const std::map<std::string, std::string>& data,
    int retryCount) const {
    
    auto enrichedData = data;
    enrichedData["_retry_count"] = std::to_string(retryCount);
    enrichedData["_retry_timestamp"] = std::to_string(
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
    );
    
    return enrichedData;
}

int RedisMessageQueue::getRetryCount(const std::map<std::string, std::string>& data) const {
    auto it = data.find("_retry_count");
    if (it != data.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

} // namespace Core::Queue

