#pragma once

#include <memory>
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../../../core/json/Json.hpp"
#include "../repositories/ProductRepository.hpp"
#include "../use_cases/CreateProductUseCase.hpp"
#include "../use_cases/GetProductUseCase.hpp"
#include "../use_cases/ListProductsUseCase.hpp"
#include "../use_cases/UpdateProductUseCase.hpp"
#include "../use_cases/DeleteProductUseCase.hpp"

namespace Domain {
namespace Product {

using Core::Http::Request;
using Core::Http::Response;
using Core::Http::StatusCode;

class ProductController {
public:
    explicit ProductController(std::shared_ptr<ProductRepository> repository)
        : repository_(repository) {}

    // POST /api/products
    Response createProduct(const Request& req);

    // GET /api/products/:id
    Response getProduct(const Request& req);

    // GET /api/products
    Response listProducts(const Request& req);

    // PUT /api/products/:id
    Response updateProduct(const Request& req);

    // DELETE /api/products/:id
    Response deleteProduct(const Request& req);

    // GET /api/products/low-stock
    Response listLowStock(const Request& req);

    // GET /api/products/search?q=query
    Response searchProducts(const Request& req);

private:
    std::shared_ptr<ProductRepository> repository_;

    // Helpers
    std::string getTenantId(const Request& req) const {
        return req.getCustomData("user_tenant_id");
    }

    std::string getUserId(const Request& req) const {
        return req.getCustomData("user_id");
    }

    std::string getQueryParam(const Request& req, const std::string& key) const {
        // Usar método do Request que acessa o mapa query_ populado
        return req.getQuery(key);
    }

    // Converte Product para JSON
    std::shared_ptr<Core::Json::JsonValue> productToJson(const Product& product);
};

} // namespace Product
} // namespace Domain

