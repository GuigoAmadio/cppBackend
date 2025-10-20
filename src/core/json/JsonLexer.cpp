#include "JsonLexer.hpp"
#include <sstream>
#include <cctype>

namespace Core::Json {

// ==================== Token ====================

std::string Token::toString() const {
    std::ostringstream oss;
    oss << "Token(";
    
    switch (type) {
        case TokenType::LBRACE:     oss << "LBRACE"; break;
        case TokenType::RBRACE:     oss << "RBRACE"; break;
        case TokenType::LBRACKET:   oss << "LBRACKET"; break;
        case TokenType::RBRACKET:   oss << "RBRACKET"; break;
        case TokenType::COMMA:      oss << "COMMA"; break;
        case TokenType::COLON:      oss << "COLON"; break;
        case TokenType::STRING:     oss << "STRING"; break;
        case TokenType::NUMBER:     oss << "NUMBER"; break;
        case TokenType::TRUE_VAL:   oss << "TRUE"; break;
        case TokenType::FALSE_VAL:  oss << "FALSE"; break;
        case TokenType::NULL_VAL:   oss << "NULL"; break;
        case TokenType::END:        oss << "END"; break;
        case TokenType::INVALID:    oss << "INVALID"; break;
    }
    
    if (!value.empty()) {
        oss << ", \"" << value << "\"";
    }
    
    oss << ")";
    return oss.str();
}

// ==================== JsonLexer ====================

std::vector<Token> JsonLexer::tokenize() {
    std::vector<Token> tokens;
    
    while (position_ < input_.length()) {
        skipWhitespace();
        
        if (position_ >= input_.length()) {
            break;
        }
        
        char c = current();
        
        // Símbolos estruturais
        if (c == '{') {
            tokens.emplace_back(TokenType::LBRACE, "{", position_);
            advance();
        }
        else if (c == '}') {
            tokens.emplace_back(TokenType::RBRACE, "}", position_);
            advance();
        }
        else if (c == '[') {
            tokens.emplace_back(TokenType::LBRACKET, "[", position_);
            advance();
        }
        else if (c == ']') {
            tokens.emplace_back(TokenType::RBRACKET, "]", position_);
            advance();
        }
        else if (c == ',') {
            tokens.emplace_back(TokenType::COMMA, ",", position_);
            advance();
        }
        else if (c == ':') {
            tokens.emplace_back(TokenType::COLON, ":", position_);
            advance();
        }
        
        // String
        else if (c == '"') {
            tokens.push_back(tokenizeString());
        }
        
        // Number (pode começar com - ou dígito)
        else if (c == '-' || isDigit(c)) {
            tokens.push_back(tokenizeNumber());
        }
        
        // Keywords (true, false, null)
        else if (std::isalpha(c)) {
            tokens.push_back(tokenizeKeyword());
        }
        
        else {
            error(std::string("Caractere inesperado: '") + c + "'");
        }
    }
    
    tokens.emplace_back(TokenType::END, "", position_);
    return tokens;
}

Token JsonLexer::tokenizeString() {
    size_t start = position_;
    advance();  // Pular '"' inicial
    
    std::string value;
    
    while (position_ < input_.length() && current() != '"') {
        if (current() == '\\') {
            // Escape sequence
            advance();
            if (position_ >= input_.length()) {
                error("String não terminada");
            }
            
            char escaped = current();
            switch (escaped) {
                case '"':  value += '"'; break;
                case '\\': value += '\\'; break;
                case '/':  value += '/'; break;
                case 'b':  value += '\b'; break;
                case 'f':  value += '\f'; break;
                case 'n':  value += '\n'; break;
                case 'r':  value += '\r'; break;
                case 't':  value += '\t'; break;
                case 'u':  
                    // TODO: Unicode \uXXXX (complexo, deixar para depois)
                    error("Unicode escape não implementado ainda");
                    break;
                default:
                    error(std::string("Escape inválido: \\") + escaped);
            }
            advance();
        }
        else {
            value += current();
            advance();
        }
    }
    
    if (position_ >= input_.length()) {
        error("String não terminada");
    }
    
    advance();  // Pular '"' final
    
    return Token(TokenType::STRING, value, start);
}

Token JsonLexer::tokenizeNumber() {
    size_t start = position_;
    std::string value;
    
    // Sinal negativo opcional
    if (current() == '-') {
        value += current();
        advance();
    }
    
    // Parte inteira
    if (current() == '0') {
        value += current();
        advance();
        // JSON não permite 0123 (leading zeros), exceto "0" sozinho
    }
    else if (isDigit(current())) {
        while (position_ < input_.length() && isDigit(current())) {
            value += current();
            advance();
        }
    }
    else {
        error("Número inválido");
    }
    
    // Parte decimal (.123)
    if (position_ < input_.length() && current() == '.') {
        value += current();
        advance();
        
        if (!isDigit(current())) {
            error("Dígito esperado após '.'");
        }
        
        while (position_ < input_.length() && isDigit(current())) {
            value += current();
            advance();
        }
    }
    
    // Exponencial (e10, E-5)
    if (position_ < input_.length() && (current() == 'e' || current() == 'E')) {
        value += current();
        advance();
        
        if (current() == '+' || current() == '-') {
            value += current();
            advance();
        }
        
        if (!isDigit(current())) {
            error("Dígito esperado no exponencial");
        }
        
        while (position_ < input_.length() && isDigit(current())) {
            value += current();
            advance();
        }
    }
    
    return Token(TokenType::NUMBER, value, start);
}

Token JsonLexer::tokenizeKeyword() {
    size_t start = position_;
    std::string value;
    
    while (position_ < input_.length() && std::isalpha(current())) {
        value += current();
        advance();
    }
    
    if (value == "true") {
        return Token(TokenType::TRUE_VAL, value, start);
    }
    else if (value == "false") {
        return Token(TokenType::FALSE_VAL, value, start);
    }
    else if (value == "null") {
        return Token(TokenType::NULL_VAL, value, start);
    }
    else {
        error("Keyword inválido: " + value);
    }
}

// ==================== Helpers ====================

char JsonLexer::current() const {
    if (position_ >= input_.length()) {
        return '\0';
    }
    return input_[position_];
}

char JsonLexer::peek(size_t offset) const {
    size_t pos = position_ + offset;
    if (pos >= input_.length()) {
        return '\0';
    }
    return input_[pos];
}

void JsonLexer::advance() {
    if (position_ < input_.length()) {
        position_++;
    }
}

void JsonLexer::skipWhitespace() {
    while (position_ < input_.length() && isWhitespace(current())) {
        advance();
    }
}

bool JsonLexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool JsonLexer::isWhitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

[[noreturn]] void JsonLexer::error(const std::string& message) const {
    std::ostringstream oss;
    oss << "Erro de Lexer na posição " << position_ << ": " << message;
    throw std::runtime_error(oss.str());
}

} // namespace Core::Json

