#include "ThreadPool.hpp"

namespace Core::Threading {

ThreadPool::ThreadPool(size_t numThreads) 
    : numThreads_(numThreads) {
    // Garante pelo menos 1 thread
    if (numThreads_ == 0) {
        numThreads_ = 1;
    }
}

ThreadPool::~ThreadPool() {
    if (running_) {
        stop();
    }
}

void ThreadPool::start() {
    if (running_) {
        return;  // Já está rodando
    }
    
    running_ = true;
    
    // Criar worker threads
    workers_.reserve(numThreads_);
    for (size_t i = 0; i < numThreads_; ++i) {
        workers_.emplace_back([this] {
            workerLoop();
        });
    }
}

void ThreadPool::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // Para a fila (acorda todas as threads)
    taskQueue_.stop();
    
    // Aguarda todas as threads terminarem
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
}

void ThreadPool::submit(Task task) {
    if (!running_) {
        // Se não está rodando, executa diretamente (fallback)
        task();
        return;
    }
    
    taskQueue_.push(std::move(task));
}

void ThreadPool::workerLoop() {
    while (running_) {
        // Tenta pegar uma tarefa da fila (bloqueia se vazia)
        auto task = taskQueue_.pop();
        
        // Se nullopt, a fila foi parada
        if (!task.has_value()) {
            break;
        }
        
        // Executa a tarefa
        try {
            (*task)();
        } catch (...) {
            // Evita que exceções em tasks quebrem o worker
            // Poderia logar aqui
        }
    }
}

} // namespace Core::Threading
