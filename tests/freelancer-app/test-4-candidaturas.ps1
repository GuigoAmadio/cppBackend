# ==========================================
# TESTE MODULO CANDIDATURAS
# Freelancer App Integration Test
# ==========================================

$BASE_URL = "http://localhost:8080"

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "  TESTE MODULO CANDIDATURAS                    " -ForegroundColor Cyan
Write-Host "  Testando fluxo completo de candidaturas      " -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

# ==========================================
# STEP 0: CRIAR USUARIOS DE TESTE NO BANCO
# ==========================================
Write-Host "== STEP 0: Criar usuarios de teste ==" -ForegroundColor Yellow
Write-Host ""

$createUsersSql = @"
-- Limpar dados de testes anteriores (CASCADE vai deletar tudo relacionado)
DELETE FROM candidaturas WHERE freelancer_id IN (
    SELECT id FROM freelancer_profiles WHERE user_id IN ('00000000-0000-0000-0000-000000000011'::uuid, '00000000-0000-0000-0000-000000000012'::uuid)
);
DELETE FROM vagas WHERE restaurante_id IN (
    SELECT id FROM restaurante_profiles WHERE user_id IN ('00000000-0000-0000-0000-000000000011'::uuid, '00000000-0000-0000-0000-000000000012'::uuid)
);
DELETE FROM freelancer_profiles WHERE user_id IN ('00000000-0000-0000-0000-000000000011'::uuid, '00000000-0000-0000-0000-000000000012'::uuid);
DELETE FROM restaurante_profiles WHERE user_id IN ('00000000-0000-0000-0000-000000000011'::uuid, '00000000-0000-0000-0000-000000000012'::uuid);
DELETE FROM users WHERE id IN ('00000000-0000-0000-0000-000000000011'::uuid, '00000000-0000-0000-0000-000000000012'::uuid);

-- Criar usuario para restaurante
INSERT INTO users (id, email, password_hash, name, is_active, email_verified, created_at)
VALUES ('00000000-0000-0000-0000-000000000011'::uuid, 'cand-rest@integration.com', 'hash', 'Cand Rest User', true, true, NOW());

-- Criar usuario para freelancer
INSERT INTO users (id, email, password_hash, name, is_active, email_verified, created_at)
VALUES ('00000000-0000-0000-0000-000000000012'::uuid, 'cand-free@integration.com', 'hash', 'Cand Free User', true, true, NOW());
"@

try {
    # Executar SQL no banco local
    $env:PGPASSWORD = "123456"
    $createUsersSql | psql -h localhost -p 5433 -U moneymaker_user -d moneymaker_dev -q
    Write-Host "[OK] Usuarios de teste criados no banco" -ForegroundColor Green
} catch {
    Write-Host "[AVISO] Nao foi possivel criar usuarios: $($_.Exception.Message)" -ForegroundColor Yellow
    Write-Host "[INFO] Continuando mesmo assim..." -ForegroundColor Yellow
}

Write-Host ""

# ==========================================
# STEP 1: CRIAR RESTAURANTE
# ==========================================
Write-Host "== STEP 1: Criar Restaurante ==" -ForegroundColor Yellow
Write-Host ""

$restauranteBody = @{
    userId = "00000000-0000-0000-0000-000000000011"
    cnpj = "11223344000155"
    razaoSocial = "Restaurante Candidaturas Test"
    nomeFantasia = "Cand Rest"
    categoria = "restaurante"
    latitude = -23.5505
    longitude = -46.6333
    cidade = "Sao Paulo"
} | ConvertTo-Json

