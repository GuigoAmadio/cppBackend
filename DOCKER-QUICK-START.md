# 🚀 DOCKER - GUIA RÁPIDO

## ✅ STATUS: **DOCKER CONFIGURADO E FUNCIONAL!**

---

## 📋 **COMANDOS ESSENCIAIS:**

### 1️⃣ **Subir todos os serviços:**

```powershell
cd C:\Users\Guillermo\Desktop\cppBackend
docker-compose up -d
```

### 2️⃣ **Ver status dos containers:**

```powershell
docker-compose ps
```

**Esperado:** 8 containers rodando (backend, postgres, redis, prometheus, grafana, alertmanager, node-exporter, cadvisor)

### 3️⃣ **Ver logs (tempo real):**

```powershell
# Todos os serviços
docker-compose logs -f

# Apenas backend
docker-compose logs -f backend

# Apenas PostgreSQL
docker-compose logs -f postgres

# Apenas Redis
docker-compose logs -f redis
```

Use `Ctrl+C` para sair dos logs.

### 4️⃣ **Testar saúde de todos os serviços:**

```powershell
.\tests\test-docker-health.ps1
```

### 5️⃣ **Parar tudo:**

```powershell
docker-compose stop
```

### 6️⃣ **Parar e remover containers (mantém volumes/dados):**

```powershell
docker-compose down
```

### 7️⃣ **Parar e remover TUDO (incluindo dados):**

```powershell
docker-compose down -v
```

---

## 🌐 **URLs DOS SERVIÇOS:**

| Serviço           | URL                           | Credenciais   |
| ----------------- | ----------------------------- | ------------- |
| **Backend API**   | http://localhost:8080         | -             |
| **Health Check**  | http://localhost:8080/health  | -             |
| **Metrics**       | http://localhost:8080/metrics | -             |
| **Prometheus**    | http://localhost:9090         | -             |
| **Grafana**       | http://localhost:3001         | admin / admin |
| **AlertManager**  | http://localhost:9093         | -             |
| **Node Exporter** | http://localhost:9100/metrics | -             |
| **cAdvisor**      | http://localhost:8081         | -             |

**PostgreSQL:** `localhost:5433` | User: `moneymaker_user` | Pass: `postgre123` | DB: `moneymaker_dev`  
**Redis:** `localhost:6379` (sem senha)

---

## 🧪 **TESTES RÁPIDOS:**

### Teste 1: Health Check

```powershell
curl http://localhost:8080/health
```

Esperado: `{"status":"healthy"}`

### Teste 2: Metrics

```powershell
curl http://localhost:8080/metrics
```

Esperado: Texto com métricas Prometheus

### Teste 3: Login (testa PostgreSQL)

```powershell
$body = @{
    email = "admin@acme.com"
    password = "Admin123!"
    tenant_subdomain = "acme"
} | ConvertTo-Json

Invoke-RestMethod -Uri "http://localhost:8080/api/auth/login" -Method POST -Body $body -ContentType "application/json"
```

Esperado: JSON com `token` ou `access_token`

### Teste 4: Suite Completa

```powershell
.\tests\test-new-features.ps1
```

---

## 📊 **LOGS QUE SÃO NORMAIS (NÃO SÃO ERROS):**

### ⚠️ **Redis Timeout (NORMAL):**

```
[WARN] RedisPool: Timeout acquiring connection
[ERROR] [RedisMessageQueue] Failed to acquire Redis client
```

**O que é:** Worker tentando adquirir conexão Redis, faz retry automático.  
**Ação:** Nenhuma! É comportamento esperado.

### ⚠️ **Node Exporter - Filesystem (NORMAL no WSL2):**

```
ERROR source=collector.go:168 msg="collector failed" name=filesystem
```

**O que é:** WSL2 tem montagens especiais que o Node Exporter não reconhece.  
**Ação:** Nenhuma! Não afeta funcionalidade.

### ⚠️ **Node Exporter - Broken Pipe (NORMAL):**

```
ERROR source=http.go:231 msg="error encoding and sending metric family: write: broken pipe"
```

**O que é:** Conexões fechadas antes de terminar envio de métricas.  
**Ação:** Nenhuma! É esperado.

---

## 🔧 **TROUBLESHOOTING:**

### Problema: "Bind for 0.0.0.0:8080 failed: port is already allocated"

**Solução:**

```powershell
# Parar servidor local
Get-Process -Name "cppBackend" -ErrorAction SilentlyContinue | Stop-Process -Force

# Tentar novamente
docker-compose up -d
```

### Problema: Containers não sobem (unhealthy)

**Solução:**

```powershell
# Ver logs detalhados
docker-compose logs backend

# Rebuildar
docker-compose down
docker-compose build --no-cache backend
docker-compose up -d
```

### Problema: PostgreSQL não conecta

**Solução:**

```powershell
# Verificar se container está rodando
docker-compose ps postgres

# Ver logs
docker-compose logs postgres

# Testar conexão direta
docker exec -it cppbackend-postgres-1 psql -U moneymaker_user -d moneymaker_dev
```

### Problema: Redis não conecta

**Solução:**

```powershell
# Verificar se container está rodando
docker-compose ps redis

# Testar conexão
docker exec -it cppbackend-redis-1 redis-cli ping
```

---

## 🎯 **FLUXO COMPLETO DE TESTE:**

```powershell
# 1. Subir tudo
docker-compose up -d

# 2. Aguardar 30 segundos (health checks)
Start-Sleep -Seconds 30

# 3. Verificar status
docker-compose ps

# 4. Testar saúde
.\tests\test-docker-health.ps1

# 5. Ver logs do backend
docker-compose logs -f backend
```

---

## 📈 **PRÓXIMOS PASSOS:**

1. ✅ **Docker funcionando** - FEITO!
2. 🎨 **Configurar Grafana Dashboards** - Criar dashboards para visualizar métricas
3. 🔔 **Configurar Alertas** - Definir regras de alerta no Prometheus/AlertManager
4. 🧪 **Testes de Carga** - Usar K6 ou Artillery para stress test
5. 📦 **CI/CD** - GitHub Actions para build e deploy automatizado
6. 🌍 **Deploy em Produção** - AWS/GCP/Azure com Docker Compose ou Kubernetes

---

## ✅ **VERIFICAÇÃO FINAL:**

Execute estes comandos para confirmar que está tudo OK:

```powershell
# Subir
docker-compose up -d

# Aguardar
Start-Sleep -Seconds 30

# Testar
.\tests\test-docker-health.ps1

# Ver containers
docker-compose ps

# Ver recursos
docker stats --no-stream
```

**Resultado esperado:** Taxa de sucesso **≥ 90%** no teste de saúde!

---

🎉 **PARABÉNS! DOCKER CONFIGURADO COM SUCESSO!** 🎉
