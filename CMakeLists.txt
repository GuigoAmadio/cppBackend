cmake_minimum_required(VERSION 3.15)
project(cppBackend VERSION 1.0.0 LANGUAGES CXX)

# C++17 obrigatório
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warnings como erros (código de qualidade)
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Otimizações
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(NOT MSVC)
        add_compile_options(-O3 -march=native)
    endif()
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Find Threads (pthread)
find_package(Threads REQUIRED)

# Find hiredis (Redis client)
find_library(HIREDIS_LIB NAMES hiredis libhiredis PATHS 
    "C:/msys64/ucrt64/lib"
    "/ucrt64/lib"
    "/usr/lib"
    "/usr/lib/x86_64-linux-gnu"
    "/usr/local/lib"
    REQUIRED
)
if(NOT HIREDIS_LIB)
    message(FATAL_ERROR "hiredis library not found")
endif()
message(STATUS "Found hiredis: ${HIREDIS_LIB}")

# hiredis include directory
include_directories(
    "C:/msys64/ucrt64/include"
    "/usr/include"
    "/usr/local/include"
)
include_directories("/ucrt64/include")

# Source files - HTTP Core (FUNCIONAL)
set(CORE_SOURCES
    src/core/http/Server.cpp
    src/core/http/Request.cpp
    src/core/http/Response.cpp
    src/core/http/Router.cpp
    src/core/http/Middleware.cpp
)

# Middleware sources
set(MIDDLEWARE_SOURCES
    src/infrastructure/middleware/TenantMiddleware.cpp
    src/infrastructure/middleware/AuthMiddleware.cpp
    src/infrastructure/middleware/RoleMiddleware.cpp
    src/infrastructure/middleware/RateLimitMiddleware.cpp
    src/infrastructure/middleware/MetricsMiddleware.cpp
)

# Source files - JSON (IMPLEMENTADO)
set(JSON_SOURCES
    src/core/json/Json.cpp
    src/core/json/JsonValue.cpp
    src/core/json/JsonLexer.cpp
    src/core/json/JsonParser.cpp
)

# Threading sources
set(THREADING_SOURCES
    src/core/threading/ThreadPool.cpp
)

# Utils sources
set(UTILS_SOURCES
    src/core/utils/LoggerNew.cpp
    src/core/utils/RateLimitStorage.cpp
    src/core/utils/MetricsCollector.cpp
)

# Cache sources (Redis)
set(CACHE_SOURCES
    src/core/cache/RedisClient.cpp
    src/core/cache/RedisPool.cpp
    src/core/cache/RedisCacheService.cpp
    src/core/cache/RedisPubSub.cpp
)

# WebSocket sources
set(WEBSOCKET_SOURCES
    src/core/websocket/WebSocketFrame.cpp
    src/core/websocket/WebSocketConnection.cpp
    src/core/websocket/WebSocketManager.cpp
    src/core/websocket/WebSocketHandler.cpp
)

# Message Queue sources (Redis Streams)
set(QUEUE_SOURCES
    src/core/queue/RedisMessageQueue.cpp
    src/core/queue/QueueWorker.cpp
)

# Database sources
set(DATABASE_SOURCES
    src/core/database/Connection.cpp
    src/core/database/ConnectionPool.cpp
)

# IAM Domain sources (Identity & Access Management)
set(IAM_SOURCES
    # Value Objects
    src/domains/iam/value_objects/Email.cpp
    src/domains/iam/value_objects/Password.cpp
    
    # Services
    src/domains/iam/services/JwtService.cpp
    src/domains/iam/services/BcryptService.cpp
    
    # Use Cases
    src/domains/iam/use_cases/RegisterUserUseCase.cpp
    src/domains/iam/use_cases/LoginUserUseCase.cpp
    src/domains/iam/use_cases/RefreshTokenUseCase.cpp
    src/domains/iam/use_cases/VerifyEmailUseCase.cpp
    
    # Controllers
    src/domains/iam/controllers/AuthController.cpp
)

