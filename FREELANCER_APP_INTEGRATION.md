# 🚀 Integração do Freelancer App no Backend

## Status dos Módulos

- ✅ **restaurantes/** - Entity, Repository, Use Cases, Controller (COMPLETO)
- ⏳ **freelances/** - Em desenvolvimento
- ⏳ **vagas/** - Pendente
- ⏳ **candidaturas/** - Pendente
- ⏳ **jobs_agreed/** - Pendente
- ⏳ **avaliacoes/** - Pendente

---

## Passo 1: Aplicar Schema SQL

```bash
# No Docker PostgreSQL
docker exec -i postgres_backend psql -U myuser -d mydb < schemas/02-freelancer-app.sql
```

Ou pelo script PowerShell:

```powershell
cd C:\Users\Guillermo\Desktop\cppBackend
docker exec -i postgres_backend psql -U myuser -d mydb -f /docker-entrypoint-initdb.d/02-freelancer-app.sql
```

---

## Passo 2: Adicionar Includes no main_new.cpp

Adicionar após as linhas 78 (após Payment includes):

```cpp
// ==================== FREELANCER APP MODULES ====================
// Restaurantes Domain
#include "../domains/restaurantes/entities/RestauranteProfile.hpp"
#include "../domains/restaurantes/repositories/IRestauranteRepository.hpp"
#include "../domains/restaurantes/repositories/RestauranteRepository.hpp"
#include "../domains/restaurantes/use_cases/CreateRestauranteUseCase.hpp"
#include "../domains/restaurantes/use_cases/GetRestauranteUseCase.hpp"
#include "../domains/restaurantes/use_cases/UpdateRestauranteUseCase.hpp"
#include "../domains/restaurantes/use_cases/ListRestaurantesUseCase.hpp"
#include "../domains/restaurantes/use_cases/SearchNearbyRestaurantesUseCase.hpp"
#include "../domains/restaurantes/controllers/RestauranteController.hpp"

// Freelances Domain (TODO)
// Vagas Domain (TODO)
// Candidaturas Domain (TODO)
// Jobs Agreed Domain (TODO)
// Avaliacoes Domain (TODO)
```

---

## Passo 3: Instanciar Controllers no main()

Adicionar na função `main()`, após a criação do `financeController`:

```cpp
// ==================== RESTAURANTES CONTROLLER ====================
LOG_INFO("[MAIN] Creating Restaurantes controller...");
auto restauranteRepository = std::make_shared<Domains::Restaurantes::Repositories::RestauranteRepository>(pool);
auto createRestauranteUseCase = std::make_shared<Domains::Restaurantes::UseCases::CreateRestauranteUseCase>(restauranteRepository);
auto getRestauranteUseCase = std::make_shared<Domains::Restaurantes::UseCases::GetRestauranteUseCase>(restauranteRepository);
auto updateRestauranteUseCase = std::make_shared<Domains::Restaurantes::UseCases::UpdateRestauranteUseCase>(restauranteRepository);
auto listRestaurantesUseCase = std::make_shared<Domains::Restaurantes::UseCases::ListRestaurantesUseCase>(restauranteRepository);
auto searchNearbyRestaurantesUseCase = std::make_shared<Domains::Restaurantes::UseCases::SearchNearbyRestaurantesUseCase>(restauranteRepository);

auto restauranteController = std::make_shared<Domains::Restaurantes::Controllers::RestauranteController>(
    createRestauranteUseCase,
    getRestauranteUseCase,
    updateRestauranteUseCase,
    listRestaurantesUseCase,
    searchNearbyRestaurantesUseCase
);
LOG_INFO("[MAIN] Restaurantes controller created!");
```

---

## Passo 4: Registrar Rotas no setupRoutes()

Adicionar no final da função `setupRoutes()`, antes do fechamento:

```cpp
// ==================== RESTAURANTES ROUTES ====================
LOG_INFO("[setupRoutes] Registering Restaurantes routes...");

// POST /api/restaurantes - Criar perfil de restaurante
router.post("/api/restaurantes", [restauranteController](const Request& req) {
    return restauranteController->create(req);
});

// GET /api/restaurantes - Listar restaurantes (com filtros)
router.get("/api/restaurantes", [restauranteController](const Request& req) {
    return restauranteController->list(req);
});

// GET /api/restaurantes/:id - Buscar por ID
router.get("/api/restaurantes/:id", [restauranteController](const Request& req) {
    return restauranteController->getById(req);
});

// GET /api/restaurantes/user/:userId - Buscar por user_id
router.get("/api/restaurantes/user/:userId", [restauranteController](const Request& req) {
    return restauranteController->getByUserId(req);
});

// PUT /api/restaurantes/:id - Atualizar perfil
router.put("/api/restaurantes/:id", [restauranteController](const Request& req) {
    return restauranteController->update(req);
});

// DELETE /api/restaurantes/:id - Deletar perfil
router.del("/api/restaurantes/:id", [restauranteController](const Request& req) {
    return restauranteController->deleteById(req);
});

// GET /api/restaurantes/nearby - Buscar próximos
router.get("/api/restaurantes/nearby", [restauranteController](const Request& req) {
    return restauranteController->nearby(req);
});

// GET /api/restaurantes/map - Para exibir no mapa
router.get("/api/restaurantes/map", [restauranteController](const Request& req) {
    return restauranteController->map(req);
});

LOG_INFO("[setupRoutes] Restaurantes routes registered!");
```

---

## Passo 5: Compilar e Testar

```bash
# Compilar
cd C:\Users\Guillermo\Desktop\cppBackend\build
cmake --build . --target cppBackend

# Rodar servidor
.\cppBackend.exe

# Testar endpoint
curl http://localhost:8080/health
curl -X POST http://localhost:8080/api/restaurantes -H "Content-Type: application/json" -d "{\"userId\":\"user123\",\"cnpj\":\"12345678000190\",\"razaoSocial\":\"Restaurante Teste\",\"latitude\":-23.5505,\"longitude\":-46.6333}"
```

---

## Endpoints Disponíveis

### Restaurantes

| Método | Endpoint                         | Descrição                                                 |
| ------ | -------------------------------- | --------------------------------------------------------- |
| POST   | `/api/restaurantes`              | Criar perfil de restaurante                               |
| GET    | `/api/restaurantes`              | Listar restaurantes (filtros: ?categoria=&cidade=&ativo=) |
| GET    | `/api/restaurantes/:id`          | Buscar restaurante por ID                                 |
| GET    | `/api/restaurantes/user/:userId` | Buscar restaurante por user_id                            |
| PUT    | `/api/restaurantes/:id`          | Atualizar perfil                                          |
| DELETE | `/api/restaurantes/:id`          | Deletar perfil                                            |
| GET    | `/api/restaurantes/nearby`       | Buscar próximos (?lat=&lng=&radius=)                      |
| GET    | `/api/restaurantes/map`          | Para exibir no mapa (dados simplificados)                 |

### Exemplo de Body - Criar Restaurante

```json
{
  "userId": "user-uuid-here",
  "cnpj": "12345678000190",
  "razaoSocial": "Restaurante Exemplo Ltda",
  "nomeFantasia": "Restaurante Exemplo",
  "descricao": "Melhor comida da cidade!",
  "categoria": "restaurante",
  "latitude": -23.5505,
  "longitude": -46.6333,
  "enderecoCompleto": "Rua Exemplo, 123",
  "cep": "01234-567",
  "cidade": "São Paulo",
  "estado": "SP",
  "horarioFuncionamento": "{\"seg\": \"08:00-18:00\", \"ter\": \"08:00-18:00\"}"
}
```

---

## TODO

- [ ] Aplicar schema SQL no PostgreSQL
- [ ] Adicionar includes no main_new.cpp
- [ ] Instanciar controllers
- [ ] Registrar rotas
- [ ] Compilar
- [ ] Testar endpoints
- [ ] Implementar módulo freelances/
- [ ] Implementar módulo vagas/
- [ ] Implementar módulo candidaturas/
- [ ] Implementar módulo jobs_agreed/
- [ ] Implementar módulo avaliacoes/
