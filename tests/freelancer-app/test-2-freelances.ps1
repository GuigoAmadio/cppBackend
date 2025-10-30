# ==========================================
# TESTE MÓDULO: FREELANCES
# ==========================================

$BASE_URL = "http://localhost:8080"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " TESTANDO MÓDULO FREELANCES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# ==========================================
# 1. CRIAR FREELANCER
# ==========================================
Write-Host "[1] Criar Freelancer..." -ForegroundColor Yellow

$createFreelancerBody = @{
    userId = "user-freelancer-001"
    especialidades = @("garcom", "barman")
    experienciaAnos = 3
    precoMinimo = 80.00
    cpf = "12345678901"
    latitude = -23.5505
    longitude = -46.6333
    raioAtuacaoKm = 15.0
    disponibilidade = '{"seg": ["08:00-12:00", "14:00-18:00"]}'
} | ConvertTo-Json

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/freelancers" `
        -Method POST `
        -Body $createFreelancerBody `
        -ContentType "application/json"
    
    Write-Host "✅ Freelancer criado com sucesso!" -ForegroundColor Green
    Write-Host "ID: $($response.id)" -ForegroundColor Cyan
    $FREELANCER_ID = $response.id
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao criar freelancer" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 2. LISTAR TODOS FREELANCERS
# ==========================================
Write-Host "[2] Listar Todos Freelancers..." -ForegroundColor Yellow

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/freelancers" -Method GET
    
    Write-Host "✅ Freelancers listados com sucesso!" -ForegroundColor Green
    Write-Host "Total: $($response.total)" -ForegroundColor Cyan
    foreach ($freelancer in $response.freelancers) {
        Write-Host "  - [$($freelancer.id)] User: $($freelancer.userId)" -ForegroundColor White
        Write-Host "    Especialidades: $($freelancer.especialidades -join ', ')" -ForegroundColor Gray
    }
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao listar freelancers" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 3. BUSCAR FREELANCER POR ID
# ==========================================
if ($FREELANCER_ID) {
    Write-Host "[3] Buscar Freelancer por ID..." -ForegroundColor Yellow

    try {
        $response = Invoke-RestMethod -Uri "$BASE_URL/api/freelancers/$FREELANCER_ID" -Method GET
        
        Write-Host "✅ Freelancer encontrado!" -ForegroundColor Green
        Write-Host "User ID: $($response.userId)" -ForegroundColor Cyan
        Write-Host "Especialidades: $($response.especialidades -join ', ')" -ForegroundColor Cyan
        Write-Host "Experiência: $($response.experienciaAnos) anos" -ForegroundColor Cyan
        Write-Host "Preço mínimo: R$ $($response.precoMinimo)" -ForegroundColor Cyan
        Write-Host ""
    } catch {
        Write-Host "❌ Erro ao buscar freelancer" -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        Write-Host ""
    }
}

# ==========================================
# 4. FILTRAR POR ESPECIALIDADE
# ==========================================
Write-Host "[4] Filtrar por Especialidade (garcom)..." -ForegroundColor Yellow

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/freelancers?especialidade=garcom" -Method GET
    
    Write-Host "✅ Freelancers filtrados!" -ForegroundColor Green
    Write-Host "Total: $($response.total)" -ForegroundColor Cyan
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao filtrar freelancers" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 5. BUSCAR FREELANCERS PRÓXIMOS
# ==========================================
Write-Host "[5] Buscar Freelancers Próximos..." -ForegroundColor Yellow

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/freelancers/nearby?lat=-23.5505&lng=-46.6333&radius=20" -Method GET
    
    Write-Host "✅ Freelancers próximos encontrados!" -ForegroundColor Green
    Write-Host "Total: $($response.total)" -ForegroundColor Cyan
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao buscar freelancers próximos" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 6. LISTAR APENAS ATIVOS
# ==========================================
Write-Host "[6] Listar Freelancers Ativos..." -ForegroundColor Yellow

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/freelancers?ativo=true" -Method GET
    
    Write-Host "✅ Freelancers ativos listados!" -ForegroundColor Green
    Write-Host "Total: $($response.total)" -ForegroundColor Cyan
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao listar freelancers ativos" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 7. ATUALIZAR FREELANCER
# ==========================================
if ($FREELANCER_ID) {
    Write-Host "[7] Atualizar Freelancer..." -ForegroundColor Yellow

    $updateBody = @{
        userId = "user-freelancer-001"
        especialidades = @("garcom", "barman", "cozinheiro")
        experienciaAnos = 5
        precoMinimo = 100.00
        latitude = -23.5505
        longitude = -46.6333
    } | ConvertTo-Json

    try {
        $response = Invoke-RestMethod -Uri "$BASE_URL/api/freelancers/$FREELANCER_ID" `
            -Method PUT `
            -Body $updateBody `
            -ContentType "application/json"
        
        Write-Host "✅ Freelancer atualizado!" -ForegroundColor Green
        Write-Host ""
    } catch {
        Write-Host "❌ Erro ao atualizar freelancer" -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        Write-Host ""
    }
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " TESTES CONCLUÍDOS" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

