#pragma once

#include <memory>
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include "../../../core/json/Json.hpp"
#include "../repositories/CategoryRepository.hpp"
#include "../use_cases/CreateCategoryUseCase.hpp"
#include "../use_cases/GetCategoryUseCase.hpp"
#include "../use_cases/ListCategoriesUseCase.hpp"
#include "../use_cases/UpdateCategoryUseCase.hpp"
#include "../use_cases/DeleteCategoryUseCase.hpp"

namespace Domain {
namespace Category {

using Core::Http::Request;
using Core::Http::Response;
using Core::Http::StatusCode;

class CategoryController {
public:
    explicit CategoryController(std::shared_ptr<CategoryRepository> repository)
        : repository_(repository) {}

    Response createCategory(const Request& req);
    Response getCategory(const Request& req);
    Response listCategories(const Request& req);
    Response getCategoryChildren(const Request& req);  // GET /:id/children
    Response getRootCategories(const Request& req);     // GET /root
    Response updateCategory(const Request& req);
    Response deleteCategory(const Request& req);

private:
    std::shared_ptr<CategoryRepository> repository_;
    
    std::string getQueryParam(const Request& req, const std::string& key) const {
        return req.getQuery(key);
    }
};

} // namespace Category
} // namespace Domain

