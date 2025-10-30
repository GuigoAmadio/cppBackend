#include "RedisClient.hpp"
#include "../utils/LoggerNew.hpp"
#include <cstdarg>
#include <cstring>
#include <vector>

namespace Core::Cache {

RedisClient::RedisClient(const std::string& host, int port, int timeoutSeconds)
    : context_(nullptr), host_(host), port_(port), timeoutSeconds_(timeoutSeconds) {
    
    // Conectar ao Redis
    struct timeval timeout = { timeoutSeconds, 0 };
    context_ = redisConnectWithTimeout(host.c_str(), port, timeout);
    
    if (context_ == nullptr) {
        LOG_ERROR("Redis: Failed to allocate context");
        throw std::runtime_error("Failed to allocate Redis context");
    }
    
    if (context_->err) {
        std::string error = context_->errstr;
        redisFree(context_);
        context_ = nullptr;
        LOG_ERROR("Redis: Connection error: " + error);
        throw std::runtime_error("Redis connection error: " + error);
    }
    
    LOG_INFO("Redis: Connected to " + host + ":" + std::to_string(port));
}

RedisClient::~RedisClient() {
    if (context_) {
        redisFree(context_);
        context_ = nullptr;
    }
}

RedisClient::RedisClient(RedisClient&& other) noexcept
    : context_(other.context_),
      host_(std::move(other.host_)),
      port_(other.port_),
      timeoutSeconds_(other.timeoutSeconds_) {
    other.context_ = nullptr;
}

RedisClient& RedisClient::operator=(RedisClient&& other) noexcept {
    if (this != &other) {
        if (context_) {
            redisFree(context_);
        }
        context_ = other.context_;
        host_ = std::move(other.host_);
        port_ = other.port_;
        timeoutSeconds_ = other.timeoutSeconds_;
        other.context_ = nullptr;
    }
    return *this;
}

bool RedisClient::isConnected() const {
    return context_ != nullptr && context_->err == 0;
}

bool RedisClient::reconnect() {
    if (context_) {
        redisFree(context_);
        context_ = nullptr;
    }
    
    try {
        struct timeval timeout = { timeoutSeconds_, 0 };
        context_ = redisConnectWithTimeout(host_.c_str(), port_, timeout);
        
        if (context_ == nullptr || context_->err) {
            LOG_ERROR("Redis: Reconnection failed");
            return false;
        }
        
        LOG_INFO("Redis: Reconnected successfully");
        return true;
    } catch (...) {
        return false;
    }
}

// ========================================
// STRING OPERATIONS
// ========================================

bool RedisClient::set(const std::string& key, const std::string& value) {
    redisReply* reply = (redisReply*)redisCommand(context_, "SET %s %s", key.c_str(), value.c_str());
    bool success = checkReply(reply, "SET");
    freeReply(reply);
    return success;
}

std::optional<std::string> RedisClient::get(const std::string& key) {
    redisReply* reply = (redisReply*)redisCommand(context_, "GET %s", key.c_str());
    
    if (!reply) {
        LOG_ERROR("Redis GET: null reply");
        return std::nullopt;
    }
    
    if (reply->type == REDIS_REPLY_STRING) {
        std::string value(reply->str, reply->len);
        freeReply(reply);
        return value;
    }
    
    if (reply->type == REDIS_REPLY_NIL) {
        freeReply(reply);
        return std::nullopt;
    }
    
    LOG_WARNING("Redis GET: unexpected reply type: " + std::to_string(reply->type));
    freeReply(reply);
    return std::nullopt;
}

bool RedisClient::del(const std::string& key) {
    redisReply* reply = (redisReply*)redisCommand(context_, "DEL %s", key.c_str());
    bool success = checkReply(reply, "DEL");
    freeReply(reply);
    return success;
}

bool RedisClient::exists(const std::string& key) {
    redisReply* reply = (redisReply*)redisCommand(context_, "EXISTS %s", key.c_str());
    
    if (!reply) {
        return false;
    }
    
    bool exists = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReply(reply);
    return exists;
}

// ========================================
// EXPIRATION
// ========================================

bool RedisClient::setex(const std::string& key, int seconds, const std::string& value) {
    redisReply* reply = (redisReply*)redisCommand(context_, "SETEX %s %d %s", 
                                                  key.c_str(), seconds, value.c_str());
    bool success = checkReply(reply, "SETEX");
    freeReply(reply);
    return success;
}

bool RedisClient::expire(const std::string& key, int seconds) {
    redisReply* reply = (redisReply*)redisCommand(context_, "EXPIRE %s %d", key.c_str(), seconds);
    bool success = checkReply(reply, "EXPIRE");
    freeReply(reply);
    return success;
}

int RedisClient::ttl(const std::string& key) {
    redisReply* reply = (redisReply*)redisCommand(context_, "TTL %s", key.c_str());
    
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        freeReply(reply);
        return -2; // Key não existe
    }
    
