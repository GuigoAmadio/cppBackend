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

# Source files - HTTP Core (FUNCIONAL)
set(CORE_SOURCES
    src/core/http/Server.cpp
    src/core/http/Request.cpp
    src/core/http/Response.cpp
    src/core/http/Router.cpp
    src/core/http/Middleware.cpp
    src/core/http/TenantMiddleware.cpp
    src/core/http/AuthMiddleware.cpp
    src/core/http/RoleMiddleware.cpp
    src/core/http/RateLimitMiddleware.cpp
    src/core/http/MetricsMiddleware.cpp
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

# Database sources
set(DATABASE_SOURCES
    src/core/database/Connection.cpp
    src/core/database/ConnectionPool.cpp
)

# Identity Domain sources (DDD)
set(IDENTITY_SOURCES
    # Value Objects
    src/domains/identity/value_objects/Email.cpp
    src/domains/identity/value_objects/Password.cpp
    
    # Entities
    src/domains/identity/entities/User.cpp
    src/domains/identity/entities/Tenant.cpp
    src/domains/identity/entities/AuditLog.cpp
    
    # Repositories
    src/domains/identity/repositories/UserRepository.cpp
    src/domains/identity/repositories/TenantRepository.cpp
    src/domains/identity/repositories/EmailVerificationRepository.cpp
    src/domains/identity/repositories/PasswordResetRepository.cpp
    src/domains/identity/repositories/AuditLogRepository.cpp
    
    # Services
    src/domains/identity/services/JwtService.cpp
    src/domains/identity/services/BcryptService.cpp
    src/domains/identity/services/EmailService.cpp
    
    # Use Cases
    src/domains/identity/use_cases/RegisterUserUseCase.cpp
    src/domains/identity/use_cases/LoginUserUseCase.cpp
    src/domains/identity/use_cases/GetUserUseCase.cpp
    src/domains/identity/use_cases/UpdateUserUseCase.cpp
    src/domains/identity/use_cases/ChangePasswordUseCase.cpp
    src/domains/identity/use_cases/DeleteUserUseCase.cpp
    src/domains/identity/use_cases/ListUsersUseCase.cpp
    src/domains/identity/use_cases/ListTenantMembersUseCase.cpp
    src/domains/identity/use_cases/UpdateUserRoleUseCase.cpp
    src/domains/identity/use_cases/RemoveUserFromTenantUseCase.cpp
    src/domains/identity/use_cases/AddUserToTenantUseCase.cpp
    src/domains/identity/use_cases/RefreshTokenUseCase.cpp
    src/domains/identity/use_cases/SendVerificationEmailUseCase.cpp
    src/domains/identity/use_cases/VerifyEmailUseCase.cpp
    src/domains/identity/use_cases/RequestPasswordResetUseCase.cpp
    src/domains/identity/use_cases/ResetPasswordUseCase.cpp
    
    # Controllers
    src/domains/identity/controllers/AuthController.cpp
)

# Main source
set(MAIN_SOURCE
    src/main/main_new.cpp
)

# Executable
add_executable(cppBackend 
    ${MAIN_SOURCE}
    ${CORE_SOURCES}
    ${JSON_SOURCES}
    ${THREADING_SOURCES}
    ${DATABASE_SOURCES}
    ${UTILS_SOURCES}
    ${IDENTITY_SOURCES}
)

# PostgreSQL
find_library(LIBPQ_LIBRARY NAMES pq libpq PATHS 
    "C:/msys64/ucrt64/lib"
    NO_DEFAULT_PATH
)
find_path(LIBPQ_INCLUDE_DIR NAMES libpq-fe.h PATHS
    "C:/msys64/ucrt64/include"
    NO_DEFAULT_PATH
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
    NO_DEFAULT_PATH
)
find_path(OPENSSL_INCLUDE_DIR NAMES openssl/evp.h PATHS
    "C:/msys64/ucrt64/include"
    NO_DEFAULT_PATH
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
