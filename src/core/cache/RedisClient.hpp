#pragma once

#include <string>
#include <optional>
#include <memory>
#include <map>
#include <vector>
#include <tuple>
#include <hiredis/hiredis.h>

namespace Core::Cache {

/**
 * @brief Cliente Redis wrapper para hiredis
 * 
 * Fornece interface C++ limpa para operações Redis básicas
 * Thread-safe quando usado com RedisPool
 */
class RedisClient {
public:
    /**
     * @brief Construtor
     * @param host Host do Redis (ex: "127.0.0.1")
     * @param port Porta do Redis (padrão: 6379)
     * @param timeoutSeconds Timeout de conexão
     */
    explicit RedisClient(
        const std::string& host = "127.0.0.1",
        int port = 6379,
        int timeoutSeconds = 5
    );
    
    /**
     * @brief Destrutor - libera conexão
     */
    ~RedisClient();
    
    // Delete copy constructor e assignment
    RedisClient(const RedisClient&) = delete;
    RedisClient& operator=(const RedisClient&) = delete;
    
    // Move constructor e assignment
    RedisClient(RedisClient&& other) noexcept;
    RedisClient& operator=(RedisClient&& other) noexcept;
    
    /**
     * @brief Verifica se está conectado
     */
    bool isConnected() const;
    
    /**
     * @brief Reconectar ao Redis
     */
    bool reconnect();
    
    // ========================================
    // STRING OPERATIONS
    // ========================================
    
    /**
     * @brief SET key value
     */
    bool set(const std::string& key, const std::string& value);
    
    /**
     * @brief GET key
     * @return valor ou std::nullopt se não encontrado
     */
    std::optional<std::string> get(const std::string& key);
    
    /**
     * @brief DEL key
     */
    bool del(const std::string& key);
    
    /**
     * @brief EXISTS key
     */
    bool exists(const std::string& key);
    
    // ========================================
    // EXPIRATION
    // ========================================
    
    /**
     * @brief SET key value com TTL (Time To Live)
     * @param key Chave
     * @param seconds TTL em segundos
     * @param value Valor
     */
    bool setex(const std::string& key, int seconds, const std::string& value);
    
    /**
     * @brief EXPIRE key seconds
     */
    bool expire(const std::string& key, int seconds);
    
    /**
     * @brief TTL key (retorna segundos restantes)
     * @return segundos ou -1 se sem TTL, -2 se não existe
     */
    int ttl(const std::string& key);
    
    // ========================================
    // HASH OPERATIONS
    // ========================================
    
    /**
     * @brief HSET key field value
     */
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    
    /**
     * @brief HGET key field
     */
    std::optional<std::string> hget(const std::string& key, const std::string& field);
    
    /**
     * @brief HDEL key field
     */
    bool hdel(const std::string& key, const std::string& field);
    
    /**
     * @brief HEXISTS key field
     */
    bool hexists(const std::string& key, const std::string& field);
    
    // ========================================
    // PUB/SUB
    // ========================================
    
    /**
     * @brief PUBLISH channel message
     * @return número de subscribers que receberam a mensagem
     */
    int publish(const std::string& channel, const std::string& message);
    
    // ========================================
    // STREAMS (MESSAGE QUEUE)
    // ========================================
    
    /**
     * @brief XADD stream * field1 value1 field2 value2 ...
     * @param stream Nome do stream
     * @param fields Mapa de campos e valores
     * @return ID da mensagem gerado pelo Redis (ex: "1234567890123-0") ou empty se erro
     */
    std::string xadd(const std::string& stream, const std::map<std::string, std::string>& fields);
    
    /**
     * @brief XREAD BLOCK milliseconds STREAMS stream1 stream2 ... id1 id2 ...
     * @param streams Mapa de stream -> último ID lido (use ">" para novos, "0" para todos)
     * @param blockMs Tempo de bloqueio em ms (0 = não bloqueia, -1 = infinito)
     * @param count Número máximo de mensagens por stream (0 = sem limite)
     * @return Mapa de stream -> lista de (id, campos)
     */
    std::map<std::string, std::vector<std::pair<std::string, std::map<std::string, std::string>>>>
    xread(const std::map<std::string, std::string>& streams, int blockMs = 0, int count = 0);
    
