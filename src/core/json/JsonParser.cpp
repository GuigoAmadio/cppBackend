#include "JsonParser.hpp"
#include <sstream>

namespace Core::Json {

JsonParser::JsonParser(const std::string& input)
    : position_(0) {
    // Tokeniza o input
    JsonLexer lexer(input);
    tokens_ = lexer.tokenize();
}

std::shared_ptr<JsonValue> JsonParser::parse() {
    if (tokens_.empty() || tokens_[0].type == TokenType::END) {
        error("Input vazio");
    }
    
    auto value = parseValue();
    
    // Deve ter chegado ao END
    if (current().type != TokenType::END) {
        error("Conteúdo extra após o valor JSON");
    }
    
    return value;
}

// ==================== Parsers Recursivos ====================

std::shared_ptr<JsonValue> JsonParser::parseValue() {
    const Token& token = current();
    
    switch (token.type) {
        case TokenType::LBRACE:
            return parseObject();
        
        case TokenType::LBRACKET:
            return parseArray();
        
        case TokenType::STRING:
            return parseString();
        
        case TokenType::NUMBER:
            return parseNumber();
        
        case TokenType::TRUE_VAL:
        case TokenType::FALSE_VAL:
            return parseBool();
        
        case TokenType::NULL_VAL:
            return parseNull();
        
        default:
            error("Valor JSON esperado");
    }
}

std::shared_ptr<JsonValue> JsonParser::parseObject() {
    expect(TokenType::LBRACE, "'{' esperado");
    advance();
    
    auto obj = std::make_shared<JsonObject>();
    
    // Object vazio?
    if (match(TokenType::RBRACE)) {
        advance();
        return obj;
    }
    
    // Parse pairs: "key": value
    while (true) {
        // Key deve ser STRING
        if (current().type != TokenType::STRING) {
            error("String esperada como chave do objeto");
        }
        std::string key = current().value;
        advance();
        
        // ':'
        expect(TokenType::COLON, "':' esperado após chave");
        advance();
        
        // Value
        auto value = parseValue();
        obj->set(key, value);
        
        // ',' ou '}'
        if (match(TokenType::COMMA)) {
            advance();
            // Continua para próximo par
        }
        else if (match(TokenType::RBRACE)) {
            advance();
            break;  // Fim do objeto
        }
        else {
            error("',' ou '}' esperado");
        }
    }
    
    return obj;
}

std::shared_ptr<JsonValue> JsonParser::parseArray() {
    expect(TokenType::LBRACKET, "'[' esperado");
    advance();
    
    auto arr = std::make_shared<JsonArray>();
    
    // Array vazio?
    if (match(TokenType::RBRACKET)) {
        advance();
        return arr;
    }
    
    // Parse values
    while (true) {
        auto value = parseValue();
        arr->push(value);
        
        // ',' ou ']'
        if (match(TokenType::COMMA)) {
            advance();
            // Continua para próximo valor
        }
        else if (match(TokenType::RBRACKET)) {
            advance();
            break;  // Fim do array
        }
        else {
            error("',' ou ']' esperado");
        }
    }
    
    return arr;
}

std::shared_ptr<JsonValue> JsonParser::parseString() {
    expect(TokenType::STRING, "String esperada");
    std::string value = current().value;
    advance();
    return makeString(value);
}

std::shared_ptr<JsonValue> JsonParser::parseNumber() {
    expect(TokenType::NUMBER, "Number esperado");
    
    // Converter string para double
    double value;
    try {
        value = std::stod(current().value);
    } catch (...) {
        error("Número inválido: " + current().value);
    }
    
    advance();
    return makeNumber(value);
}

std::shared_ptr<JsonValue> JsonParser::parseBool() {
    if (current().type == TokenType::TRUE_VAL) {
        advance();
        return makeBool(true);
    }
    else if (current().type == TokenType::FALSE_VAL) {
        advance();
        return makeBool(false);
    }
    else {
        error("Boolean esperado");
    }
}

std::shared_ptr<JsonValue> JsonParser::parseNull() {
    expect(TokenType::NULL_VAL, "null esperado");
    advance();
    return makeNull();
}

// ==================== Helpers ====================

const Token& JsonParser::current() const {
    if (position_ >= tokens_.size()) {
        static Token endToken(TokenType::END);
        return endToken;
    }
    return tokens_[position_];
}

const Token& JsonParser::peek(size_t offset) const {
    size_t pos = position_ + offset;
    if (pos >= tokens_.size()) {
        static Token endToken(TokenType::END);
        return endToken;
    }
    return tokens_[pos];
}

void JsonParser::advance() {
    if (position_ < tokens_.size()) {
        position_++;
    }
}

bool JsonParser::match(TokenType type) {
    return current().type == type;
}

void JsonParser::expect(TokenType type, const std::string& message) {
    if (!match(type)) {
        error(message);
    }
}

[[noreturn]] void JsonParser::error(const std::string& message) const {
    std::ostringstream oss;
    oss << "Erro de Parser";
    
    if (position_ < tokens_.size()) {
        oss << " no token " << position_ << " (" << tokens_[position_].toString() << ")";
    }
    
    oss << ": " << message;
    throw std::runtime_error(oss.str());
}

} // namespace Core::Json

