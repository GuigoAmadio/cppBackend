#pragma once

#include <memory>
#include "../repositories/InventoryRepository.hpp"
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../../../core/json/Json.hpp"

using namespace Core::Http;

namespace Domain {
namespace Inventory {

class InventoryController {
public:
    explicit InventoryController(std::shared_ptr<InventoryRepository> repository)
        : repository_(repository) {}

    // POST /api/inventory/transactions - Registrar nova transação
    Response recordTransaction(const Request& req);
    
    // GET /api/inventory/history?product_id=xxx&location_id=xxx - Histórico
    Response getHistory(const Request& req);
    
    // GET /api/inventory/stock/:productId - Estoque atual de um produto
    Response getCurrentStock(const Request& req);

private:
    std::shared_ptr<InventoryRepository> repository_;
    
    // Helpers
    std::string getTenantId(const Request& req) const {
        return req.getCustomData("user_tenant_id");
    }
    
    std::string getUserId(const Request& req) const {
        return req.getCustomData("user_id");
    }
    
    std::string getQueryParam(const Request& req, const std::string& key) const {
        return req.getQuery(key);
    }
    
    // Converte InventoryTransaction para JSON
    std::shared_ptr<Core::Json::JsonValue> transactionToJson(const InventoryTransaction& transaction);
};

} // namespace Inventory
} // namespace Domain

