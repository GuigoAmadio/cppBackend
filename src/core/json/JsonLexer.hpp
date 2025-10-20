#pragma once

#include <string>
#include <vector>
#include <stdexcept>

namespace Core::Json {

/**
 * Token Types do JSON
 * 
 * JSON tem apenas 6 tipos de valores:
 * - Object:  {}
 * - Array:   []
 * - String:  "texto"
 * - Number:  123, 45.67, -3.14e10
 * - Boolean: true, false
 * - Null:    null
 */
enum class TokenType {
    // Estrutura
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]
    COMMA,          // ,
    COLON,          // :
    
    // Valores
    STRING,         // "texto"
    NUMBER,         // 123
    TRUE_VAL,       // true
    FALSE_VAL,      // false
    NULL_VAL,       // null
    
    // Controle
    END,            // Fim do input
    INVALID         // Token inválido
};

/**
 * Token
 * 
 * Representa uma unidade léxica do JSON
 */
struct Token {
    TokenType type;
    std::string value;      // Valor bruto (ex: "123" para NUMBER)
    size_t position;        // Posição no texto original (para debug)
    
    Token(TokenType t, const std::string& v = "", size_t pos = 0)
        : type(t), value(v), position(pos) {}
    
    // Helper para debug
    std::string toString() const;
};

/**
 * JsonLexer
 * 
 * Tokeniza uma string JSON.
 * 
 * Exemplo de uso:
 * 
 *   JsonLexer lexer(R"({"name": "John", "age": 30})");
 *   auto tokens = lexer.tokenize();
 *   
 *   // tokens[0] = { LBRACE, "{", 0 }
 *   // tokens[1] = { STRING, "name", 1 }
 *   // tokens[2] = { COLON, ":", 7 }
 *   // ...
 */
class JsonLexer {
public:
    explicit JsonLexer(const std::string& input)
        : input_(input), position_(0) {}
    
    // Tokeniza todo o input
    std::vector<Token> tokenize();
    
private:
    std::string input_;
    size_t position_;
    
    // Helpers
    char current() const;
    char peek(size_t offset = 1) const;
    void advance();
    void skipWhitespace();
    
    // Parsers específicos
    Token tokenizeString();
    Token tokenizeNumber();
    Token tokenizeKeyword();  // true, false, null
    
    // Validações
    bool isDigit(char c) const;
    bool isWhitespace(char c) const;
    
    // Erro
    [[noreturn]] void error(const std::string& message) const;
};

} // namespace Core::Json

