# 🚀 **ROADMAP TÉCNICO COMPLETO - C++ BACKEND**

**Data:** 24/10/2025  
**Versão do Backend:** v2.0  
**Status:** 11 módulos de negócio completos, infraestrutura 80% completa

---

## 📊 **1. MÓDULO DE DATABASE & MÉTRICAS - STATUS ATUAL**

### ✅ **O QUE JÁ ESTÁ IMPLEMENTADO**

#### **1.1 MetricsCollector (Prometheus-ready)**

**Localização:** `src/core/utils/MetricsCollector.{hpp,cpp}`

**Capacidades Atuais:**

- ✅ **Counters** - Métricas que sempre crescem

  - `http_requests_total{method="POST",path="/api/login",status="200"}`
  - `email_verifications_sent_total`
  - `payments_processed_total`

- ✅ **Gauges** - Métricas que sobem/descem

  - `http_requests_in_flight` (requisições ativas)
  - `websocket_connections_active`
  - `database_connections_active`

- ✅ **Histograms** - Distribuição de valores (latências)
  - `http_request_duration_seconds{method="GET",path="/api/products"}`
  - Buckets automáticos para p50, p95, p99

**Formato de Exportação:**

```prometheus
# HELP http_requests_total Total HTTP requests
# TYPE http_requests_total counter
http_requests_total{method="POST",path="/api/login",status="200"} 1234

# HELP http_request_duration_seconds HTTP request latency
# TYPE http_request_duration_seconds histogram
http_request_duration_seconds_bucket{method="GET",path="/api/products",le="0.005"} 100
http_request_duration_seconds_bucket{method="GET",path="/api/products",le="0.01"} 150
http_request_duration_seconds_bucket{method="GET",path="/api/products",le="0.025"} 200
http_request_duration_seconds_sum{method="GET",path="/api/products"} 2.5
http_request_duration_seconds_count{method="GET",path="/api/products"} 200

# HELP http_requests_in_flight Current HTTP requests being processed
# TYPE http_requests_in_flight gauge
http_requests_in_flight 5
```

#### **1.2 MetricsMiddleware**

**Localização:** `src/infrastructure/middleware/MetricsMiddleware.{hpp,cpp}`

**Instrumentação Automática:**

```cpp
// Coleta automaticamente para TODOS os requests:
1. Incrementa counter: http_requests_total
2. Atualiza gauge: http_requests_in_flight (+1 no início, -1 no fim)
3. Registra latência: http_request_duration_seconds (em segundos)
4. Adiciona labels: method, path, status
```

**Integração Atual:**

```cpp
// main_new.cpp - linha ~2318
globalMetricsCollector = std::make_shared<Core::Utils::MetricsCollector>();

// Middleware é registrado globalmente
auto metricsMiddleware = Core::Http::createMetricsMiddleware(globalMetricsCollector);
router->use(metricsMiddleware);
```

### ❌ **O QUE ESTÁ FALTANDO**

#### **1.3 Endpoint /metrics NÃO EXISTE**

**PROBLEMA CRÍTICO:**

- O `MetricsCollector` coleta tudo corretamente ✅
- Mas **não há endpoint HTTP** para Prometheus fazer scraping ❌

**Solução Necessária:**

```cpp
// Adicionar em main_new.cpp:
router->addRoute("GET", "/metrics", [](const Request& req, Response& res) {
    std::string prometheusFormat = globalMetricsCollector->exportPrometheus();
    res.setStatus(200);
    res.setHeader("Content-Type", "text/plain; version=0.0.4");
    res.setBody(prometheusFormat);
});
```

#### **1.4 Métricas de Negócio Customizadas**

**O que falta instrumentar:**

- ❌ Transações financeiras processadas
- ❌ Pedidos criados/cancelados
- ❌ Pagamentos aprovados/rejeitados
- ❌ Assinaturas ativas/canceladas
- ❌ Emails enviados/falhados
- ❌ Tarefas criadas/completadas

**Exemplo de Como Adicionar:**