    int ttl = static_cast<int>(reply->integer);
    freeReply(reply);
    return ttl;
}

// ========================================
// HASH OPERATIONS
// ========================================

bool RedisClient::hset(const std::string& key, const std::string& field, const std::string& value) {
    redisReply* reply = (redisReply*)redisCommand(context_, "HSET %s %s %s", 
                                                  key.c_str(), field.c_str(), value.c_str());
    bool success = checkReply(reply, "HSET");
    freeReply(reply);
    return success;
}

std::optional<std::string> RedisClient::hget(const std::string& key, const std::string& field) {
    redisReply* reply = (redisReply*)redisCommand(context_, "HGET %s %s", key.c_str(), field.c_str());
    
    if (!reply) {
        return std::nullopt;
    }
    
    if (reply->type == REDIS_REPLY_STRING) {
        std::string value(reply->str, reply->len);
        freeReply(reply);
        return value;
    }
    
    freeReply(reply);
    return std::nullopt;
}

bool RedisClient::hdel(const std::string& key, const std::string& field) {
    redisReply* reply = (redisReply*)redisCommand(context_, "HDEL %s %s", key.c_str(), field.c_str());
    bool success = checkReply(reply, "HDEL");
    freeReply(reply);
    return success;
}

bool RedisClient::hexists(const std::string& key, const std::string& field) {
    redisReply* reply = (redisReply*)redisCommand(context_, "HEXISTS %s %s", key.c_str(), field.c_str());
    
    if (!reply) {
        return false;
    }
    
    bool exists = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReply(reply);
    return exists;
}

// ========================================
// PUB/SUB
// ========================================

int RedisClient::publish(const std::string& channel, const std::string& message) {
    redisReply* reply = (redisReply*)redisCommand(context_, "PUBLISH %s %s", 
                                                  channel.c_str(), message.c_str());
    
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        freeReply(reply);
        return 0;
    }
    
    int subscribers = static_cast<int>(reply->integer);
    freeReply(reply);
    return subscribers;
}

// ========================================
// STREAMS (MESSAGE QUEUE)
// ========================================

std::string RedisClient::xadd(const std::string& stream, const std::map<std::string, std::string>& fields) {
    if (fields.empty()) {
        LOG_ERROR("Redis XADD: fields cannot be empty");
        return "";
    }
    
    // Construir vetor de argumentos para redisCommandArgv
    std::vector<std::string> args;
    args.push_back("XADD");
    args.push_back(stream);
    args.push_back("*");  // Auto-generate ID
    
    for (const auto& [field, value] : fields) {
        args.push_back(field);
        args.push_back(value);
    }
    
    // Converter para array de C strings
    std::vector<const char*> argv;
    std::vector<size_t> argvlen;
    for (const auto& arg : args) {
        argv.push_back(arg.c_str());
        argvlen.push_back(arg.size());
    }
    
    redisReply* reply = (redisReply*)redisCommandArgv(context_, argv.size(), argv.data(), argvlen.data());
    
    if (!reply) {
        LOG_ERROR("Redis XADD: null reply");
        return "";
    }
    
    if (reply->type == REDIS_REPLY_STRING) {
        std::string messageId(reply->str, reply->len);
        freeReply(reply);
        return messageId;
    }
    
    if (reply->type == REDIS_REPLY_ERROR) {
        LOG_ERROR(std::string("Redis XADD: ") + reply->str);
    }
    
    freeReply(reply);
    return "";
}

