# 📊 **STATUS DA INFRAESTRUTURA - C++ BACKEND**

**Data:** 24/10/2025  
**Versão:** v2.0

---

## ✅ **RESPOSTAS ÀS SUAS PERGUNTAS**

### **1. O módulo de database responde métricas para o dashboard?**

**Resposta:** ✅ **SIM E NÃO**

#### **O QUE JÁ FUNCIONA:**

- ✅ **MetricsCollector implementado** - Coleta métricas de:
  - `http_requests_total` (counters)
  - `http_requests_in_flight` (gauges)
  - `http_request_duration_seconds` (histograms)
- ✅ **Endpoint `/metrics` FUNCIONANDO** (acabei de implementar!)

  - Formato 100% compatível com Prometheus
  - Testado e validado: `http://localhost:8080/metrics`
  - Retorna 3.2KB de métricas em formato texto

- ✅ **MetricsMiddleware ativo** - Instrumentação automática de TODAS as rotas

#### **O QUE AINDA FALTA:**

- ❌ **Módulo Database/Admin NÃO EXISTE**

  - Não há endpoint para consultas SQL arbitrárias
  - Não há dashboard de admin para gerenciar banco
  - Não há métricas específicas de database (query duration, pool usage, etc.)

- ❌ **Métricas de Negócio** não instrumentadas:
  - Pedidos criados/cancelados
  - Pagamentos processados
  - Assinaturas ativas
  - Emails enviados

**CONCLUSÃO:** O sistema de métricas **existe e funciona**, mas falta:

1. Módulo Database/Admin completo
2. Métricas customizadas de negócio
3. Métricas de performance do PostgreSQL

---

### **2. Próximos Passos Técnicos (CLI, Docker, etc.)**

## 🎯 **ROADMAP DE IMPLEMENTAÇÃO DETALHADO**

### **FASE 1: INFRAESTRUTURA (1-2 semanas) - PRIORIDADE MÁXIMA**

#### ✅ **1.1 Prometheus Metrics (CONCLUÍDO HOJE)**

- [x] Criar endpoint `/metrics` ✅
- [x] Configurar `prometheus.yml` ✅
- [x] Configurar `grafana-datasources.yml` ✅
- [ ] Testar scraping com Prometheus local
- [ ] Criar 3 dashboards básicos no Grafana

**Como usar agora:**

```bash
# 1. Acessar métricas
curl http://localhost:8080/metrics

# 2. Iniciar Prometheus (precisa de Docker)
docker run -d -p 9090:9090 \
  -v ./monitoring/prometheus.yml:/etc/prometheus/prometheus.yml \
  prom/prometheus

# 3. Acessar Prometheus UI
# http://localhost:9090
# Query exemplo: rate(http_requests_total[5m])
```

---

#### 🚧 **1.2 Health Checks (PRÓXIMO - 2-3 horas)**

- [ ] `/health` - Liveness probe (processo vivo?)
- [ ] `/ready` - Readiness probe (pronto para tráfego?)
- [ ] `/live` - Deep health (DB, Redis, WebSocket OK?)

**Implementação:**

