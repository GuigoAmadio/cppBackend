# 📊 Status da Implementação - Freelancer App

## ✅ Módulos Completos (Estrutura Pronta)

### 1. ✅ Restaurantes

- Entity: `RestauranteProfile.hpp`
- Repository: `IRestauranteRepository.hpp`, `RestauranteRepository.hpp/.cpp`
- Use Cases: Create, Get, Update, List, SearchNearby
- Controller: `RestauranteController.hpp/.cpp`
- **Status:** COMPLETO - Pronto para compilar e testar

### 2. ✅ Freelances

- Entity: `FreelancerProfile.hpp`
- Repository: `IFreelancerRepository.hpp`, `FreelancerRepository.hpp/.cpp`
- Use Cases: Create, Get, Update, List, SearchNearby
- Controller: `FreelancerController.hpp/.cpp`
- **Status:** COMPLETO - Pronto para compilar e testar

### 3. ✅ Vagas

- Entity: `Vaga.hpp`
- Repository: `IVagaRepository.hpp`, `VagaRepository.hpp/.cpp`
- Use Cases: Create, Get, Update, List
- Controller: `VagaController.hpp/.cpp`
- **Status:** COMPLETO - Pronto para compilar e testar

### 4. ⚠️ Candidaturas

- Entity: `Candidatura.hpp` ✅
- Repository: `ICandidaturaRepository.hpp` ✅, `CandidaturaRepository.hpp` ✅
- Use Cases: Submit, Accept, List ✅
- Repository.cpp: ⏳ **PENDENTE**
- Controller: ⏳ **PENDENTE**
- **Status:** 70% COMPLETO - Falta implementação do Repository.cpp e Controller

### 5. ⏳ Jobs Agreed

- **Status:** NÃO INICIADO

### 6. ⏳ Avaliações

- **Status:** NÃO INICIADO

---

## 📝 Próximos Passos

### Passo 1: Completar Módulos Restantes

#### A. Candidaturas - Falta:

1. `CandidaturaRepository.cpp` - Implementação
2. `CandidaturaController.hpp` e `.cpp`

#### B. Jobs Agreed - Criar:

1. `entities/JobAgreed.hpp`
2. `repositories/IJobAgreedRepository.hpp`
3. `repositories/JobAgreedRepository.hpp` e `.cpp`
4. `use_cases/CreateJobAgreedUseCase.hpp` (quando candidatura aceita)
5. `use_cases/CompleteJobUseCase.hpp`
6. `use_cases/ListJobsUseCase.hpp`
7. `controllers/JobAgreedController.hpp` e `.cpp`

#### C. Avaliações - Adaptar ou Criar:

- **Opção 1:** Adaptar módulo existente `domains/audit/`
- **Opção 2:** Criar novo `domains/avaliacoes/`
  1. `entities/Avaliacao.hpp`
  2. `repositories/IAvaliacaoRepository.hpp`
  3. `repositories/AvaliacaoRepository.hpp` e `.cpp`
  4. `use_cases/CreateAvaliacaoUseCase.hpp`
  5. `use_cases/GetAvaliacoesUseCase.hpp`
  6. `controllers/AvaliacaoController.hpp` e `.cpp`

### Passo 2: Adicionar CMakeLists.txt

Criar ou atualizar `CMakeLists.txt` para incluir os novos módulos:

```cmake
# Adicionar fontes dos novos módulos
set(FREELANCER_APP_SOURCES
    src/domains/restaurantes/repositories/RestauranteRepository.cpp
    src/domains/restaurantes/controllers/RestauranteController.cpp
    src/domains/freelances/repositories/FreelancerRepository.cpp
    src/domains/freelances/controllers/FreelancerController.cpp
    src/domains/vagas/repositories/VagaRepository.cpp
    src/domains/vagas/controllers/VagaController.cpp
    src/domains/candidaturas/repositories/CandidaturaRepository.cpp
    src/domains/candidaturas/controllers/CandidaturaController.cpp
    src/domains/jobs_agreed/repositories/JobAgreedRepository.cpp
    src/domains/jobs_agreed/controllers/JobAgreedController.cpp
    src/domains/avaliacoes/repositories/AvaliacaoRepository.cpp
    src/domains/avaliacoes/controllers/AvaliacaoController.cpp
)

# Adicionar ao executável
add_executable(cppBackend
    ${EXISTING_SOURCES}
    ${FREELANCER_APP_SOURCES}
)
```

