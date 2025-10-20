#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

namespace Core::Json {

/**
 * JsonValue
 * 
 * Representa um valor JSON genérico.
 * JSON tem 6 tipos: Object, Array, String, Number, Boolean, Null
 * 
 * Esta classe usa std::variant ou um union discriminado.
 * Por simplicidade didática, vamos usar herança:
 * 
 *   JsonValue (base abstrata)
 *      ├── JsonObject
 *      ├── JsonArray
 *      ├── JsonString
 *      ├── JsonNumber
 *      ├── JsonBool
 *      └── JsonNull
 */

enum class JsonType {
    OBJECT,
    ARRAY,
    STRING,
    NUMBER,
    BOOLEAN,
    NULL_TYPE
};

class JsonValue {
public:
    virtual ~JsonValue() = default;
    
    // Type checking
    virtual JsonType getType() const = 0;
    
    bool isObject() const { return getType() == JsonType::OBJECT; }
    bool isArray() const { return getType() == JsonType::ARRAY; }
    bool isString() const { return getType() == JsonType::STRING; }
    bool isNumber() const { return getType() == JsonType::NUMBER; }
    bool isBool() const { return getType() == JsonType::BOOLEAN; }
    bool isNull() const { return getType() == JsonType::NULL_TYPE; }
    
    // Conversões (com type checking)
    virtual std::map<std::string, std::shared_ptr<JsonValue>>& asObject();
    virtual std::vector<std::shared_ptr<JsonValue>>& asArray();
    virtual std::string& asString();
    virtual double asNumber() const;
    virtual bool asBool() const;
    
    // Acesso conveniente (object)
    std::shared_ptr<JsonValue> get(const std::string& key);
    bool has(const std::string& key);
    
    // Acesso conveniente (array)
    std::shared_ptr<JsonValue> get(size_t index);
    size_t size() const;
    
    // Serialização
    virtual std::string toString(int indent = 0) const = 0;
    
protected:
    [[noreturn]] void typeError(const std::string& expected) const;
};

// ==================== Implementações Concretas ====================

class JsonObject : public JsonValue {
public:
    JsonObject() = default;
    JsonObject(const std::map<std::string, std::shared_ptr<JsonValue>>& values)
        : values_(values) {}
    
    JsonType getType() const override { return JsonType::OBJECT; }
    
    std::map<std::string, std::shared_ptr<JsonValue>>& asObject() override {
        return values_;
    }
    
    void set(const std::string& key, std::shared_ptr<JsonValue> value) {
        values_[key] = value;
    }
    
    std::string toString(int indent = 0) const override;
    
private:
    std::map<std::string, std::shared_ptr<JsonValue>> values_;
};

class JsonArray : public JsonValue {
public:
    JsonArray() = default;
    JsonArray(const std::vector<std::shared_ptr<JsonValue>>& values)
        : values_(values) {}
    
    JsonType getType() const override { return JsonType::ARRAY; }
    
    std::vector<std::shared_ptr<JsonValue>>& asArray() override {
        return values_;
    }
    
    void push(std::shared_ptr<JsonValue> value) {
        values_.push_back(value);
    }
    
    std::string toString(int indent = 0) const override;
    
private:
    std::vector<std::shared_ptr<JsonValue>> values_;
};

class JsonString : public JsonValue {
public:
    explicit JsonString(const std::string& value) : value_(value) {}
    
    JsonType getType() const override { return JsonType::STRING; }
    
    std::string& asString() override { return value_; }
    
    std::string toString(int indent = 0) const override;
    
private:
    std::string value_;
};

class JsonNumber : public JsonValue {
public:
    explicit JsonNumber(double value) : value_(value) {}
    
    JsonType getType() const override { return JsonType::NUMBER; }
    
    double asNumber() const override { return value_; }
    
    std::string toString(int indent = 0) const override;
    
private:
    double value_;
};

class JsonBool : public JsonValue {
public:
    explicit JsonBool(bool value) : value_(value) {}
    
    JsonType getType() const override { return JsonType::BOOLEAN; }
    
    bool asBool() const override { return value_; }
    
    std::string toString(int indent = 0) const override;
    
private:
    bool value_;
};

class JsonNull : public JsonValue {
public:
    JsonNull() = default;
    
    JsonType getType() const override { return JsonType::NULL_TYPE; }
    
    std::string toString(int indent = 0) const override;
};

// ==================== Factory Helpers ====================

inline std::shared_ptr<JsonValue> makeObject() {
    return std::make_shared<JsonObject>();
}

inline std::shared_ptr<JsonValue> makeArray() {
    return std::make_shared<JsonArray>();
}

inline std::shared_ptr<JsonValue> makeString(const std::string& value) {
    return std::make_shared<JsonString>(value);
}

inline std::shared_ptr<JsonValue> makeNumber(double value) {
    return std::make_shared<JsonNumber>(value);
}

inline std::shared_ptr<JsonValue> makeBool(bool value) {
    return std::make_shared<JsonBool>(value);
}

inline std::shared_ptr<JsonValue> makeNull() {
    return std::make_shared<JsonNull>();
}

} // namespace Core::Json

