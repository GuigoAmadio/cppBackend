# TESTE PRODUCT - COM USUARIO FUNCIONAL
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
    # LOGIN
    Write-Host "[1] LOGIN..." -ForegroundColor Yellow
    
    $loginBody = @{
        email = "testproduct@acme.com"
        password = "Product123!"
        tenant_subdomain = "acme"
    } | ConvertTo-Json
    
    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" `
        -Method Post `
        -Body $loginBody `
        -ContentType "application/json"
    
    $token = $loginResponse.token
    $headers = @{ "Authorization" = "Bearer $token" }
    
    Test-Result "Login" ($token -ne $null)
    
    # CRIAR PRODUTO
    Write-Host "`n[2] CRIAR PRODUTO..." -ForegroundColor Yellow
    
    $createBody = @{
        code = "TEST-PROD-001"
        name = "Notebook Dell"
        description = "Notebook Dell Inspiron 15"
        type = "physical"
        price = 3500.00
        cost = 2800.00
        currency = "BRL"
        track_inventory = $true
        stock_quantity = 25
        low_stock_threshold = 5
        sku = "DELL-INSP-15-001"
        barcode = "7899123456789"
        weight = 1.8
        weight_unit = "kg"
    } | ConvertTo-Json
    
    $createResponse = Invoke-RestMethod -Uri "$baseUrl/api/products" `
        -Method Post `
        -Headers $headers `
        -Body $createBody `
        -ContentType "application/json"
    
    $productId = $createResponse.product_id
    Test-Result "Criar produto" ($productId -ne $null) "ID: $productId"
    
    # BUSCAR PRODUTO
    Write-Host "`n[3] BUSCAR PRODUTO POR ID..." -ForegroundColor Yellow
    
    $getResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Get `
        -Headers $headers
    
    $product = $getResponse.product
    Test-Result "Buscar por ID" ($product.id -eq $productId)
    Test-Result "Nome correto" ($product.name -eq "Notebook Dell")
    Test-Result "Preco correto" ($product.price -eq 3500.00)
    Test-Result "Estoque correto" ($product.stock_quantity -eq 25)
    Test-Result "Disponivel" ($product.is_available -eq $true)
    Test-Result "Nao esta em baixo estoque" ($product.is_low_stock -eq $false)
    
    # LISTAR PRODUTOS
    Write-Host "`n[4] LISTAR TODOS OS PRODUTOS..." -ForegroundColor Yellow
    
    $listResponse = Invoke-RestMethod -Uri "$baseUrl/api/products" `
        -Method Get `
        -Headers $headers
    
    Test-Result "Listar produtos" ($listResponse.products.Count -gt 0) "Total: $($listResponse.products.Count)"
    
    # BUSCAR (SEARCH)
    Write-Host "`n[5] BUSCAR PRODUTOS..." -ForegroundColor Yellow
    
    $searchResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/search?q=Dell" `
        -Method Get `
        -Headers $headers
    
    Test-Result "Search por 'Dell'" ($searchResponse.products.Count -gt 0)
    
    # ATUALIZAR
    Write-Host "`n[6] ATUALIZAR PRODUTO..." -ForegroundColor Yellow
    
    $updateBody = @{
        name = "Notebook Dell ATUALIZADO"
        price = 3299.00
        stock_quantity = 3
    } | ConvertTo-Json
    
    $updateResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Put `
        -Headers $headers `
        -Body $updateBody `
        -ContentType "application/json"
    
    Test-Result "Atualizar" ($updateResponse.success -eq $true)
    
    # VERIFICAR ATUALIZACAO
    Write-Host "`n[7] VERIFICAR ATUALIZACAO..." -ForegroundColor Yellow
    
    $updatedProduct = (Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" -Method Get -Headers $headers).product
    
    Test-Result "Nome atualizado" ($updatedProduct.name -eq "Notebook Dell ATUALIZADO")
    Test-Result "Preco atualizado" ($updatedProduct.price -eq 3299.00)
    Test-Result "Estoque atualizado" ($updatedProduct.stock_quantity -eq 3)
    Test-Result "Baixo estoque detectado" ($updatedProduct.is_low_stock -eq $true)
    
    # LISTAR BAIXO ESTOQUE
    Write-Host "`n[8] LISTAR PRODUTOS COM BAIXO ESTOQUE..." -ForegroundColor Yellow
    
    $lowStockResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/low-stock" `
        -Method Get `
        -Headers $headers
    
    Test-Result "Produtos em baixo estoque" ($lowStockResponse.products.Count -gt 0) "Total: $($lowStockResponse.products.Count)"
    
    # DELETAR (SOFT)
    Write-Host "`n[9] ARQUIVAR PRODUTO (SOFT DELETE)..." -ForegroundColor Yellow
    
    $deleteResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Delete `
        -Headers $headers
    
    Test-Result "Soft delete" ($deleteResponse.success -eq $true)
    
    # VERIFICAR ARQUIVADO
    Write-Host "`n[10] VERIFICAR PRODUTO ARQUIVADO..." -ForegroundColor Yellow
    
    $archivedProduct = (Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" -Method Get -Headers $headers).product
    
    Test-Result "Status = archived" ($archivedProduct.status -eq "archived")
    Test-Result "Nao disponivel" ($archivedProduct.is_available -eq $false)

} catch {
    Write-Host "`n[ERRO] $($_.Exception.Message)" -ForegroundColor Red
    if ($_.ErrorDetails) {
        Write-Host "$($_.ErrorDetails.Message)" -ForegroundColor Yellow
    }
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
    Write-Host "MODULO PRODUCT 100% FUNCIONAL!" -ForegroundColor Green
    exit 0
} else {
    exit 1
}

