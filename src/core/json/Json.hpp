#pragma once

#include "JsonValue.hpp"
#include "JsonParser.hpp"
#include <string>
#include <memory>

namespace Core::Json {

/**
 * Json - Interface simples para usar o parser
 * 
 * Exemplo:
 * 
 *   // Parse
 *   auto json = Json::parse(R"({"name": "John", "age": 30})");
 *   std::string name = json->get("name")->asString();
 *   double age = json->get("age")->asNumber();
 *   
 *   // Stringify
 *   std::string text = Json::stringify(json);
 */
class Json {
public:
    // Parse JSON string -> JsonValue
    static std::shared_ptr<JsonValue> parse(const std::string& input);
    
    // Stringify JsonValue -> JSON string
    static std::string stringify(const std::shared_ptr<JsonValue>& value, bool pretty = false);
    
    // Helpers para criar JSON manualmente
    static std::shared_ptr<JsonValue> object();
    static std::shared_ptr<JsonValue> array();
    static std::shared_ptr<JsonValue> string(const std::string& value);
    static std::shared_ptr<JsonValue> number(double value);
    static std::shared_ptr<JsonValue> boolean(bool value);
    static std::shared_ptr<JsonValue> null();
};

} // namespace Core::Json

