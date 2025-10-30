## AppFreela – Leilões (Sealed Bids)

### Objetivo

Modelo “Leilão + Offers” com ofertas cegas: restaurantes abrem leilões, freelancers enviam propostas sem exposição de preços na fase inicial. Seleção por perfil/shortlist; preço final calculado por clamp após a escolha.

### Status geral (agora)

- ✅ Backend base (schemas, repos, use cases, controllers)
- ✅ Fluxos E2E básicos (Leilões e Offers)
- 🟡 Shortlist/Accept (endpoint disponível; falta E2E completo)
- 🟡 Scheduler (código base; falta ligar/testar E2E)
- 🟡 Field-level auth (blind/ocultação de valores)
- 🟡 Deprecação rotas antigas e update do frontend

### Regras de negócio

- **Leilão**: campos essenciais (restaurante_id, título, categoria, valor_ideal, valor_max_quero, datas, status)
  - **Validações**: valor_ideal ≤ valor_max_quero; data_limite_offers < data_trabalho
  - Estado: ✅ implementado e testado
- **Offer**: leilao_id (quando aplicável), freelancer_id, restaurante_id, mensagem, valor_oferecido, valor_minimo_aceito, shortlisted, status
  - **Validações**: valor_minimo_aceito ≤ valor_oferecido; leilao_id e freelancer_id devem existir
  - Estado: ✅ implementado e testado
- **Blind offers**: ocultar preços na fase inicial (listagens/projeções sem valores)
  - Estado: 🟡 pendente de policy/serializer
- **Shortlist**: seleção por perfil, sem ver preço
  - Estado: 🟡 endpoint pronto; E2E completo pendente
- **Aceite + Clamp**: preço final calculado por clamp entre limites do restaurante e oferta do freelancer
  - Estado: ✅ regra implementada e testada (unit)
- **Scheduler**: fechamento automático de leilões expirados com ranking e aceite
  - Estado: 🟡 ligar no bootstrap e validar E2E

### Funcionalidades de backend

- **Banco/Migrações**
  - Tabelas `leiloes`, `offers`, índices e triggers: ✅
  - Migração legado (vagas → leiloes; candidaturas → offers): ✅
- **Domínio/Repos/Use Cases**
  - Leilões: Create, List, GetById, Close: ✅
  - Offers: Create, ListForLeilao, Shortlist, Accept: ✅ (Shortlist/Accept com E2E a estender)
- **API**
  - Leilões: `POST /leiloes`, `GET /leiloes`, `GET /leiloes/:id`, `PUT /leiloes/:id/fechar`: ✅
  - Offers: `POST /offers`, `GET /leiloes/:id/offers`, `PUT /offers/:id/shortlist`, `PUT /offers/:id/aceitar`: ✅
- **Infra**
  - Pool PostgreSQL (porta 5433), Redis, WebSocket: ✅
  - Logs detalhados (controllers, repos, use cases): ✅

### Testes

- **Unit**
  - CreateLeilaoUseCase (validações): ✅
  - CreateOfferUseCase (validações): ✅
  - AcceptOfferUseCase (clamp): ✅
  - Ranking (scheduler): ✅
- **E2E (PowerShell)**
  - Leilões: criar → listar → getById → fechar: ✅
  - Offers: criar leilão (IDs reais via psql) → criar offer → listar offers: ✅
  - Shortlist + Accept + Scheduler encadeado: 🟡 pendente

### Próximas entregas

- **Alta prioridade**
  - Ligar scheduler e expor trigger/endpoint para E2E
  - Implementar policies de projeção (blind) por papel/estado
- **Média prioridade**
  - Estender E2E: shortlist + accept (com clamp) + fechamento automático
  - Plano de deprecação das rotas antigas (comunicação/versão)
  - Atualizar frontend: UI de ofertas cegas, shortlist e seleção

### Notas operacionais

- Testes consomem psql (porta 5433) e exigem 1 `restaurante_profiles` e 1 `freelancer_profiles` existentes (scripts validam e abortam se ausentes).
- Logs mostram corpo, parâmetros, contagens e IDs, facilitando troubleshooting.
