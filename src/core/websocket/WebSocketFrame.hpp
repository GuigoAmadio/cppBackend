#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace Core::WebSocket {

/**
 * @brief Tipos de frame WebSocket (RFC 6455)
 */
enum class OpCode : uint8_t {
    Continuation = 0x0,
    Text = 0x1,
    Binary = 0x2,
    Close = 0x8,
    Ping = 0x9,
    Pong = 0xA
};

/**
 * @brief Representa um frame WebSocket
 * 
 * Estrutura do frame (RFC 6455):
 * 
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-------+-+-------------+-------------------------------+
 * |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 * |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 * |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 * | |1|2|3|       |K|             |                               |
 * +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 * |     Extended payload length continued, if payload len == 127  |
 * + - - - - - - - - - - - - - - - +-------------------------------+
 * |                               |Masking-key, if MASK set to 1  |
 * +-------------------------------+-------------------------------+
 * | Masking-key (continued)       |          Payload Data         |
 * +-------------------------------- - - - - - - - - - - - - - - - +
 * :                     Payload Data continued ...                :
 * + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 * |                     Payload Data continued ...                |
 * +---------------------------------------------------------------+
 */
class WebSocketFrame {
public:
    /**
     * @brief Construtor padrão
     */
    WebSocketFrame();
    
    /**
     * @brief Construtor com dados
     */
    WebSocketFrame(OpCode opcode, const std::string& payload, bool fin = true);
    
    /**
     * @brief Parse bytes brutos para frame
     * @param data Dados brutos
     * @param length Tamanho dos dados
     * @return Frame parseado ou std::nullopt se inválido/incompleto
     */
    static std::optional<WebSocketFrame> parse(const uint8_t* data, size_t length);
    
    /**
     * @brief Serializa frame para bytes
     * @param mask Se true, aplica máscara (cliente -> servidor)
     * @return Bytes do frame
     */
    std::vector<uint8_t> serialize(bool mask = false) const;
    
    // ========================================
    // GETTERS/SETTERS
    // ========================================
    
    bool isFin() const { return fin_; }
    void setFin(bool fin) { fin_ = fin; }
    
    OpCode getOpCode() const { return opcode_; }
    void setOpCode(OpCode opcode) { opcode_ = opcode; }
    
    bool isMasked() const { return masked_; }
    void setMasked(bool masked) { masked_ = masked; }
    
    const std::string& getPayload() const { return payload_; }
    void setPayload(const std::string& payload) { payload_ = payload; }
    
    size_t getPayloadLength() const { return payload_.size(); }
    
    // ========================================
    // HELPERS
    // ========================================
    
    /**
     * @brief Verifica se é frame de controle (Close, Ping, Pong)
     */
    bool isControlFrame() const {
        return opcode_ == OpCode::Close || 
               opcode_ == OpCode::Ping || 
               opcode_ == OpCode::Pong;
    }
    
    /**
     * @brief Verifica se é frame de dados (Text, Binary)
     */
    bool isDataFrame() const {
        return opcode_ == OpCode::Text || 
               opcode_ == OpCode::Binary;
    }
    
    /**
     * @brief Cria frame de texto
     */
    static WebSocketFrame text(const std::string& payload) {
        return WebSocketFrame(OpCode::Text, payload, true);
    }
    
    /**
     * @brief Cria frame de close
     */
    static WebSocketFrame close(uint16_t code = 1000, const std::string& reason = "") {
        std::string payload;
        payload += static_cast<char>((code >> 8) & 0xFF);
        payload += static_cast<char>(code & 0xFF);
        payload += reason;
        return WebSocketFrame(OpCode::Close, payload, true);
    }
    
    /**
     * @brief Cria frame de ping
     */
    static WebSocketFrame ping(const std::string& payload = "") {
        return WebSocketFrame(OpCode::Ping, payload, true);
    }
    
    /**
     * @brief Cria frame de pong
     */
    static WebSocketFrame pong(const std::string& payload = "") {
        return WebSocketFrame(OpCode::Pong, payload, true);
    }

private:
    bool fin_;           // FIN bit
    uint8_t rsv_;        // RSV1, RSV2, RSV3 (reservado)
    OpCode opcode_;      // Opcode
    bool masked_;        // MASK bit
    uint8_t maskingKey_[4]; // Masking key (se masked=true)
    std::string payload_; // Payload data
    
    /**
     * @brief Aplica/remove máscara no payload
     */
    static void applyMask(uint8_t* data, size_t length, const uint8_t* mask);
};

} // namespace Core::WebSocket

