# 🚀 Quick Start - C++ Backend do ZERO

## 📋 Pré-requisitos

### Windows (usando MinGW/MSYS2)

```bash
# 1. Instalar MSYS2 de https://www.msys2.org/

# 2. Abrir MSYS2 MinGW 64-bit e instalar ferramentas
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make

# 3. Adicionar ao PATH:
# C:\msys64\mingw64\bin
```

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential cmake g++
```

### macOS

```bash
brew install cmake gcc
```

## 🔧 Compilação

### Passo 1: Configurar

```bash
cd C:\Users\Guillermo\Desktop\cppBackend
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
```

### Passo 2: Compilar

```bash
make
```

Ou no Windows:

```bash
mingw32-make
```

### Alternativa: CMake direto

```bash
cmake --build . --config Release
```

## ▶️ Executar

```bash
# Windows
.\cppBackend.exe

# Linux/Mac
./cppBackend
```

O servidor vai iniciar em **http://localhost:8080**

## 🧪 Testar

### No navegador

Abra: http://localhost:8080

### Com curl

```bash
# Health check
curl http://localhost:8080/health

# API simples
curl http://localhost:8080/api/hello

# API com parâmetro
curl http://localhost:8080/api/users/123
```

### Com PowerShell

```powershell
Invoke-WebRequest -Uri http://localhost:8080/health
```

## 📊 Output Esperado

```
═══════════════════════════════════════
  🚀 C++ BACKEND DO ZERO
  Sem frameworks, apenas C++ puro!
═══════════════════════════════════════
[2025-10-20 09:30:00] [INFO]  Server criado na porta 8080
[2025-10-20 09:30:00] [INFO]  🚀 Iniciando servidor...
[2025-10-20 09:30:00] [INFO]  ✅ Servidor rodando em http://localhost:8080
[2025-10-20 09:30:00] [INFO]  📡 Aguardando conexões... (Ctrl+C para parar)
```

## 🐛 Troubleshooting

### Erro: `cmake not found`

```bash
# Instalar CMake
# Windows: https://cmake.org/download/
# Linux: sudo apt install cmake
# Mac: brew install cmake
```

### Erro: `winsock2.h not found`

Você está compilando no Windows sem MinGW. Instale MSYS2.

### Erro: Port 8080 already in use

Outro programa está usando a porta. Mude a porta:

```bash
./cppBackend 3000  # Usa porta 3000
```

### Servidor não responde

Verifique o firewall. No Windows:

```powershell
netsh advfirewall firewall add rule name="C++ Backend" dir=in action=allow protocol=TCP localport=8080
```

## 🎯 Próximos Passos

Agora que o básico está funcionando, você pode:

1. **Implementar JSON Parser** - `src/core/json/`
2. **Adicionar Thread Pool** - `src/core/threading/`
3. **Conectar Database** - `src/core/database/`
4. **Criar Domains** - `src/domains/identity/`

Veja `README.md` para o roteiro completo!

## 📚 Arquivos Principais

| Arquivo                      | Descrição                   |
| ---------------------------- | --------------------------- |
| `src/main/main.cpp`          | Entry point, setup de rotas |
| `src/core/http/Server.cpp`   | HTTP Server (sockets)       |
| `src/core/http/Router.cpp`   | URL routing                 |
| `src/core/http/Request.cpp`  | HTTP parsing                |
| `src/core/http/Response.cpp` | HTTP response               |

## 🔍 Debug

### Verificar se está rodando

```bash
netstat -an | findstr 8080
```

### Memory leaks (Linux)

```bash
valgrind --leak-check=full ./cppBackend
```

### GDB debugging

```bash
gdb ./cppBackend
(gdb) break main
(gdb) run
(gdb) step
```

## 🎓 Entendendo o Código

### Como o HTTP funciona aqui?

1. **Server.cpp** cria um socket TCP
2. **accept()** espera conexões
3. **recv()** lê dados do cliente
4. **parseRequest()** transforma texto em objeto Request
5. **Router** encontra handler para a URL
6. **Handler** processa e retorna Response
7. **sendResponse()** envia texto de volta
8. **close()** fecha conexão

### Fluxo de uma requisição

```
Cliente               Server                Router               Handler
  |                     |                     |                     |
  |--GET /api/hello--->|                     |                     |
  |                     |--parseRequest()-->  |                     |
  |                     |                     |--findRoute()------->|
  |                     |                     |                     |--execute()
  |                     |                     |<----Response--------|
  |                     |<--Response----------|                     |
  |<---HTTP/1.1 200 OK--|                     |                     |
```

---

**Dúvidas?** Leia o código! Está comentado linha por linha. 📖