```cpp
// Em CreateOrderUseCase.hpp:
#include "../../core/utils/MetricsCollector.hpp"

std::string CreateOrderUseCase::execute(const CreateOrderDTO& dto) {
    // ... business logic ...

    if (orderCreated) {
        globalMetricsCollector->incrementCounter(
            "orders_created_total",
            {{"tenant_id", dto.tenantId}, {"status", "success"}}
        );
    }

    return orderId;
}
```

---

## 🗄️ **2. MÓDULO DATABASE/ADMIN - NÃO IMPLEMENTADO**

### **2.1 Database Management Service**

**Objetivo:** Permitir consultas SQL arbitrárias via API (para admins)

**Estrutura Proposta:**

```
src/domains/database/
├── controllers/
│   └── DatabaseController.{hpp,cpp}
├── use_cases/
│   ├── ExecuteQueryUseCase.hpp
│   ├── ListTablesUseCase.hpp
│   ├── DescribeTableUseCase.hpp
│   ├── ExportDataUseCase.hpp
│   └── ImportDataUseCase.hpp
└── services/
    ├── QueryValidator.hpp  (prevenir SQL injection)
    └── QueryLogger.hpp     (auditoria de queries)
```

**Endpoints Necessários:**

```cpp
POST /api/admin/database/query
  Body: { "sql": "SELECT * FROM users LIMIT 10" }
  Response: { "rows": [...], "rowCount": 10, "executionTime": "15ms" }

GET /api/admin/database/tables
  Response: ["users", "products", "orders", ...]

GET /api/admin/database/tables/:tableName/schema
  Response: { "columns": [...], "indexes": [...], "constraints": [...] }

POST /api/admin/database/export
  Body: { "tables": ["users", "products"], "format": "json|csv|sql" }
  Response: { "downloadUrl": "/exports/backup-20251024.zip" }
```

**Segurança CRÍTICA:**

- ❌ **NUNCA** permitir queries destrutivas (DROP, TRUNCATE, DELETE) sem confirmação
- ✅ Exigir autenticação de SUPERADMIN
- ✅ Validar SQL (whitelist de comandos permitidos)
- ✅ Rate limiting agressivo (máx 10 queries/min)
- ✅ Auditoria completa (log de TODAS as queries)

### **2.2 Métricas do Banco de Dados**

**O que precisa ser monitorado:**

```prometheus
# Pool de Conexões
database_connections_active 8
database_connections_idle 2
database_connections_max 10
database_connection_wait_duration_seconds{quantile="0.5"} 0.001

# Queries
database_queries_total{type="SELECT",status="success"} 1234
database_queries_total{type="INSERT",status="error"} 5
database_query_duration_seconds{type="SELECT",table="users"} 0.002

# Locks & Deadlocks
database_deadlocks_total 0
database_lock_wait_time_seconds 0.5

# Cache Hits (PostgreSQL)
database_cache_hits_total 5000
database_cache_misses_total 100
database_cache_hit_ratio 0.98
```

**Como Coletar:**

```cpp
// Modificar Connection.cpp para instrumentar queries:
QueryResult Connection::execute(const std::string& sql) {
    auto start = std::chrono::high_resolution_clock::now();

    PGresult* result = PQexec(conn_, sql.c_str());

    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(end - start).count();

    // Coletar métrica
    globalMetricsCollector->observeHistogram(
        "database_query_duration_seconds",
        duration,
        {{"type", detectQueryType(sql)}}  // SELECT, INSERT, UPDATE, DELETE
    );

    if (PQresultStatus(result) == PGRES_TUPLES_OK) {
        globalMetricsCollector->incrementCounter(
            "database_queries_total",
            {{"type", detectQueryType(sql)}, {"status", "success"}}
        );
    } else {
        globalMetricsCollector->incrementCounter(
            "database_queries_total",
            {{"type", detectQueryType(sql)}, {"status", "error"}}
        );
    }

    return QueryResult(result);
}
```

---

## 🛠️ **3. CLI (Command Line Interface) - NÃO IMPLEMENTADO**

### **3.1 Estrutura do CLI**

**Objetivo:** Ferramenta de linha de comando para gerenciar o backend

