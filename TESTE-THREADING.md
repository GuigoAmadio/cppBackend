# 🧪 Guia de Teste: ThreadPool

Este guia mostra como testar o **ThreadPool** implementado do zero em C++.

---

## 🚀 **Como Funciona o ThreadPool**

O servidor cria **N threads de worker** (padrão: `hardware_concurrency`) que ficam esperando por tarefas em uma **fila thread-safe**. Quando uma conexão HTTP chega:

1. O socket é aceito pelo thread principal
2. A tarefa é enviada para a **Queue** (fila thread-safe com `mutex` e `condition_variable`)
3. Um **worker thread disponível** pega a tarefa e processa
4. Múltiplas requisições podem ser processadas **simultaneamente**

**Vantagens:**

- ✅ Não cria/destroi threads a cada requisição (performance)
- ✅ Limite de threads simultâneas (evita sobrecarga)
- ✅ Thread-safe com `std::mutex` e `std::condition_variable`

---

## 📝 **Passo 1: Iniciar o Servidor**

```bash
cd build
.\cppBackend.exe
```

Você verá:

```
[INFO] ThreadPool criado com 12 threads
[INFO] ✅ Servidor rodando em http://localhost:8080
```

---

## 🧪 **Passo 2: Testes Manuais**

### **Teste A: Requisição Lenta (Simula I/O)**

Abra **5 terminais PowerShell** e execute **simultaneamente** em cada um:

```powershell
# Terminal 1
curl http://localhost:8080/api/slow/3

# Terminal 2
curl http://localhost:8080/api/slow/3

# Terminal 3
curl http://localhost:8080/api/slow/3

# Terminal 4
curl http://localhost:8080/api/slow/3

# Terminal 5
curl http://localhost:8080/api/slow/3
```

**O que você verá no servidor:**

```
[INFO] ⏳ Dormindo por 3 segundos... (5x quase simultâneo)
[INFO] ✅ Acordei! (todos após ~3 segundos)
```

**✅ Esperado:** Todas as 5 requisições completam em **~3 segundos** (paralelo)  
**❌ Sem ThreadPool:** Demoraria **~15 segundos** (sequencial)

---

### **Teste B: Contador Thread-Safe**

Execute múltiplas vezes rapidamente:

```powershell
curl http://localhost:8080/api/counter
curl http://localhost:8080/api/counter
curl http://localhost:8080/api/counter
```

**Resposta esperada:**

```json
{"count": 1, "message": "Requisição #1"}
{"count": 2, "message": "Requisição #2"}
{"count": 3, "message": "Requisição #3"}
```

**✅ Verifica:** O contador usa `std::atomic<int>` para evitar **race conditions**.

---

## 🤖 **Passo 3: Teste Automatizado**

Execute o script PowerShell:

```powershell
cd C:\Users\Guillermo\Desktop\cppBackend
.\test-threading.ps1
```

O script:

1. **Teste 1:** Envia 5 requisições lentas simultaneamente e mede o tempo
2. **Teste 2:** Envia 10 requisições ao contador e verifica thread-safety

**Resultado esperado:**

```
====================================
  🧪 TESTE DE THREADING - ThreadPool
====================================

📊 TESTE 1: Requisições Lentas Simultâneas
✅ Resultado:
   Tempo total: 3.12 segundos
   Requisições completadas: 5
   🎉 SUCESSO! ThreadPool está funcionando (processou em paralelo)

📊 TESTE 2: Contador Thread-Safe (Atomic)
✅ Resultado:
   Requisição #1
   Requisição #2
   ...
   🎉 SUCESSO! Contador é thread-safe (sem race conditions)

====================================
  ✅ Testes Completos!
====================================
```

---

## 🔍 **O que Observar nos Logs**

No terminal do servidor, você verá:

```
[INFO] 📨 Nova conexão aceita  <- Thread principal
[INFO] 📥 GET /api/slow/3      <- Worker thread #1
[INFO] ⏳ Dormindo...          <- Worker thread #1
[INFO] 📨 Nova conexão aceita  <- Thread principal (continua aceitando!)
[INFO] 📥 GET /api/slow/3      <- Worker thread #2
[INFO] ⏳ Dormindo...          <- Worker thread #2
...
[INFO] ✅ Acordei!             <- Todos os workers acordam juntos
[INFO] 📤 200 OK               <- Responses enviados
```

**Isso prova que:**

- ✅ Thread principal continua aceitando conexões (não bloqueia)
- ✅ Workers processam requisições em paralelo
- ✅ ThreadPool reutiliza threads (performance)

---

## 📊 **Informações do ThreadPool**

Para ver quantas threads foram criadas:

```bash
# No log ao iniciar o servidor:
[INFO] ThreadPool criado com 12 threads
```

Esse número é `std::thread::hardware_concurrency()` (número de cores lógicos da CPU).

---

## 🎯 **Endpoints de Teste Disponíveis**

| Endpoint             | Método | Descrição                     |
| -------------------- | ------ | ----------------------------- |
| `/health`            | GET    | Health check rápido           |
| `/api/slow/:seconds` | GET    | Simula operação lenta (sleep) |
| `/api/counter`       | GET    | Contador thread-safe (atomic) |
| `/api/echo`          | POST   | Echo JSON (testa parsing)     |
| `/api/hello`         | GET    | Hello World simples           |

---

## ✅ **Conclusão**

Se os testes passaram, você tem:

- ✅ **ThreadPool funcional** processando requisições em paralelo
- ✅ **Queue thread-safe** com `mutex` e `condition_variable`
- ✅ **Servidor HTTP** robusto sem frameworks
- ✅ **JSON Parser** implementado do zero

Tudo isso em **C++ puro**, sem nenhuma biblioteca externa! 🚀
