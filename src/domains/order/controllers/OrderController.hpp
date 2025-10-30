#pragma once
#include <memory>
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../../../core/json/Json.hpp"
#include "../repositories/OrderRepository.hpp"
#include "../use_cases/CreateOrderUseCase.hpp"
#include "../use_cases/GetOrderUseCase.hpp"
#include "../use_cases/ListOrdersUseCase.hpp"

namespace Domain {
namespace Order {

class OrderController {
public:
    explicit OrderController(std::shared_ptr<OrderRepository> repository)
        : repository_(repository) {}
    
    Core::Http::Response createOrder(const Core::Http::Request& req);
    Core::Http::Response getOrder(const Core::Http::Request& req);
    Core::Http::Response listOrders(const Core::Http::Request& req);
    Core::Http::Response updateStatus(const Core::Http::Request& req);
    Core::Http::Response updatePaymentStatus(const Core::Http::Request& req);
    Core::Http::Response cancelOrder(const Core::Http::Request& req);
    Core::Http::Response listCustomerOrders(const Core::Http::Request& req);
    
private:
    std::shared_ptr<OrderRepository> repository_;
    
    std::string getQueryParam(const Core::Http::Request& req, const std::string& key) {
        std::string path = req.getPath();
        size_t pos = path.find('?');
        if (pos == std::string::npos) return "";
        
        std::string query = path.substr(pos + 1);
        std::string searchKey = key + "=";
        size_t keyPos = query.find(searchKey);
        if (keyPos == std::string::npos) return "";
        
        size_t valueStart = keyPos + searchKey.length();
        size_t valueEnd = query.find('&', valueStart);
        
        if (valueEnd == std::string::npos) {
            return query.substr(valueStart);
        }
        return query.substr(valueStart, valueEnd - valueStart);
    }
};

} // namespace Order
} // namespace Domain

