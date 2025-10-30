# ==============================================================================
# TESTE COMPLETO - MÓDULO INVENTORY
# ==============================================================================

$baseUrl = "http://localhost:8080"
$passedTests = 0
$failedTests = 0

Write-Host "`n========================================"
Write-Host " TESTE INVENTORY MODULE"
Write-Host "========================================`n"

# ==============================================================================
# 1. LOGIN
# ==============================================================================
Write-Host "[1] LOGIN..." -ForegroundColor Cyan

$loginBody = @{
    email = "testproduct@acme.com"
    password = "Product123!"
    tenant_subdomain = "acme"
} | ConvertTo-Json

try {
    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method Post -Body $loginBody -ContentType "application/json"
    $token = $loginResponse.token
    $headers = @{ "Authorization" = "Bearer $token" }
    Write-Host "[OK] Login" -ForegroundColor Green
    $passedTests++
} catch {
    Write-Host "[ERRO] Login falhou" -ForegroundColor Red
    Write-Host $_.Exception.Message
    exit 1
}

# ==============================================================================
# 2. CRIAR PRODUTO PARA TESTE
# ==============================================================================
Write-Host "`n[2] CRIAR PRODUTO PARA TESTE..." -ForegroundColor Cyan

$productBody = @{
    code = "INV-TEST-001"
    name = "Produto para Teste de Inventario"
    type = "physical"
    price = 100
    track_inventory = $true
    stock_quantity = 0
} | ConvertTo-Json

try {
    $productResponse = Invoke-RestMethod -Uri "$baseUrl/api/products" -Method Post -Body $productBody -Headers $headers -ContentType "application/json"
    $productId = $productResponse.product_id
    Write-Host "[OK] Produto criado: $productId" -ForegroundColor Green
    $passedTests++
} catch {
    Write-Host "[ERRO] Falhou ao criar produto" -ForegroundColor Red
    Write-Host $_.Exception.Message
    $failedTests++
}

# ==============================================================================
# 3. REGISTRAR ENTRADA (IN)
# ==============================================================================
Write-Host "`n[3] REGISTRAR ENTRADA (IN) - 50 unidades..." -ForegroundColor Cyan

$inBody = @{
    product_id = $productId
    type = "in"
    quantity = 50
    reference_type = "purchase"
    notes = "Compra inicial"
} | ConvertTo-Json

try {
    $inResponse = Invoke-RestMethod -Uri "$baseUrl/api/inventory/transactions" -Method Post -Body $inBody -Headers $headers -ContentType "application/json"
    
    if ($inResponse.success -and $inResponse.new_stock -eq 50) {
        Write-Host "[OK] Entrada registrada - Novo estoque: $($inResponse.new_stock)" -ForegroundColor Green
        $passedTests++
    } else {
        Write-Host "[FAIL] Estoque incorreto" -ForegroundColor Yellow
        $failedTests++
    }
} catch {
    Write-Host "[ERRO] Falhou ao registrar entrada" -ForegroundColor Red
    Write-Host $_.Exception.Message
    $failedTests++
}

# ==============================================================================
# 4. REGISTRAR SAÍDA (OUT)
# ==============================================================================
Write-Host "`n[4] REGISTRAR SAÍDA (OUT) - 15 unidades..." -ForegroundColor Cyan

$outBody = @{
    product_id = $productId
    type = "out"
    quantity = 15
    reference_type = "order"
    notes = "Venda"
} | ConvertTo-Json

try {
    $outResponse = Invoke-RestMethod -Uri "$baseUrl/api/inventory/transactions" -Method Post -Body $outBody -Headers $headers -ContentType "application/json"
    
    if ($outResponse.success -and $outResponse.new_stock -eq 35) {
        Write-Host "[OK] Saída registrada - Novo estoque: $($outResponse.new_stock)" -ForegroundColor Green
        $passedTests++
    } else {
        Write-Host "[FAIL] Estoque incorreto - esperado 35, recebido $($outResponse.new_stock)" -ForegroundColor Yellow
        $failedTests++
    }
} catch {
    Write-Host "[ERRO] Falhou ao registrar saída" -ForegroundColor Red
    Write-Host $_.Exception.Message
    $failedTests++
}

# ==============================================================================
# 5. AJUSTE DE ESTOQUE
# ==============================================================================
Write-Host "`n[5] AJUSTE DE ESTOQUE - para 100 unidades..." -ForegroundColor Cyan

