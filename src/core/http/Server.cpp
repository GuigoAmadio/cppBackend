#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Router.hpp"
#include "../utils/Logger.hpp"
#include "../utils/LoggerNew.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <thread>
#include <vector>

namespace Core::Http {

Server::Server(int port) 
    : port_(port), 
      serverSocket_(INVALID_SOCKET),
      running_(false),
      threadCount_(std::thread::hardware_concurrency()),
      threadPool_(std::make_unique<Threading::ThreadPool>(threadCount_)) {
    
#ifdef _WIN32
    initWinsock();
#endif
    
    Utils::Logger::info("Server criado na porta " + std::to_string(port));
    Utils::Logger::info("ThreadPool criado com " + std::to_string(threadCount_) + " threads");
}

Server::~Server() {
    stop();
    
#ifdef _WIN32
    cleanupWinsock();
#endif
}

void Server::start() {
    Utils::Logger::info("🚀 Iniciando servidor...");
    
    // 1. Criar e configurar socket
    createSocket();
    
    // 2. Bind (associar socket à porta)
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0 (todas as interfaces)
    serverAddr.sin_port = htons(port_);       // Host to network byte order
    
    if (bind(serverSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        Utils::Logger::error("Falha ao fazer bind na porta " + std::to_string(port_));
        throw std::runtime_error("Bind failed");
    }
    
    // 3. Listen (começar a aceitar conexões)
    if (listen(serverSocket_, SOMAXCONN) == SOCKET_ERROR) {
        Utils::Logger::error("Falha ao fazer listen");
        throw std::runtime_error("Listen failed");
    }
    
    running_ = true;
    
    // 4. Iniciar ThreadPool
    threadPool_->start();
    
    Utils::Logger::info("✅ Servidor rodando em http://localhost:" + std::to_string(port_));
    Utils::Logger::info("📡 Aguardando conexões... (Ctrl+C para parar)");
    
    // 5. Loop principal - aceitar conexões
    acceptLoop();
}

void Server::stop() {
    if (!running_) return;
    
    running_ = false;
    Utils::Logger::info("🛑 Parando servidor...");
    
    // Parar ThreadPool primeiro (aguarda tarefas terminarem)
    if (threadPool_) {
        threadPool_->stop();
    }
    
    if (serverSocket_ != INVALID_SOCKET) {
        closeSocket(serverSocket_);
        serverSocket_ = INVALID_SOCKET;
    }
    
    Utils::Logger::info("✅ Servidor parado");
}

void Server::setRouter(std::shared_ptr<Router> router) {
    router_ = router;
}

void Server::setThreadCount(int count) {
    threadCount_ = count;
}

void Server::createSocket() {
    // AF_INET = IPv4
    // SOCK_STREAM = TCP (vs SOCK_DGRAM = UDP)
    // 0 = Protocol (deixa o OS escolher)
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    
    if (serverSocket_ == INVALID_SOCKET) {
        Utils::Logger::error("Falha ao criar socket");
        throw std::runtime_error("Socket creation failed");
    }
    
    // Permitir reusar porta (útil para desenvolvimento)
    int opt = 1;
#ifdef _WIN32
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, 
               (char*)&opt, sizeof(opt));
#else
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, 
               &opt, sizeof(opt));
#endif
}

void Server::acceptLoop() {
    std::vector<std::thread> workers;
    
    while (running_) {
        // Accept bloqueia até uma conexão chegar
        sockaddr_in clientAddr{};
        int clientAddrLen = sizeof(clientAddr);
        
        socket_t clientSocket = accept(serverSocket_, 
                                      (sockaddr*)&clientAddr, 
#ifdef _WIN32
                                      &clientAddrLen
#else
                                      (socklen_t*)&clientAddrLen
#endif
        );
        
        if (clientSocket == INVALID_SOCKET) {
            if (running_) {
                Utils::Logger::error("Falha ao aceitar conexão");
            }
            continue;
        }
        
        Utils::Logger::debug("📨 Nova conexão aceita");
        
        // Enviar para ThreadPool processar
        threadPool_->submit([this, clientSocket]() {
            handleConnection(clientSocket);
        });
    }
}