std::map<std::string, std::vector<std::pair<std::string, std::map<std::string, std::string>>>>
RedisClient::xread(const std::map<std::string, std::string>& streams, int blockMs, int count) {
    std::map<std::string, std::vector<std::pair<std::string, std::map<std::string, std::string>>>> result;
    
    if (streams.empty()) {
        return result;
    }
    
    // Construir comando: XREAD [BLOCK ms] [COUNT n] STREAMS stream1 stream2 ... id1 id2 ...
    std::string cmd = "XREAD";
    if (blockMs > 0) {
        cmd += " BLOCK " + std::to_string(blockMs);
    }
    if (count > 0) {
        cmd += " COUNT " + std::to_string(count);
    }
    cmd += " STREAMS";
    
    // Adicionar nomes dos streams
    for (const auto& [stream, _] : streams) {
        cmd += " " + stream;
    }
    // Adicionar IDs
    for (const auto& [_, id] : streams) {
        cmd += " " + id;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, cmd.c_str());
    
    if (!reply || reply->type == REDIS_REPLY_NIL) {
        freeReply(reply);
        return result; // Timeout ou sem mensagens
    }
    
    if (reply->type != REDIS_REPLY_ARRAY) {
        freeReply(reply);
        return result;
    }
    
    // Parse: [[stream1, [[id1, [field1, value1, ...]], ...]], ...]
    for (size_t i = 0; i < reply->elements; i++) {
        redisReply* streamReply = reply->element[i];
        if (streamReply->type != REDIS_REPLY_ARRAY || streamReply->elements < 2) continue;
        
        std::string streamName(streamReply->element[0]->str, streamReply->element[0]->len);
        redisReply* messagesArray = streamReply->element[1];
        
        std::vector<std::pair<std::string, std::map<std::string, std::string>>> messages;
        
        for (size_t j = 0; j < messagesArray->elements; j++) {
            redisReply* msgReply = messagesArray->element[j];
            if (msgReply->type != REDIS_REPLY_ARRAY || msgReply->elements < 2) continue;
            
            std::string msgId(msgReply->element[0]->str, msgReply->element[0]->len);
            redisReply* fieldsArray = msgReply->element[1];
            
            std::map<std::string, std::string> fields;
            for (size_t k = 0; k < fieldsArray->elements; k += 2) {
                if (k + 1 < fieldsArray->elements) {
                    std::string field(fieldsArray->element[k]->str, fieldsArray->element[k]->len);
                    std::string value(fieldsArray->element[k+1]->str, fieldsArray->element[k+1]->len);
                    fields[field] = value;
                }
            }
            
            messages.emplace_back(msgId, fields);
        }
        
        result[streamName] = messages;
    }
    
    freeReply(reply);
    return result;
}

std::map<std::string, std::vector<std::pair<std::string, std::map<std::string, std::string>>>>
RedisClient::xreadgroup(const std::string& group, const std::string& consumer,
                        const std::map<std::string, std::string>& streams, 
                        int blockMs, int count) {
    std::map<std::string, std::vector<std::pair<std::string, std::map<std::string, std::string>>>> result;
    
    if (streams.empty()) {
        return result;
    }
    
    // XREADGROUP GROUP group consumer [BLOCK ms] [COUNT n] STREAMS stream1 ... id1 ...
    std::string cmd = "XREADGROUP GROUP " + group + " " + consumer;
    if (blockMs > 0) {
        cmd += " BLOCK " + std::to_string(blockMs);
    }
    if (count > 0) {
        cmd += " COUNT " + std::to_string(count);
    }
    cmd += " STREAMS";
    
    for (const auto& [stream, _] : streams) {
        cmd += " " + stream;
    }
    for (const auto& [_, id] : streams) {
        cmd += " " + id;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, cmd.c_str());
    
    if (!reply || reply->type == REDIS_REPLY_NIL) {
        freeReply(reply);
        return result;
    }
    
    if (reply->type != REDIS_REPLY_ARRAY) {
        freeReply(reply);
        return result;
    }
    
    // Parse idêntico ao XREAD
    for (size_t i = 0; i < reply->elements; i++) {
        redisReply* streamReply = reply->element[i];
        if (streamReply->type != REDIS_REPLY_ARRAY || streamReply->elements < 2) continue;
        
        std::string streamName(streamReply->element[0]->str, streamReply->element[0]->len);
        redisReply* messagesArray = streamReply->element[1];
        
        std::vector<std::pair<std::string, std::map<std::string, std::string>>> messages;
        
        for (size_t j = 0; j < messagesArray->elements; j++) {
            redisReply* msgReply = messagesArray->element[j];
            if (msgReply->type != REDIS_REPLY_ARRAY || msgReply->elements < 2) continue;
            
            std::string msgId(msgReply->element[0]->str, msgReply->element[0]->len);
            redisReply* fieldsArray = msgReply->element[1];
            
            std::map<std::string, std::string> fields;
            for (size_t k = 0; k < fieldsArray->elements; k += 2) {
                if (k + 1 < fieldsArray->elements) {
                    std::string field(fieldsArray->element[k]->str, fieldsArray->element[k]->len);
                    std::string value(fieldsArray->element[k+1]->str, fieldsArray->element[k+1]->len);
                    fields[field] = value;
                }
            }
            
            messages.emplace_back(msgId, fields);
        }
        
        result[streamName] = messages;
    }
    
    freeReply(reply);
    return result;
}

