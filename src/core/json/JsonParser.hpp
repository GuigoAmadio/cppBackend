#pragma once

#include "JsonLexer.hpp"
#include "JsonValue.hpp"
#include <memory>
#include <vector>

namespace Core::Json {

/**
 * JsonParser
 * 
 * Constrói uma árvore de JsonValue a partir de tokens.
 * 
 * Gramática JSON (simplificada):
 * 
 *   value    := object | array | string | number | true | false | null
 *   object   := '{' (pair (',' pair)*)? '}'
 *   pair     := string ':' value
 *   array    := '[' (value (',' value)*)? ']'
 * 
 * Exemplo de uso:
 * 
 *   std::string json = R"({"name": "John", "age": 30})";
 *   JsonParser parser(json);
 *   auto value = parser.parse();
 *   
 *   // value é um JsonObject com:
 *   // - "name" => JsonString("John")
 *   // - "age"  => JsonNumber(30)
 */
class JsonParser {
public:
    explicit JsonParser(const std::string& input);
    
    // Parseia e retorna o valor raiz
    std::shared_ptr<JsonValue> parse();
    
private:
    std::vector<Token> tokens_;
    size_t position_;
    
    // Helpers
    const Token& current() const;
    const Token& peek(size_t offset = 1) const;
    void advance();
    bool match(TokenType type);
    void expect(TokenType type, const std::string& message);
    
    // Parsers recursivos
    std::shared_ptr<JsonValue> parseValue();
    std::shared_ptr<JsonValue> parseObject();
    std::shared_ptr<JsonValue> parseArray();
    std::shared_ptr<JsonValue> parseString();
    std::shared_ptr<JsonValue> parseNumber();
    std::shared_ptr<JsonValue> parseBool();
    std::shared_ptr<JsonValue> parseNull();
    
    // Erro
    [[noreturn]] void error(const std::string& message) const;
};

} // namespace Core::Json