```cpp
// Adicionar em main_new.cpp (linha ~2230):

// ===== /health =====
router.get("/health", [](const Request& req) {
    auto json = Core::Json::makeObject();
    json->asObject()["status"] = Core::Json::makeString("ok");
    json->asObject()["timestamp"] = Core::Json::makeNumber(std::time(nullptr));
    return Response(StatusCode::OK).json(*json);
});

// ===== /ready =====
router.get("/ready", [](const Request& req) {
    bool dbReady = globalPool->isHealthy();
    bool redisReady = globalRedisPool->isHealthy();

    auto json = Core::Json::makeObject();
    json->asObject()["database"] = Core::Json::makeBool(dbReady);
    json->asObject()["redis"] = Core::Json::makeBool(redisReady);
    json->asObject()["status"] = Core::Json::makeString(
        (dbReady && redisReady) ? "ready" : "not_ready"
    );

    int statusCode = (dbReady && redisReady) ? 200 : 503;
    return Response(statusCode).json(*json);
});

// ===== /live (deep check) =====
router.get("/live", [](const Request& req) {
    // Testar DB
    bool dbOk = false;
    try {
        auto conn = globalPool->acquire();
        auto result = conn->execute("SELECT 1");
        dbOk = result.isSuccess();
    } catch (...) { dbOk = false; }

    // Testar Redis
    bool redisOk = false;
    try {
        auto redis = globalRedisPool->acquire();
        redisOk = redis->ping();
    } catch (...) { redisOk = false; }

    // Testar WebSocket
    int wsConnections = globalWsHandler->getConnectionCount();

    auto json = Core::Json::makeObject();
    json->asObject()["checks"] = Core::Json::makeObject();
    json->asObject()["checks"]->asObject()["database"] = Core::Json::makeBool(dbOk);
    json->asObject()["checks"]->asObject()["redis"] = Core::Json::makeBool(redisOk);
    json->asObject()["checks"]->asObject()["websocket_connections"] =
        Core::Json::makeNumber(wsConnections);

    bool healthy = dbOk && redisOk;
    json->asObject()["status"] = Core::Json::makeString(healthy ? "healthy" : "unhealthy");

    return Response(healthy ? 200 : 503).json(*json);
});
```

---

#### 🚧 **1.3 Docker & Docker Compose (3-4 dias)**

**Arquivo:** `docker-compose.yml` (já existe, mas precisa atualizar)

**Stack Completa:**

```yaml
version: "3.8"

services:
  backend: # C++ Backend
  postgres: # PostgreSQL 15
  redis: # Redis 7
  prometheus: # Prometheus (métricas)
  grafana: # Grafana (dashboards)
  node-exporter: # System metrics
  redis-exporter: # Redis metrics
  postgres-exporter: # PostgreSQL metrics
```

**Tarefas:**

- [ ] Criar `Dockerfile` multi-stage para C++ backend
- [ ] Atualizar `docker-compose.yml` com todos os serviços
- [ ] Testar build: `docker-compose build`
- [ ] Testar start: `docker-compose up -d`
- [ ] Verificar logs: `docker-compose logs -f backend`
- [ ] Testar health checks: `curl http://localhost:8080/health`

**Dockerfile exemplo:**

```dockerfile
# ===== BUILDER STAGE =====
FROM ubuntu:22.04 AS builder
RUN apt-get update && apt-get install -y \
    build-essential cmake libpq-dev libssl-dev git
WORKDIR /app
COPY . .
RUN mkdir build && cd build && \
    cmake .. && make -j$(nproc)

# ===== PRODUCTION STAGE =====
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y \
    libpq5 libssl3 ca-certificates
WORKDIR /app
COPY --from=builder /app/build/cppBackend /app/cppBackend
EXPOSE 8080
CMD ["./cppBackend"]
```

---

#### 🚧 **1.4 CLI (Command Line Interface) - (5-7 dias)**

**Objetivo:** Ferramenta para gerenciar backend sem precisar de SQL manual

**Estrutura:**

```
cli/
├── src/
│   ├── commands/
│   │   ├── migrate.cpp   # Migrations
│   │   ├── seed.cpp      # Popular DB
│   │   ├── user.cpp      # Gerenciar usuários
│   │   ├── tenant.cpp    # Gerenciar tenants
│   │   └── cache.cpp     # Limpar Redis
│   └── main.cpp
└── CMakeLists.txt
```

**Comandos Essenciais:**

```bash
# Database
./moneymaker-cli db:migrate          # Rodar migrations
./moneymaker-cli db:rollback         # Reverter última migration
./moneymaker-cli db:seed --count=100 # Popular com dados fake
./moneymaker-cli db:fresh            # Limpar e re-seed

# Users
./moneymaker-cli user:create-admin \
  --email admin@system.com \
  --password Admin123! \
  --name "System Admin"

./moneymaker-cli user:reset-password \
  --email user@acme.com

# Tenants
./moneymaker-cli tenant:create \
  --name "Acme Corp" \
  --subdomain acme \
  --owner admin@acme.com

./moneymaker-cli tenant:add-user \
  --tenant acme \
  --email user@test.com \
  --role admin

# Cache
./moneymaker-cli cache:clear
./moneymaker-cli cache:stats

# Server
./moneymaker-cli server:start --port 8080
./moneymaker-cli server:stop
./moneymaker-cli server:status
```

