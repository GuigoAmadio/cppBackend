# ==========================================
# TESTE MÓDULO: RESTAURANTES
# ==========================================

$BASE_URL = "http://localhost:8080"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " TESTANDO MÓDULO RESTAURANTES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# ==========================================
# 1. CRIAR RESTAURANTE
# ==========================================
Write-Host "[1] Criar Restaurante..." -ForegroundColor Yellow

$createRestauranteBody = @{
    userId = "user-test-001"
    cnpj = "12345678000190"
    razaoSocial = "Restaurante Teste Ltda"
    nomeFantasia = "Restaurante Teste"
    descricao = "Melhor comida da cidade!"
    categoria = "restaurante"
    latitude = -23.5505
    longitude = -46.6333
    enderecoCompleto = "Rua Exemplo, 123"
    cep = "01234-567"
    cidade = "São Paulo"
    estado = "SP"
    horarioFuncionamento = '{"seg": "08:00-18:00", "ter": "08:00-18:00"}'
} | ConvertTo-Json

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/restaurantes" `
        -Method POST `
        -Body $createRestauranteBody `
        -ContentType "application/json"
    
    Write-Host "✅ Restaurante criado com sucesso!" -ForegroundColor Green
    Write-Host "ID: $($response.id)" -ForegroundColor Cyan
    $RESTAURANTE_ID = $response.id
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao criar restaurante" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 2. LISTAR TODOS RESTAURANTES
# ==========================================
Write-Host "[2] Listar Todos Restaurantes..." -ForegroundColor Yellow

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/restaurantes" -Method GET
    
    Write-Host "✅ Restaurantes listados com sucesso!" -ForegroundColor Green
    Write-Host "Total: $($response.total)" -ForegroundColor Cyan
    foreach ($rest in $response.restaurantes) {
        Write-Host "  - [$($rest.id)] $($rest.nomeFantasia) - $($rest.cidade)" -ForegroundColor White
    }
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao listar restaurantes" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 3. BUSCAR RESTAURANTE POR ID
# ==========================================
if ($RESTAURANTE_ID) {
    Write-Host "[3] Buscar Restaurante por ID..." -ForegroundColor Yellow

    try {
        $response = Invoke-RestMethod -Uri "$BASE_URL/api/restaurantes/$RESTAURANTE_ID" -Method GET
        
        Write-Host "✅ Restaurante encontrado!" -ForegroundColor Green
        Write-Host "Nome: $($response.nomeFantasia)" -ForegroundColor Cyan
        Write-Host "Cidade: $($response.cidade)" -ForegroundColor Cyan
        Write-Host "Categoria: $($response.categoria)" -ForegroundColor Cyan
        Write-Host ""
    } catch {
        Write-Host "❌ Erro ao buscar restaurante" -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        Write-Host ""
    }
}

# ==========================================
# 4. FILTRAR POR CATEGORIA
# ==========================================
Write-Host "[4] Filtrar por Categoria (restaurante)..." -ForegroundColor Yellow

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/restaurantes?categoria=restaurante" -Method GET
    
    Write-Host "✅ Restaurantes filtrados!" -ForegroundColor Green
    Write-Host "Total: $($response.total)" -ForegroundColor Cyan
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao filtrar restaurantes" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 5. BUSCAR RESTAURANTES PRÓXIMOS
# ==========================================
Write-Host "[5] Buscar Restaurantes Próximos..." -ForegroundColor Yellow

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/restaurantes/nearby?lat=-23.5505&lng=-46.6333&radius=10" -Method GET
    
    Write-Host "✅ Restaurantes próximos encontrados!" -ForegroundColor Green
    Write-Host "Total: $($response.total)" -ForegroundColor Cyan
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao buscar restaurantes próximos" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 6. BUSCAR PARA MAPA
# ==========================================
Write-Host "[6] Buscar Restaurantes para Mapa..." -ForegroundColor Yellow

try {
    $response = Invoke-RestMethod -Uri "$BASE_URL/api/restaurantes/map" -Method GET
    
    Write-Host "✅ Markers obtidos!" -ForegroundColor Green
    Write-Host "Total de markers: $($response.total)" -ForegroundColor Cyan
    foreach ($marker in $response.markers) {
        Write-Host "  - $($marker.nome) (Lat: $($marker.latitude), Lng: $($marker.longitude))" -ForegroundColor White
    }
    Write-Host ""
} catch {
    Write-Host "❌ Erro ao buscar markers" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
}

# ==========================================
# 7. ATUALIZAR RESTAURANTE
# ==========================================
if ($RESTAURANTE_ID) {
    Write-Host "[7] Atualizar Restaurante..." -ForegroundColor Yellow

    $updateBody = @{
        userId = "user-test-001"
        razaoSocial = "Restaurante Teste ATUALIZADO"
        nomeFantasia = "Restaurante Teste NOVO NOME"
        descricao = "Descrição atualizada!"
        categoria = "bar"
        latitude = -23.5505
        longitude = -46.6333
    } | ConvertTo-Json

    try {
        $response = Invoke-RestMethod -Uri "$BASE_URL/api/restaurantes/$RESTAURANTE_ID" `
            -Method PUT `
            -Body $updateBody `
            -ContentType "application/json"
        
        Write-Host "✅ Restaurante atualizado!" -ForegroundColor Green
        Write-Host ""
    } catch {
        Write-Host "❌ Erro ao atualizar restaurante" -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        Write-Host ""
    }
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " TESTES CONCLUÍDOS" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

