#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace Core::Threading {

/**
 * @brief Thread-safe queue usando mutex e condition_variable.
 * 
 * Esta fila bloqueia threads consumidoras quando vazia e notifica
 * quando novos itens são adicionados.
 * 
 * @tparam T Tipo dos elementos na fila
 */
template <typename T>
class Queue {
public:
    Queue() = default;
    ~Queue() = default;
    
    // Desabilitar cópia e movimentação (simplificar)
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;
    Queue(Queue&&) = delete;
    Queue& operator=(Queue&&) = delete;
    
    /**
     * @brief Adiciona um item na fila (thread-safe).
     * @param item Item a ser adicionado
     */
    void push(T item) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(item));
        }
        // Notifica uma thread esperando
        cv_.notify_one();
    }
    
    /**
     * @brief Remove e retorna o primeiro item da fila (bloqueia se vazia).
     * @return Item removido, ou std::nullopt se a fila foi parada
     */
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Espera até ter item ou a fila ser parada
        cv_.wait(lock, [this] {
            return !queue_.empty() || stopped_;
        });
        
        // Se parada e vazia, retorna nullopt
        if (stopped_ && queue_.empty()) {
            return std::nullopt;
        }
        
        // Pega o item
        T item = std::move(queue_.front());
        queue_.pop();
        
        return item;
    }
    
    /**
     * @brief Tenta remover um item (não bloqueia).
     * @return Item removido, ou std::nullopt se vazia
     */
    std::optional<T> tryPop() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (queue_.empty()) {
            return std::nullopt;
        }
        
        T item = std::move(queue_.front());
        queue_.pop();
        
        return item;
    }
    
    /**
     * @brief Para a fila e acorda todas as threads esperando.
     */
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }
        cv_.notify_all();
    }
    
    /**
     * @brief Verifica se a fila está vazia (thread-safe).
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    
    /**
     * @brief Retorna o tamanho da fila (thread-safe).
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;
};

} // namespace Core::Threading