**Estrutura Proposta:**

```
cli/
├── src/
│   ├── commands/
│   │   ├── migrate.cpp      # Rodar migrations
│   │   ├── seed.cpp         # Popular banco com dados fake
│   │   ├── user.cpp         # Gerenciar usuários
│   │   ├── tenant.cpp       # Gerenciar tenants
│   │   ├── cache.cpp        # Gerenciar Redis
│   │   └── server.cpp       # Start/stop servidor
│   ├── utils/
│   │   ├── args_parser.cpp  # Parser de argumentos
│   │   └── colors.cpp       # Output colorido
│   └── main.cpp
├── CMakeLists.txt
└── README.md
```

### **3.2 Comandos Essenciais**

#### **3.2.1 Database Migrations**

```bash
# Rodar todas as migrations pendentes
./moneymaker-cli db:migrate

# Rollback última migration
./moneymaker-cli db:rollback

# Verificar status das migrations
./moneymaker-cli db:status
  ✅ 2025-10-01-create-users.sql (applied)
  ✅ 2025-10-10-create-products.sql (applied)
  ⏭️  2025-10-24-add-subscriptions.sql (pending)

# Criar nova migration
./moneymaker-cli db:make-migration add_payment_gateway
  Created: migrations/2025-10-24-add-payment-gateway.sql
```

#### **3.2.2 Database Seeding**

```bash
# Popular banco com dados fake para dev/testing
./moneymaker-cli db:seed

# Popular módulo específico
./moneymaker-cli db:seed --module=products --count=100

# Limpar banco e re-seed
./moneymaker-cli db:fresh
```

#### **3.2.3 User Management**

```bash
# Criar super admin
./moneymaker-cli user:create-admin \
  --email admin@system.com \
  --password Admin123! \
  --name "System Admin"

# Listar usuários
./moneymaker-cli user:list --tenant=acme

# Resetar senha
./moneymaker-cli user:reset-password --email user@acme.com

# Promover a admin
./moneymaker-cli user:promote --email user@acme.com --role superadmin
```

#### **3.2.4 Tenant Management**

```bash
# Criar novo tenant
./moneymaker-cli tenant:create \
  --name "Acme Corp" \
  --subdomain acme \
  --owner admin@acme.com

# Listar tenants
./moneymaker-cli tenant:list

# Adicionar usuário ao tenant
./moneymaker-cli tenant:add-user \
  --tenant acme \
  --email user@example.com \
  --role admin
```

#### **3.2.5 Cache Management**

```bash
# Limpar todo o cache Redis
./moneymaker-cli cache:clear

# Limpar cache de um tenant específico
./moneymaker-cli cache:clear --tenant=acme

# Ver estatísticas do Redis
./moneymaker-cli cache:stats
  Keys: 1,234
  Memory: 15.3 MB
  Hit Rate: 98.5%
```

#### **3.2.6 Server Management**

```bash
# Iniciar servidor
./moneymaker-cli server:start --port 8080 --workers 4

# Parar servidor
./moneymaker-cli server:stop

# Reiniciar servidor
./moneymaker-cli server:restart

# Ver status
./moneymaker-cli server:status
  Status: Running
  PID: 12345
  Uptime: 2h 30m
  Requests: 15,234
  Errors: 3
```

### **3.3 Implementação do CLI**

**Exemplo de Estrutura:**

```cpp
// cli/src/main.cpp
#include <iostream>
#include <string>
#include <map>
#include "commands/migrate.hpp"
#include "commands/seed.hpp"
#include "utils/args_parser.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    std::string command = argv[1];

    std::map<std::string, std::function<int(int, char*[])>> commands = {
        {"db:migrate", dbMigrate},
        {"db:seed", dbSeed},
        {"user:create-admin", userCreateAdmin},
        {"tenant:create", tenantCreate},
        {"cache:clear", cacheClear},
        {"server:start", serverStart}
    };

    if (commands.find(command) != commands.end()) {
        return commands[command](argc, argv);
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
        return 1;
    }
}
```

---

