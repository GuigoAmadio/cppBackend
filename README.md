# 🚀 C++ Backend Puro - Zero Frameworks

Backend HTTP **100% C++ puro** sem frameworks externos. Tudo implementado do zero para aprendizado máximo!

## 🎯 Filosofia do Projeto

Este projeto implementa um backend HTTP completo em C++ **SEM usar frameworks** prontos. Você vai aprender:

- **Socket Programming** - Como TCP/IP funciona na prática
- **HTTP Protocol** - Parsing manual de requests/responses
- **JSON** - Parser e serializer do zero
- **Threading** - Thread pools e concorrência
- **Memory Management** - RAII, smart pointers, zero leaks
- **DDD** - Domain-Driven Design em C++ puro
- **Database** - SQL direto com libpq (PostgreSQL)

## 📁 Estrutura do Projeto

```
cppBackend/
├── src/
│   ├── core/                    # Fundações do sistema
│   │   ├── http/                # HTTP Server do zero
│   │   │   ├── Server.hpp       # TCP Server
│   │   │   ├── Request.hpp      # HTTP Request parser
│   │   │   ├── Response.hpp     # HTTP Response builder
│   │   │   └── Router.hpp       # URL routing
│   │   │
│   │   ├── json/                # JSON parser do zero
│   │   │   ├── Json.hpp         # JSON parser/serializer
│   │   │   └── JsonValue.hpp    # JSON value types
│   │   │
│   │   ├── database/            # Database layer
│   │   │   ├── Connection.hpp   # DB connection pool
│   │   │   └── QueryBuilder.hpp # SQL query builder
│   │   │
│   │   ├── threading/           # Concorrência
│   │   │   ├── ThreadPool.hpp   # Thread pool
│   │   │   └── Queue.hpp        # Thread-safe queue
│   │   │
│   │   └── utils/               # Utilitários
│   │       ├── String.hpp       # String helpers
│   │       ├── Logger.hpp       # Logging
│   │       └── Crypto.hpp       # Hash, encryption
│   │
│   ├── domains/                 # Business logic (DDD)
│   │   ├── identity/            # Domain: Identity
│   │   │   ├── entities/        # User entity
│   │   │   ├── value_objects/   # Email, Password, etc
│   │   │   ├── repositories/    # Data access
│   │   │   ├── use_cases/       # Business operations
│   │   │   └── controllers/     # HTTP endpoints
│   │   │
│   │   └── commerce/            # Domain: Commerce
│   │       ├── entities/        # Product, Order
│   │       └── ...
│   │
│   ├── infrastructure/          # Implementações técnicas
│   │   ├── database/            # PostgreSQL implementation
│   │   ├── cache/               # Redis/Memory cache
│   │   └── security/            # JWT, bcrypt
│   │
│   └── main/
│       └── main.cpp             # Entry point
│
├── include/                     # Headers públicos
├── tests/                       # Unit tests
├── build/                       # Build output
├── lib/                         # Bibliotecas externas
├── config/                      # Configurações
└── docs/                        # Documentação
```

## 🔧 Dependências Mínimas

Usamos **APENAS** bibliotecas de baixo nível (não frameworks):

```bash
# Obrigatórias
- libpq (PostgreSQL client C)
- pthread (POSIX threads)

# Opcionais (para produção)
- OpenSSL (HTTPS, crypto)
- zlib (compression)
```

## 🏗️ Como Funciona

### 1. **HTTP Server (do zero)**

```cpp
// Implementamos um servidor TCP que:
1. Escuta em uma porta (ex: 8080)
2. Aceita conexões (accept())
3. Lê dados do socket (recv())
4. Parseia HTTP request (manual!)
5. Roteia para handler correto
6. Executa business logic
7. Serializa resposta
8. Envia via socket (send())
9. Fecha conexão
```

### 2. **HTTP Protocol Parsing**

```cpp
// Request HTTP é texto plano:
"GET /users/123 HTTP/1.1\r\n"
"Host: localhost:8080\r\n"
"Content-Type: application/json\r\n"
"\r\n"
"{ \"name\": \"John\" }"

// Você vai implementar parser que extrai:
- Method (GET, POST, etc)
- Path (/users/123)
- Headers (Host, Content-Type, etc)
- Body (JSON payload)
```

### 3. **JSON Parser**

```cpp
// JSON é texto estruturado:
"{ \"name\": \"John\", \"age\": 30 }"

// Seu parser vai:
1. Tokenizar (lexer)
2. Construir árvore (parser)
3. Validar sintaxe
4. Criar objeto C++ navegável
```

### 4. **Thread Pool**

```cpp
// Para alta concorrência:
1. Cria N threads na inicialização
2. Cada request vai para fila
3. Threads ociosos pegam da fila
4. Processam e devolvem resposta
5. Voltam para idle state
```