int RedisClient::xack(const std::string& stream, const std::string& group, 
                      const std::vector<std::string>& ids) {
    if (ids.empty()) {
        return 0;
    }
    
    // XACK stream group id1 id2 ...
    std::string cmd = "XACK " + stream + " " + group;
    for (const auto& id : ids) {
        cmd += " " + id;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, cmd.c_str());
    
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        freeReply(reply);
        return 0;
    }
    
    int acknowledged = static_cast<int>(reply->integer);
    freeReply(reply);
    return acknowledged;
}

bool RedisClient::xgroupCreate(const std::string& stream, const std::string& group, 
                               const std::string& startId, bool mkstream) {
    std::string cmd = "XGROUP CREATE " + stream + " " + group + " " + startId;
    if (mkstream) {
        cmd += " MKSTREAM";
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, cmd.c_str());
    
    if (!reply) {
        return false;
    }
    
    // Pode retornar OK ou erro (ex: BUSYGROUP se já existe)
    bool success = (reply->type == REDIS_REPLY_STATUS && 
                   std::string(reply->str) == "OK");
    
    if (reply->type == REDIS_REPLY_ERROR) {
        std::string error(reply->str);
        // BUSYGROUP não é erro crítico (grupo já existe)
        if (error.find("BUSYGROUP") != std::string::npos) {
            LOG_DEBUG("Redis XGROUP CREATE: group already exists");
            freeReply(reply);
            return true;
        }
        LOG_ERROR("Redis XGROUP CREATE: " + error);
    }
    
    freeReply(reply);
    return success;
}

bool RedisClient::xgroupDestroy(const std::string& stream, const std::string& group) {
    redisReply* reply = (redisReply*)redisCommand(context_, "XGROUP DESTROY %s %s", 
                                                  stream.c_str(), group.c_str());
    
    if (!reply) {
        return false;
    }
    
    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReply(reply);
    return success;
}

std::vector<std::tuple<std::string, std::string, long long, int>>
RedisClient::xpending(const std::string& stream, const std::string& group, int count) {
    std::vector<std::tuple<std::string, std::string, long long, int>> result;
    
    // XPENDING stream group - start + count
    redisReply* reply = (redisReply*)redisCommand(context_, 
        "XPENDING %s %s - + %d", stream.c_str(), group.c_str(), count);
    
    if (!reply || reply->type != REDIS_REPLY_ARRAY) {
        freeReply(reply);
        return result;
    }
    
    // Parse: [[id, consumer, idle_time, delivery_count], ...]
    for (size_t i = 0; i < reply->elements; i++) {
        redisReply* entryReply = reply->element[i];
        if (entryReply->type != REDIS_REPLY_ARRAY || entryReply->elements < 4) continue;
        
        std::string id(entryReply->element[0]->str, entryReply->element[0]->len);
        std::string consumer(entryReply->element[1]->str, entryReply->element[1]->len);
        long long idleTime = entryReply->element[2]->integer;
        int deliveryCount = static_cast<int>(entryReply->element[3]->integer);
        
        result.emplace_back(id, consumer, idleTime, deliveryCount);
    }
    
    freeReply(reply);
    return result;
}

std::vector<std::pair<std::string, std::map<std::string, std::string>>>
RedisClient::xclaim(const std::string& stream, const std::string& group, 
                    const std::string& consumer, long long minIdleMs,
                    const std::vector<std::string>& ids) {
    std::vector<std::pair<std::string, std::map<std::string, std::string>>> result;
    
    if (ids.empty()) {
        return result;
    }
    
    // XCLAIM stream group consumer min-idle-ms id1 id2 ...
    std::string cmd = "XCLAIM " + stream + " " + group + " " + consumer + " " + 
                      std::to_string(minIdleMs);
    for (const auto& id : ids) {
        cmd += " " + id;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, cmd.c_str());
    
    if (!reply || reply->type != REDIS_REPLY_ARRAY) {
        freeReply(reply);
        return result;
    }
    
    // Parse: [[id, [field1, value1, ...]], ...]
    for (size_t i = 0; i < reply->elements; i++) {
        redisReply* msgReply = reply->element[i];
        if (msgReply->type != REDIS_REPLY_ARRAY || msgReply->elements < 2) continue;
        
        std::string msgId(msgReply->element[0]->str, msgReply->element[0]->len);
        redisReply* fieldsArray = msgReply->element[1];
        
        std::map<std::string, std::string> fields;
        for (size_t k = 0; k < fieldsArray->elements; k += 2) {
            if (k + 1 < fieldsArray->elements) {
                std::string field(fieldsArray->element[k]->str, fieldsArray->element[k]->len);
                std::string value(fieldsArray->element[k+1]->str, fieldsArray->element[k+1]->len);
                fields[field] = value;
            }
        }
        
        result.emplace_back(msgId, fields);
    }
    
    freeReply(reply);
    return result;
}