## 🐳 **4. DOCKER & DOCKER COMPOSE - PARCIALMENTE IMPLEMENTADO**

### **4.1 Estado Atual**

**Arquivo:** `docker-compose.yml` **EXISTE** na raiz, mas provavelmente desatualizado.

Vou ler o arquivo atual:

```bash
# Verificar o que já existe
cat docker-compose.yml
```

### **4.2 Stack Completa Necessária**

**Serviços a serem containerizados:**

#### **4.2.1 Backend C++**

```dockerfile
# Dockerfile
FROM ubuntu:22.04 AS builder

# Dependências de build
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libpq-dev \
    libssl-dev \
    git

WORKDIR /app
COPY . .

# Build
RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Production image
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libpq5 \
    libssl3 \
    ca-certificates

WORKDIR /app
COPY --from=builder /app/build/cppBackend /app/cppBackend
COPY --from=builder /app/config /app/config

EXPOSE 8080
CMD ["./cppBackend"]
```

#### **4.2.2 Docker Compose Completo**

```yaml
version: "3.8"

services:
  # ==================== BACKEND C++ ====================
  backend:
    build:
      context: .
      dockerfile: Dockerfile
    container_name: moneymaker-backend
    ports:
      - "8080:8080"
    environment:
      - DB_HOST=postgres
      - DB_PORT=5432
      - DB_NAME=moneymaker_dev
      - DB_USER=moneymaker_user
      - DB_PASSWORD=postgre123
      - REDIS_HOST=redis
      - REDIS_PORT=6379
      - JWT_SECRET=${JWT_SECRET:-supersecretkey}
    depends_on:
      postgres:
        condition: service_healthy
      redis:
        condition: service_healthy
    networks:
      - moneymaker-network
    restart: unless-stopped
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8080/health"]
      interval: 30s
      timeout: 10s
      retries: 3

  # ==================== POSTGRESQL ====================
  postgres:
    image: postgres:15-alpine
    container_name: moneymaker-postgres
    ports:
      - "5433:5432"
    environment:
      - POSTGRES_DB=moneymaker_dev
      - POSTGRES_USER=moneymaker_user
      - POSTGRES_PASSWORD=postgre123
    volumes:
      - postgres-data:/var/lib/postgresql/data
      - ./schemas:/docker-entrypoint-initdb.d
    networks:
      - moneymaker-network
    restart: unless-stopped
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U moneymaker_user -d moneymaker_dev"]
      interval: 10s
      timeout: 5s
      retries: 5

  # ==================== REDIS ====================
  redis:
    image: redis:7-alpine
    container_name: moneymaker-redis
    ports:
      - "6379:6379"
    command: redis-server --appendonly yes
    volumes:
      - redis-data:/data
    networks:
      - moneymaker-network
    restart: unless-stopped
    healthcheck:
      test: ["CMD", "redis-cli", "ping"]
      interval: 10s
      timeout: 5s
      retries: 5

  # ==================== PROMETHEUS ====================
  prometheus:
    image: prom/prometheus:latest
    container_name: moneymaker-prometheus
    ports:
      - "9090:9090"
    volumes:
      - ./monitoring/prometheus.yml:/etc/prometheus/prometheus.yml
      - prometheus-data:/prometheus
    command:
      - "--config.file=/etc/prometheus/prometheus.yml"
      - "--storage.tsdb.path=/prometheus"
      - "--storage.tsdb.retention.time=30d"
    networks:
      - moneymaker-network
    restart: unless-stopped
    depends_on:
      - backend

  # ==================== GRAFANA ====================
  grafana:
    image: grafana/grafana:latest
    container_name: moneymaker-grafana
    ports:
      - "3001:3000"
    environment:
      - GF_SECURITY_ADMIN_PASSWORD=admin
      - GF_INSTALL_PLUGINS=redis-datasource
    volumes:
      - ./monitoring/grafana-dashboards:/etc/grafana/provisioning/dashboards
      - ./monitoring/grafana-datasources.yml:/etc/grafana/provisioning/datasources/datasources.yml
      - grafana-data:/var/lib/grafana
    networks:
      - moneymaker-network
    restart: unless-stopped
    depends_on:
      - prometheus

  # ==================== NODE EXPORTER (System Metrics) ====================
  node-exporter:
    image: prom/node-exporter:latest
    container_name: moneymaker-node-exporter
    ports:
      - "9100:9100"
    command:
      - "--path.procfs=/host/proc"
      - "--path.sysfs=/host/sys"
      - "--collector.filesystem.mount-points-exclude=^/(sys|proc|dev|host|etc)($$|/)"
    volumes:
      - /proc:/host/proc:ro
      - /sys:/host/sys:ro
      - /:/rootfs:ro
    networks:
      - moneymaker-network
    restart: unless-stopped

  # ==================== REDIS EXPORTER (Redis Metrics) ====================
  redis-exporter:
    image: oliver006/redis_exporter:latest
    container_name: moneymaker-redis-exporter
    ports:
      - "9121:9121"
    environment:
      - REDIS_ADDR=redis:6379
    networks:
      - moneymaker-network
    restart: unless-stopped
    depends_on:
      - redis

  # ==================== POSTGRES EXPORTER (DB Metrics) ====================
  postgres-exporter:
    image: quay.io/prometheuscommunity/postgres-exporter:latest
    container_name: moneymaker-postgres-exporter
    ports:
      - "9187:9187"
    environment:
      - DATA_SOURCE_NAME=postgresql://moneymaker_user:postgre123@postgres:5432/moneymaker_dev?sslmode=disable
    networks:
      - moneymaker-network
    restart: unless-stopped
    depends_on:
      - postgres

networks:
  moneymaker-network:
    driver: bridge

volumes:
  postgres-data:
  redis-data:
  prometheus-data:
  grafana-data:
```

