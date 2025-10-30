# 🧪 Scripts de Teste - Freelancer App

## 📋 Pré-requisitos

1. ✅ Backend compilado e rodando em `http://localhost:8080`
2. ✅ PostgreSQL com schema aplicado (`02-freelancer-app.sql`)
3. ✅ Docker containers rodando (`docker-compose up -d`)

---

## 🚀 Como Executar

### 1. Rodar Servidor

```powershell
cd C:\Users\Guillermo\Desktop\cppBackend\build
.\cppBackend.exe
```

### 2. Executar Testes (em outro terminal)

```powershell
# Navegar para pasta de testes
cd C:\Users\Guillermo\Desktop\cppBackend\tests\freelancer-app

# Testar módulo Restaurantes
.\test-1-restaurantes.ps1

# Testar módulo Freelances
.\test-2-freelances.ps1

# Testar módulo Vagas
.\test-3-vagas.ps1

# Teste integração completa (todos módulos)
.\test-all-modules.ps1
```

---

## 📄 Descrição dos Scripts

### test-1-restaurantes.ps1

Testa o módulo de **Restaurantes**:
- ✅ Criar restaurante
- ✅ Listar todos
- ✅ Buscar por ID
- ✅ Filtrar por categoria
- ✅ Buscar próximos (geolocalização)
- ✅ Buscar markers para mapa
- ✅ Atualizar restaurante

### test-2-freelances.ps1

Testa o módulo de **Freelances**:
- ✅ Criar freelancer
- ✅ Listar todos
- ✅ Buscar por ID
- ✅ Filtrar por especialidade
- ✅ Buscar próximos (geolocalização)
- ✅ Listar apenas ativos
- ✅ Atualizar freelancer

### test-3-vagas.ps1

Testa o módulo de **Vagas**:
- ✅ Criar vaga
- ✅ Listar todas
- ✅ Buscar por ID
- ✅ Filtrar por categoria
- ✅ Listar abertas
- ✅ Listar por restaurante
- ✅ Atualizar vaga
- ✅ Fechar vaga manualmente

### test-all-modules.ps1

**Teste de Integração Completa**:
1. ✅ Cria restaurante
2. ✅ Cria freelancer
3. ✅ Restaurante cria vaga
4. ✅ Verifica status da vaga
5. ✅ Busca freelancers próximos
6. ✅ Busca markers para mapa

---

## 🎨 Output Esperado

Os scripts mostram output colorido:

- 🟢 **Verde**: Sucesso
- 🔴 **Vermelho**: Erro
- 🟡 **Amarelo**: Informação
- 🔵 **Cyan**: Dados retornados

---

## 🐛 Troubleshooting

### Erro: "Conexão recusada"

```
❌ Erro ao criar restaurante
The remote server returned an error: (404) Not Found.
```

**Solução:**
- Verificar se o servidor está rodando
- Verificar se a porta é 8080
- Verificar se os módulos estão compilados

### Erro: "CNPJ já cadastrado"

```
❌ CNPJ já cadastrado
```

**Solução:**
- Normal! Significa que o banco já tem dados
- Editar o script e trocar o CNPJ por outro

### Erro: "Restaurante não encontrado"

```
❌ Vaga ID: null
❌ Erro ao buscar vaga
```

**Solução:**
- Executar primeiro `test-1-restaurantes.ps1`
- Copiar o ID retornado
- Colar no script `test-3-vagas.ps1` na variável `$RESTAURANTE_ID`

---

## 📊 Sequência Recomendada

1. **Primeiro:** `test-1-restaurantes.ps1`
2. **Segundo:** `test-2-freelances.ps1`
3. **Terceiro:** `test-3-vagas.ps1`
4. **Por último:** `test-all-modules.ps1`

---

## 🔍 Verificar Dados no PostgreSQL

```bash
# Acessar PostgreSQL
docker exec -it postgres_backend psql -U myuser -d mydb

# Ver restaurantes
SELECT id, nome_fantasia, cidade, ativo FROM restaurante_profiles;

# Ver freelancers
SELECT id, user_id, especialidades FROM freelancer_profiles;

# Ver vagas
SELECT id, titulo, categoria, preco_oferecido, status FROM vagas;
```

---

## 📝 Próximos Testes (Quando Implementados)

### test-4-candidaturas.ps1 (TODO)

- Freelancer se candidata à vaga
- Restaurante vê candidatos
- Restaurante aceita/recusa candidatura
- Freelancer cancela candidatura

### test-5-jobs-agreed.ps1 (TODO)

- Criar job agreed (após aceitar candidatura)
- Iniciar trabalho
- Concluir trabalho
- Cancelar trabalho

### test-6-avaliacoes.ps1 (TODO)

- Criar avaliação após trabalho
- Ver avaliações de um usuário
- Calcular score médio

---

## 🎯 Objetivos dos Testes

- ✅ Validar que todos os endpoints funcionam
- ✅ Verificar comunicação entre módulos
- ✅ Testar filtros e buscas
- ✅ Validar geolocalização (nearby)
- ✅ Garantir integridade dos dados

---

## 📞 Suporte

Se encontrar erros:
1. Verificar logs do servidor
2. Verificar schema SQL aplicado
3. Verificar se todos os .cpp compilaram
4. Ver `IMPLEMENTATION_STATUS.md` para pendências

---

**Boa sorte nos testes!** 🚀

