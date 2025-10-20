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
)

# Source files - Stubs (TODO: Implementar depois)
set(STUB_SOURCES
    src/core/json/Json.cpp
    src/core/json/JsonValue.cpp
    src/core/threading/ThreadPool.cpp
    src/core/threading/Queue.cpp
    src/core/utils/String.cpp
    src/core/database/Connection.cpp
    src/domains/identity/entities/User.cpp
    src/domains/identity/value_objects/Email.cpp
    src/domains/identity/value_objects/Password.cpp
    src/domains/identity/controllers/UsersController.cpp
)

# Main source
set(MAIN_SOURCE
    src/main/main.cpp
)

# Executable
add_executable(cppBackend 
    ${MAIN_SOURCE}
    ${CORE_SOURCES}
    ${STUB_SOURCES}
)

# Link libraries
target_link_libraries(cppBackend 
    Threads::Threads
)

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