# User Management Domain sources
set(USER_MANAGEMENT_SOURCES
    # Entities
    src/domains/user_management/entities/User.cpp
    
    # Repositories
    src/domains/user_management/repositories/UserRepository.cpp
    
    # Use Cases
    src/domains/user_management/use_cases/GetUserUseCase.cpp
    src/domains/user_management/use_cases/UpdateUserUseCase.cpp
    src/domains/user_management/use_cases/ChangePasswordUseCase.cpp
    src/domains/user_management/use_cases/DeleteUserUseCase.cpp
    src/domains/user_management/use_cases/ListUsersUseCase.cpp
)

# Tenant Management Domain sources
set(TENANT_MANAGEMENT_SOURCES
    # Entities
    src/domains/tenant_management/entities/Tenant.cpp
    
    # Repositories
    src/domains/tenant_management/repositories/TenantRepository.cpp
    
    # Use Cases
    src/domains/tenant_management/use_cases/ListTenantMembersUseCase.cpp
    src/domains/tenant_management/use_cases/UpdateUserRoleUseCase.cpp
    src/domains/tenant_management/use_cases/RemoveUserFromTenantUseCase.cpp
    src/domains/tenant_management/use_cases/AddUserToTenantUseCase.cpp
)

# Audit Domain sources
set(AUDIT_SOURCES
    # Entities
    src/domains/audit/entities/AuditLog.cpp
    
    # Repositories
    src/domains/audit/repositories/AuditLogRepository.cpp
)

# Notification Domain sources
set(NOTIFICATION_SOURCES
    # Services
    src/domains/notification/services/EmailService.cpp
    src/domains/notification/services/EmailVerificationRepository.cpp
    src/domains/notification/services/PasswordResetRepository.cpp
    
    # Use Cases
    src/domains/notification/use_cases/SendVerificationEmailUseCase.cpp
    src/domains/notification/use_cases/RequestPasswordResetUseCase.cpp
    src/domains/notification/use_cases/ResetPasswordUseCase.cpp
)

# Workspace Domain sources
set(WORKSPACE_SOURCES
    # Repositories
    src/domains/workspace/repositories/WorkspaceRepository.cpp
    
    # Controllers
    src/domains/workspace/controllers/WorkspaceController.cpp
)

# Product Domain sources
set(PRODUCT_SOURCES
    # Repositories
    src/domains/product/repositories/ProductRepository.cpp
    
    # Controllers
    src/domains/product/controllers/ProductController.cpp
)

# Inventory Domain sources
set(INVENTORY_SOURCES
    # Repositories
    src/domains/inventory/repositories/InventoryRepository.cpp
    
    # Controllers
    src/domains/inventory/controllers/InventoryController.cpp
)

# Customer Domain sources
set(CUSTOMER_SOURCES
    # Repositories
    src/domains/customer/repositories/CustomerRepository.cpp
    
    # Controllers
    src/domains/customer/controllers/CustomerController.cpp
)

# Category Domain sources
set(CATEGORY_SOURCES
    # Repositories
    src/domains/category/repositories/CategoryRepository.cpp
    
    # Controllers
    src/domains/category/controllers/CategoryController.cpp
)

# Order Domain sources
set(ORDER_SOURCES
    # Repositories
    src/domains/order/repositories/OrderRepository.cpp
    
    # Controllers
    src/domains/order/controllers/OrderController.cpp
)

# Payment Domain sources
set(PAYMENT_SOURCES
    # Repositories
    src/domains/payment/repositories/PaymentRepositoryImpl.cpp
    
    # Controllers
    src/domains/payment/controllers/PaymentController.cpp
)

