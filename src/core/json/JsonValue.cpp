#include "JsonValue.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace Core::Json {

// ==================== JsonValue Base ====================

[[noreturn]] void JsonValue::typeError(const std::string& expected) const {
    std::ostringstream oss;
    oss << "Type error: esperado " << expected << ", mas valor é de tipo diferente";
    throw std::runtime_error(oss.str());
}

std::map<std::string, std::shared_ptr<JsonValue>>& JsonValue::asObject() {
    typeError("Object");
}

std::vector<std::shared_ptr<JsonValue>>& JsonValue::asArray() {
    typeError("Array");
}

std::string& JsonValue::asString() {
    typeError("String");
}

double JsonValue::asNumber() const {
    typeError("Number");
}

bool JsonValue::asBool() const {
    typeError("Boolean");
}

std::shared_ptr<JsonValue> JsonValue::get(const std::string& key) {
    if (!isObject()) {
        typeError("Object");
    }
    auto& obj = asObject();
    auto it = obj.find(key);
    if (it == obj.end()) {
        return nullptr;
    }
    return it->second;
}

bool JsonValue::has(const std::string& key) {
    if (!isObject()) {
        return false;
    }
    auto& obj = asObject();
    return obj.find(key) != obj.end();
}

std::shared_ptr<JsonValue> JsonValue::get(size_t index) {
    if (!isArray()) {
        typeError("Array");
    }
    auto& arr = asArray();
    if (index >= arr.size()) {
        throw std::out_of_range("Índice fora dos limites");
    }
    return arr[index];
}

size_t JsonValue::size() const {
    if (isArray()) {
        return const_cast<JsonValue*>(this)->asArray().size();
    }
    else if (isObject()) {
        return const_cast<JsonValue*>(this)->asObject().size();
    }
    return 0;
}

// ==================== toString() Implementations ====================

std::string JsonObject::toString(int indent) const {
    if (values_.empty()) {
        return "{}";
    }
    
    std::ostringstream oss;
    std::string indentStr(indent, ' ');
    std::string indentStr2(indent + 2, ' ');
    
    oss << "{\n";
    
    bool first = true;
    for (const auto& [key, value] : values_) {
        if (!first) {
            oss << ",\n";
        }
        first = false;
        
        oss << indentStr2 << "\"" << key << "\": ";
        
        if (value) {
            oss << value->toString(indent + 2);
        } else {
            oss << "null";
        }
    }
    
    oss << "\n" << indentStr << "}";
    return oss.str();
}

std::string JsonArray::toString(int indent) const {
    if (values_.empty()) {
        return "[]";
    }
    
    std::ostringstream oss;
    std::string indentStr(indent, ' ');
    std::string indentStr2(indent + 2, ' ');
    
    oss << "[\n";
    
    bool first = true;
    for (const auto& value : values_) {
        if (!first) {
            oss << ",\n";
        }
        first = false;
        
        oss << indentStr2;
        
        if (value) {
            oss << value->toString(indent + 2);
        } else {
            oss << "null";
        }
    }
    
    oss << "\n" << indentStr << "]";
    return oss.str();
}

std::string JsonString::toString(int indent) const {
    (void)indent;  // Não usado para strings
    
    std::ostringstream oss;
    oss << "\"";
    
    // Escapar caracteres especiais
    for (char c : value_) {
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (c < 32) {
                    // Caracteres de controle: \uXXXX
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else {
                    oss << c;
                }
        }
    }
    
    oss << "\"";
    return oss.str();
}

std::string JsonNumber::toString(int indent) const {
    (void)indent;  // Não usado para números
    
    std::ostringstream oss;
    
    // Verificar se é inteiro
    if (std::floor(value_) == value_ && std::abs(value_) < 1e15) {
        oss << static_cast<long long>(value_);
    } else {
        oss << std::fixed << std::setprecision(6) << value_;
        
        // Remover zeros trailing
        std::string str = oss.str();
        str.erase(str.find_last_not_of('0') + 1, std::string::npos);
        if (str.back() == '.') {
            str.pop_back();
        }
        return str;
    }
    
    return oss.str();
}

std::string JsonBool::toString(int indent) const {
    (void)indent;  // Não usado para booleans
    return value_ ? "true" : "false";
}

std::string JsonNull::toString(int indent) const {
    (void)indent;  // Não usado para null
    return "null";
}

} // namespace Core::Json