### **4.3 Arquivos de Configuração Necessários**

#### **4.3.1 Prometheus Config**

```yaml
# monitoring/prometheus.yml
global:
  scrape_interval: 15s
  evaluation_interval: 15s

scrape_configs:
  # Backend C++
  - job_name: "moneymaker-backend"
    static_configs:
      - targets: ["backend:8080"]
    metrics_path: "/metrics"

  # Node Exporter (System)
  - job_name: "node-exporter"
    static_configs:
      - targets: ["node-exporter:9100"]

  # Redis
  - job_name: "redis"
    static_configs:
      - targets: ["redis-exporter:9121"]

  # PostgreSQL
  - job_name: "postgres"
    static_configs:
      - targets: ["postgres-exporter:9187"]
```

#### **4.3.2 Grafana Datasources**

```yaml
# monitoring/grafana-datasources.yml
apiVersion: 1

datasources:
  - name: Prometheus
    type: prometheus
    access: proxy
    url: http://prometheus:9090
    isDefault: true
```

---

## 🏥 **5. HEALTH CHECKS - PARCIALMENTE IMPLEMENTADO**

### **5.1 Endpoints Necessários**

```cpp
// Adicionar em main_new.cpp:

// ===== /health (Liveness Probe) =====
// Verifica se o processo está rodando
router->addRoute("GET", "/health", [](const Request& req, Response& res) {
    res.setStatus(200);
    res.setBody("{\"status\":\"ok\",\"timestamp\":" +
                std::to_string(std::time(nullptr)) + "}");
});

// ===== /ready (Readiness Probe) =====
// Verifica se está pronto para receber tráfego
router->addRoute("GET", "/ready", [](const Request& req, Response& res) {
    bool dbReady = globalPool->isHealthy();
    bool redisReady = globalRedisPool->isHealthy();

    if (dbReady && redisReady) {
        res.setStatus(200);
        res.setBody("{\"status\":\"ready\",\"database\":true,\"redis\":true}");
    } else {
        res.setStatus(503);
        res.setBody("{\"status\":\"not_ready\",\"database\":" +
                    std::string(dbReady ? "true" : "false") +
                    ",\"redis\":" + std::string(redisReady ? "true" : "false") + "}");
    }
});

// ===== /live (Deep Health Check) =====
// Verifica TODOS os componentes
router->addRoute("GET", "/live", [](const Request& req, Response& res) {
    auto start = std::chrono::high_resolution_clock::now();

    // Testar Database
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

    // Testar Message Queue
    bool mqOk = (globalMessageQueue != nullptr);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    bool healthy = dbOk && redisOk && mqOk;

    std::ostringstream json;
    json << "{"
         << "\"status\":\"" << (healthy ? "healthy" : "unhealthy") << "\","
         << "\"uptime\":" << std::time(nullptr) - serverStartTime << ","
         << "\"checks\":{"
         << "\"database\":" << (dbOk ? "true" : "false") << ","
         << "\"redis\":" << (redisOk ? "true" : "false") << ","
         << "\"message_queue\":" << (mqOk ? "true" : "false") << ","
         << "\"websocket_connections\":" << wsConnections
         << "},"
         << "\"response_time_ms\":" << duration.count()
         << "}";

    res.setStatus(healthy ? 200 : 503);
    res.setBody(json.str());
});
```