# Subscription sources
set(SUBSCRIPTION_SOURCES
    # Repositories
    src/domains/subscription/repositories/impl/PlanRepositoryImpl.cpp
    src/domains/subscription/repositories/impl/SubscriptionRepositoryImpl.cpp
    src/domains/subscription/repositories/impl/BillingHistoryRepositoryImpl.cpp
    
    # Controllers
    src/domains/subscription/controllers/SubscriptionController.cpp
)

# Invoicing sources
set(INVOICING_SOURCES
    # Repositories
    src/domains/invoicing/repositories/impl/InvoiceRepositoryImpl.cpp
    
    # Controllers
    src/domains/invoicing/controllers/InvoiceController.cpp
)

# Task Management Domain sources
set(TASK_SOURCES
    # Repositories
    src/domains/task/repositories/impl/TaskRepositoryImpl.cpp
    
    # Controllers
    src/domains/task/controllers/TaskController.cpp
)

# Database/Admin Domain sources
set(DATABASE_ADMIN_SOURCES
    # Validators
    src/domains/database/validators/QueryValidator.cpp
    
    # Controllers
    src/domains/database/controllers/DatabaseController.cpp
)

# Calendar Domain sources
set(CALENDAR_SOURCES
    # Repositories
    src/domains/calendar/repositories/impl/EventRepositoryImpl.cpp
    
    # Controllers
    src/domains/calendar/controllers/EventController.cpp
)

# Finance Domain sources
set(FINANCE_SOURCES
    # Repositories
    src/domains/finance/repositories/impl/AccountRepositoryImpl.cpp
    src/domains/finance/repositories/impl/TransactionRepositoryImpl.cpp
    src/domains/finance/repositories/impl/CategoryRepositoryImpl.cpp
    src/domains/finance/repositories/impl/BudgetRepositoryImpl.cpp
    src/domains/finance/repositories/impl/ReconciliationRepositoryImpl.cpp
    
    # Controllers
    src/domains/finance/controllers/FinanceController.cpp
)

# ==================== FREELANCER APP MODULES ====================

# Restaurantes Domain sources
set(RESTAURANTES_SOURCES
    # Repositories
    src/domains/restaurantes/repositories/RestauranteRepository.cpp
    
    # Controllers
    src/domains/restaurantes/controllers/RestauranteController.cpp
)

# Freelances Domain sources
set(FREELANCES_SOURCES
    # Repositories
    src/domains/freelances/repositories/FreelancerRepository.cpp
    
    # Controllers
    src/domains/freelances/controllers/FreelancerController.cpp
)

# Vagas Domain sources
set(VAGAS_SOURCES
    # Repositories
    src/domains/vagas/repositories/VagaRepository.cpp
    
    # Controllers
    src/domains/vagas/controllers/VagaController.cpp
)

# Candidaturas Domain sources
set(CANDIDATURAS_SOURCES
    # Repositories
    src/domains/candidaturas/repositories/CandidaturaRepository.cpp
    
    # Controllers
    src/domains/candidaturas/controllers/CandidaturaController.cpp
)

# Leilões (Sealed Bids)
set(LEILOES_SOURCES
    # Repositories
    src/domains/leiloes/repositories/LeilaoRepository.cpp
    
    # Use Cases
    src/domains/leiloes/use_cases/CreateLeilaoUseCase.cpp
)

# Offers (Sealed Bids)
set(OFFERS_SOURCES
    # Repositories
    src/domains/offers/repositories/OfferRepository.cpp
    
    # Use Cases
    src/domains/offers/use_cases/CreateOfferUseCase.cpp
    src/domains/offers/use_cases/AcceptOfferUseCase.cpp
)

# Main source
set(MAIN_SOURCE
    src/main/main_new.cpp
)

