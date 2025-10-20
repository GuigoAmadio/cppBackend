#pragma once

#include "Queue.hpp"
#include <vector>
#include <thread>
#include <functional>
#include <atomic>

namespace Core::Threading {

/**
 * @brief Pool de threads para executar tarefas assíncronas.
 * 
 * Cria N threads trabalhadoras que consomem tarefas de uma fila.
 * Ideal para servidores HTTP que precisam processar múltiplas
 * requisições simultaneamente sem criar/destruir threads.
 * 
 * Exemplo:
 *   ThreadPool pool(4);  // 4 worker threads
 *   pool.start();
 *   
 *   pool.submit([](){ std::cout << "Task 1\n"; });
 *   pool.submit([](){ std::cout << "Task 2\n"; });
 *   
 *   pool.stop();
 */
class ThreadPool {
public:
    using Task = std::function<void()>;
    
    /**
     * @brief Constrói um pool com N threads.
     * @param numThreads Número de worker threads (padrão: hardware_concurrency)
     */
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
    
    /**
     * @brief Destrutor - para o pool automaticamente.
     */
    ~ThreadPool();
    
    // Desabilitar cópia e movimentação
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
    
    /**
     * @brief Inicia as worker threads.
     */
    void start();
    
    /**
     * @brief Para o pool e aguarda todas as threads terminarem.
     */
    void stop();
    
    /**
     * @brief Submete uma tarefa para execução assíncrona.
     * @param task Função/lambda a ser executada
     */
    void submit(Task task);
    
    /**
     * @brief Retorna o número de threads no pool.
     */
    size_t getThreadCount() const { return numThreads_; }
    
    /**
     * @brief Retorna o número de tarefas pendentes.
     */
    size_t getPendingTasks() const { return taskQueue_.size(); }
    
    /**
     * @brief Verifica se o pool está rodando.
     */
    bool isRunning() const { return running_; }

private:
    /**
     * @brief Função executada por cada worker thread.
     */
    void workerLoop();
    
    size_t numThreads_;
    std::vector<std::thread> workers_;
    Queue<Task> taskQueue_;
    std::atomic<bool> running_{false};
};

} // namespace Core::Threading