$adjustBody = @{
    product_id = $productId
    type = "adjustment"
    quantity = 100
    reference_type = "adjustment"
    notes = "Correcao de inventario"
} | ConvertTo-Json

try {
    $adjustResponse = Invoke-RestMethod -Uri "$baseUrl/api/inventory/transactions" -Method Post -Body $adjustBody -Headers $headers -ContentType "application/json"
    
    if ($adjustResponse.success -and $adjustResponse.new_stock -eq 100) {
        Write-Host "[OK] Ajuste registrado - Novo estoque: $($adjustResponse.new_stock)" -ForegroundColor Green
        $passedTests++
    } else {
        Write-Host "[FAIL] Estoque incorreto" -ForegroundColor Yellow
        $failedTests++
    }
} catch {
    Write-Host "[ERRO] Falhou ao ajustar estoque" -ForegroundColor Red
    Write-Host $_.Exception.Message
    $failedTests++
}

# ==============================================================================
# 6. CONSULTAR ESTOQUE ATUAL
# ==============================================================================
Write-Host "`n[6] CONSULTAR ESTOQUE ATUAL..." -ForegroundColor Cyan

try {
    $stockResponse = Invoke-RestMethod -Uri "$baseUrl/api/inventory/stock/$productId" -Method Get -Headers $headers
    
    if ($stockResponse.stock -eq 100) {
        Write-Host "[OK] Estoque atual: $($stockResponse.stock)" -ForegroundColor Green
        $passedTests++
    } else {
        Write-Host "[FAIL] Estoque incorreto - esperado 100, recebido $($stockResponse.stock)" -ForegroundColor Yellow
        $failedTests++
    }
} catch {
    Write-Host "[ERRO] Falhou ao consultar estoque" -ForegroundColor Red
    Write-Host $_.Exception.Message
    $failedTests++
}

# ==============================================================================
# 7. CONSULTAR HISTORICO
# ==============================================================================
Write-Host "`n[7] CONSULTAR HISTORICO..." -ForegroundColor Cyan

try {
    $historyResponse = Invoke-RestMethod -Uri "$baseUrl/api/inventory/history?product_id=$productId" -Method Get -Headers $headers
    
    if ($historyResponse.total -eq 3) {
        Write-Host "[OK] Historico: $($historyResponse.total) transacoes" -ForegroundColor Green
        $passedTests++
    } else {
        Write-Host "[FAIL] Total incorreto - esperado 3, recebido $($historyResponse.total)" -ForegroundColor Yellow
        $failedTests++
    }
} catch {
    Write-Host "[ERRO] Falhou ao consultar historico" -ForegroundColor Red
    Write-Host $_.Exception.Message
    $failedTests++
}

# ==============================================================================
# 8. TENTAR SAÍDA SEM ESTOQUE SUFICIENTE
# ==============================================================================
Write-Host "`n[8] TENTAR SAÍDA SEM ESTOQUE..." -ForegroundColor Cyan

$invalidOutBody = @{
    product_id = $productId
    type = "out"
    quantity = 150
    reference_type = "order"
} | ConvertTo-Json

try {
    $invalidResponse = Invoke-RestMethod -Uri "$baseUrl/api/inventory/transactions" -Method Post -Body $invalidOutBody -Headers $headers -ContentType "application/json" -ErrorAction Stop
    Write-Host "[FAIL] Deveria ter falhado!" -ForegroundColor Yellow
    $failedTests++
} catch {
    if ($_.Exception.Response.StatusCode -eq 400) {
        Write-Host "[OK] Bloqueou estoque insuficiente" -ForegroundColor Green
        $passedTests++
    } else {
        Write-Host "[FAIL] Erro inesperado" -ForegroundColor Yellow
        $failedTests++
    }
}

# ==============================================================================
# RESUMO
# ==============================================================================
Write-Host "`n========================================"
Write-Host " RESUMO"
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Passou: $passedTests" -ForegroundColor Green
Write-Host "Falhou: $failedTests" -ForegroundColor $(if ($failedTests -eq 0) { "Green" } else { "Red" })
Write-Host "========================================`n"

if ($failedTests -eq 0) {
    Write-Host "MODULO INVENTORY 100 PORCENTO FUNCIONAL!" -ForegroundColor Green
}