### Passo 3: Aplicar Schema SQL

```bash
cd C:\Users\Guillermo\Desktop\cppBackend
docker exec -i postgres_backend psql -U myuser -d mydb -f /schemas/02-freelancer-app.sql
```

Ou copiar para o container:

```powershell
docker cp schemas/02-freelancer-app.sql postgres_backend:/tmp/
docker exec -i postgres_backend psql -U myuser -d mydb -f /tmp/02-freelancer-app.sql
```

### Passo 4: Integrar no main_new.cpp

Ver arquivo `FREELANCER_APP_INTEGRATION.md` para detalhes de:

1. Adicionar includes
2. Instanciar controllers
3. Registrar rotas

### Passo 5: Compilar

```bash
cd build
cmake ..
cmake --build . --target cppBackend
```

### Passo 6: Testar Endpoints

```bash
# Rodar servidor
.\cppBackend.exe

# Testar (em outro terminal)
curl http://localhost:8080/api/restaurantes
curl http://localhost:8080/api/freelancers
curl http://localhost:8080/api/vagas
curl http://localhost:8080/api/candidaturas
```

---

## 🔧 Implementações Pendentes (TODO)

### CandidaturaRepository.cpp (ALTA PRIORIDADE)

```cpp
#include "CandidaturaRepository.hpp"
#include "../../../core/utils/LoggerNew.hpp"
#include "../../../core/utils/UuidGenerator.hpp"

namespace Domains::Candidaturas::Repositories {

Candidatura CandidaturaRepository::mapToCandidatura(const QueryResult& result, int row) {
    std::string id = result.getValue(row, 0);
    std::string vagaId = result.getValue(row, 1);
    std::string freelancerId = result.getValue(row, 2);

    Candidatura cand(id, vagaId, freelancerId);

    std::string mensagem = result.getValue(row, 3);
    if (!mensagem.empty()) cand.setMensagem(mensagem);

    std::string preco = result.getValue(row, 4);
    if (!preco.empty()) cand.setPrecoProposto(std::stod(preco));

    std::string status = result.getValue(row, 5);
    if (!status.empty()) cand.setStatus(status);

    cand.setSubmittedAt(result.getValue(row, 6));

    std::string respondedAt = result.getValue(row, 7);
    if (!respondedAt.empty()) cand.setRespondedAt(respondedAt);

    std::string responseMsg = result.getValue(row, 8);
    if (!responseMsg.empty()) cand.setResponseMessage(responseMsg);

    return cand;
}

std::string CandidaturaRepository::create(const Candidatura& cand) {
    LOG_INFO("[CandidaturaRepository] create()");
    auto conn = pool_->acquire();
    std::string id = Core::Utils::UuidGenerator::generate();

    std::string query = R"(
        INSERT INTO candidaturas (
            id, vaga_id, freelancer_id, mensagem, preco_proposto, status
        ) VALUES ($1, $2, $3, NULLIF($4,''), NULLIF($5,'')::decimal, $6)
    )";

    std::vector<std::string> params = {
        id, cand.getVagaId(), cand.getFreelancerId(),
        cand.getMensagem().value_or(""),
        cand.getPrecoPropost().has_value() ? std::to_string(*cand.getPrecoPropost()) : "",
        cand.getStatus()
    };

    conn->executeQuery(query, params);
    return id;
}

std::optional<Candidatura> CandidaturaRepository::findById(const std::string& id) {
    auto conn = pool_->acquire();
    auto result = conn->executeQuery("SELECT * FROM candidaturas WHERE id = $1", {id});
    return result.getRowCount() > 0 ? std::optional<Candidatura>(mapToCandidatura(result, 0)) : std::nullopt;
}

std::vector<Candidatura> CandidaturaRepository::findByVaga(const std::string& vagaId) {
    auto conn = pool_->acquire();
    auto result = conn->executeQuery("SELECT * FROM candidaturas WHERE vaga_id = $1 ORDER BY submitted_at DESC", {vagaId});

    std::vector<Candidatura> candidaturas;
    for (int i = 0; i < result.getRowCount(); i++) {
        candidaturas.push_back(mapToCandidatura(result, i));
    }
    return candidaturas;
}

std::vector<Candidatura> CandidaturaRepository::findByFreelancer(const std::string& freelancerId) {
    auto conn = pool_->acquire();
    auto result = conn->executeQuery("SELECT * FROM candidaturas WHERE freelancer_id = $1 ORDER BY submitted_at DESC", {freelancerId});

    std::vector<Candidatura> candidaturas;
    for (int i = 0; i < result.getRowCount(); i++) {
        candidaturas.push_back(mapToCandidatura(result, i));
    }
    return candidaturas;
}

std::optional<Candidatura> CandidaturaRepository::findByVagaAndFreelancer(const std::string& vagaId, const std::string& freelancerId) {
    auto conn = pool_->acquire();
    auto result = conn->executeQuery("SELECT * FROM candidaturas WHERE vaga_id = $1 AND freelancer_id = $2", {vagaId, freelancerId});
    return result.getRowCount() > 0 ? std::optional<Candidatura>(mapToCandidatura(result, 0)) : std::nullopt;
}

bool CandidaturaRepository::update(const Candidatura& cand) {
    auto conn = pool_->acquire();
    std::string query = R"(
        UPDATE candidaturas SET
            mensagem = NULLIF($1,''),
            preco_proposto = NULLIF($2,'')::decimal,
            status = $3,
            responded_at = CASE WHEN $3 IN ('aceita', 'recusada') THEN CURRENT_TIMESTAMP ELSE responded_at END,
            response_message = NULLIF($4,'')
        WHERE id = $5
    )";

    std::vector<std::string> params = {
        cand.getMensagem().value_or(""),
        cand.getPrecoPropost().has_value() ? std::to_string(*cand.getPrecoPropost()) : "",
        cand.getStatus(),
        cand.getResponseMessage().value_or(""),
        cand.getId()
    };

    conn->executeQuery(query, params);
    return true;
}

bool CandidaturaRepository::deleteById(const std::string& id) {
    auto conn = pool_->acquire();
    conn->executeQuery("DELETE FROM candidaturas WHERE id = $1", {id});
    return true;
}

} // namespace Domains::Candidaturas::Repositories
```

