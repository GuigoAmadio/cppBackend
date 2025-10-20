#include "Json.hpp"

namespace Core::Json {

std::shared_ptr<JsonValue> Json::parse(const std::string& input) {
    JsonParser parser(input);
    return parser.parse();
}

std::string Json::stringify(const std::shared_ptr<JsonValue>& value, bool pretty) {
    if (!value) {
        return "null";
    }
    
    if (pretty) {
        return value->toString(0);
    } else {
        // TODO: Implementar versão compacta (sem indentação/newlines)
        // Por enquanto, retorna versão "pretty"
        return value->toString(0);
    }
}

std::shared_ptr<JsonValue> Json::object() {
    return makeObject();
}

std::shared_ptr<JsonValue> Json::array() {
    return makeArray();
}

std::shared_ptr<JsonValue> Json::string(const std::string& value) {
    return makeString(value);
}

std::shared_ptr<JsonValue> Json::number(double value) {
    return makeNumber(value);
}

std::shared_ptr<JsonValue> Json::boolean(bool value) {
    return makeBool(value);
}

std::shared_ptr<JsonValue> Json::null() {
    return makeNull();
}

} // namespace Core::Json

