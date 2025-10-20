# 🏗️ Arquitetura - Como Tudo Funciona

## 📚 Índice

1. [Socket Programming 101](#socket-programming-101)
2. [HTTP Protocol](#http-protocol)
3. [Threading Model](#threading-model)
4. [Memory Management](#memory-management)
5. [Design Patterns](#design-patterns)

---

## 1. Socket Programming 101

### O que é um Socket?

Um socket é um **file descriptor** que representa uma conexão de rede. Pense como um "tubo" bidirecional entre dois programas.

```cpp
// Criar socket
socket_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//                              ↑         ↑           ↑
//                          IPv4      TCP(stream)   Protocol(auto)
```

### Fluxo Completo

```cpp
// SERVIDOR (nosso código)
1. socket()    // Criar socket
2. bind()      // Associar à porta 8080
3. listen()    // Começar a escutar
4. accept()    // Esperar conexão (BLOQUEIA)
5. recv()      // Receber dados
6. send()      // Enviar dados
7. close()     // Fechar conexão

// CLIENTE (navegador/curl)
1. socket()    // Criar socket
2. connect()   // Conectar ao servidor
3. send()      // Enviar HTTP request
4. recv()      // Receber HTTP response
5. close()     // Fechar
```

### Por que é "baixo nível"?

```cpp
// Socket trabalha com BYTES, não com objetos

char buffer[8192];  // Buffer de bytes
recv(socket, buffer, sizeof(buffer), 0);

// Buffer contém TEXTO PURO:
// "GET /users HTTP/1.1\r\n"
// "Host: localhost\r\n"
// "\r\n"

// Você precisa PARSEAR manualmente!
```

---

## 2. HTTP Protocol

### Anatomia de um HTTP Request

```
GET /api/users/123 HTTP/1.1\r\n          ← Linha de requisição
Host: localhost:8080\r\n                  ← Headers
Content-Type: application/json\r\n
Content-Length: 27\r\n
\r\n                                      ← Linha vazia
{ "name": "John" }                        ← Body (opcional)
```

### Como Parseamos?

```cpp
std::istringstream stream(rawRequest);

// 1. Parsear primeira linha
std::string method, path, version;
stream >> method >> path >> version;  // "GET" "/api/users/123" "HTTP/1.1"

// 2. Parsear headers (linha por linha)
std::string line;
while (std::getline(stream, line) && line != "\r") {
    // "Host: localhost:8080"
    size_t colonPos = line.find(':');
    std::string name = line.substr(0, colonPos);     // "Host"
    std::string value = line.substr(colonPos + 2);   // "localhost:8080"
}

// 3. Body = resto
std::string body((std::istreambuf_iterator<char>(stream)),
                  std::istreambuf_iterator<char>());
```

### Anatomia de um HTTP Response

```
HTTP/1.1 200 OK\r\n                       ← Status line
Server: cppBackend/1.0\r\n                ← Headers
Content-Type: text/plain\r\n
Content-Length: 21\r\n
\r\n                                      ← Linha vazia
Hello from C++ Backend!                   ← Body
```

### Como Construímos?

```cpp
std::ostringstream oss;

// Status line
oss << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";

// Headers
for (auto& [name, value] : headers) {
    oss << name << ": " << value << "\r\n";
}

// Blank line
oss << "\r\n";

// Body
oss << body;

return oss.str();
```

---

## 3. Threading Model

### Problema: Concorrência

```cpp
// SEM threads: Processa 1 request por vez
while (true) {
    socket_t client = accept(serverSocket, ...);  // Espera cliente
    handleConnection(client);                     // Processa (BLOQUEIA)
    close(client);                                 // Próximo
}
// ❌ Se handleConnection() demora 1s, throughput = 1 req/s
```

### Solução 1: Thread por Request (nossa implementação atual)

```cpp
// COM threads: Processa N requests em paralelo
while (true) {
    socket_t client = accept(serverSocket, ...);

    // Criar thread dedicada
    std::thread worker([client]() {
        handleConnection(client);
        close(client);
    });

    worker.detach();  // Deixa rodar independente
}
// ✅ 1000 clients = 1000 threads (funciona até ~10K)
```

**Problemas:**

- Criar thread é caro (~1-2ms)
- Threads consomem memória (~1-2MB stack cada)
- 10K threads = 10-20GB RAM

### Solução 2: Thread Pool (TODO)

```cpp
// Pool de threads pré-criadas
ThreadPool pool(16);  // 16 worker threads

while (true) {
    socket_t client = accept(serverSocket, ...);

    // Adicionar à fila
    pool.enqueue([client]() {
        handleConnection(client);
        close(client);
    });
}
// ✅ 1M clients = 16 threads (escala infinitamente)
```

### Solução 3: Async I/O (Avançado)

```cpp
// epoll (Linux), kqueue (Mac), IOCP (Windows)
// NÃO bloqueia em recv/send
// 1 thread processa milhares de conexões

// Exemplo com epoll:
while (true) {
    int n = epoll_wait(epollfd, events, MAX_EVENTS, -1);

    for (int i = 0; i < n; i++) {
        if (events[i].data.fd == serverSocket) {
            // Nova conexão
            accept(...);
        } else {
            // Dados disponíveis
            recv(...);
        }
    }
}
// ✅ 1M connections com 1-4 threads (nginx faz isso)
```

---

## 4. Memory Management

### Stack vs Heap

```cpp
// STACK (automático, rápido, limitado)
void foo() {
    int x = 42;                    // Stack
    std::string s = "hello";       // Stack (+ heap interno para chars)
    Request req;                   // Stack
}  // Tudo destruído automaticamente

// HEAP (manual, lento, ilimitado)
void bar() {
    int* p = new int(42);          // Heap (você DEVE fazer delete!)

    // ❌ Esqueceu delete? MEMORY LEAK!
    // ✅ Solução:
    delete p;
}
```

### Smart Pointers (RAII)

```cpp
// unique_ptr: Ownership único
std::unique_ptr<Request> req = std::make_unique<Request>();
// Não precisa delete! Destruído automaticamente

// shared_ptr: Ownership compartilhado
std::shared_ptr<Router> router = std::make_shared<Router>();
// Usa reference counting
// Deletado quando última referência morre

// REGRA: SEMPRE use smart pointers, NUNCA raw pointers!
```

### Move Semantics

```cpp
// Copiar é caro
std::string s1 = "Hello World with lots of text...";
std::string s2 = s1;  // ❌ Copia todos os chars (lento)

// Move é barato
std::string s3 = std::move(s1);  // ✅ "Rouba" os chars (rápido)
// s1 agora está vazio, s3 tem o conteúdo
```

---

## 5. Design Patterns

### 1. Builder Pattern (Response)

```cpp
// Ao invés de:
Response r;
r.setStatus(StatusCode::OK);
r.setHeader("Content-Type", "text/plain");
r.setBody("Hello");

// Usamos:
Response()
    .setStatus(StatusCode::OK)
    .text("Hello");  // Encadeamento!
```

### 2. Factory Pattern (Request parsing)

```cpp
// Ao invés de new Request()
auto req = Request::parse(socket);  // Factory method

// Encapsula lógica de criação
```

### 3. Strategy Pattern (Router handlers)

```cpp
// Handler é uma função strategy
using Handler = std::function<Response(const Request&)>;

router.get("/path", [](const Request& req) {
    return Response().text("Hello");  // Strategy específica
});
```

### 4. Command Pattern (Routing)

```cpp
struct Route {
    Method method;
    std::string pattern;
    Handler handler;  // Command a executar
};

// Router despacha commands
```

### 5. RAII (Resource Acquisition Is Initialization)

```cpp
class Server {
public:
    Server() {
        // Adquire resource (socket)
        serverSocket_ = socket(...);
    }

    ~Server() {
        // Libera resource
        close(serverSocket_);
    }

    // Delete copy
    Server(const Server&) = delete;
};

// Uso:
{
    Server server;
    server.start();
}  // Destrutor chamado automaticamente, socket fechado!
```

---

## 🎓 Exercícios Sugeridos

### Nível 1: Básico

1. Adicionar novo endpoint GET /api/time que retorna hora atual
2. Adicionar logging de todas as requests
3. Adicionar header "X-Powered-By: C++"

### Nível 2: Intermediário

4. Implementar POST /api/echo que retorna o body recebido
5. Adicionar suporte a query parameters (?name=value)
6. Implementar middleware de CORS

### Nível 3: Avançado

7. Implementar JSON parser completo
8. Adicionar thread pool
9. Conectar ao PostgreSQL
10. Implementar autenticação JWT

### Nível 4: Expert

11. Implementar async I/O com epoll/IOCP
12. Adicionar HTTPS (OpenSSL)
13. Implementar WebSockets
14. Fazer benchmark vs nginx

---

## 📚 Referências Essenciais

### Livros

- **Beej's Guide to Network Programming** (FREE!)
- **The C++ Programming Language** - Bjarne Stroustrup
- **TCP/IP Sockets in C** - Donahoo & Calvert

### RFCs (Especificações oficiais)

- RFC 2616 - HTTP/1.1
- RFC 793 - TCP
- RFC 8259 - JSON

### Tutoriais Online

- https://beej.us/guide/bgnet/
- https://en.cppreference.com/
- https://www.learncpp.com/

---

**Dúvida?** Leia o código fonte! Está documentado linha por linha. 🧠
