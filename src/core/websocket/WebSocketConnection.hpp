#pragma once

#include "WebSocketFrame.hpp"
#include <string>
#include <functional>
#include <memory>
#include <atomic>
#include <mutex>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    typedef int socket_t;
#endif

namespace Core::WebSocket {

/**
 * @brief Estado da conexão WebSocket
 */
enum class ConnectionState {
    Connecting,    // Handshake em progresso
    Open,          // Conexão estabelecida
    Closing,       // Close frame enviado/recebido
    Closed         // Conexão fechada
};

/**
 * @brief Representa uma conexão WebSocket individual
 * 
 * Gerencia:
 * - Socket TCP
 * - WebSocket handshake (HTTP Upgrade)
 * - Envio/recebimento de frames
 * - Estado da conexão
 * - Ping/Pong heartbeat
 */
class WebSocketConnection : public std::enable_shared_from_this<WebSocketConnection> {
public:
    // Callbacks
    using OnMessageCallback = std::function<void(const std::string& message)>;
    using OnCloseCallback = std::function<void(uint16_t code, const std::string& reason)>;
    using OnErrorCallback = std::function<void(const std::string& error)>;
    
    /**
     * @brief Construtor
     * @param socket Socket TCP já conectado
     * @param id ID único da conexão
     */
    WebSocketConnection(socket_t socket, const std::string& id);
    
    /**
     * @brief Destrutor
     */
    ~WebSocketConnection();
    
    // Delete copy
    WebSocketConnection(const WebSocketConnection&) = delete;
    WebSocketConnection& operator=(const WebSocketConnection&) = delete;
    
    // ========================================
    // HANDSHAKE
    // ========================================
    
    /**
     * @brief Realiza WebSocket handshake (upgrade HTTP -> WebSocket)
     * @param request HTTP request completo
     * @return true se handshake bem-sucedido
     */
    bool performHandshake(const std::string& request);
    
    // ========================================
    // SEND/RECEIVE
    // ========================================
    
    /**
     * @brief Envia mensagem de texto
     */
    bool sendText(const std::string& message);
    
    /**
     * @brief Envia mensagem binária
     */
    bool sendBinary(const std::vector<uint8_t>& data);
    
    /**
     * @brief Envia frame
     */
    bool sendFrame(const WebSocketFrame& frame);
    
    /**
     * @brief Lê e processa frames recebidos
     * @return true se deve continuar lendo
     */
    bool receiveFrame();
    
    /**
     * @brief Envia ping
     */
    bool sendPing(const std::string& payload = "");
    
    /**
     * @brief Envia pong
     */
    bool sendPong(const std::string& payload = "");
    
    // ========================================
    // CLOSE
    // ========================================
    
    /**
     * @brief Fecha a conexão gracefully
     */
    void close(uint16_t code = 1000, const std::string& reason = "");
    
    /**
     * @brief Fecha a conexão abruptamente
     */
    void forceClose();
    
    // ========================================
    // STATE
    // ========================================
    
    ConnectionState getState() const { return state_; }
    bool isOpen() const { return state_ == ConnectionState::Open; }
    bool isClosed() const { return state_ == ConnectionState::Closed; }
    
    // ========================================
    // METADATA
    // ========================================
    
    const std::string& getId() const { return id_; }
    socket_t getSocket() const { return socket_; }
    
    const std::string& getPath() const { return path_; }
    const std::string& getQuery() const { return query_; }
    
    // ========================================
    // CALLBACKS
    // ========================================
    
    void setOnMessage(OnMessageCallback callback) { onMessage_ = callback; }
    void setOnClose(OnCloseCallback callback) { onClose_ = callback; }
    void setOnError(OnErrorCallback callback) { onError_ = callback; }

private:
    socket_t socket_;
    std::string id_;
    std::atomic<ConnectionState> state_;
    
    // Request metadata
    std::string path_;
    std::string query_;
    std::string origin_;
    
    // Callbacks
    OnMessageCallback onMessage_;
    OnCloseCallback onClose_;
    OnErrorCallback onError_;
    
    // Fragmentação de mensagens
    std::string fragmentedMessage_;
    OpCode fragmentedOpCode_;
    
    // Mutex para envio thread-safe
    std::mutex sendMutex_;
    
    /**
     * @brief Parse HTTP request para extrair cabeçalhos
     */
    std::string getHeader(const std::string& request, const std::string& headerName);
    
    /**
     * @brief Gera Sec-WebSocket-Accept para handshake
     */
    std::string generateAcceptKey(const std::string& key);
    
    /**
     * @brief Envia dados brutos no socket
     */
    bool sendRaw(const uint8_t* data, size_t length);
    
    /**
     * @brief Recebe dados brutos do socket
     */
    ssize_t receiveRaw(uint8_t* buffer, size_t maxLength);
    
    /**
     * @brief Processa frame recebido
     */
    void processFrame(const WebSocketFrame& frame);
};

} // namespace Core::WebSocket