# Executable
add_executable(cppBackend 
    ${MAIN_SOURCE}
    ${CORE_SOURCES}
    ${MIDDLEWARE_SOURCES}
    ${JSON_SOURCES}
    ${THREADING_SOURCES}
    ${DATABASE_SOURCES}
    ${UTILS_SOURCES}
    ${CACHE_SOURCES}
    ${WEBSOCKET_SOURCES}
    ${QUEUE_SOURCES}
    ${IAM_SOURCES}
    ${USER_MANAGEMENT_SOURCES}
    ${TENANT_MANAGEMENT_SOURCES}
    ${AUDIT_SOURCES}
    ${NOTIFICATION_SOURCES}
    ${WORKSPACE_SOURCES}
    ${PRODUCT_SOURCES}
    ${INVENTORY_SOURCES}
    ${CUSTOMER_SOURCES}
    ${CATEGORY_SOURCES}
    ${ORDER_SOURCES}
    ${PAYMENT_SOURCES}
    ${SUBSCRIPTION_SOURCES}
    ${INVOICING_SOURCES}
    ${TASK_SOURCES}
    ${DATABASE_ADMIN_SOURCES}
    ${CALENDAR_SOURCES}
    ${FINANCE_SOURCES}
    ${RESTAURANTES_SOURCES}
    ${FREELANCES_SOURCES}
    ${VAGAS_SOURCES}
    ${CANDIDATURAS_SOURCES}
    ${LEILOES_SOURCES}
    ${OFFERS_SOURCES}
)

# PostgreSQL
find_library(LIBPQ_LIBRARY NAMES pq libpq PATHS 
    "C:/msys64/ucrt64/lib"
    "/usr/lib"
    "/usr/lib/x86_64-linux-gnu"
    "/usr/local/lib"
)
find_path(LIBPQ_INCLUDE_DIR NAMES libpq-fe.h PATHS
    "C:/msys64/ucrt64/include"
    "/usr/include"
    "/usr/include/postgresql"
    "/usr/local/include"
)

if(LIBPQ_LIBRARY AND LIBPQ_INCLUDE_DIR)
    message(STATUS "PostgreSQL found: ${LIBPQ_LIBRARY}")
    include_directories(${LIBPQ_INCLUDE_DIR})
else()
    message(WARNING "PostgreSQL not found - database features will not be available")
endif()

# OpenSSL (for JWT and Bcrypt)
find_library(OPENSSL_CRYPTO_LIBRARY NAMES crypto libcrypto PATHS 
    "C:/msys64/ucrt64/lib"
    "/usr/lib"
    "/usr/lib/x86_64-linux-gnu"
    "/usr/local/lib"
)
find_path(OPENSSL_INCLUDE_DIR NAMES openssl/evp.h PATHS
    "C:/msys64/ucrt64/include"
    "/usr/include"
    "/usr/local/include"
)

if(OPENSSL_CRYPTO_LIBRARY AND OPENSSL_INCLUDE_DIR)
    message(STATUS "OpenSSL found: ${OPENSSL_CRYPTO_LIBRARY}")
    include_directories(${OPENSSL_INCLUDE_DIR})
else()
    message(WARNING "OpenSSL not found - JWT and Bcrypt will not work")
endif()

# Link libraries
target_link_libraries(cppBackend 
    Threads::Threads
)

if(LIBPQ_LIBRARY)
    target_link_libraries(cppBackend ${LIBPQ_LIBRARY})
endif()

if(OPENSSL_CRYPTO_LIBRARY)
    target_link_libraries(cppBackend ${OPENSSL_CRYPTO_LIBRARY})
endif()

# hiredis (Redis)
if(HIREDIS_LIB)
    target_link_libraries(cppBackend ${HIREDIS_LIB})
endif()

# Se Windows, adicionar Winsock
if(WIN32)
    target_link_libraries(cppBackend ws2_32)
endif()

# Install
install(TARGETS cppBackend DESTINATION bin)

# Print configuration
message(STATUS "═══════════════════════════════════════")
message(STATUS "  C++ Backend Configuration")
message(STATUS "═══════════════════════════════════════")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Threads: ${CMAKE_THREAD_LIBS_INIT}")
message(STATUS "═══════════════════════════════════════")
