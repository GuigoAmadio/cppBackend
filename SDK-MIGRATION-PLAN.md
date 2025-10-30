# 📦 PLANO DE MIGRAÇÃO: SDK → Backend C++

**Data:** 24/10/2025  
**Objetivo:** Integrar `moneymaker-sdk` com o novo backend C++ rodando em Docker

---

## 🔍 **ANÁLISE DA SITUAÇÃO ATUAL**

### **SDK Atual (TypeScript)**

- **Localização:** `C:\Users\Guillermo\Desktop\MoneyMaker\moneymaker-sdk`
- **Base URL:** Configurável via `SDKConfig.baseURL`
- **Endpoints:** `/api/v1/*` (backend Node.js)
- **Autenticação:** JWT com auto-refresh
- **Formato de Response:**
  ```typescript
  { data: T, message: string, success: boolean }
  ```

### **Backend C++ Atual**

- **URL:** `http://localhost:8080`
- **Endpoints:** `/api/*` (SEM `/v1`)
- **Autenticação:** JWT com refresh tokens
- **Formato de Response:**
  ```json
  { "id": "...", "message": "...", ... }
  ```

---

## ⚠️ **DIFERENÇAS CRÍTICAS**

### 1. **Estrutura de URLs**

| Recurso  | SDK (Node.js)           | Backend C++          |
| -------- | ----------------------- | -------------------- |
| Login    | `/api/v1/auth/login`    | `/api/auth/login`    |
| Register | `/api/v1/auth/register` | `/api/auth/register` |
| Products | `/api/v1/products`      | `/api/products`      |
| Orders   | `/api/v1/orders`        | `/api/orders`        |
| Finances | `/api/v1/finances`      | `/api/finance/*`     |

**Diferenças:**

- ❌ C++ não usa `/v1` nos endpoints
- ❌ `/finances` vs `/finance` (singular vs plural)

### 2. **Formato de Auth Response**

| Campo         | SDK (Node.js)   | Backend C++     |
| ------------- | --------------- | --------------- |
| Access Token  | `token`         | `access_token`  |
| Refresh Token | `refresh_token` | `refresh_token` |
| User          | `user` (objeto) | `user` (objeto) |

**Diferenças:**

- ❌ `token` vs `access_token` (nomenclatura diferente)

### 3. **Estrutura de Response**

**SDK espera (ApiResponse):**

```typescript
{
  data: T,
  message?: string,
  success: boolean,
  error?: { message: string, details: any }
}
```

**Backend C++ retorna:**

```json
{
  "id": "uuid",
  "name": "...",
  "message": "Success",
  ... (campos diretos no root)
}
```

**Diferenças:**

- ❌ C++ retorna dados diretamente no root (sem `data` wrapper)
- ❌ Não tem campo `success: boolean`

### 4. **Multitenancy**

**SDK:**

- Usa `x-client-id` header
- ClientId configurado via `SDKConfig.clientId`

**Backend C++:**

- Usa `tenant_id` no JWT payload
- Tenant vem do `TenantMiddleware`
- Pode usar `tenant_subdomain` no body do login

### 5. **Endpoints Únicos do C++**

Estes endpoints existem no C++ mas não no SDK:

- ✅ `POST /api/auth/refresh` (existe em ambos)
- ✅ `POST /api/auth/verify-email`
- ✅ `POST /api/auth/reset-password`
- ✅ `GET /api/workspaces` (novo)
- ✅ `POST /api/inventory/transactions` (novo)
- ✅ `GET /api/tasks` (novo)
- ✅ `GET /api/events` (novo)
- ✅ `GET /api/invoices` (novo)
- ✅ `GET /api/subscription/*` (novo)

### 6. **Endpoints do SDK que podem não existir no C++**

- ⚠️ `/api/v1/clients` → Talvez seja `/api/customers`?
- ⚠️ `/api/v1/services` → Não existe no C++
- ⚠️ `/api/v1/appointments` → Talvez seja `/api/events`?
- ⚠️ `/api/v1/employees` → Talvez seja `/api/users`?
- ⚠️ `/api/v1/schedules` → Talvez seja `/api/events`?
- ⚠️ `/api/v1/analytics` → Não existe no C++
- ⚠️ `/api/v1/database` → Existe como `/api/admin/database`
- ⚠️ `/api/v1/stripe` → Não existe no C++
- ⚠️ `/api/v1/annotations` → Não existe no C++