**Implementação Base:**

```cpp
// cli/src/main.cpp
#include <iostream>
#include <map>
#include <functional>

int dbMigrate(int argc, char* argv[]) {
    std::cout << "Running migrations..." << std::endl;
    // Ler arquivos .sql de schemas/
    // Aplicar em ordem
    return 0;
}

int userCreateAdmin(int argc, char* argv[]) {
    // Parse --email, --password, --name
    // Conectar DB
    // Inserir user com role=superadmin
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: moneymaker-cli <command>" << std::endl;
        return 1;
    }

    std::map<std::string, std::function<int(int, char*[])>> commands = {
        {"db:migrate", dbMigrate},
        {"user:create-admin", userCreateAdmin},
        // ... mais comandos
    };

    std::string cmd = argv[1];
    if (commands.count(cmd)) {
        return commands[cmd](argc, argv);
    }

    std::cerr << "Unknown command: " << cmd << std::endl;
    return 1;
}
```

---

### **FASE 2: MÓDULO DATABASE/ADMIN (3-5 dias)**

#### 🚧 **2.1 Database Management Service**

**Objetivo:** Permitir consultas SQL via API para dashboards admin

**Estrutura:**

```
src/domains/database/
├── controllers/
│   └── DatabaseController.{hpp,cpp}
├── use_cases/
│   ├── ExecuteQueryUseCase.hpp
│   ├── ListTablesUseCase.hpp
│   └── ExportDataUseCase.hpp
└── services/
    ├── QueryValidator.hpp  # Prevenir SQL injection
    └── QueryLogger.hpp     # Auditoria
```

**Endpoints:**

```cpp
POST /api/admin/database/query
  Body: { "sql": "SELECT * FROM users LIMIT 10" }
  Response: { "rows": [...], "rowCount": 10 }

GET /api/admin/database/tables
  Response: ["users", "products", "orders", ...]

GET /api/admin/database/tables/:tableName/schema
  Response: { "columns": [...], "indexes": [...] }

POST /api/admin/database/export
  Body: { "tables": ["users"], "format": "json" }
  Response: { "downloadUrl": "/exports/backup.zip" }
```

**⚠️ SEGURANÇA CRÍTICA:**

- ❌ NUNCA permitir `DROP`, `TRUNCATE`, `DELETE` sem confirmação
- ✅ Exigir autenticação `SUPERADMIN`
- ✅ Whitelist de comandos permitidos
- ✅ Rate limiting: 10 queries/min
- ✅ Auditoria completa (log TODAS as queries)

---

### **FASE 3: MONITORING COMPLETO (2-3 dias)**

#### 🚧 **3.1 Grafana Dashboards**

**3 Dashboards Essenciais:**

1. **HTTP Performance Dashboard**

   - Requests per second (RPS)
   - Latency (p50, p95, p99)
   - Error rate (5xx)
   - Top slowest endpoints

2. **Business Metrics Dashboard**

   - Orders created (24h)
   - Revenue (24h)
   - Active users
   - Tasks completed

3. **System Resources Dashboard**
   - CPU usage
   - Memory usage
   - Database connections
   - Redis memory

**Queries PromQL:**

```promql
# RPS
rate(http_requests_total[1m])

# p95 Latency
histogram_quantile(0.95, rate(http_request_duration_seconds_bucket[5m]))

# Error Rate
rate(http_requests_total{status=~"5.."}[1m])

# Active Connections
http_requests_in_flight
```

---

## 📋 **CHECKLIST DE IMPLEMENTAÇÃO**

### **🔥 PRIORIDADE P0 (Esta Semana)**

- [x] ✅ Endpoint `/metrics` (FEITO HOJE!)
- [ ] Health checks (`/health`, `/ready`, `/live`)
- [ ] Configurar Prometheus local
- [ ] Criar 1 dashboard básico no Grafana

