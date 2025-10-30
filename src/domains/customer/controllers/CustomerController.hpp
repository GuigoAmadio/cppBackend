#pragma once

#include <memory>
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../../../core/json/Json.hpp"
#include "../repositories/CustomerRepository.hpp"
#include "../use_cases/CreateCustomerUseCase.hpp"
#include "../use_cases/GetCustomerUseCase.hpp"
#include "../use_cases/ListCustomersUseCase.hpp"
#include "../use_cases/UpdateCustomerUseCase.hpp"
#include "../use_cases/DeleteCustomerUseCase.hpp"

namespace Domain {
namespace Customer {

using Core::Http::Request;
using Core::Http::Response;
using Core::Http::StatusCode;

class CustomerController {
public:
    explicit CustomerController(std::shared_ptr<CustomerRepository> repository)
        : repository_(repository) {}

    Response createCustomer(const Request& req);
    Response getCustomer(const Request& req);
    Response listCustomers(const Request& req);
    Response searchCustomers(const Request& req);
    Response updateCustomer(const Request& req);
    Response deleteCustomer(const Request& req);

private:
    std::shared_ptr<CustomerRepository> repository_;
    
    std::string getQueryParam(const Request& req, const std::string& key) const {
        return req.getQuery(key);
    }
};

} // namespace Customer
} // namespace Domain