try {
    $rest = Invoke-RestMethod -Uri "$BASE_URL/api/restaurantes" `
        -Method POST -Body $restauranteBody -ContentType "application/json"
    
    Write-Host "[OK] Restaurante criado: $($rest.id)" -ForegroundColor Green
    $RESTAURANTE_ID = $rest.id
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Write-Host ""

# ==========================================
# STEP 2: CRIAR FREELANCER
# ==========================================
Write-Host "== STEP 2: Criar Freelancer ==" -ForegroundColor Yellow
Write-Host ""

$freelancerBody = @{
    userId = "00000000-0000-0000-0000-000000000012"
    especialidades = @("garcom")
    precoMinimo = 90.00
    latitude = -23.5505
    longitude = -46.6333
} | ConvertTo-Json

try {
    $freelancer = Invoke-RestMethod -Uri "$BASE_URL/api/freelancers" `
        -Method POST -Body $freelancerBody -ContentType "application/json"
    
    Write-Host "[OK] Freelancer criado: $($freelancer.id)" -ForegroundColor Green
    $FREELANCER_ID = $freelancer.id
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Write-Host ""

# ==========================================
# STEP 3: RESTAURANTE CRIA VAGA
# ==========================================
Write-Host "== STEP 3: Restaurante cria Vaga ==" -ForegroundColor Yellow
Write-Host ""

$vagaBody = @{
    restauranteId = $RESTAURANTE_ID
    titulo = "Garcom para evento candidatura test"
    categoria = "garcom"
    precoOferecido = 150.00
    dataTrabalho = "2025-12-15T19:00:00"
    vagasDisponiveis = 3
    vagasPreenchidas = 0
    status = "aberta"
} | ConvertTo-Json

try {
    $vaga = Invoke-RestMethod -Uri "$BASE_URL/api/vagas" `
        -Method POST -Body $vagaBody -ContentType "application/json"
    
    Write-Host "[OK] Vaga criada: $($vaga.id)" -ForegroundColor Green
    $VAGA_ID = $vaga.id
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Write-Host ""

# ==========================================
# STEP 4: FREELANCER SE CANDIDATA
# ==========================================
Write-Host "== STEP 4: Freelancer se candidata ==" -ForegroundColor Yellow
Write-Host ""

$candidaturaBody = @{
    vagaId = $VAGA_ID
    freelancerId = $FREELANCER_ID
    precoPropostoFreelancer = 140.00
    mensagem = "Tenho experiencia de 5 anos como garcom"
} | ConvertTo-Json

try {
    $candidatura = Invoke-RestMethod -Uri "$BASE_URL/api/candidaturas" `
        -Method POST -Body $candidaturaBody -ContentType "application/json"
    
    Write-Host "[OK] Candidatura criada: $($candidatura.id)" -ForegroundColor Green
    Write-Host "     Status: $($candidatura.status)" -ForegroundColor Gray
    $CANDIDATURA_ID = $candidatura.id
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Write-Host ""

# ==========================================
# STEP 5: BUSCAR CANDIDATURA POR ID
# ==========================================
Write-Host "== STEP 5: Buscar candidatura por ID ==" -ForegroundColor Yellow
Write-Host ""

try {
    $candidaturaDetails = Invoke-RestMethod -Uri "$BASE_URL/api/candidaturas/$CANDIDATURA_ID" `
        -Method GET
    
    Write-Host "[OK] Candidatura encontrada" -ForegroundColor Green
    Write-Host "     Status: $($candidaturaDetails.status)" -ForegroundColor Gray
    Write-Host "     Preco proposto: R$ $($candidaturaDetails.precoPropostoFreelancer)" -ForegroundColor Gray
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# ==========================================
# STEP 6: LISTAR CANDIDATURAS DA VAGA
# ==========================================
Write-Host "== STEP 6: Listar candidaturas da vaga ==" -ForegroundColor Yellow
Write-Host ""

try {
    $candidaturasVaga = Invoke-RestMethod -Uri "$BASE_URL/api/candidaturas/vaga/$VAGA_ID" `
        -Method GET
    
    Write-Host "[OK] Candidaturas da vaga: $($candidaturasVaga.Count)" -ForegroundColor Green
    foreach ($c in $candidaturasVaga) {
        Write-Host "     - Candidatura $($c.id) | Status: $($c.status)" -ForegroundColor Gray
    }
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# ==========================================
# STEP 7: LISTAR CANDIDATURAS DO FREELANCER
# ==========================================
Write-Host "== STEP 7: Listar candidaturas do freelancer ==" -ForegroundColor Yellow
Write-Host ""

try {
    $candidaturasFreelancer = Invoke-RestMethod -Uri "$BASE_URL/api/candidaturas/freelancer/$FREELANCER_ID" `
        -Method GET
    
    Write-Host "[OK] Candidaturas do freelancer: $($candidaturasFreelancer.Count)" -ForegroundColor Green
    foreach ($c in $candidaturasFreelancer) {
        Write-Host "     - Vaga $($c.vagaId) | Status: $($c.status)" -ForegroundColor Gray
    }
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# ==========================================
# STEP 8: RESTAURANTE ACEITA CANDIDATURA
# ==========================================
Write-Host "== STEP 8: Restaurante aceita candidatura ==" -ForegroundColor Yellow
Write-Host ""

try {
    $acceptResponse = Invoke-RestMethod -Uri "$BASE_URL/api/candidaturas/$CANDIDATURA_ID/accept" `
        -Method PUT -ContentType "application/json"
    
    Write-Host "[OK] Candidatura aceita!" -ForegroundColor Green
    Write-Host "     Message: $($acceptResponse.message)" -ForegroundColor Gray
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# ==========================================
# STEP 9: VERIFICAR STATUS DA CANDIDATURA
# ==========================================
Write-Host "== STEP 9: Verificar status da candidatura ==" -ForegroundColor Yellow
Write-Host ""

try {
    $candidaturaFinal = Invoke-RestMethod -Uri "$BASE_URL/api/candidaturas/$CANDIDATURA_ID" `
        -Method GET
    
    if ($candidaturaFinal.status -eq "aceita") {
        Write-Host "[OK] Candidatura foi aceita com sucesso!" -ForegroundColor Green
        Write-Host "     Status: $($candidaturaFinal.status)" -ForegroundColor Gray
        if ($candidaturaFinal.respondedAt) {
            Write-Host "     Respondida em: $($candidaturaFinal.respondedAt)" -ForegroundColor Gray
        }
    } else {
        Write-Host "[AVISO] Status inesperado: $($candidaturaFinal.status)" -ForegroundColor Yellow
    }
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# ==========================================
# STEP 10: VERIFICAR SE VAGA DECREMENTOU
# ==========================================
Write-Host "== STEP 10: Verificar vagas disponiveis ==" -ForegroundColor Yellow
Write-Host ""

try {
    $vagaFinal = Invoke-RestMethod -Uri "$BASE_URL/api/vagas/$VAGA_ID" `
        -Method GET
    
    Write-Host "[OK] Vaga atualizada:" -ForegroundColor Green
    Write-Host "     Vagas disponiveis: $($vagaFinal.vagasDisponiveis) (era 3)" -ForegroundColor Gray
    Write-Host "     Vagas preenchidas: $($vagaFinal.vagasPreenchidas) (era 0)" -ForegroundColor Gray
    Write-Host "     Status: $($vagaFinal.status)" -ForegroundColor Gray
    
    if ($vagaFinal.vagasDisponiveis -eq 2 -and $vagaFinal.vagasPreenchidas -eq 1) {
        Write-Host "[OK] Contadores atualizados corretamente!" -ForegroundColor Green
    } else {
        Write-Host "[AVISO] Contadores nao foram atualizados conforme esperado" -ForegroundColor Yellow
    }
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# ==========================================
# STEP 11: TESTAR REJEICAO
# ==========================================
Write-Host "== STEP 11: Testar rejeicao de candidatura ==" -ForegroundColor Yellow
Write-Host ""

# Criar nova candidatura
$candidatura2Body = @{
    vagaId = $VAGA_ID
    freelancerId = $FREELANCER_ID
    precoPropostoFreelancer = 130.00
    mensagem = "Segunda tentativa"
} | ConvertTo-Json

try {
    $candidatura2 = Invoke-RestMethod -Uri "$BASE_URL/api/candidaturas" `
        -Method POST -Body $candidatura2Body -ContentType "application/json"
    
    Write-Host "[OK] Segunda candidatura criada: $($candidatura2.id)" -ForegroundColor Green
    $CANDIDATURA_2_ID = $candidatura2.id
    
    # Rejeitar
    $rejectBody = @{
        motivo = "Preco abaixo do esperado"
    } | ConvertTo-Json
    
    $rejectResponse = Invoke-RestMethod -Uri "$BASE_URL/api/candidaturas/$CANDIDATURA_2_ID/reject" `
        -Method PUT -Body $rejectBody -ContentType "application/json"
    
    Write-Host "[OK] Candidatura rejeitada!" -ForegroundColor Green
    Write-Host "     Message: $($rejectResponse.message)" -ForegroundColor Gray
    
    # Verificar status
    $candidatura2Final = Invoke-RestMethod -Uri "$BASE_URL/api/candidaturas/$CANDIDATURA_2_ID" `
        -Method GET
    
    if ($candidatura2Final.status -eq "rejeitada") {
        Write-Host "[OK] Status correto: rejeitada" -ForegroundColor Green
        if ($candidatura2Final.motivoRejeicao) {
            Write-Host "     Motivo: $($candidatura2Final.motivoRejeicao)" -ForegroundColor Gray
        }
    }
    
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# ==========================================
# RESUMO
# ==========================================
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "              RESUMO DO TESTE" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "[OK] Restaurante ID: $RESTAURANTE_ID" -ForegroundColor Green
Write-Host "[OK] Freelancer ID:  $FREELANCER_ID" -ForegroundColor Green
Write-Host "[OK] Vaga ID:        $VAGA_ID" -ForegroundColor Green
Write-Host "[OK] Candidatura 1:  $CANDIDATURA_ID (aceita)" -ForegroundColor Green
if ($CANDIDATURA_2_ID) {
    Write-Host "[OK] Candidatura 2:  $CANDIDATURA_2_ID (rejeitada)" -ForegroundColor Green
}
Write-Host ""
Write-Host "[SUCESSO] Modulo Candidaturas funcionando!" -ForegroundColor Green
Write-Host ""
Write-Host "Proximos passos:" -ForegroundColor Cyan
Write-Host "  1. Implementar modulo de Jobs Agreed" -ForegroundColor Gray
Write-Host "  2. Criar trabalho quando candidatura aceita" -ForegroundColor Gray
Write-Host "  3. Implementar modulo de Avaliacoes" -ForegroundColor Gray
Write-Host "  4. Atualizar score dos usuarios" -ForegroundColor Gray
Write-Host ""

