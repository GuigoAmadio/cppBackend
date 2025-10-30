#pragma once

#include <string>
#include <optional>
#include <memory>

namespace Core::Cache {

/**
 * @brief Interface abstrata para serviços de cache
 * 
 * Define o contrato para implementações de cache (Redis, Memcached, In-Memory, etc)
 * Usa Strategy Pattern para permitir trocar implementações facilmente
 */
class CacheService {
public:
    virtual ~CacheService() = default;
    
    // ========================================
    // STRING OPERATIONS
    // ========================================
    
    /**
     * @brief Armazena um valor no cache
     * @param key Chave
     * @param value Valor (JSON, string, etc)
     * @param ttl Time-To-Live em segundos (0 = sem expiração)
     * @return true se sucesso
     */
    virtual bool set(const std::string& key, const std::string& value, int ttl = 0) = 0;
    
    /**
     * @brief Recupera um valor do cache
     * @param key Chave
     * @return valor ou std::nullopt se não encontrado
     */
    virtual std::optional<std::string> get(const std::string& key) = 0;
    
    /**
     * @brief Remove uma chave do cache
     * @param key Chave
     * @return true se removido
     */
    virtual bool del(const std::string& key) = 0;
    
    /**
     * @brief Verifica se uma chave existe
     * @param key Chave
     * @return true se existe
     */
    virtual bool exists(const std::string& key) = 0;
    
    /**
     * @brief Limpa todo o cache
     */
    virtual void flush() = 0;
    
    // ========================================
    // HASH OPERATIONS (opcional, para implementações avançadas)
    // ========================================
    
    /**
     * @brief Define um campo em um hash
     * @param key Chave do hash
     * @param field Nome do campo
     * @param value Valor do campo
     * @return true se sucesso
     */
    virtual bool hset(const std::string& key, const std::string& field, const std::string& value) = 0;
    
    /**
     * @brief Recupera um campo de um hash
     * @param key Chave do hash
     * @param field Nome do campo
     * @return valor ou std::nullopt
     */
    virtual std::optional<std::string> hget(const std::string& key, const std::string& field) = 0;
    
    // ========================================
    // TTL OPERATIONS
    // ========================================
    
    /**
     * @brief Define ou atualiza o TTL de uma chave
     * @param key Chave
     * @param seconds Segundos até expirar
     * @return true se sucesso
     */
    virtual bool expire(const std::string& key, int seconds) = 0;
    
    /**
     * @brief Retorna o TTL restante de uma chave
     * @param key Chave
     * @return segundos ou -1 se sem TTL, -2 se não existe
     */
    virtual int ttl(const std::string& key) = 0;
    
    // ========================================
    // PATTERNS DE CACHE COMUNS
    // ========================================
    
    /**
     * @brief Cache-aside pattern helper
     * 
     * Tenta recuperar do cache primeiro, se não encontrar,
     * executa o loader, salva no cache e retorna
     * 
     * @param key Chave do cache
     * @param ttl TTL em segundos
     * @param loader Função que carrega os dados (DB query, API call, etc)
     * @return Dados (do cache ou do loader)
     */
    template<typename LoaderFunc>
    std::optional<std::string> getOrLoad(
        const std::string& key,
        int ttl,
        LoaderFunc loader
    ) {
        // 1. Tentar cache primeiro
        auto cached = get(key);
        if (cached.has_value()) {
            return cached;
        }
        
        // 2. Cache miss - executar loader
        auto data = loader();
        if (!data.has_value()) {
            return std::nullopt;
        }
        
        // 3. Salvar no cache
        set(key, data.value(), ttl);
        
        return data;
    }
};

} // namespace Core::Cache