    /**
     * @brief XREADGROUP GROUP group consumer BLOCK ms STREAMS stream id
     * @param group Nome do consumer group
     * @param consumer Nome do consumer
     * @param streams Mapa de stream -> ID (use ">" para novos pendentes)
     * @param blockMs Tempo de bloqueio
     * @param count Número máximo de mensagens
     * @return Mapa de stream -> lista de (id, campos)
     */
    std::map<std::string, std::vector<std::pair<std::string, std::map<std::string, std::string>>>>
    xreadgroup(const std::string& group, const std::string& consumer,
               const std::map<std::string, std::string>& streams, 
               int blockMs = 0, int count = 0);
    
    /**
     * @brief XACK stream group id1 id2 ... (confirma processamento)
     * @param stream Nome do stream
     * @param group Nome do consumer group
     * @param ids Lista de IDs de mensagens a confirmar
     * @return Número de mensagens confirmadas
     */
    int xack(const std::string& stream, const std::string& group, 
             const std::vector<std::string>& ids);
    
    /**
     * @brief XGROUP CREATE stream group id [MKSTREAM]
     * @param stream Nome do stream
     * @param group Nome do consumer group
     * @param startId ID inicial ("0" = início, "$" = fim/novos)
     * @param mkstream Cria stream se não existir
     * @return true se criado com sucesso
     */
    bool xgroupCreate(const std::string& stream, const std::string& group, 
                      const std::string& startId = "$", bool mkstream = true);
    
    /**
     * @brief XGROUP DESTROY stream group (remove consumer group)
     */
    bool xgroupDestroy(const std::string& stream, const std::string& group);
    
    /**
     * @brief XPENDING stream group [start end count] (lista mensagens pendentes)
     * @param stream Nome do stream
     * @param group Nome do consumer group
     * @return Lista de (id, consumer, idle_time_ms, delivery_count)
     */
    std::vector<std::tuple<std::string, std::string, long long, int>>
    xpending(const std::string& stream, const std::string& group, int count = 10);
    
    /**
     * @brief XCLAIM stream group consumer min-idle-time id1 id2 ... 
     * (reclama mensagens de outros consumers)
     * @param stream Nome do stream
     * @param group Nome do consumer group
     * @param consumer Nome do novo consumer
     * @param minIdleMs Tempo mínimo de idle em ms
     * @param ids Lista de IDs a reclamar
     * @return Lista de (id, campos) reclamadas
     */
    std::vector<std::pair<std::string, std::map<std::string, std::string>>>
    xclaim(const std::string& stream, const std::string& group, 
           const std::string& consumer, long long minIdleMs,
           const std::vector<std::string>& ids);
    
    /**
     * @brief XDEL stream id1 id2 ... (remove mensagens)
     */
    int xdel(const std::string& stream, const std::vector<std::string>& ids);
    
    /**
     * @brief XLEN stream (retorna tamanho do stream)
     */
    long long xlen(const std::string& stream);
    
    // ========================================
    // UTILITY
    // ========================================
    
    /**
     * @brief PING
     */
    bool ping();
    
    /**
     * @brief FLUSHDB (limpa database atual)
     */
    bool flushdb();
    
    /**
     * @brief Executa comando Redis arbitrário
     * @param format Formato do comando (ex: "SET %s %s")
     * @return Reply do Redis
     */
    redisReply* command(const char* format, ...);
    
    /**
     * @brief PUBSUB CHANNELS - Lista canais ativos (com pattern opcional)
     * @param pattern Pattern opcional (ex: "chat:*")
     * @return Lista de canais
     */
    std::vector<std::string> pubsubChannels(const std::string& pattern = "*");
    
    /**
     * @brief PUBSUB NUMSUB - Retorna número de subscribers por canal
     * @param channels Canais a verificar
     * @return Map de canal -> número de subscribers
     */
    std::map<std::string, int> pubsubNumsub(const std::vector<std::string>& channels);

private:
    redisContext* context_;
    std::string host_;
    int port_;
    int timeoutSeconds_;
    
    /**
     * @brief Verifica se reply é válido e trata erros
     */
    bool checkReply(redisReply* reply, const char* operation);
    
    /**
     * @brief Libera reply
     */
    void freeReply(redisReply* reply);
};

} // namespace Core::Cache

