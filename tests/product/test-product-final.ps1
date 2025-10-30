# TESTE FINAL - PRODUCT MODULE
$baseUrl = "http://localhost:8080"
$testsPassed = 0
$testsFailed = 0

Write-Host "`n========================================"
Write-Host " TESTE PRODUCT MODULE - FINAL"
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
    # 1. REGISTRAR NOVO USUARIO DE TESTE
    Write-Host "`n[1] REGISTRAR USUARIO DE TESTE..." -ForegroundColor Yellow
    
    $registerBody = @{
        email = "producttest@acme.com"
        password = "Test123!"
        name = "Product Test User"
    } | ConvertTo-Json
    
    try {
        $registerResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/register" `
            -Method Post `
            -Body $registerBody `
            -ContentType "application/json" `
            -ErrorAction Stop
        
        Test-Result "Registrar usuario" $true "Usuario criado: producttest@acme.com"
    }
    catch {
        # Usuario pode ja existir
        Write-Host "    Usuario ja existe ou erro no registro (continuando...)" -ForegroundColor Yellow
    }
    
    # Adicionar usuario ao tenant acme
    Write-Host "`n[2] ADICIONAR USUARIO AO TENANT..." -ForegroundColor Yellow
    
    # Primeiro, fazer login como admin@acme.com para adicionar o usuario
    $adminLoginBody = @{
        email = "admin@acme.com"
        password = "Admin123!"
        tenant_subdomain = "acme"
    } | ConvertTo-Json
    
    try {
        $adminLoginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" `
            -Method Post `
            -Body $adminLoginBody `
            -ContentType "application/json" `
            -ErrorAction Stop
        
        $adminToken = $adminLoginResponse.token
        $adminHeaders = @{ "Authorization" = "Bearer $adminToken" }
        
        # Buscar ID do usuario producttest@acme.com
        $usersResponse = Invoke-RestMethod -Uri "$baseUrl/api/users" `
            -Method Get `
            -Headers $adminHeaders `
            -ErrorAction Stop
        
        $testUser = $usersResponse.users | Where-Object { $_.email -eq "producttest@acme.com" } | Select-Object -First 1
        
        if ($testUser) {
            # Adicionar ao tenant
            $addUserBody = @{
                user_id = $testUser.id
                role = "admin"
            } | ConvertTo-Json
            
            try {
                Invoke-RestMethod -Uri "$baseUrl/api/admin/tenants/acme/users" `
                    -Method Post `
                    -Headers $adminHeaders `
                    -Body $addUserBody `
                    -ContentType "application/json" `
                    -ErrorAction Stop
                
                Test-Result "Adicionar usuario ao tenant" $true "Usuario adicionado ao tenant acme"
            }
            catch {
                Write-Host "    Usuario ja esta no tenant (continuando...)" -ForegroundColor Yellow
            }
        }
    }
    catch {
        Write-Host "    Erro ao adicionar usuario (tentando continuar...)" -ForegroundColor Yellow
    }
    
    # 3. LOGIN COM USUARIO DE TESTE
    Write-Host "`n[3] LOGIN..." -ForegroundColor Yellow
    
    $loginBody = @{
        email = "producttest@acme.com"
        password = "Test123!"
        tenant_subdomain = "acme"
    } | ConvertTo-Json
    
    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" `
        -Method Post `
        -Body $loginBody `
        -ContentType "application/json"
    
    $token = $loginResponse.token
    $headers = @{ "Authorization" = "Bearer $token" }
    
    Test-Result "Login" ($token -ne $null) "Token obtido"
    
    # 4. CRIAR PRODUTO
    Write-Host "`n[4] CRIAR PRODUTO..." -ForegroundColor Yellow
    
    $createBody = @{
        code = "PROD-FINAL-001"
        name = "Produto Teste Final"
        description = "Descricao do produto de teste"
        type = "physical"
        price = 199.90
        cost = 100.00
        currency = "BRL"
        track_inventory = $true
        stock_quantity = 50
        low_stock_threshold = 10
        sku = "SKU-FINAL-001"
        barcode = "1234567890123"
        weight = 2.5
        weight_unit = "kg"
    } | ConvertTo-Json
    
    $createResponse = Invoke-RestMethod -Uri "$baseUrl/api/products" `
        -Method Post `
        -Headers $headers `
        -Body $createBody `
        -ContentType "application/json"
    
    $productId = $createResponse.product_id
    Test-Result "Criar produto" ($productId -ne $null) "ID: $productId"
    
    # 5. BUSCAR PRODUTO
    Write-Host "`n[5] BUSCAR PRODUTO..." -ForegroundColor Yellow
    
    $getResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Get `
        -Headers $headers
    
    $product = $getResponse.product
    Test-Result "Buscar produto" ($product.id -eq $productId) "Nome: $($product.name)"
    Test-Result "Validar preco" ($product.price -eq 199.90) "Preco: R$ $($product.price)"
    Test-Result "Validar estoque" ($product.stock_quantity -eq 50) "Estoque: $($product.stock_quantity)"
    Test-Result "Validar disponibilidade" ($product.is_available -eq $true) "Disponivel"
    
    # 6. LISTAR PRODUTOS
    Write-Host "`n[6] LISTAR PRODUTOS..." -ForegroundColor Yellow
    
    $listResponse = Invoke-RestMethod -Uri "$baseUrl/api/products" `
        -Method Get `
        -Headers $headers
    
    $productsCount = $listResponse.products.Count
    Test-Result "Listar produtos" ($productsCount -gt 0) "Total: $productsCount produtos"
    
    # 7. BUSCAR PRODUTOS
    Write-Host "`n[7] BUSCAR PRODUTOS (SEARCH)..." -ForegroundColor Yellow
    
    $searchResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/search?q=Final" `
        -Method Get `
        -Headers $headers
    
    $searchCount = $searchResponse.products.Count
    Test-Result "Buscar produtos" ($searchCount -gt 0) "Encontrados: $searchCount"
    
    # 8. ATUALIZAR PRODUTO
    Write-Host "`n[8] ATUALIZAR PRODUTO..." -ForegroundColor Yellow
    
    $updateBody = @{
        name = "Produto ATUALIZADO Final"
        price = 249.90
        stock_quantity = 5
    } | ConvertTo-Json
    
    $updateResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Put `
        -Headers $headers `
        -Body $updateBody `
        -ContentType "application/json"
    
    Test-Result "Atualizar produto" ($updateResponse.success -eq $true) $updateResponse.message
    
    # 9. VERIFICAR ATUALIZACAO
    Write-Host "`n[9] VERIFICAR ATUALIZACAO..." -ForegroundColor Yellow
    
    $getUpdatedResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Get `
        -Headers $headers
    
    $updatedProduct = $getUpdatedResponse.product
    Test-Result "Nome atualizado" ($updatedProduct.name -eq "Produto ATUALIZADO Final")
    Test-Result "Preco atualizado" ($updatedProduct.price -eq 249.90)
    Test-Result "Estoque baixo detectado" ($updatedProduct.is_low_stock -eq $true)
    
    # 10. LISTAR ESTOQUE BAIXO
    Write-Host "`n[10] LISTAR PRODUTOS COM ESTOQUE BAIXO..." -ForegroundColor Yellow
    
    $lowStockResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/low-stock" `
        -Method Get `
        -Headers $headers
    
    $lowStockCount = $lowStockResponse.products.Count
    Test-Result "Listar estoque baixo" ($lowStockCount -gt 0) "Produtos: $lowStockCount"
    
    # 11. ARQUIVAR PRODUTO
    Write-Host "`n[11] ARQUIVAR PRODUTO (SOFT DELETE)..." -ForegroundColor Yellow
    
    $deleteResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Delete `
        -Headers $headers
    
    Test-Result "Arquivar produto" ($deleteResponse.success -eq $true) $deleteResponse.message
    
    # 12. VERIFICAR PRODUTO ARQUIVADO
    Write-Host "`n[12] VERIFICAR STATUS ARQUIVADO..." -ForegroundColor Yellow
    
    $getArchivedResponse = Invoke-RestMethod -Uri "$baseUrl/api/products/$productId" `
        -Method Get `
        -Headers $headers
    
    $archivedProduct = $getArchivedResponse.product
    Test-Result "Status arquivado" ($archivedProduct.status -eq "archived")
    Test-Result "Nao disponivel" ($archivedProduct.is_available -eq $false)

} catch {
    Write-Host "`n[ERRO] $($_.Exception.Message)" -ForegroundColor Red
    if ($_.ErrorDetails) {
        Write-Host "Detalhes: $($_.ErrorDetails.Message)" -ForegroundColor Yellow
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
    Write-Host "TODOS OS TESTES PASSARAM!" -ForegroundColor Green
    Write-Host "MODULO PRODUCT 100% FUNCIONAL!" -ForegroundColor Cyan
    exit 0
} else {
    Write-Host "ALGUNS TESTES FALHARAM" -ForegroundColor Red
    exit 1
}