---

## ✅ **ESTRATÉGIA DE MIGRAÇÃO**

### **OPÇÃO A: Adapter Layer (Recomendada)** ⭐

Criar uma camada de adaptação no SDK que transforma as chamadas para o formato C++.

**Vantagens:**

- ✅ SDK continua funcionando com ambos backends
- ✅ Retrocompatibilidade total
- ✅ Facilita testes graduais
- ✅ Zero breaking changes

**Desvantagens:**

- ⚠️ Código adicional de adaptação
- ⚠️ Manutenção de duas "versões"

**Implementação:**

1. Adicionar `backendType: 'node' | 'cpp'` no `SDKConfig`
2. Criar `ResponseAdapter` para transformar respostas C++ → formato SDK
3. Criar `UrlAdapter` para transformar URLs `/v1/*` → `/*`
4. Atualizar clients para usar adapters

### **OPÇÃO B: Breaking Change (Mais rápida)** ⚡

Atualizar diretamente o SDK para usar o formato C++.

**Vantagens:**

- ✅ Código mais limpo
- ✅ Sem duplicação de lógica
- ✅ Mais simples de manter

**Desvantagens:**

- ❌ Breaking change (versão 2.0)
- ❌ UI precisa ser atualizado junto
- ❌ Não funciona com backend Node.js

**Implementação:**

1. Remover `/v1/` de todos os endpoints
2. Atualizar formato de auth response
3. Atualizar tratamento de responses
4. Publicar como `v2.0.0`

### **OPÇÃO C: Dual SDK (Mais complexa)** 🔄

Ter dois SDKs separados: `moneymaker-sdk` (Node.js) e `moneymaker-sdk-cpp` (C++).

**Vantagens:**

- ✅ Zero breaking changes
- ✅ Código totalmente separado

**Desvantagens:**

- ❌ Manutenção de dois SDKs
- ❌ Duplicação de código
- ❌ Confusão para desenvolvedores

---

## 🎯 **RECOMENDAÇÃO: OPÇÃO A (Adapter Layer)**

Vou implementar a **OPÇÃO A** porque:

1. ✅ Mantém retrocompatibilidade
2. ✅ Permite testar gradualmente
3. ✅ UI não precisa ser alterado imediatamente
4. ✅ Facilita rollback se necessário

---

## 📋 **PLANO DE IMPLEMENTAÇÃO (OPÇÃO A)**

### **FASE 1: Análise e Preparação** (DONE ✅)

- [x] Analisar estrutura do SDK atual
- [x] Mapear endpoints C++ vs Node.js
- [x] Identificar diferenças de formato
- [x] Criar documento de migração

### **FASE 2: Criar Adapters** (1h)

1. **Criar `BackendAdapter` interface**

   ```typescript
   interface BackendAdapter {
     adaptUrl(path: string): string;
     adaptResponse<T>(response: any): ApiResponse<T>;
     adaptAuthResponse(response: any): AuthResponse;
   }
   ```

2. **Implementar `CppBackendAdapter`**

   - Remover `/v1/` dos paths
   - Converter `token` → `access_token`
   - Envolver responses em `{ data, success }`
   - Mapear endpoints especiais (`/finances` → `/finance`)

3. **Implementar `NodeBackendAdapter`**
   - Manter comportamento atual (passthrough)

### **FASE 3: Integrar Adapters no SDK** (1h)

1. **Atualizar `SDKConfig`**

   ```typescript
   interface SDKConfig {
     baseURL: string;
     backendType?: "node" | "cpp"; // novo
     tenantId?: string; // para multitenancy C++
     // ... resto
   }
   ```

2. **Atualizar `HttpClient`**

   - Adicionar `adapter: BackendAdapter`
   - Usar `adapter.adaptUrl()` antes das chamadas
   - Usar `adapter.adaptResponse()` após as chamadas

