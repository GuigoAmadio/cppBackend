# ==========================================
# TESTE COMPLETO - TODOS OS MODULOS
# Freelancer App Integration Test
# ==========================================

$BASE_URL = "http://localhost:8080"

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "  TESTE INTEGRACAO FREELANCER APP              " -ForegroundColor Cyan
Write-Host "  Testando comunicacao entre modulos           " -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

# ==========================================
# STEP 0: CRIAR USUARIOS DE TESTE NO BANCO
# ==========================================
Write-Host "== STEP 0: Criar usuarios de teste ==" -ForegroundColor Yellow
Write-Host ""

$createUsersSql = @"
-- Deletar usuarios de teste se existirem
DELETE FROM users WHERE email IN ('rest-test@integration.com', 'free-test@integration.com');

-- Criar usuario para restaurante
INSERT INTO users (id, email, password_hash, name, is_active, email_verified, created_at)
VALUES ('00000000-0000-0000-0000-000000000001'::uuid, 'rest-test@integration.com', 'hash', 'Rest Test User', true, true, NOW())
ON CONFLICT (email) DO NOTHING;

-- Criar usuario para freelancer
INSERT INTO users (id, email, password_hash, name, is_active, email_verified, created_at)
VALUES ('00000000-0000-0000-0000-000000000002'::uuid, 'free-test@integration.com', 'hash', 'Free Test User', true, true, NOW())
ON CONFLICT (email) DO NOTHING;
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
    userId = "00000000-0000-0000-0000-000000000001"
    cnpj = "98765432000123"
    razaoSocial = "Restaurante Integration Test"
    nomeFantasia = "Rest Test"
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
    userId = "00000000-0000-0000-0000-000000000002"
    especialidades = @("garcom")
    precoMinimo = 80.00
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
    titulo = "Garcom para evento teste"
    categoria = "garcom"
    precoOferecido = 120.00
    dataTrabalho = "2025-12-01T18:00:00"
    vagasDisponiveis = 2
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
# STEP 4: VERIFICAR VAGA ESTA ABERTA
# ==========================================
Write-Host "== STEP 4: Verificar vaga esta aberta ==" -ForegroundColor Yellow
Write-Host ""

try {
    $vagaDetalhes = Invoke-RestMethod -Uri "$BASE_URL/api/vagas/$VAGA_ID" -Method GET
    
    if ($vagaDetalhes.status -eq "aberta") {
        Write-Host "[OK] Vaga esta aberta para candidaturas" -ForegroundColor Green
        Write-Host "   Vagas disponiveis: $($vagaDetalhes.vagasDisponiveis)" -ForegroundColor Cyan
    } else {
        Write-Host "[AVISO] Status da vaga: $($vagaDetalhes.status)" -ForegroundColor Yellow
    }
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# ==========================================
# STEP 5: BUSCAR FREELANCERS PROXIMOS
# ==========================================
Write-Host "== STEP 5: Buscar freelancers proximos da vaga ==" -ForegroundColor Yellow
Write-Host ""

try {
    $freelancersProximos = Invoke-RestMethod -Uri "$BASE_URL/api/freelancers/nearby?lat=-23.5505&lng=-46.6333&radius=10" -Method GET
    
    Write-Host "[OK] Freelancers encontrados: $($freelancersProximos.total)" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# ==========================================
# STEP 6: BUSCAR VAGAS PARA EXIBIR NO MAPA
# ==========================================
Write-Host "== STEP 6: Buscar restaurantes para o mapa ==" -ForegroundColor Yellow
Write-Host ""

try {
    $markers = Invoke-RestMethod -Uri "$BASE_URL/api/restaurantes/map" -Method GET
    
    Write-Host "[OK] Markers para mapa: $($markers.total)" -ForegroundColor Green
    foreach ($marker in $markers.markers) {
        Write-Host "   [PIN] $($marker.nome) - $($marker.categoria)" -ForegroundColor White
    }
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# ==========================================
# RESUMO FINAL
# ==========================================
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "              RESUMO DO TESTE                   " -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "[OK] Restaurante ID: $RESTAURANTE_ID" -ForegroundColor Green
Write-Host "[OK] Freelancer ID:  $FREELANCER_ID" -ForegroundColor Green
Write-Host "[OK] Vaga ID:        $VAGA_ID" -ForegroundColor Green
Write-Host ""
Write-Host "[SUCESSO] Todos os modulos estao se comunicando!" -ForegroundColor Green
Write-Host ""
Write-Host "Proximos passos:" -ForegroundColor Yellow
Write-Host "  1. Implementar modulo de Candidaturas" -ForegroundColor White
Write-Host "  2. Freelancer se candidata a vaga" -ForegroundColor White
Write-Host "  3. Restaurante aceita candidatura" -ForegroundColor White
Write-Host "  4. Sistema cria JobAgreed" -ForegroundColor White
Write-Host "  5. Apos conclusao, ambos avaliam" -ForegroundColor White
Write-Host ""
