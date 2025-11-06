# 🚀 C++ Backend - Documentação Completa de Rotas e Módulos

> **Última atualização:** 2024  
> **Versão da API:** v2 (sem prefixo `/v1`)

---

## 📋 Índice

- [Visão Geral](#-visão-geral)
- [Autenticação e Autorização](#-autenticação-e-autorização)
- [Middlewares](#-middlewares)
- [Módulos da API](#-módulos-da-api)
  - [🔐 Autenticação (Auth)](#-autenticação-auth)
  - [👤 Usuários (Users)](#-usuários-users)
  - [🏢 Tenants](#-tenants)
  - [📦 Workspaces](#-workspaces)
  - [🛍️ Produtos (Products)](#️-produtos-products)
  - [📊 Inventário (Inventory)](#-inventário-inventory)
  - [👥 Clientes (Customers)](#-clientes-customers)
  - [📁 Categorias (Categories)](#-categorias-categories)
  - [🛒 Pedidos (Orders)](#-pedidos-orders)
  - [💳 Pagamentos (Payments)](#-pagamentos-payments)
  - [📋 Faturas (Invoices)](#-faturas-invoices)
  - [📅 Tarefas (Tasks)](#-tarefas-tasks)
  - [📆 Eventos (Events)](#-eventos-events)
  - [💰 Finanças (Finance)](#-finanças-finance)
  - [💎 Assinaturas (Subscriptions)](#-assinaturas-subscriptions)
  - [🔍 Admin](#-admin)
  - [🌐 Sistema/Health](#-sistemahealth)
  - [📌 Módulos Especiais](#-módulos-especiais)

---

## 🌐 Visão Geral

O backend C++ implementa uma arquitetura baseada em **domínios** (DDD), onde cada módulo possui:

- **Controllers**: Responsáveis pelas rotas HTTP e validação de entrada
- **Use Cases**: Lógica de negócio
- **Repositories**: Acesso a dados (PostgreSQL)
- **Entities**: Entidades de domínio
- **Value Objects**: Objetos de valor

### Estrutura de Resposta Padrão

```json
{
  "status": "success" | "error",
  "message": "Mensagem descritiva",
  "data": { ... }
}
```

### Códigos de Status HTTP

- `200 OK` - Sucesso
- `201 Created` - Recurso criado
- `204 No Content` - Sucesso sem conteúdo
- `400 Bad Request` - Dados inválidos
- `401 Unauthorized` - Não autenticado
- `403 Forbidden` - Sem permissão
- `404 Not Found` - Recurso não encontrado
- `500 Internal Server Error` - Erro do servidor

---

## 🔐 Autenticação e Autorização

### Token JWT

Todas as rotas protegidas requerem um **Bearer Token** no header:

```
Authorization: Bearer <access_token>
```

### Informações do Token

O token JWT contém:

- `user_id`: ID do usuário
- `user_email`: Email do usuário
- `tenant_id`: ID do tenant atual
- `tenant_subdomain`: Subdomínio do tenant
- `role`: Role do usuário no tenant (`admin`, `member`, `viewer`)

---

## 🛡️ Middlewares

### `withAuth`

**Função:** Valida se o usuário está autenticado

**Uso:** Aplica em rotas que requerem autenticação

```cpp
router.get("/api/me", withAuth([controller](const Request& req) {
    return controller->method(req);
}));
```

**Comportamento:**

- Extrai token do header `Authorization`
- Valida token JWT
- Injeta dados do usuário em `req.getCustomData()`:
  - `user_id`
  - `user_email`
  - `user_tenant_id`
  - `user_role`
- Retorna `401 Unauthorized` se token inválido/missing

---

### `withAuthAndRole`

**Função:** Valida autenticação + verifica role específica

**Uso:** Aplica em rotas que requerem roles específicas (ex: admin only)

```cpp
router.get("/api/users", withAuthAndRole({"admin", "owner"}, [controller](const Request& req) {
    return controller->list(req);
}));
```

**Comportamento:**

- Aplica `withAuth` primeiro
- Verifica se `user_role` está na lista de roles permitidas
- Retorna `403 Forbidden` se role não permitida

**Roles disponíveis:**

- `admin` - Administrador do tenant
- `owner` - Proprietário do tenant
- `member` - Membro padrão
- `viewer` - Apenas visualização

---

### `withRateLimit`

**Função:** Aplica rate limiting (limite de requisições por tempo)

**Uso:** Aplica em rotas sensíveis (login, register)

```cpp
router.post("/api/auth/login", withRateLimit(rateLimitLogin, [controller](const Request& req) {
    return controller->login(req);
}));
```

**Comportamento:**

- Limita requisições por IP
- Retorna `429 Too Many Requests` se limite excedido
- Configurado por rota (rateLimitRegister, rateLimitLogin)

---

## 🔐 Autenticação (Auth)

**Base Path:** `/api/auth`  
**Controller:** `AuthController`  
**Domínio:** `iam`

### ⚠️ Regras e Exceções

- **Rate Limiting**: Rotas `/register` e `/login` têm rate limiting (proteção contra brute force)
- **Tokens JWT**: Retornados automaticamente no registro e login
- **Tenant Opcional**: `tenant_subdomain` é opcional no registro - se não fornecido, cria novo tenant
- **Auto-creação de Tenant**: Se `tenant_subdomain` não existe, é criado automaticamente no registro
- **Role Padrão**: Usuário recebe role `admin` no tenant criado/associado durante registro
- **Prioridade de tenant_subdomain no login**:
  1. Super Admin com tenant_subdomain no body → usa do body
  2. Token JWT válido → usa do token
  3. Body fornecido → usa do body
  4. Fallback para subdomain da URL

### Rotas Públicas (sem autenticação)

| Método | Rota                          | Middleware      | Descrição                  |
| ------ | ----------------------------- | --------------- | -------------------------- |
| `POST` | `/api/auth/register`          | `withRateLimit` | Registra novo usuário      |
| `POST` | `/api/auth/login`             | `withRateLimit` | Login do usuário           |
| `POST` | `/api/auth/refresh`           | -               | Renova access token        |
| `POST` | `/api/auth/send-verification` | -               | Envia email de verificação |
| `POST` | `/api/auth/verify-email`      | -               | Verifica email com token   |
| `POST` | `/api/auth/forgot-password`   | -               | Solicita reset de senha    |
| `POST` | `/api/auth/reset-password`    | -               | Reseta senha com token     |

---

### 📝 `POST /api/auth/register`

**Descrição:** Registra um novo usuário no sistema

**Rate Limit:** ✅ Sim (rateLimitRegister)

**Body:**

```json
{
  "email": "user@example.com",
  "password": "senha123",
  "name": "Nome do Usuário",
  "tenant_subdomain": "meu-tenant" // Opcional: cria novo tenant se não fornecido
}
```

**Resposta (201 Created):**

```json
{
  "status": "success",
  "message": "User registered successfully",
  "user": {
    "id": "uuid",
    "email": "user@example.com",
    "name": "Nome do Usuário",
    "is_active": true,
    "email_verified": false
  },
  "tokens": {
    "accessToken": "jwt_token",
    "refreshToken": "refresh_token"
  },
  "tenant": {
    "id": "tenant_uuid",
    "subdomain": "meu-tenant"
  },
  "role": "admin"
}
```

**Notas:**

- Se `tenant_subdomain` não fornecido, cria novo tenant
- Role padrão: `admin` no tenant criado/associado
- Retorna tokens JWT para login automático
- Tenant é criado automaticamente se não existir

---

### 🔑 `POST /api/auth/login`

**Descrição:** Autentica usuário e retorna tokens

**Rate Limit:** ✅ Sim (rateLimitLogin)

**Body:**

```json
{
  "email": "user@example.com",
  "password": "senha123",
  "tenant_subdomain": "meu-tenant" // Opcional
}
```

**Resposta (200 OK):**

```json
{
  "status": "success",
  "user": { ... },
  "token": "jwt_access_token",
  "refreshToken": "refresh_token",
  "tenant": {
    "id": "tenant_uuid",
    "subdomain": "meu-tenant"
  }
}
```

**Notas:**

- **Prioridade de `tenant_subdomain`** (se múltiplas fontes disponíveis):
  1. **Super Admin**: Se o usuário tem role `"super_admin"` no token JWT e forneceu `tenant_subdomain` no body → usa o do **body** (permite acessar outro tenant)
  2. **Token JWT**: Se o usuário já está autenticado (tem token válido) → usa o `tenant_subdomain` do **token**
  3. **Body**: Se `tenant_subdomain` fornecido no body (login normal) → usa o do **body**
  4. **Middleware**: Fallback para subdomain extraído da URL (ex: `acme.example.com` → `acme`)
- Token JWT inclui informações do tenant e role
- Se `tenant_subdomain` fornecido, valida acesso ao tenant antes de gerar token

---

### 🔄 `POST /api/auth/refresh`

**Descrição:** Renova access token usando refresh token

**Body:**

```json
{
  "refreshToken": "refresh_token_string"
}
```

**Resposta (200 OK):**

```json
{
  "status": "success",
  "accessToken": "novo_jwt_token",
  "refreshToken": "novo_refresh_token" // Opcional: pode reutilizar o antigo
}
```

---

### 📧 `POST /api/auth/send-verification`

**Descrição:** Envia email de verificação

**Body:**

```json
{
  "email": "user@example.com"
}
```

---

### ✅ `POST /api/auth/verify-email`

**Descrição:** Verifica email usando token do email

**Body:**

```json
{
  "token": "verification_token"
}
```

---

### 🔐 `POST /api/auth/forgot-password`

**Descrição:** Solicita reset de senha (envia email)

**Body:**

```json
{
  "email": "user@example.com"
}
```

---

### 🔓 `POST /api/auth/reset-password`

**Descrição:** Reseta senha usando token do email

**Body:**

```json
{
  "token": "reset_token",
  "new_password": "nova_senha123"
}
```

---

## 👤 Usuários (Users)

**Base Path:** `/api/users`  
**Controller:** `UsersController`  
**Domínio:** `user_management`

### ⚠️ Regras e Exceções

- **`/api/me`**: Retorna dados do usuário logado com `current_tenant_id` e `current_role` do token JWT
- **Autorização**: `GET /api/users` requer role `admin` ou `owner` (não é público)
- **Self-service**: Usuário pode atualizar/deletar próprio perfil (sem precisar ser admin)
- **Email Verification**: Alterar email reseta `email_verified` para `false`
- **Soft Delete**: `DELETE` não remove registro, apenas marca `is_active = false`
- **Deprecated**: `POST /api/users/:id/change-password` está deprecated - usar `PUT /api/users/:id/password`

### Rotas Protegidas (todas requerem autenticação)

| Método   | Rota                             | Middleware                            | Descrição                               |
| -------- | -------------------------------- | ------------------------------------- | --------------------------------------- |
| `GET`    | `/api/me`                        | `withAuth`                            | Dados do usuário logado                 |
| `GET`    | `/api/users/:id`                 | `withAuth`                            | Busca usuário por ID                    |
| `PUT`    | `/api/users/:id`                 | `withAuth`                            | Atualiza usuário                        |
| `PUT`    | `/api/users/:id/password`        | `withAuth`                            | Troca senha (snake_case)                |
| `POST`   | `/api/users/:id/change-password` | `withAuth`                            | ⚠️ Troca senha (camelCase - DEPRECATED) |
| `DELETE` | `/api/users/:id`                 | `withAuth`                            | Deleta usuário (soft delete)            |
| `GET`    | `/api/users`                     | `withAuthAndRole({"admin", "owner"})` | Lista usuários                          |

---

### 👤 `GET /api/me`

**Descrição:** Retorna dados do usuário logado com informações de tenant e role

**Autenticação:** ✅ Requerida

**Resposta (200 OK):**

```json
{
  "id": "user_uuid",
  "email": "user@example.com",
  "name": "Nome do Usuário",
  "is_active": true,
  "email_verified": true,
  "current_tenant_id": "tenant_uuid", // Do token JWT
  "current_role": "admin" // Do token JWT
}
```

**Notas:**

- `current_tenant_id` e `current_role` vêm do token JWT
- Representa o tenant/contexto atual da sessão

---

### 🔍 `GET /api/users/:id`

**Descrição:** Busca usuário por ID

**Autenticação:** ✅ Requerida

**Parâmetros:**

- `:id` - UUID do usuário

**Resposta (200 OK):**

```json
{
  "id": "user_uuid",
  "email": "user@example.com",
  "name": "Nome",
  "is_active": true,
  "email_verified": true
}
```

**Erros:**

- `404 Not Found` - Usuário não encontrado

---

### ✏️ `PUT /api/users/:id`

**Descrição:** Atualiza dados do usuário

**Autenticação:** ✅ Requerida

**Body:**

```json
{
  "name": "Novo Nome",
  "email": "novo@email.com" // Se alterado, email_verified vira false
}
```

**Notas:**

- Usuário só pode atualizar próprio perfil (ou admin/owner)
- Alterar email requer nova verificação

---

### 🔒 `PUT /api/users/:id/password`

**Descrição:** Troca senha do usuário (formato snake_case)

**Autenticação:** ✅ Requerida

**Body:**

```json
{
  "old_password": "senha_atual",
  "new_password": "nova_senha123"
}
```

**Notas:**

- ⚠️ Formato preferido (snake_case)
- Requer senha antiga para validação

---

### ⚠️ `POST /api/users/:id/change-password` (DEPRECATED)

**Descrição:** Troca senha (formato camelCase) - **DEPRECATED**

**Body:**

```json
{
  "oldPassword": "senha_atual",
  "newPassword": "nova_senha123"
}
```

**Notas:**

- ⚠️ **Use `/api/users/:id/password` (PUT) ao invés desta rota**
- Mantida apenas para compatibilidade

---

### 🗑️ `DELETE /api/users/:id`

**Descrição:** Deleta usuário (soft delete)

**Autenticação:** ✅ Requerida

**Notas:**

- Soft delete: marca `is_active = false`
- Usuário só pode deletar próprio perfil (ou admin/owner)

---

### 📋 `GET /api/users`

**Descrição:** Lista usuários do sistema

**Autenticação:** ✅ Requerida  
**Autorização:** ⚠️ Apenas `admin` ou `owner`

**Query Params:**

- `page` - Número da página (padrão: 1)
- `limit` - Itens por página (padrão: 20)
- `search` - Busca por nome/email

**Resposta (200 OK):**

```json
{
  "status": "success",
  "data": [
    { "id": "...", "email": "...", "name": "..." },
    ...
  ],
  "total": 100,
  "page": 1,
  "limit": 20
}
```

---

## 🏢 Tenants

**Base Path:** `/api/tenants`  
**Controller:** `Use Cases` diretos (sem controller dedicado)  
**Domínio:** `tenant_management`

### ⚠️ Regras e Exceções

- **Autorização Especial**: Todas as rotas requerem role `super_admin` (não apenas `admin` ou `owner`)
- **Sem Controller**: Rotas usam Use Cases diretamente (não há `TenantsController`)
- **Soft Delete**: Remover usuário faz soft delete (`is_active = false`) na tabela `user_tenants`
- **Proteção de Admin**: Não permite remover último `admin` do tenant
- **Filter Active**: `GET /members` retorna apenas membros com `is_active = true`
- **Roles Permitidas**: `admin`, `member`, `viewer` (não há role `owner` neste contexto)

### Rotas

| Método   | Rota                                        | Middleware                         | Descrição                  |
| -------- | ------------------------------------------- | ---------------------------------- | -------------------------- |
| `GET`    | `/api/tenants/:tenantId/members`            | `withAuthAndRole({"super_admin"})` | Lista membros do tenant    |
| `POST`   | `/api/tenants/:tenantId/users`              | `withAuthAndRole({"super_admin"})` | Adiciona usuário ao tenant |
| `PUT`    | `/api/tenants/:tenantId/users/:userId/role` | `withAuthAndRole({"super_admin"})` | Atualiza role do usuário   |
| `DELETE` | `/api/tenants/:tenantId/users/:userId`      | `withAuthAndRole({"super_admin"})` | Remove usuário do tenant   |

---

### 👥 `GET /api/tenants/:tenantId/members`

**Descrição:** Lista membros de um tenant com suas roles

**Autenticação:** ✅ Requerida  
**Autorização:** ⚠️ Apenas `super_admin`

**Parâmetros:**

- `:tenantId` - ID do tenant

**Resposta (200 OK):**

```json
{
  "status": "success",
  "total": 5,
  "members": [
    {
      "userId": "user_uuid",
      "email": "user@example.com",
      "name": "Nome do Usuário",
      "role": "admin",
      "isActive": true
    },
    ...
  ]
}
```

**Notas:**

- Retorna apenas membros com `is_active = true`
- Roles: `admin`, `member`, `viewer`

---

### ➕ `POST /api/tenants/:tenantId/users`

**Descrição:** Adiciona usuário existente ao tenant

**Autenticação:** ✅ Requerida  
**Autorização:** ⚠️ Apenas `super_admin`

**Body:**

```json
{
  "user_id": "user_uuid",
  "role": "member" // admin, member, viewer
}
```

**Resposta (201 Created):**

```json
{
  "status": "success",
  "message": "User added to tenant"
}
```

---

### 🔄 `PUT /api/tenants/:tenantId/users/:userId/role`

**Descrição:** Atualiza role de um usuário no tenant

**Autenticação:** ✅ Requerida  
**Autorização:** ⚠️ Apenas `super_admin`

**Body:**

```json
{
  "role": "admin" // admin, member, viewer
}
```

**Notas:**

- Não permite remover último admin do tenant

---

### ➖ `DELETE /api/tenants/:tenantId/users/:userId`

**Descrição:** Remove usuário do tenant (soft delete: `is_active = false`)

**Autenticação:** ✅ Requerida  
**Autorização:** ⚠️ Apenas `super_admin`

**Notas:**

- Soft delete: marca `is_active = false` na tabela `user_tenants`
- Não permite remover último admin do tenant

---

## 📦 Workspaces

**Base Path:** `/api/workspaces`  
**Controller:** `WorkspaceController`  
**Domínio:** `workspace`

### ⚠️ Regras e Exceções

- **Ordem de Rotas**: Rotas sem parâmetros (`/api/workspaces`) DEVEM ser registradas ANTES das com parâmetros (`/api/workspaces/:id`)
- **Todos Autenticados**: Todas as rotas requerem `withAuth`, mas não há restrição de role específica
- **Multi-tenant**: Workspaces são isolados por tenant automaticamente via JWT

### Rotas

| Método   | Rota                                       | Middleware | Descrição          |
| -------- | ------------------------------------------ | ---------- | ------------------ |
| `GET`    | `/api/workspaces`                          | `withAuth` | Lista workspaces   |
| `POST`   | `/api/workspaces`                          | `withAuth` | Cria workspace     |
| `GET`    | `/api/workspaces/:id`                      | `withAuth` | Busca workspace    |
| `PUT`    | `/api/workspaces/:id`                      | `withAuth` | Atualiza workspace |
| `DELETE` | `/api/workspaces/:id`                      | `withAuth` | Deleta workspace   |
| `POST`   | `/api/workspaces/:id/members`              | `withAuth` | Convidar membro    |
| `GET`    | `/api/workspaces/:id/members`              | `withAuth` | Lista membros      |
| `DELETE` | `/api/workspaces/:id/members/:userId`      | `withAuth` | Remove membro      |
| `PUT`    | `/api/workspaces/:id/members/:userId/role` | `withAuth` | Atualiza role      |

**⚠️ Importante:** Rotas sem parâmetros (`/api/workspaces`) devem ser registradas **ANTES** das rotas com parâmetros (`/api/workspaces/:id`)

---

## 🛍️ Produtos (Products)

**Base Path:** `/api/products`  
**Controller:** `ProductController`  
**Domínio:** `product`

### ⚠️ Regras e Exceções

- **Ordem CRÍTICA**: Rotas especiais (`/search`, `/low-stock`) DEVEM ser registradas ANTES de `/:id`, senão nunca serão alcançadas
- **Soft Delete**: `DELETE` marca produto como inativo, não remove do banco
- **Filtros**: `GET /products` suporta filtros por categoria, busca, paginação
- **Estoque Baixo**: `GET /low-stock` tem parâmetro `threshold` configurável (padrão: 10)

### Rotas

| Método   | Rota                      | Middleware | Descrição                                              |
| -------- | ------------------------- | ---------- | ------------------------------------------------------ |
| `GET`    | `/api/products/search`    | `withAuth` | 🔍 Busca produtos (DEVE VIR ANTES DE /:id)             |
| `GET`    | `/api/products/low-stock` | `withAuth` | ⚠️ Produtos com estoque baixo (DEVE VIR ANTES DE /:id) |
| `GET`    | `/api/products`           | `withAuth` | 📋 Lista produtos (DEVE VIR ANTES DE /:id)             |
| `POST`   | `/api/products`           | `withAuth` | ➕ Cria produto                                        |
| `GET`    | `/api/products/:id`       | `withAuth` | 🔍 Busca produto                                       |
| `PUT`    | `/api/products/:id`       | `withAuth` | ✏️ Atualiza produto                                    |
| `DELETE` | `/api/products/:id`       | `withAuth` | 🗑️ Deleta produto (soft delete)                        |

**⚠️ Ordem Importante:** Rotas especiais (`/search`, `/low-stock`) devem ser registradas **ANTES** da rota genérica `/:id`

---

### 🔍 `GET /api/products/search`

**Query Params:**

- `q` - Termo de busca
- `category_id` - Filtrar por categoria
- `page`, `limit` - Paginação

---

### ⚠️ `GET /api/products/low-stock`

**Query Params:**

- `threshold` - Limite de estoque (padrão: 10)

---

## 📊 Inventário (Inventory)

**Base Path:** `/api/inventory`  
**Controller:** `InventoryController`  
**Domínio:** `inventory`

### ⚠️ Regras e Exceções

- **Transações**: Tipos permitidos: `in`, `out`, `adjustment`
- **Location Opcional**: `location_id` é opcional nas transações
- **Histórico Filtrado**: `GET /history` permite filtrar por produto, localização, tipo e período
- **Estoque Atual**: `GET /stock/:productId` calcula estoque baseado em todas as transações

### Rotas

| Método | Rota                              | Middleware | Descrição                |
| ------ | --------------------------------- | ---------- | ------------------------ |
| `POST` | `/api/inventory/transactions`     | `withAuth` | Registra transação       |
| `GET`  | `/api/inventory/history`          | `withAuth` | Histórico de transações  |
| `GET`  | `/api/inventory/stock/:productId` | `withAuth` | Estoque atual do produto |

---

### 📝 `POST /api/inventory/transactions`

**Body:**

```json
{
  "product_id": "product_uuid",
  "type": "in" | "out" | "adjustment",
  "quantity": 10,
  "location_id": "location_uuid", // Opcional
  "notes": "Observações"
}
```

---

### 📜 `GET /api/inventory/history`

**Query Params:**

- `product_id` - Filtrar por produto
- `location_id` - Filtrar por localização
- `type` - Filtrar por tipo (`in`, `out`, `adjustment`)
- `start_date`, `end_date` - Período

---

## 👥 Clientes (Customers)

**Base Path:** `/api/customers`  
**Controller:** `CustomerController`  
**Domínio:** `customer`

### ⚠️ Regras e Exceções

- **Ordem de Rotas**: `GET /search` deve vir ANTES de `GET /:id`
- **Soft Delete**: `DELETE` marca cliente como inativo
- **Pedidos Relacionados**: Clientes podem ter múltiplos pedidos (rota separada: `/customers/:id/orders`)
- **Busca**: `GET /search` suporta busca por nome, email, documento

### Rotas

| Método   | Rota                    | Middleware | Descrição                         |
| -------- | ----------------------- | ---------- | --------------------------------- |
| `GET`    | `/api/customers/search` | `withAuth` | 🔍 Busca clientes (ANTES DE /:id) |
| `GET`    | `/api/customers`        | `withAuth` | 📋 Lista clientes                 |
| `POST`   | `/api/customers`        | `withAuth` | ➕ Cria cliente                   |
| `GET`    | `/api/customers/:id`    | `withAuth` | 🔍 Busca cliente                  |
| `PUT`    | `/api/customers/:id`    | `withAuth` | ✏️ Atualiza cliente               |
| `DELETE` | `/api/customers/:id`    | `withAuth` | 🗑️ Deleta cliente (soft delete)   |

---

## 📁 Categorias (Categories)

**Base Path:** `/api/categories`  
**Controller:** `CategoryController`  
**Domínio:** `category`

### ⚠️ Regras e Exceções

- **Hierarquia**: Categorias podem ter `parent_id` (suporta árvore de categorias)
- **Rotas Especiais**: `GET /root` e `GET /:id/children` devem vir ANTES de `GET /:id`
- **Soft Delete Restrito**: Não permite deletar categoria se houver produtos ou categorias filhas associadas
- **Categorias Raiz**: `GET /root` retorna apenas categorias sem `parent_id`

### Rotas

| Método   | Rota                           | Middleware | Descrição                          |
| -------- | ------------------------------ | ---------- | ---------------------------------- |
| `GET`    | `/api/categories/root`         | `withAuth` | 🌳 Categorias raiz (ANTES DE /:id) |
| `GET`    | `/api/categories`              | `withAuth` | 📋 Lista categorias                |
| `POST`   | `/api/categories`              | `withAuth` | ➕ Cria categoria                  |
| `GET`    | `/api/categories/:id/children` | `withAuth` | 👶 Filhos da categoria             |
| `GET`    | `/api/categories/:id`          | `withAuth` | 🔍 Busca categoria                 |
| `PUT`    | `/api/categories/:id`          | `withAuth` | ✏️ Atualiza categoria              |
| `DELETE` | `/api/categories/:id`          | `withAuth` | 🗑️ Deleta categoria (soft delete)  |

**Notas:**

- Categorias podem ter hierarquia (parent_id)
- Soft delete: não permite deletar se houver produtos/categorias filhas

---

## 🛒 Pedidos (Orders)

**Base Path:** `/api/orders`  
**Controller:** `OrderController`  
**Domínio:** `order`

### ⚠️ Regras e Exceções

- **Status**: Status válidos: `pending`, `processing`, `shipped`, `delivered`, `cancelled`
- **Pagamento Separado**: Status de pagamento é gerenciado separadamente (`PUT /:id/payment`)
- **Cancelamento**: `POST /:id/cancel` pode ter regras de negócio (ex: não cancelar após enviado)
- **Pedidos do Cliente**: Rota especial `/customers/:customerId/orders` (não é `/orders?customer_id=...`)

### Rotas

| Método | Rota                                | Middleware | Descrição                       |
| ------ | ----------------------------------- | ---------- | ------------------------------- |
| `GET`  | `/api/orders`                       | `withAuth` | 📋 Lista pedidos                |
| `POST` | `/api/orders`                       | `withAuth` | ➕ Cria pedido                  |
| `GET`  | `/api/customers/:customerId/orders` | `withAuth` | 📋 Pedidos do cliente           |
| `GET`  | `/api/orders/:id`                   | `withAuth` | 🔍 Busca pedido                 |
| `PUT`  | `/api/orders/:id/status`            | `withAuth` | 🔄 Atualiza status              |
| `PUT`  | `/api/orders/:id/payment`           | `withAuth` | 💳 Atualiza status de pagamento |
| `POST` | `/api/orders/:id/cancel`            | `withAuth` | ❌ Cancela pedido               |

---

### 🔄 `PUT /api/orders/:id/status`

**Body:**

```json
{
  "status": "pending" | "processing" | "shipped" | "delivered" | "cancelled"
}
```

---

## 💳 Pagamentos (Payments)

**Base Path:** `/api/payments`  
**Controller:** `PaymentController`  
**Domínio:** `payment`

### ⚠️ Regras e Exceções

- **Estados**: Payment pode ter estados: `pending`, `processing`, `completed`, `failed`, `refunded`, `cancelled`
- **Reembolso**: `POST /:id/refund` pode ter limites de tempo ou condições específicas
- **Transações**: `GET /:id/transactions` retorna histórico de tentativas/pagamentos
- **Filtros**: `GET /payments` suporta filtros por `order_id`, `customer_id`, `tenant_id`

### Rotas

| Método | Rota                             | Middleware | Descrição                  |
| ------ | -------------------------------- | ---------- | -------------------------- |
| `POST` | `/api/payments`                  | `withAuth` | ➕ Cria payment            |
| `GET`  | `/api/payments`                  | `withAuth` | 📋 Lista payments          |
| `GET`  | `/api/payments/:id`              | `withAuth` | 🔍 Busca payment           |
| `POST` | `/api/payments/:id/process`      | `withAuth` | ✅ Processa payment        |
| `POST` | `/api/payments/:id/refund`       | `withAuth` | 💸 Reembolsa payment       |
| `POST` | `/api/payments/:id/cancel`       | `withAuth` | ❌ Cancela payment         |
| `GET`  | `/api/payments/:id/transactions` | `withAuth` | 📜 Histórico de transações |

---

## 📋 Faturas (Invoices)

**Base Path:** `/api/invoices`  
**Controller:** `InvoiceController`  
**Domínio:** `invoicing`

### ⚠️ Regras e Exceções

- **Status**: Faturas podem ter status: `draft`, `sent`, `paid`, `overdue`, `cancelled`
- **Itens Separados**: Itens da fatura são gerenciados separadamente (`GET /:id/items`)
- **Pagamento Parcial**: `POST /:id/payment` pode aplicar pagamento parcial
- **Faturas do Cliente**: Rota especial `/customers/:customerId/invoices` para listar faturas de um cliente

### Rotas

| Método | Rota                                  | Middleware | Descrição                |
| ------ | ------------------------------------- | ---------- | ------------------------ |
| `POST` | `/api/invoices`                       | `withAuth` | ➕ Cria fatura           |
| `GET`  | `/api/invoices`                       | `withAuth` | 📋 Lista faturas         |
| `GET`  | `/api/invoices/:id`                   | `withAuth` | 🔍 Busca fatura          |
| `PUT`  | `/api/invoices/:id/status`            | `withAuth` | 🔄 Atualiza status       |
| `POST` | `/api/invoices/:id/payment`           | `withAuth` | 💳 Aplica pagamento      |
| `GET`  | `/api/invoices/:id/items`             | `withAuth` | 📦 Lista itens da fatura |
| `GET`  | `/api/customers/:customerId/invoices` | `withAuth` | 📋 Faturas do cliente    |

---

## 📅 Tarefas (Tasks)

**Base Path:** `/api/tasks`  
**Controller:** `TaskController`  
**Domínio:** `task`

### ⚠️ Regras e Exceções

- **Filtros**: `GET /tasks` suporta filtros por `workspace_id`, `project_id`, `assigned_to`, `status`, `created_by`
- **Comentários**: Comentários são recursos separados (não estão no objeto task principal)
- **Checklists**: Checklists são arrays de itens (pode ter estado checked/unchecked por item)
- **Status**: Status pode variar (ex: `todo`, `in_progress`, `done`, `blocked`)

### Rotas

| Método   | Rota                        | Middleware | Descrição              |
| -------- | --------------------------- | ---------- | ---------------------- |
| `POST`   | `/api/tasks`                | `withAuth` | ➕ Cria task           |
| `GET`    | `/api/tasks`                | `withAuth` | 📋 Lista tasks         |
| `GET`    | `/api/tasks/:id`            | `withAuth` | 🔍 Busca task          |
| `PUT`    | `/api/tasks/:id`            | `withAuth` | ✏️ Atualiza task       |
| `PUT`    | `/api/tasks/:id/status`     | `withAuth` | 🔄 Atualiza status     |
| `DELETE` | `/api/tasks/:id`            | `withAuth` | 🗑️ Deleta task         |
| `POST`   | `/api/tasks/:id/comments`   | `withAuth` | 💬 Adiciona comentário |
| `GET`    | `/api/tasks/:id/comments`   | `withAuth` | 💬 Lista comentários   |
| `POST`   | `/api/tasks/:id/checklists` | `withAuth` | ✅ Cria checklist      |
| `GET`    | `/api/tasks/:id/checklists` | `withAuth` | ✅ Lista checklists    |

---

## 📆 Eventos (Events)

**Base Path:** `/api/events`  
**Controller:** `EventController`  
**Domínio:** `calendar`

### ⚠️ Regras e Exceções

- **Ordem de Rotas**: `GET /events` deve vir ANTES de `GET /events/:id`
- **Participantes**: Participantes têm status próprio (`accepted`, `declined`, `pending`)
- **Rota de Status**: `PUT /events/participants/:id/status` (não é `/events/:id/participants/:id/status`)
- **Lembretes**: Lembretes podem ser múltiplos por evento (notificações programadas)

### Rotas

| Método   | Rota                                  | Middleware | Descrição                        |
| -------- | ------------------------------------- | ---------- | -------------------------------- |
| `GET`    | `/api/events`                         | `withAuth` | 📋 Lista eventos (ANTES DE /:id) |
| `POST`   | `/api/events`                         | `withAuth` | ➕ Cria evento                   |
| `GET`    | `/api/events/:id`                     | `withAuth` | 🔍 Busca evento                  |
| `PUT`    | `/api/events/:id`                     | `withAuth` | ✏️ Atualiza evento               |
| `DELETE` | `/api/events/:id`                     | `withAuth` | 🗑️ Deleta evento                 |  
| `POST`   | `/api/events/:id/participants`        | `withAuth` | 👥 Adiciona participante         |
| `GET`    | `/api/events/:id/participants`        | `withAuth` | 👥 Lista participantes           |
| `PUT`    | `/api/events/participants/:id/status` | `withAuth` | 🔄 Atualiza status participante  |
| `POST`   | `/api/events/:id/reminders`           | `withAuth` | ⏰ Adiciona lembrete             |
| `GET`    | `/api/events/:id/reminders`           | `withAuth` | ⏰ Lista lembretes               |

---

## 💰 Finanças (Finance)

**Base Path:** `/api/finance`  
**Controller:** `FinanceController`  
**Domínio:** `finance`

### ⚠️ Regras e Exceções

- **Rota de Teste**: `POST /api/finance/test` NÃO requer autenticação (⚠️ apenas para debug)
- **Transferência**: `POST /transactions/transfer` é rota especial para transferir entre contas
- **Estados de Transação**: Transações podem ter estados que impedem edição após `completed`
- **Orçamentos**: Orçamentos podem ter período (start_date, end_date) e categoria associada
- **Relatórios**: Todos os relatórios podem aceitar filtros de data/período
- **Saldo Calculado**: `GET /accounts/:id/balance` calcula saldo baseado em transações

### Rotas

#### Contas (Accounts)

| Método   | Rota                                | Middleware | Descrição         |
| -------- | ----------------------------------- | ---------- | ----------------- |
| `POST`   | `/api/finance/accounts`             | `withAuth` | ➕ Cria conta     |
| `GET`    | `/api/finance/accounts`             | `withAuth` | 📋 Lista contas   |
| `GET`    | `/api/finance/accounts/:id/balance` | `withAuth` | 💰 Saldo da conta |
| `GET`    | `/api/finance/accounts/:id`         | `withAuth` | 🔍 Busca conta    |
| `PUT`    | `/api/finance/accounts/:id`         | `withAuth` | ✏️ Atualiza conta |
| `DELETE` | `/api/finance/accounts/:id`         | `withAuth` | 🗑️ Deleta conta   |

#### Transações (Transactions)

| Método   | Rota                                     | Middleware | Descrição                 |
| -------- | ---------------------------------------- | ---------- | ------------------------- |
| `POST`   | `/api/finance/transactions`              | `withAuth` | ➕ Cria transação         |
| `POST`   | `/api/finance/transactions/transfer`     | `withAuth` | 🔄 Transfere entre contas |
| `GET`    | `/api/finance/transactions`              | `withAuth` | 📋 Lista transações       |
| `GET`    | `/api/finance/transactions/:id`          | `withAuth` | 🔍 Busca transação        |
| `PUT`    | `/api/finance/transactions/:id`          | `withAuth` | ✏️ Atualiza transação     |
| `POST`   | `/api/finance/transactions/:id/complete` | `withAuth` | ✅ Completa transação     |
| `POST`   | `/api/finance/transactions/:id/cancel`   | `withAuth` | ❌ Cancela transação      |
| `DELETE` | `/api/finance/transactions/:id`          | `withAuth` | 🗑️ Deleta transação       |

#### Categorias (Categories)

| Método   | Rota                          | Middleware | Descrição             |
| -------- | ----------------------------- | ---------- | --------------------- |
| `POST`   | `/api/finance/categories`     | `withAuth` | ➕ Cria categoria     |
| `GET`    | `/api/finance/categories`     | `withAuth` | 📋 Lista categorias   |
| `GET`    | `/api/finance/categories/:id` | `withAuth` | 🔍 Busca categoria    |
| `PUT`    | `/api/finance/categories/:id` | `withAuth` | ✏️ Atualiza categoria |
| `DELETE` | `/api/finance/categories/:id` | `withAuth` | 🗑️ Deleta categoria   |

#### Orçamentos (Budgets)

| Método   | Rota                       | Middleware | Descrição             |
| -------- | -------------------------- | ---------- | --------------------- |
| `POST`   | `/api/finance/budgets`     | `withAuth` | ➕ Cria orçamento     |
| `GET`    | `/api/finance/budgets`     | `withAuth` | 📋 Lista orçamentos   |
| `GET`    | `/api/finance/budgets/:id` | `withAuth` | 🔍 Busca orçamento    |
| `PUT`    | `/api/finance/budgets/:id` | `withAuth` | ✏️ Atualiza orçamento |
| `DELETE` | `/api/finance/budgets/:id` | `withAuth` | 🗑️ Deleta orçamento   |

#### Relatórios (Reports)

| Método | Rota                                      | Middleware | Descrição                   |
| ------ | ----------------------------------------- | ---------- | --------------------------- |
| `GET`  | `/api/finance/reports/cashflow`           | `withAuth` | 💰 Fluxo de caixa           |
| `GET`  | `/api/finance/reports/income-vs-expense`  | `withAuth` | 📊 Receitas vs Despesas     |
| `GET`  | `/api/finance/reports/by-category`        | `withAuth` | 📁 Por categoria            |
| `GET`  | `/api/finance/reports/profit-loss`        | `withAuth` | 💵 Lucro/Prejuízo           |
| `GET`  | `/api/finance/reports/budget-performance` | `withAuth` | 📈 Performance do orçamento |

**⚠️ Test Route:**

- `POST /api/finance/test` - ⚠️ **SEM AUTENTICAÇÃO** (apenas para debug)

---

## 💎 Assinaturas (Subscriptions)

**Base Path:** `/api/subscription`  
**Controller:** `SubscriptionController`  
**Domínio:** `subscription`

### ⚠️ Regras e Exceções

- **Planos vs Assinaturas**: Planos são templates, Assinaturas são instâncias de planos para clientes
- **Status**: Assinaturas podem ter status: `active`, `cancelled`, `expired`, `pending`, `past_due`
- **Renovação**: `POST /:id/renew` processa renovação (pode gerar nova cobrança)
- **Cancelamento**: Cancelar pode manter acesso até fim do período pago (não remove imediatamente)
- **Billing History**: Histórico de cobranças inclui tentativas de pagamento e estados

### Rotas

#### Planos (Plans)

| Método | Rota                          | Middleware | Descrição       |
| ------ | ----------------------------- | ---------- | --------------- |
| `POST` | `/api/subscription/plans`     | `withAuth` | ➕ Cria plano   |
| `GET`  | `/api/subscription/plans`     | `withAuth` | 📋 Lista planos |
| `GET`  | `/api/subscription/plans/:id` | `withAuth` | 🔍 Busca plano  |

#### Assinaturas (Subscriptions)

| Método | Rota                                          | Middleware | Descrição                 |
| ------ | --------------------------------------------- | ---------- | ------------------------- |
| `POST` | `/api/subscription/subscriptions`             | `withAuth` | ➕ Cria assinatura        |
| `GET`  | `/api/subscription/subscriptions`             | `withAuth` | 📋 Lista assinaturas      |
| `GET`  | `/api/subscription/subscriptions/:id`         | `withAuth` | 🔍 Busca assinatura       |
| `POST` | `/api/subscription/subscriptions/:id/cancel`  | `withAuth` | ❌ Cancela assinatura     |
| `POST` | `/api/subscription/subscriptions/:id/renew`   | `withAuth` | 🔄 Renova assinatura      |
| `GET`  | `/api/subscription/subscriptions/:id/billing` | `withAuth` | 💳 Histórico de cobranças |

#### Estatísticas

| Método | Rota                      | Middleware | Descrição                      |
| ------ | ------------------------- | ---------- | ------------------------------ |
| `GET`  | `/api/subscription/stats` | `withAuth` | 📊 Estatísticas de assinaturas |

---

## 🔍 Admin

**Base Path:** `/api/admin`  
**Domínio:** `audit`, `database`

### ⚠️ Regras e Exceções

- **Audit Logs**: Requer role `admin` ou `owner` (mais restritivo que outras rotas de admin)
- **Database Query**: ⚠️ **PERIGOSO** - Executa SQL direto (pode causar problemas de segurança)
- **Schema Info**: `GET /tables/:name/schema` retorna estrutura completa da tabela
- **Stats**: Estatísticas podem incluir contagem de registros, tamanho, índices
- **Produção**: ⚠️ Desabilitar ou restringir fortemente estas rotas em produção

### Rotas

| Método | Rota                                      | Middleware                            | Descrição                |
| ------ | ----------------------------------------- | ------------------------------------- | ------------------------ |
| `GET`  | `/api/admin/audit-logs`                   | `withAuthAndRole({"admin", "owner"})` | 📜 Logs de auditoria     |
| `POST` | `/api/admin/database/query`               | `withAuth`                            | 🔍 Executa query SQL     |
| `GET`  | `/api/admin/database/tables`              | `withAuth`                            | 📋 Lista tabelas         |
| `GET`  | `/api/admin/database/tables/:name/schema` | `withAuth`                            | 📐 Schema da tabela      |
| `GET`  | `/api/admin/database/tables/:name/count`  | `withAuth`                            | 🔢 Contagem de registros |
| `GET`  | `/api/admin/database/stats`               | `withAuth`                            | 📊 Estatísticas do banco |

**⚠️ Atenção:** Rotas de admin são sensíveis. Use com cuidado em produção.

---

## 🌐 Sistema/Health

**Base Path:** `/`, `/health`, `/metrics`  
**Controller:** Handlers diretos

### Rotas

| Método | Rota                       | Middleware | Descrição                          |
| ------ | -------------------------- | ---------- | ---------------------------------- |
| `GET`  | `/`                        | -          | 🏠 Página inicial (HTML com rotas) |
| `GET`  | `/health`                  | -          | ❤️ Health check                    |
| `GET`  | `/ready`                   | -          | ✅ Readiness check                 |
| `GET`  | `/live`                    | -          | 💓 Liveness check                  |
| `GET`  | `/metrics`                 | -          | 📊 Métricas Prometheus             |
| `GET`  | `/ws`                      | -          | 🔌 WebSocket endpoint              |
| `GET`  | `/api/pool/stats`          | -          | 📊 Stats do connection pool        |
| `GET`  | `/api/db/test`             | -          | 🧪 Teste de conexão DB             |
| `GET`  | `/api/db/query`            | -          | 🔍 Query DB direta                 |
| `GET`  | `/api/stress/:connections` | -          | ⚠️ Stress test                     |
| `GET`  | `/api/cache/test`          | -          | 🧪 Teste de cache                  |
| `POST` | `/api/queue/publish`       | -          | 📨 Publica na fila                 |
| `GET`  | `/api/queue/status`        | -          | 📊 Status da fila                  |
| `GET`  | `/api/ws/info`             | -          | ℹ️ Info WebSocket                  |
| `POST` | `/api/pubsub/broadcast`    | -          | 📢 Broadcast pub/sub               |

**Notas:**

- Rotas de sistema não requerem autenticação
- Algumas rotas são apenas para debug/teste
- Remover rotas de debug em produção

---

## 📌 Módulos Especiais

### Leilões (Leilões)

**Base Path:** `/leiloes`, `/offers`

| Método | Rota                    | Descrição            |
| ------ | ----------------------- | -------------------- |
| `POST` | `/leiloes`              | Cria leilão          |
| `GET`  | `/leiloes`              | Lista leilões        |
| `GET`  | `/leiloes/:id`          | Busca leilão         |
| `PUT`  | `/leiloes/:id/fechar`   | Fecha leilão         |
| `POST` | `/offers`               | Cria oferta          |
| `GET`  | `/leiloes/:id/offers`   | Ofertas do leilão    |
| `PUT`  | `/offers/:id/shortlist` | Adiciona a shortlist |
| `PUT`  | `/offers/:id/aceitar`   | Aceita oferta        |

---

### Restaurantes

**Base Path:** `/api/restaurantes`

| Método   | Rota                             | Descrição               |
| -------- | -------------------------------- | ----------------------- |
| `POST`   | `/api/restaurantes`              | Cria restaurante        |
| `GET`    | `/api/restaurantes`              | Lista restaurantes      |
| `GET`    | `/api/restaurantes/nearby`       | Restaurantes próximos   |
| `GET`    | `/api/restaurantes/map`          | Mapa de restaurantes    |
| `GET`    | `/api/restaurantes/user/:userId` | Restaurantes do usuário |
| `GET`    | `/api/restaurantes/:id`          | Busca restaurante       |
| `PUT`    | `/api/restaurantes/:id`          | Atualiza restaurante    |
| `DELETE` | `/api/restaurantes/:id`          | Deleta restaurante      |

---

### Freelancers

**Base Path:** `/api/freelancers`

| Método   | Rota                            | Descrição             |
| -------- | ------------------------------- | --------------------- |
| `POST`   | `/api/freelancers`              | Cria freelancer       |
| `GET`    | `/api/freelancers`              | Lista freelancers     |
| `GET`    | `/api/freelancers/nearby`       | Freelancers próximos  |
| `GET`    | `/api/freelancers/user/:userId` | Freelancer do usuário |
| `GET`    | `/api/freelancers/:id`          | Busca freelancer      |
| `PUT`    | `/api/freelancers/:id`          | Atualiza freelancer   |
| `DELETE` | `/api/freelancers/:id`          | Deleta freelancer     |

---

### Vagas

**Base Path:** `/api/vagas`

| Método   | Rota                   | Descrição     |
| -------- | ---------------------- | ------------- |
| `POST`   | `/api/vagas`           | Cria vaga     |
| `GET`    | `/api/vagas`           | Lista vagas   |
| `GET`    | `/api/vagas/:id`       | Busca vaga    |
| `PUT`    | `/api/vagas/:id`       | Atualiza vaga |
| `DELETE` | `/api/vagas/:id`       | Deleta vaga   |
| `POST`   | `/api/vagas/:id/close` | Fecha vaga    |

---

### Candidaturas

**Base Path:** `/api/candidaturas`

| Método | Rota                                         | Descrição                  |
| ------ | -------------------------------------------- | -------------------------- |
| `POST` | `/api/candidaturas`                          | Cria candidatura           |
| `GET`  | `/api/candidaturas/vaga/:vagaId`             | Candidaturas da vaga       |
| `GET`  | `/api/candidaturas/freelancer/:freelancerId` | Candidaturas do freelancer |
| `GET`  | `/api/candidaturas/:id`                      | Busca candidatura          |
| `PUT`  | `/api/candidaturas/:id/accept`               | Aceita candidatura         |
| `PUT`  | `/api/candidaturas/:id/reject`               | Rejeita candidatura        |

---

## 📝 Notas Importantes

### Ordem de Rotas

⚠️ **ATENÇÃO:** Rotas mais específicas devem ser registradas **ANTES** das rotas genéricas:

```cpp
// ✅ CORRETO
router.get("/api/products/search", ...);      // Específica primeiro
router.get("/api/products/low-stock", ...);   // Específica primeiro
router.get("/api/products", ...);             // Genérica depois
router.get("/api/products/:id", ...);         // Com parâmetro por último

// ❌ ERRADO (/:id vai capturar /search e /low-stock)
router.get("/api/products/:id", ...);
router.get("/api/products/search", ...);      // Nunca será alcançada!
```

### Soft Delete

A maioria das rotas `DELETE` fazem **soft delete**:

- Não remove o registro do banco
- Marca `is_active = false` ou `deleted_at = timestamp`
- Permite recuperação futura

### Multi-tenancy

- Todos os dados são isolados por `tenant_id`
- O `tenant_id` vem do token JWT automaticamente
- Queries filtram por `tenant_id` automaticamente
- Usuários podem ter acesso a múltiplos tenants (tabela `user_tenants`)

### Roles e Permissões

- **admin/owner**: Acesso total ao tenant
- **member**: Acesso padrão (cria/edita/deleta próprios recursos)
- **viewer**: Apenas leitura

---

## 🔧 Desenvolvimento

### Adicionar Nova Rota

1. Criar/atualizar Controller no domínio apropriado
2. Registrar rota em `src/main/main_new.cpp`
3. ⚠️ Atentar para ordem das rotas (específicas antes de genéricas)
4. Adicionar middleware apropriado (`withAuth`, `withAuthAndRole`)
5. Atualizar esta documentação

### Padrão de Código

- Controllers devem ser finos (só validação + chamada de use case)
- Lógica de negócio fica em Use Cases
- Repositories fazem acesso a dados
- Entities representam domínio

---

**📚 Documentação gerada automaticamente - Não editar manualmente sem atualizar o código**