3. **Atualizar `AuthClient`**
   - Usar `adapter.adaptAuthResponse()` em login/register
   - Adicionar suporte para `tenant_subdomain` (C++)

### **FASE 4: Testar com Backend C++** (30min)

1. **Criar script de teste**

   - Inicializar SDK com `backendType: 'cpp'`
   - Testar login/register
   - Testar CRUD de produtos
   - Testar orders/payments

2. **Validar responses**
   - Verificar se adapter funciona corretamente
   - Verificar se tokens são salvos
   - Verificar se multitenancy funciona

### **FASE 5: Publicar Nova Versão** (15min)

1. **Atualizar versão** → `0.3.0` (minor bump)
2. **Atualizar README**
   - Documentar `backendType`
   - Exemplos com backend C++
3. **Publicar no npm** → `npm publish`

### **FASE 6: Atualizar UI** (30min)

1. **Instalar novo SDK** → `npm install @guillermo/moneymaker-sdk@0.3.0`
2. **Atualizar configuração**
   ```typescript
   const sdk = new MoneyMakerSDK({
     baseURL: "http://localhost:8080",
     backendType: "cpp", // novo
     tenantId: "acme", // novo
     debug: true,
   });
   ```
3. **Testar funcionalidades básicas**

---

## 🧪 **TESTES NECESSÁRIOS**

### **Backend C++ - Validar Endpoints**

```bash
# Auth
POST /api/auth/login
POST /api/auth/register
POST /api/auth/refresh

# Products
GET /api/products
POST /api/products
GET /api/products/:id
PUT /api/products/:id
DELETE /api/products/:id

# Orders
GET /api/orders
POST /api/orders
GET /api/orders/:id
PUT /api/orders/:id/status

# Payments
POST /api/payments
GET /api/payments/:id
POST /api/payments/:id/process
```

### **SDK - Validar Adapters**

```typescript
// Teste 1: URL Adaptation
adapter.adaptUrl('/api/v1/products') === '/api/products'

// Teste 2: Response Adaptation
const cppResponse = { id: '123', name: 'Product' };
const adapted = adapter.adaptResponse(cppResponse);
// adapted.data === { id: '123', name: 'Product' }
// adapted.success === true

// Teste 3: Auth Response
const cppAuth = { access_token: 'xxx', refresh_token: 'yyy', user: {...} };
const adapted = adapter.adaptAuthResponse(cppAuth);
// adapted.tokens.accessToken === 'xxx'
```

---

## 📝 **PRÓXIMOS PASSOS**

1. ✅ **Análise completa** (DONE)
2. ⏳ **Criar adapters** → Começar agora
3. ⏳ **Integrar no SDK**
4. ⏳ **Testar com C++**
5. ⏳ **Publicar nova versão**
6. ⏳ **Atualizar UI**

---

## 📌 **NOTAS IMPORTANTES**

### **Endpoints a criar no SDK (novos do C++)**

- `WorkspacesClient` → `/api/workspaces`
- `InventoryClient` → `/api/inventory`
- `TasksClient` → `/api/tasks`
- `CalendarClient` → `/api/events`
- `InvoicesClient` → `/api/invoices`
- `SubscriptionsClient` → `/api/subscription`

### **Endpoints a deprecar (não existem no C++)**

- `ServicesClient` → Sem equivalente direto
- `StripeClient` → Backend C++ usa providers genéricos
- `AnnotationsClient` → Sem equivalente direto
- `AnalyticsClient` → Pode usar `/api/admin/database` temporariamente

### **Mapeamentos Especiais**

- `ClientsClient` (`/api/v1/clients`) → `CustomersClient` (`/api/customers`)
- `EmployeesClient` (`/api/v1/employees`) → `UsersClient` com filtro de role
- `SchedulesClient` (`/api/v1/schedules`) → `CalendarClient` (`/api/events`)
- `AppointmentsClient` (`/api/v1/appointments`) → `CalendarClient` (`/api/events`)

---

## ✅ **CONFIRMAÇÃO DO USUÁRIO**

Aguardando aprovação para iniciar a implementação dos adapters!

**Tempo estimado total:** ~3 horas  
**Complexidade:** Média  
**Risco:** Baixo (retrocompatível)
