#include "RedisPubSub.hpp"
#include "../../core/utils/LoggerNew.hpp"
#include <cstring>

using namespace Core::Cache;

RedisPubSub::RedisPubSub(const std::string& host, int port, int timeoutSeconds)
    : host_(host)
    , port_(port)
    , timeoutSeconds_(timeoutSeconds)
    , subscribeContext_(nullptr)
    , publishContext_(nullptr)
    , running_(false)
    , stopRequested_(false)
{
}

RedisPubSub::~RedisPubSub() {
    stop();
    disconnect();
}

bool RedisPubSub::connectSubscribe() {
    if (subscribeContext_) {
        return true;  // Já conectado
    }
    
    timeval timeout = { timeoutSeconds_, 0 };
    subscribeContext_ = redisConnectWithTimeout(host_.c_str(), port_, timeout);
    
    if (!subscribeContext_ || subscribeContext_->err) {
        if (subscribeContext_) {
            LOG_ERROR("[RedisPubSub] Subscribe connection error: " + std::string(subscribeContext_->errstr));
            redisFree(subscribeContext_);
            subscribeContext_ = nullptr;
        } else {
            LOG_ERROR("[RedisPubSub] Subscribe connection error: Can't allocate redis context");
        }
        return false;
    }
    
    LOG_DEBUG("[RedisPubSub] Subscribe context connected to " + host_ + ":" + std::to_string(port_));
    return true;
}

bool RedisPubSub::connectPublish() {
    if (publishContext_) {
        return true;  // Já conectado
    }
    
    timeval timeout = { timeoutSeconds_, 0 };
    publishContext_ = redisConnectWithTimeout(host_.c_str(), port_, timeout);
    
    if (!publishContext_ || publishContext_->err) {
        if (publishContext_) {
            LOG_ERROR("[RedisPubSub] Publish connection error: " + std::string(publishContext_->errstr));
            redisFree(publishContext_);
            publishContext_ = nullptr;
        } else {
            LOG_ERROR("[RedisPubSub] Publish connection error: Can't allocate redis context");
        }
        return false;
    }
    
    LOG_DEBUG("[RedisPubSub] Publish context connected to " + host_ + ":" + std::to_string(port_));
    return true;
}

void RedisPubSub::disconnect() {
    if (subscribeContext_) {
        redisFree(subscribeContext_);
        subscribeContext_ = nullptr;
    }
    
    if (publishContext_) {
        redisFree(publishContext_);
        publishContext_ = nullptr;
    }
}

bool RedisPubSub::subscribe(const std::string& channel, PubSubMessageHandler handler) {
    {
        std::lock_guard<std::mutex> lock(handlersMutex_);
        handlers_[channel] = handler;
    }
    
    // Se já está rodando, enviar SUBSCRIBE na conexão existente
    if (running_ && subscribeContext_) {
        redisReply* reply = (redisReply*)redisCommand(subscribeContext_, "SUBSCRIBE %s", channel.c_str());
        if (!reply) {
            LOG_ERROR("[RedisPubSub] Failed to subscribe to " + channel);
            return false;
        }
        freeReplyObject(reply);
        LOG_INFO("[RedisPubSub] Subscribed to channel: " + channel);
        return true;
    }
    
    LOG_DEBUG("[RedisPubSub] Channel registered: " + channel + " (will subscribe when started)");
    return true;
}

bool RedisPubSub::unsubscribe(const std::string& channel) {
    {
        std::lock_guard<std::mutex> lock(handlersMutex_);
        handlers_.erase(channel);
    }
    
    if (running_ && subscribeContext_) {
        redisReply* reply = (redisReply*)redisCommand(subscribeContext_, "UNSUBSCRIBE %s", channel.c_str());
        if (!reply) {
            LOG_ERROR("[RedisPubSub] Failed to unsubscribe from " + channel);
            return false;
        }
        freeReplyObject(reply);
        LOG_INFO("[RedisPubSub] Unsubscribed from channel: " + channel);
        return true;
    }
    
    return true;
}

int RedisPubSub::publish(const std::string& channel, const std::string& message) {
    if (!connectPublish()) {
        return -1;
    }
    
    redisReply* reply = (redisReply*)redisCommand(publishContext_, "PUBLISH %s %s", 
                                                   channel.c_str(), message.c_str());
    
    if (!reply) {
        LOG_ERROR("[RedisPubSub] PUBLISH failed: connection error");
        return -1;
    }
    
    if (reply->type == REDIS_REPLY_ERROR) {
        LOG_ERROR("[RedisPubSub] PUBLISH failed: " + std::string(reply->str));
        freeReplyObject(reply);
        return -1;
    }
    
    int receivers = 0;
    if (reply->type == REDIS_REPLY_INTEGER) {
        receivers = reply->integer;
    }
    
    freeReplyObject(reply);
    return receivers;
}