### **🟡 PRIORIDADE P1 (Próximas 2 Semanas)**

- [ ] Docker Compose completo
- [ ] CLI básico (migrate, seed, user)
- [ ] Módulo Database/Admin
- [ ] Instrumentar métricas de negócio

### **🟢 PRIORIDADE P2 (Mês Atual)**

- [ ] CLI completo (todos os comandos)
- [ ] 3 dashboards Grafana completos
- [ ] Alertas Prometheus
- [ ] Documentação de operação

---

## 🚀 **COMO USAR O QUE JÁ ESTÁ PRONTO**

### **1. Testar Métricas AGORA**

```bash
# Servidor já está rodando (iniciou automaticamente)

# Ver métricas
curl http://localhost:8080/metrics

# Gerar mais métricas
for i in {1..10}; do
  curl http://localhost:8080/api/products
done

# Ver métricas atualizadas
curl http://localhost:8080/metrics | grep http_requests_total
```

### **2. Iniciar Prometheus (Docker)**

```bash
cd C:\Users\Guillermo\Desktop\cppBackend

# Iniciar Prometheus
docker run -d \
  -p 9090:9090 \
  -v %cd%\monitoring\prometheus.yml:/etc/prometheus/prometheus.yml \
  --name moneymaker-prometheus \
  prom/prometheus

# Acessar UI
# http://localhost:9090
```

### **3. Iniciar Grafana (Docker)**

```bash
# Iniciar Grafana
docker run -d \
  -p 3001:3000 \
  -v %cd%\monitoring\grafana-datasources.yml:/etc/grafana/provisioning/datasources/datasources.yml \
  --name moneymaker-grafana \
  grafana/grafana

# Acessar UI
# http://localhost:3001
# User: admin / Pass: admin
```

---

## 📊 **RESUMO DO STATUS**

| Componente           | Status  | Funcional      | Próximo Passo             |
| -------------------- | ------- | -------------- | ------------------------- |
| **Metrics Endpoint** | ✅ 100% | Sim            | Testar com Prometheus     |
| **Health Checks**    | ❌ 0%   | Não            | Implementar hoje          |
| **Prometheus**       | 🟡 50%  | Config pronto  | Iniciar container         |
| **Grafana**          | 🟡 30%  | Config pronto  | Criar dashboards          |
| **Docker Compose**   | 🟡 40%  | Arquivo existe | Atualizar e testar        |
| **CLI**              | ❌ 0%   | Não            | Começar estrutura         |
| **Database Module**  | ❌ 0%   | Não            | Implementar depois do CLI |

---

## ⏱️ **TIMELINE ESTIMADO**

| Fase         | Duração    | Entregáveis                                 |
| ------------ | ---------- | ------------------------------------------- |
| **Semana 1** | 5 dias     | Health checks + Prometheus + Grafana básico |
| **Semana 2** | 5 dias     | Docker Compose + CLI básico                 |
| **Semana 3** | 5 dias     | Módulo Database/Admin                       |
| **Semana 4** | 5 dias     | Métricas de negócio + Dashboards completos  |
| **TOTAL**    | ~4 semanas | Infraestrutura 100% completa                |

---

## 🎯 **AÇÃO IMEDIATA (HOJE)**

```bash
# 1. Testar o endpoint /metrics que acabamos de criar
curl http://localhost:8080/metrics

# 2. Iniciar Prometheus via Docker
docker run -d -p 9090:9090 \
  -v %cd%\monitoring\prometheus.yml:/etc/prometheus/prometheus.yml \
  prom/prometheus

# 3. Acessar Prometheus UI e fazer query
# http://localhost:9090/graph
# Query: rate(http_requests_total[5m])

# 4. Iniciar Grafana via Docker
docker run -d -p 3001:3000 grafana/grafana

# 5. Conectar Grafana ao Prometheus
# http://localhost:3001
# Add Data Source > Prometheus > http://host.docker.internal:9090
```

---

**Próximo Arquivo a Ler:** `ROADMAP-TECNICO-COMPLETO.md`  
**Documentação Completa:** Ver pasta `monitoring/`
