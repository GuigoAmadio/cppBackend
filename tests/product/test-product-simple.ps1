# TESTE SIMPLES - PRODUCT MODULE
$baseUrl = "http://localhost:8080"
$testsPassed = 0
$testsFailed = 0

Write-Host "`n========================================"
Write-Host " TESTE PRODUCT MODULE"
Write-Host "========================================`n"

function Test-Result($TestName, $Success, $Details = "") {
    if ($Success) {
        Write-Host "[OK] $TestName" -ForegroundColor Green
        if ($Details) { Write-Host "    $Details" -ForegroundColor Gray }
        $script:testsPassed++
    }
    else {
        Write-Host "[FAIL] $TestName" -ForegroundColor Red
        if ($Details) { Write-Host "    $Details" -ForegroundColor Yellow }
        $script:testsFailed++
    }
}

try {
    # 1. LOGIN
    Write-Host "`n[1] LOGIN..." -ForegroundColor Yellow
    
    $loginBody = @{
        email = "admin@acme.com"
        password = "Admin123!"
        tenant_subdomain = "acme"
    } | ConvertTo-Json
    
    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" `
        -Method Post `
        -Body $loginBody `
        -ContentType "application/json"
    
    $token = $loginResponse.token
    $headers = @{ "Authorization" = "Bearer $token" }
    
    Test-Result "Login" ($token -ne $null) "Token obtido"
    
    # 2. CRIAR PRODUTO
    Write-Host "`n[2] CRIAR PRODUTO..." -ForegroundColor Yellow
    
    $createBody = @{
        code = "PROD-TEST-001"
        name = "Produto de Teste"
        description = "Descricao do produto de teste"
        type = "physical"
        price = 99.90
        cost = 50.00
        currency = "BRL"
        track_inventory = $true
        stock_quantity = 100
        low_stock_threshold = 10
        sku = "SKU-TEST-001"
        barcode = "7891234567890"
        weight = 1.5
        weight_unit = "kg"
    } | ConvertTo-Json
    
    $createResponse = Invoke-RestMethod -Uri "$baseUrl/api/products" `
        -Method Post `
        -Headers $headers `
        -Body $createBody `
        -ContentType "application/json"
    
    $productId = $createResponse.product_id
    Test-Result "Criar produto" ($productId -ne $null) "ID: $productId"
    
    # 3. BUSCAR PRODUTO
    Write-Host "`n[3] BUSCAR PRODUTO..." -ForegroundColor Yellow
    
    $getResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Get `
        -Headers $headers
    
    $product = $getResponse.product
    Test-Result "Buscar produto" ($product.id -eq $productId) "Nome: $($product.name)"
    Test-Result "Validar preco" ($product.price -eq 99.90) "Preco: R$ $($product.price)"
    Test-Result "Validar estoque" ($product.stock_quantity -eq 100) "Estoque: $($product.stock_quantity)"
    
    # 4. LISTAR PRODUTOS
    Write-Host "`n[4] LISTAR PRODUTOS..." -ForegroundColor Yellow
    
    $listResponse = Invoke-RestMethod -Uri "$baseUrl/api/products" `
        -Method Get `
        -Headers $headers
    
    $productsCount = $listResponse.products.Count
    Test-Result "Listar produtos" ($productsCount -gt 0) "Total: $productsCount produtos"
    
    # 5. ATUALIZAR PRODUTO
    Write-Host "`n[5] ATUALIZAR PRODUTO..." -ForegroundColor Yellow
    
    $updateBody = @{
        name = "Produto ATUALIZADO"
        price = 149.90
        stock_quantity = 5
    } | ConvertTo-Json
    
    $updateResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Put `
        -Headers $headers `
        -Body $updateBody `
        -ContentType "application/json"
    
    Test-Result "Atualizar produto" ($updateResponse.success -eq $true) $updateResponse.message
    
    # 6. VERIFICAR ATUALIZACAO
    Write-Host "`n[6] VERIFICAR ATUALIZACAO..." -ForegroundColor Yellow
    
    $getUpdatedResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Get `
        -Headers $headers
    
    $updatedProduct = $getUpdatedResponse.product
    Test-Result "Nome atualizado" ($updatedProduct.name -eq "Produto ATUALIZADO")
    Test-Result "Preco atualizado" ($updatedProduct.price -eq 149.90)
    Test-Result "Estoque baixo detectado" ($updatedProduct.is_low_stock -eq $true)
    
    # 7. DELETAR PRODUTO
    Write-Host "`n[7] DELETAR PRODUTO..." -ForegroundColor Yellow
    
    $deleteResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Delete `
        -Headers $headers
    
    Test-Result "Deletar produto" ($deleteResponse.success -eq $true) $deleteResponse.message

} catch {
    Write-Host "`n[ERRO] $($_.Exception.Message)" -ForegroundColor Red
    $script:testsFailed++
}

# RESUMO
Write-Host "`n========================================"
Write-Host " RESUMO"
Write-Host "========================================"
Write-Host "Passou: $testsPassed" -ForegroundColor Green
Write-Host "Falhou: $testsFailed" -ForegroundColor $(if ($testsFailed -eq 0) { "Green" } else { "Red" })
Write-Host "========================================`n"

if ($testsFailed -eq 0) {
    Write-Host "TODOS OS TESTES PASSARAM!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "ALGUNS TESTES FALHARAM" -ForegroundColor Red
    exit 1
}