bool RedisPubSub::start() {
    if (running_) {
        LOG_WARNING("[RedisPubSub] Already running");
        return true;
    }
    
    if (!connectSubscribe()) {
        return false;
    }
    
    // Subscrever a todos os canais registrados
    {
        std::lock_guard<std::mutex> lock(handlersMutex_);
        for (const auto& [channel, handler] : handlers_) {
            redisReply* reply = (redisReply*)redisCommand(subscribeContext_, "SUBSCRIBE %s", channel.c_str());
            if (!reply) {
                LOG_ERROR("[RedisPubSub] Failed to subscribe to " + channel);
                continue;
            }
            freeReplyObject(reply);
            LOG_INFO("[RedisPubSub] Subscribed to channel: " + channel);
        }
    }
    
    // Iniciar thread de listening
    stopRequested_ = false;
    running_ = true;
    listenerThread_ = std::thread(&RedisPubSub::listenLoop, this);
    
    LOG_INFO("[RedisPubSub] Listener thread started");
    return true;
}

void RedisPubSub::stop() {
    if (!running_) {
        return;
    }
    
    LOG_INFO("[RedisPubSub] Stopping listener thread...");
    stopRequested_ = true;
    running_ = false;
    
    if (listenerThread_.joinable()) {
        listenerThread_.join();
    }
    
    LOG_INFO("[RedisPubSub] Listener thread stopped");
}

void RedisPubSub::listenLoop() {
    LOG_INFO("[RedisPubSub] Listen loop started");
    
    while (!stopRequested_) {
        if (!subscribeContext_) {
            LOG_ERROR("[RedisPubSub] Lost connection, attempting to reconnect...");
            if (!connectSubscribe()) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }
        }
        
        redisReply* reply = nullptr;
        
        // redisGetReply bloqueia até receber uma mensagem
        int result = redisGetReply(subscribeContext_, (void**)&reply);
        
        if (result != REDIS_OK) {
            if (!stopRequested_) {
                LOG_WARNING("[RedisPubSub] Failed to get reply: " + 
                         (subscribeContext_->errstr ? std::string(subscribeContext_->errstr) : "unknown error"));
                // Reconectar
                redisFree(subscribeContext_);
                subscribeContext_ = nullptr;
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
            continue;
        }
        
        if (!reply) {
            continue;
        }
        
        handleSubscribeReply(reply);
        freeReplyObject(reply);
    }
    
    LOG_INFO("[RedisPubSub] Listen loop ended");
}

void RedisPubSub::handleSubscribeReply(redisReply* reply) {
    if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements < 3) {
        return;
    }
    
    // Formato: [tipo, canal, mensagem/count]
    // tipo pode ser: "subscribe", "unsubscribe", "message"
    
    std::string type(reply->element[0]->str, reply->element[0]->len);
    std::string channel(reply->element[1]->str, reply->element[1]->len);
    
    if (type == "message") {
        std::string message(reply->element[2]->str, reply->element[2]->len);
        
        // Buscar handler para este canal
        PubSubMessageHandler handler;
        {
            std::lock_guard<std::mutex> lock(handlersMutex_);
            auto it = handlers_.find(channel);
            if (it != handlers_.end()) {
                handler = it->second;
            }
        }
        
        if (handler) {
            try {
                handler(channel, message);
            } catch (const std::exception& e) {
                LOG_ERROR("[RedisPubSub] Handler exception for channel " + channel + ": " + e.what());
            }
        }
        
        LOG_DEBUG("[RedisPubSub] Message received on " + channel + ": " + message.substr(0, 50));
        
    } else if (type == "subscribe") {
        int count = reply->element[2]->integer;
        LOG_DEBUG("[RedisPubSub] Subscribed to " + channel + " (total: " + std::to_string(count) + ")");
        
    } else if (type == "unsubscribe") {
        int count = reply->element[2]->integer;
        LOG_DEBUG("[RedisPubSub] Unsubscribed from " + channel + " (remaining: " + std::to_string(count) + ")");
    }
}

std::vector<std::string> RedisPubSub::getSubscribedChannels() const {
    std::vector<std::string> channels;
    std::lock_guard<std::mutex> lock(handlersMutex_);
    for (const auto& [channel, _] : handlers_) {
        channels.push_back(channel);
    }
    return channels;
}