### CandidaturaController.hpp (ALTA PRIORIDADE)

```cpp
#pragma once
#include "../use_cases/SubmitCandidaturaUseCase.hpp"
#include "../use_cases/AcceptCandidaturaUseCase.hpp"
#include "../use_cases/ListCandidaturasUseCase.hpp"
#include "../../../core/http/Request.hpp"
#include "../../../core/http/Response.hpp"
#include <memory>

namespace Domains::Candidaturas::Controllers {

class CandidaturaController {
public:
    CandidaturaController(
        std::shared_ptr<SubmitCandidaturaUseCase> submitUseCase,
        std::shared_ptr<AcceptCandidaturaUseCase> acceptUseCase,
        std::shared_ptr<ListCandidaturasUseCase> listUseCase
    );

    Response submit(const Request& req);  // POST /api/candidaturas
    Response listByVaga(const Request& req);  // GET /api/vagas/:vagaId/candidaturas
    Response listMy(const Request& req);  // GET /api/candidaturas/my
    Response accept(const Request& req);  // POST /api/candidaturas/:id/accept
    Response reject(const Request& req);  // POST /api/candidaturas/:id/reject
    Response cancel(const Request& req);  // DELETE /api/candidaturas/:id

private:
    std::shared_ptr<SubmitCandidaturaUseCase> submitUseCase_;
    std::shared_ptr<AcceptCandidaturaUseCase> acceptUseCase_;
    std::shared_ptr<ListCandidaturasUseCase> listUseCase_;

    // Helper methods
};

} // namespace Domains::Candidaturas::Controllers
```

---

## 📚 Recursos Úteis

- **Padrão de Implementação:** Ver módulos completos (`restaurantes/`, `freelances/`, `vagas/`)
- **Schema SQL:** `schemas/02-freelancer-app.sql`
- **Integração:** `FREELANCER_APP_INTEGRATION.md`
- **Compilação:** `CMakeLists.txt`

---

## 🎯 Meta

Ter todos os 6 módulos funcionais e testáveis:

1. ✅ Restaurantes
2. ✅ Freelances
3. ✅ Vagas
4. ⏳ Candidaturas (70%)
5. ⏳ Jobs Agreed (0%)
6. ⏳ Avaliações (0%)

**Próxima Ação:** Completar os arquivos pendentes de Candidaturas, depois criar Jobs Agreed e Avaliações.