### 5. **DDD (Domain-Driven Design)**

```cpp
// Business logic isolada:
Domain Entity -> Use Case -> Repository -> DB

// Exemplo:
User user = User::create(email, password);  // Entity
userService.register(user);                  // Use Case
userRepo.save(user);                         // Repository
```

## 🚀 Compilação e Execução

### Pré-requisitos

```bash
# Windows (usando MSYS2)
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-postgresql

# Linux (Ubuntu/Debian)
sudo apt install build-essential cmake libpq-dev

# macOS
brew install cmake postgresql
```

### Build

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Run

```bash
./cppBackend

# Server starting at http://localhost:8080
```

## 📚 Learning Path

Recomendo implementar nesta ordem:

### **Fase 1: Core (Semana 1-2)**

1. ✅ Estrutura de diretórios
2. ⏭️ HTTP Server básico (accept, recv, send)
3. ⏭️ HTTP Request parser
4. ⏭️ HTTP Response builder
5. ⏭️ Router (URL matching)

### **Fase 2: JSON (Semana 2-3)**

6. ⏭️ JSON lexer (tokenização)
7. ⏭️ JSON parser (AST)
8. ⏭️ JSON serializer
9. ⏭️ JSON Value types

### **Fase 3: Threading (Semana 3-4)**

10. ⏭️ Thread-safe queue
11. ⏭️ Thread pool
12. ⏭️ Connection handling

### **Fase 4: Business (Semana 4-6)**

13. ⏭️ Domain entities (User, Product)
14. ⏭️ Value objects (Email, Password)
15. ⏭️ Use cases
16. ⏭️ Controllers

### **Fase 5: Database (Semana 6-8)**

17. ⏭️ PostgreSQL integration (libpq)
18. ⏭️ Connection pooling
19. ⏭️ Repositories
20. ⏭️ Transactions

### **Fase 6: Production (Semana 8+)**

21. ⏭️ HTTPS (OpenSSL)
22. ⏭️ Authentication (JWT)
23. ⏭️ Logging
24. ⏭️ Error handling
25. ⏭️ Performance tuning

## 🎓 Conceitos que Você Vai Aprender

### **Low-Level**

- System calls (socket, bind, listen, accept)
- File descriptors
- Buffers e memory management
- Endianness (byte order)
- TCP/IP stack

### **Concurrency**

- Threads vs processes
- Mutexes e locks
- Race conditions
- Deadlocks
- Thread-safe data structures

### **Networking**

- TCP sockets
- HTTP protocol
- Keep-alive connections
- Request pipelining

### **Memory**

- Stack vs heap
- RAII pattern
- Smart pointers
- Memory pools
- Zero-copy optimizations

## 🔥 Performance Goals

Após otimizações, esperamos:

| Métrica          | Target       |
| ---------------- | ------------ |
| **Latência p50** | < 200µs      |
| **Latência p99** | < 2ms        |
| **Throughput**   | > 50K req/s  |
| **Memory**       | < 100MB      |
| **CPU**          | 1 core @ 50% |

## 📖 Recursos Úteis

### Livros

- **"TCP/IP Sockets in C"** - Michael J. Donahoo
- **"C++ Concurrency in Action"** - Anthony Williams
- **"Effective Modern C++"** - Scott Meyers

### Referências

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [HTTP/1.1 RFC 2616](https://www.rfc-editor.org/rfc/rfc2616)
- [JSON RFC 8259](https://www.rfc-editor.org/rfc/rfc8259)

## 🐛 Debugging

```bash
# Verificar sockets abertos
netstat -tulnp | grep 8080

# Memory leaks
valgrind --leak-check=full ./cppBackend

# Profiling
perf record ./cppBackend
perf report

# GDB
gdb ./cppBackend
(gdb) break main
(gdb) run
```

## ⚠️ Avisos Importantes

1. **Este é um projeto EDUCACIONAL** - Não use em produção sem auditar
2. **Segurança** - Implemente validação, sanitização, rate limiting
3. **Error Handling** - Trate TODOS os erros (não assuma sucesso)
4. **Memory** - Use smart pointers, evite raw pointers
5. **Testing** - Escreva testes para tudo

## 🎯 Próximos Passos

1. ✅ Estrutura criada
2. ⏭️ Implementar HTTP Server básico
3. ⏭️ Testar com `curl`
4. ⏭️ Adicionar JSON parsing
5. ⏭️ Implementar primeiro endpoint (GET /health)

---

**Dificuldade:** 🔥🔥🔥🔥🔥 (Expert)  
**Tempo Estimado:** 2-3 meses para versão completa  
**Recompensa:** Conhecimento profundo de como sistemas funcionam!

Vamos começar! 🚀