---

## 📊 **6. GRAFANA DASHBOARDS - NÃO IMPLEMENTADO**

### **6.1 Dashboards Essenciais**

#### **Dashboard 1: HTTP Metrics**

```json
{
  "title": "HTTP Performance",
  "panels": [
    {
      "title": "Requests per Second",
      "query": "rate(http_requests_total[1m])"
    },
    {
      "title": "Request Latency (p50, p95, p99)",
      "query": "histogram_quantile(0.95, rate(http_request_duration_seconds_bucket[5m]))"
    },
    {
      "title": "Error Rate",
      "query": "rate(http_requests_total{status=~\"5..\"}[1m])"
    },
    {
      "title": "Top Slowest Endpoints",
      "query": "topk(10, http_request_duration_seconds_sum / http_request_duration_seconds_count)"
    }
  ]
}
```

#### **Dashboard 2: Business Metrics**

```json
{
  "title": "Business KPIs",
  "panels": [
    {
      "title": "Orders Created (24h)",
      "query": "increase(orders_created_total[24h])"
    },
    {
      "title": "Revenue (24h)",
      "query": "sum(increase(payments_processed_total[24h]))"
    },
    {
      "title": "Active Users",
      "query": "websocket_connections_active"
    },
    {
      "title": "Tasks Completed",
      "query": "increase(tasks_completed_total[24h])"
    }
  ]
}
```

#### **Dashboard 3: System Resources**

```json
{
  "title": "System Health",
  "panels": [
    {
      "title": "CPU Usage",
      "query": "rate(node_cpu_seconds_total{mode!='idle'}[5m])"
    },
    {
      "title": "Memory Usage",
      "query": "node_memory_MemTotal_bytes - node_memory_MemAvailable_bytes"
    },
    {
      "title": "Database Connections",
      "query": "database_connections_active"
    },
    {
      "title": "Redis Memory",
      "query": "redis_memory_used_bytes"
    }
  ]
}
```

---

## 🚀 **7. CRONOGRAMA DE IMPLEMENTAÇÃO**

### **Semana 1: Métricas & Monitoring**

- [ ] **Dia 1-2:** Criar endpoint `/metrics` para Prometheus
- [ ] **Dia 2-3:** Instrumentar métricas de negócio (orders, payments, etc)
- [ ] **Dia 3-4:** Configurar Prometheus + Grafana no Docker Compose
- [ ] **Dia 4-5:** Criar 3 dashboards principais no Grafana

### **Semana 2: Database & Health Checks**

- [ ] **Dia 1-2:** Implementar módulo Database (consultas admin)
- [ ] **Dia 2-3:** Adicionar health checks completos (/health, /ready, /live)
- [ ] **Dia 3-4:** Implementar métricas de database (query duration, pool stats)
- [ ] **Dia 4-5:** Testes de stress e validação

### **Semana 3: CLI**

- [ ] **Dia 1-2:** Estrutura básica do CLI + parser de argumentos
- [ ] **Dia 2-3:** Comandos de database (migrate, seed, rollback)
- [ ] **Dia 3-4:** Comandos de user/tenant management
- [ ] **Dia 4-5:** Comandos de cache e server management

