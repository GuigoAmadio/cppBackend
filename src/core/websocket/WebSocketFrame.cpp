#include "WebSocketFrame.hpp"
#include "../utils/LoggerNew.hpp"
#include <cstring>
#include <random>

namespace Core::WebSocket {

WebSocketFrame::WebSocketFrame()
    : fin_(true), rsv_(0), opcode_(OpCode::Text), masked_(false), payload_("") {
    std::memset(maskingKey_, 0, 4);
}

WebSocketFrame::WebSocketFrame(OpCode opcode, const std::string& payload, bool fin)
    : fin_(fin), rsv_(0), opcode_(opcode), masked_(false), payload_(payload) {
    std::memset(maskingKey_, 0, 4);
}

std::optional<WebSocketFrame> WebSocketFrame::parse(const uint8_t* data, size_t length) {
    if (length < 2) {
        // Frame incompleto - precisa de pelo menos 2 bytes
        return std::nullopt;
    }
    
    WebSocketFrame frame;
    size_t offset = 0;
    
    // Byte 0: FIN, RSV, Opcode
    uint8_t byte0 = data[offset++];
    frame.fin_ = (byte0 & 0x80) != 0;
    frame.rsv_ = (byte0 & 0x70) >> 4;
    frame.opcode_ = static_cast<OpCode>(byte0 & 0x0F);
    
    // Byte 1: MASK, Payload length
    uint8_t byte1 = data[offset++];
    frame.masked_ = (byte1 & 0x80) != 0;
    uint64_t payloadLength = byte1 & 0x7F;
    
    // Extended payload length
    if (payloadLength == 126) {
        if (length < offset + 2) {
            return std::nullopt; // Incompleto
        }
        payloadLength = (static_cast<uint64_t>(data[offset]) << 8) |
                       static_cast<uint64_t>(data[offset + 1]);
        offset += 2;
    } else if (payloadLength == 127) {
        if (length < offset + 8) {
            return std::nullopt; // Incompleto
        }
        payloadLength = 0;
        for (int i = 0; i < 8; ++i) {
            payloadLength = (payloadLength << 8) | static_cast<uint64_t>(data[offset + i]);
        }
        offset += 8;
    }
    
    // Masking key (se masked=true)
    if (frame.masked_) {
        if (length < offset + 4) {
            return std::nullopt; // Incompleto
        }
        std::memcpy(frame.maskingKey_, data + offset, 4);
        offset += 4;
    }
    
    // Payload data
    if (length < offset + payloadLength) {
        return std::nullopt; // Incompleto
    }
    
    frame.payload_.resize(payloadLength);
    if (payloadLength > 0) {
        std::memcpy(&frame.payload_[0], data + offset, payloadLength);
        
        // Unmask se necessário
        if (frame.masked_) {
            applyMask(reinterpret_cast<uint8_t*>(&frame.payload_[0]), 
                     payloadLength, 
                     frame.maskingKey_);
        }
    }
    
    return frame;
}

std::vector<uint8_t> WebSocketFrame::serialize(bool mask) const {
    std::vector<uint8_t> buffer;
    
    // Byte 0: FIN, RSV, Opcode
    uint8_t byte0 = (fin_ ? 0x80 : 0x00) | (rsv_ << 4) | static_cast<uint8_t>(opcode_);
    buffer.push_back(byte0);
    
    // Byte 1: MASK, Payload length
    size_t payloadLength = payload_.size();
    uint8_t byte1 = (mask ? 0x80 : 0x00);
    
    if (payloadLength < 126) {
        byte1 |= static_cast<uint8_t>(payloadLength);
        buffer.push_back(byte1);
    } else if (payloadLength <= 0xFFFF) {
        byte1 |= 126;
        buffer.push_back(byte1);
        buffer.push_back(static_cast<uint8_t>((payloadLength >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(payloadLength & 0xFF));
    } else {
        byte1 |= 127;
        buffer.push_back(byte1);
        for (int i = 7; i >= 0; --i) {
            buffer.push_back(static_cast<uint8_t>((payloadLength >> (i * 8)) & 0xFF));
        }
    }
    
    // Masking key (se mask=true)
    uint8_t maskingKey[4] = {0};
    if (mask) {
        // Gerar masking key aleatório
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        for (int i = 0; i < 4; ++i) {
            maskingKey[i] = static_cast<uint8_t>(dis(gen));
            buffer.push_back(maskingKey[i]);
        }
    }
    
    // Payload data
    if (payloadLength > 0) {
        size_t startSize = buffer.size();
        buffer.resize(startSize + payloadLength);
        std::memcpy(&buffer[startSize], payload_.data(), payloadLength);
        
        // Apply mask se necessário
        if (mask) {
            applyMask(&buffer[startSize], payloadLength, maskingKey);
        }
    }
    
    return buffer;
}

void WebSocketFrame::applyMask(uint8_t* data, size_t length, const uint8_t* mask) {
    for (size_t i = 0; i < length; ++i) {
        data[i] ^= mask[i % 4];
    }
}

} // namespace Core::WebSocket

