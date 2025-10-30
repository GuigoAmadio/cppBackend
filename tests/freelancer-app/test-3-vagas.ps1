# ==========================================
# TESTE MÓDULO: VAGAS
# ==========================================

$BASE_URL = "http://localhost:8080"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " TESTANDO MÓDULO VAGAS" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Pré-requisito: ter um restaurante criado
$RESTAURANTE_ID = "rest-id-teste"  # Substituir por ID real

# ==========================================
# 1. CRIAR VAGA
# ==========================================
Write-Host "[1] Criar Vaga..." -ForegroundColor Yellow

$createVagaBody = @{
    restauranteId = $RESTAURANTE_ID
    titulo = "Garçom para evento - Sábado"
    descricao = "Precisamos de garçom experiente para evento corporativo"
    categoria = "garcom"
    precoOferecido = 150.00
    dataTrabalho = "2025-11-15T19:00:00"
    horarioInicio = "19:00"
    horarioFim = "23:00"
    duracaoHoras = 4.0
    vagasDisponiveis = 3
} | ConvertTo-Json

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/vagas" `
        -Method POST `
        -Body $createVagaBody `
        -ContentType "application/json"
    
    Write-Host "✅ Vaga criada com sucesso!" -ForegroundColor Green
    Write-Host "ID: $($response.id)" -ForegroundColor Cyan
    $VAGA_ID = $response.id
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao criar vaga" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 2. LISTAR TODAS VAGAS
# ==========================================
Write-Host "[2] Listar Todas Vagas..." -ForegroundColor Yellow

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/vagas" -Method GET
    
    Write-Host "✅ Vagas listadas com sucesso!" -ForegroundColor Green
    Write-Host "Total: $($response.total)" -ForegroundColor Cyan
    foreach ($vaga in $response.vagas) {
        Write-Host "  - [$($vaga.id)] $($vaga.titulo)" -ForegroundColor White
        Write-Host "    Categoria: $($vaga.categoria) | Preço: R$ $($vaga.precoOferecido) | Status: $($vaga.status)" -ForegroundColor Gray
    }
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao listar vagas" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 3. BUSCAR VAGA POR ID
# ==========================================
if ($VAGA_ID) {
    Write-Host "[3] Buscar Vaga por ID..." -ForegroundColor Yellow

    try {
        $response = Invoke-RestMethod -Uri "$BASE_URL/api/vagas/$VAGA_ID" -Method GET
        
        Write-Host "✅ Vaga encontrada!" -ForegroundColor Green
        Write-Host "Título: $($response.titulo)" -ForegroundColor Cyan
        Write-Host "Categoria: $($response.categoria)" -ForegroundColor Cyan
        Write-Host "Preço: R$ $($response.precoOferecido)" -ForegroundColor Cyan
        Write-Host "Data: $($response.dataTrabalho)" -ForegroundColor Cyan
        Write-Host "Status: $($response.status)" -ForegroundColor Cyan
        Write-Host "Vagas: $($response.vagasPreenchidas)/$($response.vagasDisponiveis)" -ForegroundColor Cyan
        Write-Host ""
    } catch {
        Write-Host "❌ Erro ao buscar vaga" -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        Write-Host ""
    }
}

# ==========================================
# 4. FILTRAR POR CATEGORIA
# ==========================================
Write-Host "[4] Filtrar por Categoria (garcom)..." -ForegroundColor Yellow

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/vagas?categoria=garcom" -Method GET
    
    Write-Host "✅ Vagas filtradas!" -ForegroundColor Green
    Write-Host "Total: $($response.total)" -ForegroundColor Cyan
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao filtrar vagas" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 5. LISTAR VAGAS ABERTAS
# ==========================================
Write-Host "[5] Listar Vagas Abertas..." -ForegroundColor Yellow

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/vagas?status=aberta" -Method GET
    
    Write-Host "✅ Vagas abertas listadas!" -ForegroundColor Green
    Write-Host "Total: $($response.total)" -ForegroundColor Cyan
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao listar vagas abertas" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 6. LISTAR VAGAS POR RESTAURANTE
# ==========================================
Write-Host "[6] Listar Vagas por Restaurante..." -ForegroundColor Yellow

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/vagas?restauranteId=$RESTAURANTE_ID" -Method GET
    
    Write-Host "✅ Vagas do restaurante listadas!" -ForegroundColor Green
    Write-Host "Total: $($response.total)" -ForegroundColor Cyan
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao listar vagas do restaurante" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 7. ATUALIZAR VAGA
# ==========================================
if ($VAGA_ID) {
    Write-Host "[7] Atualizar Vaga..." -ForegroundColor Yellow

    $updateBody = @{
        restauranteId = $RESTAURANTE_ID
        titulo = "Garçom para evento - URGENTE"
        descricao = "Vaga atualizada - precisa-se com urgência"
        categoria = "garcom"
        precoOferecido = 180.00
        dataTrabalho = "2025-11-15T19:00:00"
        vagasDisponiveis = 5
    } | ConvertTo-Json

    try {
        $response = Invoke-RestMethod -Uri "$BASE_URL/api/vagas/$VAGA_ID" `
            -Method PUT `
            -Body $updateBody `
            -ContentType "application/json"
        
        Write-Host "✅ Vaga atualizada!" -ForegroundColor Green
        Write-Host ""
    } catch {
        Write-Host "❌ Erro ao atualizar vaga" -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        Write-Host ""
    }
}

# ==========================================
# 8. FECHAR VAGA MANUALMENTE
# ==========================================
if ($VAGA_ID) {
    Write-Host "[8] Fechar Vaga Manualmente..." -ForegroundColor Yellow

    try {
        $response = Invoke-RestMethod -Uri "$BASE_URL/api/vagas/$VAGA_ID/close" `
            -Method POST `
            -ContentType "application/json"
        
        Write-Host "✅ Vaga fechada!" -ForegroundColor Green
        Write-Host ""
    } catch {
        Write-Host "❌ Erro ao fechar vaga" -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        Write-Host ""
    }
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " TESTES CONCLUÍDOS" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

