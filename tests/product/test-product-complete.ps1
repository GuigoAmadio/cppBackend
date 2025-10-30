# ==============================================================================
# TESTE COMPLETO - PRODUCT MODULE
# ==============================================================================

$baseUrl = "http://localhost:8080"
$testsPassed = 0
$testsFailed = 0

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host " TESTE COMPLETO - PRODUCT MODULE" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Helper function
function Test-Result {
    param(
        [string]$TestName,
        [bool]$Success,
        [string]$Details = ""
    )
    
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
    # ==============================================================================
    # STEP 1: LOGIN
    # ==============================================================================
    
    Write-Host "`n[1] LOGIN..." -ForegroundColor Yellow
    
    $loginBody = @{
        email = "admin@acme.com"
        password = "Admin123!"
        tenant_subdomain = "acme"
    } | ConvertTo-Json
    
    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" `
        -Method Post `
        -Body $loginBody `
        -ContentType "application/json" `
        -ErrorAction Stop
    
    $token = $loginResponse.token
    $headers = @{ "Authorization" = "Bearer $token" }
    
    Test-Result "Login" ($token -ne $null) "Token obtido"
    
    # ==============================================================================
    # STEP 2: CRIAR PRODUTO
    # ==============================================================================
    
    Write-Host "`n[2] CRIAR PRODUTO..." -ForegroundColor Yellow
    
    $createBody = @{
        code = "PROD-TEST-001"
        name = "Produto de Teste"
        description = "Descrição do produto de teste"
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
        -ContentType "application/json" `
        -ErrorAction Stop
    
    $productId = $createResponse.product_id
    Test-Result "Criar produto" ($productId -ne $null) "ID: $productId"
    
    # ==============================================================================
    # STEP 3: BUSCAR PRODUTO POR ID
    # ==============================================================================
    
    Write-Host "`n[3] BUSCAR PRODUTO POR ID..." -ForegroundColor Yellow
    
    $getResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Get `
        -Headers $headers `
        -ErrorAction Stop
    
    $product = $getResponse.product
    Test-Result "Buscar produto" ($product.id -eq $productId) "Nome: $($product.name)"
    Test-Result "Validar preço" ($product.price -eq 99.90) "Preço: R$ $($product.price)"
    Test-Result "Validar estoque" ($product.stock_quantity -eq 100) "Estoque: $($product.stock_quantity)"
    Test-Result "Validar disponibilidade" ($product.is_available -eq $true) "Disponível: sim"
    
    # ==============================================================================
    # STEP 4: LISTAR PRODUTOS
    # ==============================================================================
    
    Write-Host "`n[4] LISTAR PRODUTOS..." -ForegroundColor Yellow
    
    $listResponse = Invoke-RestMethod -Uri "$baseUrl/api/products" `
        -Method Get `
        -Headers $headers `
        -ErrorAction Stop
    
    $productsCount = $listResponse.products.Count
    Test-Result "Listar produtos" ($productsCount -gt 0) "Total: $productsCount produtos"
    
    # ==============================================================================
    # STEP 5: BUSCAR PRODUTOS (SEARCH)
    # ==============================================================================
    
    Write-Host "`n[5] BUSCAR PRODUTOS (SEARCH)..." -ForegroundColor Yellow
    
    $searchResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/search?q=Teste" `
        -Method Get `
        -Headers $headers `
        -ErrorAction Stop
    
    $searchCount = $searchResponse.products.Count
    Test-Result "Buscar produtos" ($searchCount -gt 0) "Encontrados: $searchCount produtos"
    
    # ==============================================================================
    # STEP 6: ATUALIZAR PRODUTO
    # ==============================================================================
    
    Write-Host "`n[6] ATUALIZAR PRODUTO..." -ForegroundColor Yellow
    
    $updateBody = @{
        name = "Produto de Teste ATUALIZADO"
        description = "Nova descrição"
        price = 149.90
        stock_quantity = 50
    } | ConvertTo-Json
    
    $updateResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Put `
        -Headers $headers `
        -Body $updateBody `
        -ContentType "application/json" `
        -ErrorAction Stop
    
    Test-Result "Atualizar produto" ($updateResponse.success -eq $true) $updateResponse.message
    
    # ==============================================================================
    # STEP 7: VERIFICAR ATUALIZAÇÃO
    # ==============================================================================
    
    Write-Host "`n[7] VERIFICAR ATUALIZAÇÃO..." -ForegroundColor Yellow
    
    $getUpdatedResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Get `
        -Headers $headers `
        -ErrorAction Stop
    
    $updatedProduct = $getUpdatedResponse.product
    Test-Result "Nome atualizado" ($updatedProduct.name -eq "Produto de Teste ATUALIZADO")
    Test-Result "Preço atualizado" ($updatedProduct.price -eq 149.90) "Novo preço: R$ $($updatedProduct.price)"
    Test-Result "Estoque atualizado" ($updatedProduct.stock_quantity -eq 50) "Novo estoque: $($updatedProduct.stock_quantity)"
    Test-Result "Estoque baixo" ($updatedProduct.is_low_stock -eq $true) "Estoque baixo detectado"
    
    # ==============================================================================
    # STEP 8: LISTAR PRODUTOS COM ESTOQUE BAIXO
    # ==============================================================================
    
    Write-Host "`n[8] LISTAR PRODUTOS COM ESTOQUE BAIXO..." -ForegroundColor Yellow
    
    $lowStockResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/low-stock" `
        -Method Get `
        -Headers $headers `
        -ErrorAction Stop
    
    $lowStockCount = $lowStockResponse.products.Count
    Test-Result "Listar estoque baixo" ($lowStockCount -gt 0) "Produtos com estoque baixo: $lowStockCount"
    
    # ==============================================================================
    # STEP 9: ARQUIVAR PRODUTO (SOFT DELETE)
    # ==============================================================================
    
    Write-Host "`n[9] ARQUIVAR PRODUTO (SOFT DELETE)..." -ForegroundColor Yellow
    
    $deleteResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Delete `
        -Headers $headers `
        -ErrorAction Stop
    
    Test-Result "Arquivar produto" ($deleteResponse.success -eq $true) $deleteResponse.message
    
    # ==============================================================================
    # STEP 10: VERIFICAR PRODUTO ARQUIVADO
    # ==============================================================================
    
    Write-Host "`n[10] VERIFICAR PRODUTO ARQUIVADO..." -ForegroundColor Yellow
    
    $getArchivedResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Get `
        -Headers $headers `
        -ErrorAction Stop
    
    $archivedProduct = $getArchivedResponse.product
    Test-Result "Status arquivado" ($archivedProduct.status -eq "archived") "Status: $($archivedProduct.status)"
    Test-Result "Não disponível" ($archivedProduct.is_available -eq $false) "Disponibilidade: não"
    
    # ==============================================================================
    # STEP 11: DELETE FÍSICO
    # ==============================================================================
    
    Write-Host "`n[11] DELETE FÍSICO..." -ForegroundColor Yellow
    
    $hardDeleteResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId`?hard=true" `
        -Method Delete `
        -Headers $headers `
        -ErrorAction Stop
    
    Test-Result "Delete físico" ($hardDeleteResponse.success -eq $true) $hardDeleteResponse.message
    
    # ==============================================================================
    # STEP 12: VERIFICAR PRODUTO DELETADO
    # ==============================================================================
    
    Write-Host "`n[12] VERIFICAR PRODUTO DELETADO..." -ForegroundColor Yellow
    
    try {
        $getDeletedResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
            -Method Get `
            -Headers $headers `
            -ErrorAction Stop
        
        Test-Result "Produto deve estar deletado" $false "Produto ainda existe!"
    } catch {
        Test-Result "Produto deletado corretamente" $true "404 Not Found"
    }

} catch {
    Write-Host "`n[✗] ERRO NO TESTE: $_" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Yellow
    $script:testsFailed++
}

# ==============================================================================
# SUMMARY
# ==============================================================================

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host " RESUMO DOS TESTES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "✓ Testes passou: $testsPassed" -ForegroundColor Green
Write-Host "✗ Testes falhou: $testsFailed" -ForegroundColor $(if ($testsFailed -eq 0) { "Green" } else { "Red" })
Write-Host "========================================`n" -ForegroundColor Cyan

if ($testsFailed -eq 0) {
    Write-Host "🎉 TODOS OS TESTES PASSARAM!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "❌ ALGUNS TESTES FALHARAM" -ForegroundColor Red
    exit 1
}