### **Semana 4: Docker & Produção**

- [ ] **Dia 1-2:** Dockerfile otimizado (multi-stage build)
- [ ] **Dia 2-3:** Docker Compose completo com todos os serviços
- [ ] **Dia 3-4:** Scripts de deploy automatizado
- [ ] **Dia 4-5:** Documentação completa de deploy

### **Semana 5-6: SDK & Frontend Integration**

- [ ] **Semana 5:** Reescrever moneymaker-sdk v2.0
- [ ] **Semana 6:** Integrar ultradashboard com novo SDK

### **Semana 7-9: Módulos Faltantes**

- [ ] **Semana 7:** Analytics + Appointments
- [ ] **Semana 8:** Schedules + Employees
- [ ] **Semana 9:** Completar Task + Calendar

---

## 🎯 **8. PRIORIDADES IMEDIATAS (PRÓXIMOS 7 DIAS)**

### **P0 - CRÍTICO (Fazer HOJE)**

1. ✅ **Criar endpoint /metrics**

   ```bash
   # Permite Prometheus começar a scraping imediatamente
   curl http://localhost:8080/metrics
   ```

2. ✅ **Adicionar health checks básicos**

   ```bash
   curl http://localhost:8080/health
   curl http://localhost:8080/ready
   ```

3. ✅ **Atualizar docker-compose.yml**
   - Adicionar Prometheus
   - Adicionar Grafana
   - Configurar networking

### **P1 - ALTA (Esta Semana)**

1. ✅ Instrumentar métricas de negócio nos use cases principais
2. ✅ Criar dashboard básico no Grafana
3. ✅ Implementar CLI para migrations

### **P2 - MÉDIA (Próximas 2 Semanas)**

1. ✅ Módulo Database/Admin completo
2. ✅ CLI completo com todos os comandos
3. ✅ Exportadores para Redis/Postgres no Prometheus

### **P3 - BAIXA (Mês Atual)**

1. ✅ Dashboards avançados no Grafana
2. ✅ Alertas no Prometheus (PagerDuty/Slack)
3. ✅ Documentação completa de operação

---

## 📚 **9. REFERÊNCIAS & RECURSOS**

### **Prometheus**

- 📖 [Prometheus Documentation](https://prometheus.io/docs/)
- 📖 [Best Practices for Metric Naming](https://prometheus.io/docs/practices/naming/)
- 📖 [Exposition Format](https://prometheus.io/docs/instrumenting/exposition_formats/)

### **Grafana**

- 📖 [Grafana Dashboards](https://grafana.com/grafana/dashboards/)
- 📖 [PromQL Cheat Sheet](https://promlabs.com/promql-cheat-sheet/)

### **Docker**

- 📖 [Multi-stage Builds](https://docs.docker.com/build/building/multi-stage/)
- 📖 [Docker Compose Best Practices](https://docs.docker.com/compose/production/)

### **CLI Design**

- 📖 [Command Line Interface Guidelines](https://clig.dev/)
- 📖 [Docopt - CLI Parser](http://docopt.org/)

---

## ✅ **CONCLUSÃO**

**Estado Atual:**

- ✅ **Backend Core:** 100% funcional
- ✅ **Business Modules:** 11/20 completos (55%)
- ⚠️ **Infrastructure:** 80% completo
- ❌ **Monitoring/Ops:** 30% completo
- ❌ **CLI/Tooling:** 0% completo
- ⚠️ **Docker/Prod:** 40% completo

**Gap Crítico:**

- ❌ **Endpoint /metrics NÃO EXISTE** (Prometheus não consegue scraping)
- ❌ **Módulo Database/Admin** ausente (necessário para o ultradashboard)

**Próximo Passo IMEDIATO:**

```cpp
// 1. Adicionar endpoint /metrics em main_new.cpp (5 minutos)
// 2. Testar: curl http://localhost:8080/metrics
// 3. Configurar Prometheus para scraping
```

**Tempo até Produção:**

- Com foco em infra: **2-3 semanas**
- Com implementação de módulos: **7-9 semanas**
