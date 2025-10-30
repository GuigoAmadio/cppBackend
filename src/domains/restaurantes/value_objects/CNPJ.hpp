#pragma once

#include <string>
#include <stdexcept>
#include <algorithm>

namespace Domains::Restaurantes::ValueObjects {

/**
 * @brief Value Object para CNPJ
 */
class CNPJ {
public:
    explicit CNPJ(const std::string& cnpj) : value_(cnpj) {
        validate();
    }

    std::string getValue() const { return value_; }

    /**
     * @brief Remove caracteres não numéricos do CNPJ
     */
    static std::string limpar(const std::string& cnpj) {
        std::string limpo;
        std::copy_if(cnpj.begin(), cnpj.end(), std::back_inserter(limpo),
                     [](char c) { return std::isdigit(c); });
        return limpo;
    }

    bool operator==(const CNPJ& other) const {
        return value_ == other.value_;
    }

private:
    std::string value_;

    void validate() const {
        std::string limpo = limpar(value_);
        
        if (limpo.length() != 14) {
            throw std::invalid_argument("CNPJ deve conter 14 dígitos");
        }
        
        // Verifica se todos os dígitos são iguais (CNPJ inválido)
        bool todosIguais = std::all_of(limpo.begin(), limpo.end(),
            [&limpo](char c) { return c == limpo[0]; });
        
        if (todosIguais) {
            throw std::invalid_argument("CNPJ inválido: todos os dígitos são iguais");
        }
        
        // TODO: Implementar validação completa do CNPJ (dígitos verificadores)
        // Por enquanto, apenas valida formato básico
    }
};

} // namespace Domains::Restaurantes::ValueObjects

