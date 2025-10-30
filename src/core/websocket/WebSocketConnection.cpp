#include "WebSocketConnection.hpp"
#include "../utils/LoggerNew.hpp"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <sstream>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <mswsock.h>  // Para SIO_KEEPALIVE_VALS
#else
    #include <sys/time.h>
    #include <errno.h>
    #include <netinet/tcp.h>  // Para TCP_KEEPIDLE, TCP_KEEPINTVL, TCP_KEEPCNT
#endif

namespace Core::WebSocket {

// Magic string do WebSocket protocol (RFC 6455)
static const char* WEBSOCKET_MAGIC = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

// Base64 encoding helper
static std::string base64Encode(const unsigned char* data, size_t length) {
    static const char* base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    std::string result;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    while (length--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                result += base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(int j = i; j < 3; j++)
            char_array_3[j] = '\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (int j = 0; j < i + 1; j++)
            result += base64_chars[char_array_4[j]];
        
        while(i++ < 3)
            result += '=';
    }
    
    return result;
}

WebSocketConnection::WebSocketConnection(socket_t socket, const std::string& id)
    : socket_(socket), 
      id_(id), 
      state_(ConnectionState::Connecting),
      fragmentedOpCode_(OpCode::Text) {
    
    // ✅ HABILITAR TCP KEEP-ALIVE (nível SO)
    int enable = 1;
    setsockopt(socket_, SOL_SOCKET, SO_KEEPALIVE, (const char*)&enable, sizeof(enable));
    
#ifdef _WIN32
    // Windows: configurar keep-alive com WSAIoctl
    struct tcp_keepalive keepalive_vals;
    keepalive_vals.onoff = 1;
    keepalive_vals.keepalivetime = 60000;      // 60 segundos de idle
    keepalive_vals.keepaliveinterval = 5000;   // 5 segundos entre probes
    
    DWORD bytes_returned;
    WSAIoctl(socket_, SIO_KEEPALIVE_VALS, &keepalive_vals, 
             sizeof(keepalive_vals), NULL, 0, &bytes_returned, NULL, NULL);
    
    LOG_DEBUG("WebSocket: TCP Keep-Alive habilitado (Windows) - idle: 60s, interval: 5s");
#else
    // Linux/Unix: configurar keep-alive com setsockopt
    int keepidle = 60;   // 60 segundos de idle antes do primeiro probe
    int keepintvl = 5;   // 5 segundos entre probes
    int keepcnt = 3;     // 3 probes sem resposta = conexão morta
    
    setsockopt(socket_, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
    setsockopt(socket_, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
    setsockopt(socket_, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));
    
    LOG_DEBUG("WebSocket: TCP Keep-Alive habilitado (Linux/Unix) - idle: 60s, interval: 5s, count: 3");
#endif
}

WebSocketConnection::~WebSocketConnection() {
    if (!isClosed()) {
        forceClose();
    }
}

bool WebSocketConnection::performHandshake(const std::string& request) {
    // Extrair Sec-WebSocket-Key
    std::string key = getHeader(request, "Sec-WebSocket-Key");
    if (key.empty()) {
        LOG_ERROR("WebSocket: Missing Sec-WebSocket-Key");
        return false;
    }
    
    // Extrair path e query da primeira linha
    size_t pathStart = request.find("GET ");
    if (pathStart != std::string::npos) {
        pathStart += 4;
        size_t pathEnd = request.find(" HTTP", pathStart);
        if (pathEnd != std::string::npos) {
            std::string fullPath = request.substr(pathStart, pathEnd - pathStart);
            size_t queryPos = fullPath.find('?');
            if (queryPos != std::string::npos) {
                path_ = fullPath.substr(0, queryPos);
                query_ = fullPath.substr(queryPos + 1);
            } else {
                path_ = fullPath;
            }
        }
    }
    
    // Gerar Sec-WebSocket-Accept
    std::string acceptKey = generateAcceptKey(key);
    
    // Construir resposta de handshake
    std::ostringstream response;
    response << "HTTP/1.1 101 Switching Protocols\r\n";
    response << "Upgrade: websocket\r\n";
    response << "Connection: Upgrade\r\n";
    response << "Sec-WebSocket-Accept: " << acceptKey << "\r\n";
    response << "\r\n";
    
    std::string responseStr = response.str();
    
    // Enviar resposta
    if (!sendRaw(reinterpret_cast<const uint8_t*>(responseStr.data()), responseStr.size())) {
        LOG_ERROR("WebSocket: Failed to send handshake response");
        return false;
    }
    
    state_ = ConnectionState::Open;
    LOG_INFO("WebSocket: Handshake successful - Connection " + id_ + " opened");
    
    return true;
}

bool WebSocketConnection::sendText(const std::string& message) {
    return sendFrame(WebSocketFrame::text(message));
}

bool WebSocketConnection::sendBinary(const std::vector<uint8_t>& data) {
    std::string payload(data.begin(), data.end());
    return sendFrame(WebSocketFrame(OpCode::Binary, payload, true));
}

bool WebSocketConnection::sendFrame(const WebSocketFrame& frame) {
    if (state_ != ConnectionState::Open) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(sendMutex_);
    
    // Serialize frame (servidor NÃO mascara)
    auto buffer = frame.serialize(false);
    
    return sendRaw(buffer.data(), buffer.size());
}

bool WebSocketConnection::receiveFrame() {
    uint8_t buffer[65536]; // 64KB buffer
    
    // Ler dados do socket (bloqueia até receber dados)
    ssize_t bytesRead = receiveRaw(buffer, sizeof(buffer));
    
    if (bytesRead <= 0) {
        // Conexão fechada ou erro
        LOG_DEBUG("WebSocket: Socket closed or error (bytesRead: " + std::to_string(bytesRead) + ")");
        state_ = ConnectionState::Closed;
        if (onClose_) {
            onClose_(1006, "Connection closed by peer");
        }
        return false;
    }
    
    // Parse frame
    auto frameOpt = WebSocketFrame::parse(buffer, bytesRead);
    
    if (!frameOpt.has_value()) {
        LOG_WARNING("WebSocket: Failed to parse frame");
        return true; // Continuar tentando
    }
    
    // Processar frame
    processFrame(frameOpt.value());
    
    return state_ != ConnectionState::Closed;
}

bool WebSocketConnection::sendPing(const std::string& payload) {
    return sendFrame(WebSocketFrame::ping(payload));
}

bool WebSocketConnection::sendPong(const std::string& payload) {
    return sendFrame(WebSocketFrame::pong(payload));
}

void WebSocketConnection::close(uint16_t code, const std::string& reason) {
    if (state_ == ConnectionState::Closed || state_ == ConnectionState::Closing) {
        return;
    }
    
    state_ = ConnectionState::Closing;
    
    // Enviar close frame
    sendFrame(WebSocketFrame::close(code, reason));
    
    // Fechar socket
    forceClose();
}

void WebSocketConnection::forceClose() {
    state_ = ConnectionState::Closed;
    
    if (socket_ != -1) {
#ifdef _WIN32
        closesocket(socket_);
#else
        ::close(socket_);
#endif
        socket_ = -1;
    }
}

std::string WebSocketConnection::getHeader(const std::string& request, const std::string& headerName) {
    std::string searchStr = headerName + ": ";
    size_t pos = request.find(searchStr);
    if (pos == std::string::npos) {
        // Try lowercase
        std::string lowerSearch = headerName;
        std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);
        searchStr = lowerSearch + ": ";
        pos = request.find(searchStr);
        if (pos == std::string::npos) {
            return "";
        }
    }
    
    pos += searchStr.size();
    size_t endPos = request.find("\r\n", pos);
    if (endPos == std::string::npos) {
        return "";
    }
    
    return request.substr(pos, endPos - pos);
}

std::string WebSocketConnection::generateAcceptKey(const std::string& key) {
    // Concatenar key + magic string
    std::string combined = key + WEBSOCKET_MAGIC;
    
    // SHA-1 hash
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.size(), hash);
    
