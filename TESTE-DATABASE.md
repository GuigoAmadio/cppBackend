# 🐘 Guia de Teste: PostgreSQL

Este guia mostra como testar a **integração com PostgreSQL** implementada do zero em C++.

---

## 🚀 **O que foi implementado**

✅ **Connection.hpp/cpp** - Classe de conexão com PostgreSQL usando `libpq`  
✅ **QueryResult** - Classe para manipular resultados de queries  
✅ **Endpoint `/api/db/test`** - Teste de conexão com o banco  
✅ **CMakeLists.txt** - Configurado para linkar com PostgreSQL UCRT64

---

## 📋 **Pré-requisitos**

Você tem **3 opções** para rodar PostgreSQL:

### **Opção 1: Docker (MAIS SIMPLES) 🐳**

```bash
# Iniciar PostgreSQL via Docker
docker-compose up -d

# Verificar se está rodando
docker ps

# Ver logs
docker-compose logs -f postgres
```

**Vantagens:**

- ✅ Não precisa instalar PostgreSQL no Windows
- ✅ Isolado e fácil de gerenciar
- ✅ Portável entre máquinas

### **Opção 2: PostgreSQL Windows Nativo**

Baixe e instale de: https://www.postgresql.org/download/windows/

**Depois:**

1. Ajuste a senha no instalador
2. Anote as credenciais (user: `postgres`, senha: `...`)
3. Ajuste `main.cpp` linha 244 com suas credenciais

### **Opção 3: PostgreSQL no WSL/Linux**

Se você usa WSL:

```bash
sudo apt install postgresql
sudo service postgresql start
```

---

## 🧪 **Testando a Conexão**

### **Passo 1: Iniciar PostgreSQL**

**Com Docker:**

```bash
docker-compose up -d
```

**Verificar se está rodando:**

```bash
# Docker
docker ps | grep postgres

# Windows nativo
# Services → PostgreSQL → Status: Running

# WSL
sudo service postgresql status
```

### **Passo 2: Ajustar Connection String (se necessário)**

Edite `src/main/main.cpp` linha 244:

```cpp
Core::Database::Connection conn(
    "host=localhost port=5432 dbname=postgres user=postgres password=postgres"
);
```

**Se seu PostgreSQL usa credenciais diferentes:**

- `user=seu_usuario`
- `password=sua_senha`
- `dbname=seu_banco`

### **Passo 3: Recompilar (se mudou credenciais)**

```bash
cd build
mingw32-make
```

### **Passo 4: Iniciar o Servidor**

```bash
.\cppBackend.exe
```

Você verá:

```
[INFO] Server criado na porta 8080
[INFO] ThreadPool criado com 12 threads
[INFO] ✅ Servidor rodando em http://localhost:8080
```

### **Passo 5: Testar a Conexão**

```powershell
curl http://localhost:8080/api/db/test
```

**Resposta de SUCESSO:**

```json
{
  "status": "success",
  "connected": true,
  "version": "PostgreSQL 16.3 on x86_64-pc-linux-musl, compiled by gcc...",
  "message": "PostgreSQL conectado com sucesso!"
}
```

**Resposta de ERRO (banco não está rodando):**

```json
{
  "status": "error",
  "message": "Failed to connect to database: ..."
}
```

---

## 🔍 **Troubleshooting**

### **Erro: "connection refused"**

**Causa:** PostgreSQL não está rodando

**Solução:**

```bash
# Docker
docker-compose up -d

# Windows
Services → PostgreSQL → Start

# WSL
sudo service postgresql start
```

### **Erro: "password authentication failed"**

**Causa:** Credenciais incorretas

**Solução:**

1. Verifique user/password do PostgreSQL
2. Ajuste em `main.cpp` linha 244
3. Recompile: `mingw32-make`

### **Erro: "database does not exist"**

**Causa:** O banco especificado não existe

**Solução:**

```bash
# Docker (já cria automaticamente)
docker-compose up -d

# Windows/WSL
psql -U postgres
CREATE DATABASE meu_banco;
\q
```

---

## 📊 **Próximos Passos**

Agora que a conexão funciona, você pode:

1. **Criar tabelas:**

```cpp
conn.execute("CREATE TABLE users (id SERIAL PRIMARY KEY, name TEXT)");
```

2. **Inserir dados:**

```cpp
conn.execute("INSERT INTO users (name) VALUES ('John Doe')");
```

3. **Consultar dados:**

```cpp
auto result = conn.execute("SELECT * FROM users");
for (int i = 0; i < result.rowCount(); i++) {
    std::cout << result.getValue(i, 1) << std::endl;
}
```

4. **Usar prepared statements (seguro contra SQL injection):**

```cpp
std::vector<std::string> params = {"John Doe"};
auto result = conn.executeParams(
    "INSERT INTO users (name) VALUES ($1)",
    params
);
```

---

## 🎯 **Endpoints Disponíveis**

| Endpoint             | Método | Descrição                   |
| -------------------- | ------ | --------------------------- |
| `/health`            | GET    | Health check                |
| `/api/db/test`       | GET    | Teste de conexão PostgreSQL |
| `/api/counter`       | GET    | Contador thread-safe        |
| `/api/slow/:seconds` | GET    | Teste de threading          |
| `/api/echo`          | POST   | Echo JSON                   |
| `/api/hello`         | GET    | Hello World                 |

---

## ✅ **Checklist**

- [ ] PostgreSQL instalado (Docker/Windows/WSL)
- [ ] PostgreSQL rodando
- [ ] Connection string ajustada em `main.cpp`
- [ ] Projeto recompilado
- [ ] Servidor iniciado
- [ ] Endpoint `/api/db/test` retorna sucesso

---

## 🎉 **Conclusão**

Se o teste passou, você tem:

✅ **Backend C++ completo** sem frameworks  
✅ **HTTP Server** com sockets do zero  
✅ **JSON Parser** do zero  
✅ **ThreadPool** com fila thread-safe  
✅ **PostgreSQL** integrado com `libpq`

Tudo isso em **C++ puro**! 🚀