int RedisClient::xdel(const std::string& stream, const std::vector<std::string>& ids) {
    if (ids.empty()) {
        return 0;
    }
    
    std::string cmd = "XDEL " + stream;
    for (const auto& id : ids) {
        cmd += " " + id;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, cmd.c_str());
    
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        freeReply(reply);
        return 0;
    }
    
    int deleted = static_cast<int>(reply->integer);
    freeReply(reply);
    return deleted;
}

long long RedisClient::xlen(const std::string& stream) {
    redisReply* reply = (redisReply*)redisCommand(context_, "XLEN %s", stream.c_str());
    
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        freeReply(reply);
        return 0;
    }
    
    long long length = reply->integer;
    freeReply(reply);
    return length;
}

// ========================================
// UTILITY
// ========================================

bool RedisClient::ping() {
    redisReply* reply = (redisReply*)redisCommand(context_, "PING");
    
    if (!reply) {
        return false;
    }
    
    bool success = (reply->type == REDIS_REPLY_STATUS && 
                   std::string(reply->str) == "PONG");
    freeReply(reply);
    return success;
}

bool RedisClient::flushdb() {
    redisReply* reply = (redisReply*)redisCommand(context_, "FLUSHDB");
    bool success = checkReply(reply, "FLUSHDB");
    freeReply(reply);
    return success;
}

redisReply* RedisClient::command(const char* format, ...) {
    va_list args;
    va_start(args, format);
    redisReply* reply = (redisReply*)redisvCommand(context_, format, args);
    va_end(args);
    return reply;
}

std::vector<std::string> RedisClient::pubsubChannels(const std::string& pattern) {
    std::vector<std::string> channels;
    
    redisReply* reply = (redisReply*)redisCommand(context_, "PUBSUB CHANNELS %s", pattern.c_str());
    
    if (!reply || reply->type != REDIS_REPLY_ARRAY) {
        if (reply) freeReply(reply);
        return channels;
    }
    
    for (size_t i = 0; i < reply->elements; i++) {
        if (reply->element[i]->type == REDIS_REPLY_STRING) {
            channels.emplace_back(reply->element[i]->str, reply->element[i]->len);
        }
    }
    
    freeReply(reply);
    return channels;
}

std::map<std::string, int> RedisClient::pubsubNumsub(const std::vector<std::string>& channels) {
    std::map<std::string, int> result;
    
    if (channels.empty()) return result;
    
    // Construir comando: PUBSUB NUMSUB channel1 channel2 ...
    std::string cmd = "PUBSUB NUMSUB";
    for (const auto& ch : channels) {
        cmd += " " + ch;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context_, cmd.c_str());
    
    if (!reply || reply->type != REDIS_REPLY_ARRAY) {
        if (reply) freeReply(reply);
        return result;
    }
    
    // Resultado vem em pares: [channel1, count1, channel2, count2, ...]
    for (size_t i = 0; i + 1 < reply->elements; i += 2) {
        if (reply->element[i]->type == REDIS_REPLY_STRING &&
            reply->element[i+1]->type == REDIS_REPLY_INTEGER) {
            std::string channel(reply->element[i]->str, reply->element[i]->len);
            int count = reply->element[i+1]->integer;
            result[channel] = count;
        }
    }
    
    freeReply(reply);
    return result;
}

// ========================================
// PRIVATE METHODS
// ========================================

bool RedisClient::checkReply(redisReply* reply, const char* operation) {
    if (!reply) {
        LOG_ERROR(std::string("Redis ") + operation + ": null reply");
        return false;
    }
    
    if (reply->type == REDIS_REPLY_ERROR) {
        LOG_ERROR(std::string("Redis ") + operation + ": " + reply->str);
        return false;
    }
    
    if (reply->type == REDIS_REPLY_STATUS) {
        return std::string(reply->str) == "OK";
    }
    
    return true;
}

void RedisClient::freeReply(redisReply* reply) {
    if (reply) {
        freeReplyObject(reply);
    }
}

} // namespace Core::Cache