void Server::handleConnection(socket_t clientSocket) {
    try {
        // 1. Parsear request
        auto request = parseRequest(clientSocket);
        
        if (!request) {
            Utils::Logger::error("Falha ao parsear request");
            closeSocket(clientSocket);
            return;
        }
        
        Utils::Logger::info("📥 " + request->methodToString() + " " + request->getPath());
        
        // 2. Rotear e processar
        Response response;
        
        if (router_) {
            response = router_->handle(*request);
        } else {
            response = Response(StatusCode::NotImplemented)
                .text("No router configured");
        }
        
        // 3. Enviar response
        sendResponse(clientSocket, response);
        
        Utils::Logger::info("📤 " + std::to_string((int)response.getStatus()) + " " + 
                          Response::statusToString(response.getStatus()));
        
    } catch (const std::exception& e) {
        Utils::Logger::error(std::string("Erro ao processar conexão: ") + e.what());
        
        // Enviar erro 500
        Response errorResponse(StatusCode::InternalServerError);
        errorResponse.text("Internal Server Error");
        sendResponse(clientSocket, errorResponse);
    }
    
    // 4. Fechar conexão
    closeSocket(clientSocket);
}

std::unique_ptr<Request> Server::parseRequest(socket_t socket) {
    // Buffer para ler dados
    char buffer[8192];
    
    // Ler dados do socket (primeiro recv para headers)
    int bytesReceived = recv(socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesReceived <= 0) {
        return nullptr;
    }
    
    buffer[bytesReceived] = '\0';  // Null-terminate
    
    // Parsear manualmente!
    std::string rawRequest(buffer, bytesReceived);
    
    LOG_DEBUG("First recv() - bytes received: " + std::to_string(bytesReceived));
    
    std::istringstream stream(rawRequest);
    
    auto request = std::make_unique<Request>();
    
    // Parsear primeira linha: "GET /path HTTP/1.1"
    std::string methodStr, path, version;
    stream >> methodStr >> path >> version;
    
    request->setMethod(Request::parseMethod(methodStr));
    request->setPath(path);
    request->setVersion(version);
    
    // Parsear headers
    std::string line;
    std::getline(stream, line);  // Consumir \r\n da primeira linha
    
    while (std::getline(stream, line) && line != "\r") {
        // Remover \r se existir
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (line.empty()) break;  // Linha vazia = fim dos headers
        
        // Parsear header "Name: Value"
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string name = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Remover espaços
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            
            request->addHeader(name, value);
        }
    }
    
    // Parsear body (se existir)
    // Verificar Content-Length header
    std::string contentLengthStr = request->getHeader("Content-Length");
    
    // DEBUG LOG: Log do raw request
    LOG_DEBUG("=== RAW REQUEST DEBUG ===");
    LOG_DEBUG("Total rawRequest size: " + std::to_string(rawRequest.length()));
    LOG_DEBUG("Content-Length header: " + contentLengthStr);
    
    // Log primeiros 500 caracteres do rawRequest
    if (rawRequest.length() > 0) {
        size_t previewLen = std::min(rawRequest.length(), size_t(500));
        LOG_DEBUG("Raw request preview (first " + std::to_string(previewLen) + " chars): " + rawRequest.substr(0, previewLen));
    }
    
    if (!contentLengthStr.empty()) {
        try {
            int contentLength = std::stoi(contentLengthStr);
            LOG_DEBUG("Parsed Content-Length: " + std::to_string(contentLength));
            
            if (contentLength > 0) {
                // Calcular posição atual no buffer
                size_t headerEndPos = stream.tellg();
                if (headerEndPos == static_cast<size_t>(-1)) {
                    headerEndPos = 0;
                }
                
                // Pegar o body do rawRequest original
                size_t bodyStartPos = rawRequest.find("\r\n\r\n");
                LOG_DEBUG("Body start position: " + std::to_string(bodyStartPos));
                
                if (bodyStartPos != std::string::npos) {
                    bodyStartPos += 4;  // Pular "\r\n\r\n"
                    LOG_DEBUG("Body start (after \\r\\n\\r\\n): " + std::to_string(bodyStartPos));
                    
                    if (bodyStartPos < rawRequest.length()) {
                        std::string body = rawRequest.substr(bodyStartPos);
                        LOG_DEBUG("Body extracted (length=" + std::to_string(body.length()) + "): " + body);
                        
                        // Limitar ao Content-Length
                        if (body.length() > static_cast<size_t>(contentLength)) {
                            body = body.substr(0, contentLength);
                            LOG_DEBUG("Body trimmed to Content-Length: " + body);
                        }
                        
                        request->setBody(body);
                        LOG_DEBUG("Body set successfully!");
                    } else {
                        // Body não veio no primeiro recv(), precisamos ler mais!
                        LOG_WARNING("Body not in first packet! Body should start at " + std::to_string(bodyStartPos) + 
                                   " but rawRequest length is " + std::to_string(rawRequest.length()));
                        LOG_INFO("Reading additional " + std::to_string(contentLength) + " bytes for body...");
                        
                        // Ler o body em um segundo recv()
                        char bodyBuffer[8192];
                        int totalBodyReceived = 0;
                        std::string body;
                        
                        while (totalBodyReceived < contentLength) {
                            int remainingBytes = contentLength - totalBodyReceived;
                            int bytesToRead = std::min(remainingBytes, (int)sizeof(bodyBuffer) - 1);
                            
                            int bodyBytesReceived = recv(socket, bodyBuffer, bytesToRead, 0);
                            
                            if (bodyBytesReceived <= 0) {
                                LOG_ERROR("Failed to read body from socket (received " + 
                                         std::to_string(bodyBytesReceived) + " bytes)");
                                break;
                            }
                            
                            bodyBuffer[bodyBytesReceived] = '\0';
                            body.append(bodyBuffer, bodyBytesReceived);
                            totalBodyReceived += bodyBytesReceived;
                            
                            LOG_DEBUG("Body chunk received: " + std::to_string(bodyBytesReceived) + 
                                     " bytes (total: " + std::to_string(totalBodyReceived) + "/" + 
                                     std::to_string(contentLength) + ")");
                        }
                        
                        if (totalBodyReceived == contentLength) {
                            LOG_INFO("Body successfully read from second recv(): " + body);
                            request->setBody(body);
                            LOG_DEBUG("Body set successfully!");
                        } else {
                            LOG_ERROR("Failed to read complete body! Expected " + std::to_string(contentLength) + 
                                     " bytes, got " + std::to_string(totalBodyReceived));
                        }
                    }
                } else {
                    LOG_WARNING("Could not find \\r\\n\\r\\n separator in raw request!");
                }
            } else {
                LOG_DEBUG("Content-Length is 0 or negative, skipping body parsing");
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Error parsing Content-Length: " + std::string(e.what()));
        }
    } else {
        LOG_DEBUG("No Content-Length header found");
    }
    
    LOG_DEBUG("=== END RAW REQUEST DEBUG ===");
    
    return request;
}

void Server::sendResponse(socket_t socket, const Response& response) {
    std::string responseStr = response.toString();
    
    send(socket, responseStr.c_str(), responseStr.length(), 0);
}

void Server::closeSocket(socket_t socket) {
#ifdef _WIN32
    closesocket(socket);
#else
    close(socket);
#endif
}

#ifdef _WIN32
void Server::initWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
}

void Server::cleanupWinsock() {
    WSACleanup();
}
#else
void Server::initWinsock() {}
void Server::cleanupWinsock() {}
#endif

} // namespace Core::Http