    // Base64 encode
    return base64Encode(hash, SHA_DIGEST_LENGTH);
}

bool WebSocketConnection::sendRaw(const uint8_t* data, size_t length) {
    ssize_t sent = send(socket_, reinterpret_cast<const char*>(data), length, 0);
    return sent == static_cast<ssize_t>(length);
}

ssize_t WebSocketConnection::receiveRaw(uint8_t* buffer, size_t maxLength) {
    // ✅ SEM TIMEOUT - Bloquear indefinidamente
    // TCP Keep-Alive cuida da conexão em nível de SO
    // Se houver problema de rede, SO detecta e fecha socket
    
    ssize_t result = recv(socket_, reinterpret_cast<char*>(buffer), maxLength, 0);
    return result;
}

void WebSocketConnection::processFrame(const WebSocketFrame& frame) {
    switch (frame.getOpCode()) {
        case OpCode::Text:
        case OpCode::Binary: {
            if (frame.isFin()) {
                // Mensagem completa
                if (onMessage_) {
                    if (!fragmentedMessage_.empty()) {
                        // Continuar fragmentação
                        fragmentedMessage_ += frame.getPayload();
                        onMessage_(fragmentedMessage_);
                        fragmentedMessage_.clear();
                    } else {
                        onMessage_(frame.getPayload());
                    }
                }
            } else {
                // Início de fragmentação
                fragmentedOpCode_ = frame.getOpCode();
                fragmentedMessage_ = frame.getPayload();
            }
            break;
        }
            
        case OpCode::Continuation: {
            if (frame.isFin()) {
                // Fim da fragmentação
                fragmentedMessage_ += frame.getPayload();
                if (onMessage_) {
                    onMessage_(fragmentedMessage_);
                }
                fragmentedMessage_.clear();
            } else {
                // Continuar fragmentação
                fragmentedMessage_ += frame.getPayload();
            }
            break;
        }
            
        case OpCode::Close: {
            LOG_INFO("WebSocket: Received close frame");
            state_ = ConnectionState::Closing;
            
            // Extrair close code e reason
            uint16_t code = 1000;
            std::string reason;
            const std::string& payload = frame.getPayload();
            if (payload.size() >= 2) {
                code = (static_cast<uint8_t>(payload[0]) << 8) | static_cast<uint8_t>(payload[1]);
                if (payload.size() > 2) {
                    reason = payload.substr(2);
                }
            }
            
            if (onClose_) {
                onClose_(code, reason);
            }
            
            // Responder com close frame
            sendFrame(WebSocketFrame::close(code, ""));
            forceClose();
            break;
        }
            
        case OpCode::Ping: {
            LOG_DEBUG("WebSocket: Received ping, sending pong");
            sendPong(frame.getPayload());
            break;
        }
            
        case OpCode::Pong: {
            LOG_DEBUG("WebSocket: Received pong");
            // Heartbeat response
            break;
        }
    }
}

} // namespace Core::WebSocket

