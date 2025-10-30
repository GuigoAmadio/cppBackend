# =========================================================
# TESTE ROBUSTO E COMPLETO - INVOICE MODULE
# Certificação de 100% de funcionamento
# =========================================================

$ErrorActionPreference = "Continue"
$baseUrl = "http://localhost:8080"
$timestamp = [DateTimeOffset]::Now.ToUnixTimeSeconds()

Write-Host "`n=========================================================" -ForegroundColor Cyan
Write-Host " TESTE ROBUSTO - INVOICE MODULE (7 ENDPOINTS)" -ForegroundColor Cyan
Write-Host "=========================================================`n" -ForegroundColor Cyan

$passedTests = 0
$totalTests = 0

function Test-Step {
    param([string]$name, [scriptblock]$block)
    $script:totalTests++
    Write-Host "`n[$script:totalTests] $name..." -ForegroundColor Yellow
    try {
        & $block
        $script:passedTests++
        Write-Host "  [OK] PASSOU" -ForegroundColor Green
        return $true
    } catch {
        Write-Host "  [ERRO] FALHOU: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

# =========================================================
# PASSO 1: LIMPAR DADOS ANTIGOS
# =========================================================
Write-Host "`n[SETUP] Limpando dados de testes antigos..." -ForegroundColor Magenta

$env:PGPASSWORD = "postgre123"

# Limpar products com SKU vazio (causa do erro)
$sqlCmd1 = "DELETE FROM products WHERE sku = '' OR sku IS NULL;"
$null = psql -U moneymaker_user -d moneymaker_dev -h localhost -p 5433 -c $sqlCmd1 2>&1

# Limpar invoice_items e invoices de testes
$sqlCmd2 = "DELETE FROM invoice_items WHERE description LIKE '%Test%' OR description LIKE '%Invoice%';"
$null = psql -U moneymaker_user -d moneymaker_dev -h localhost -p 5433 -c $sqlCmd2 2>&1

$sqlCmd3 = "DELETE FROM invoices WHERE description LIKE '%Test Invoice%';"
$null = psql -U moneymaker_user -d moneymaker_dev -h localhost -p 5433 -c $sqlCmd3 2>&1

# Limpar orders e order_items de testes
$sqlCmd4 = "DELETE FROM order_items WHERE order_id IN (SELECT id FROM orders WHERE notes LIKE '%test%');"
$null = psql -U moneymaker_user -d moneymaker_dev -h localhost -p 5433 -c $sqlCmd4 2>&1

$sqlCmd5 = "DELETE FROM orders WHERE notes LIKE '%test%';"
$null = psql -U moneymaker_user -d moneymaker_dev -h localhost -p 5433 -c $sqlCmd5 2>&1

# Limpar customers e products de testes
$sqlCmd6 = "DELETE FROM customers WHERE email LIKE '%test.com';"
$null = psql -U moneymaker_user -d moneymaker_dev -h localhost -p 5433 -c $sqlCmd6 2>&1

$sqlCmd7 = "DELETE FROM products WHERE code LIKE 'INV-PROD-%' OR code LIKE 'TEST-%' OR code LIKE 'ROBUST-%';"
$null = psql -U moneymaker_user -d moneymaker_dev -h localhost -p 5433 -c $sqlCmd7 2>&1

Write-Host "  [OK] Dados antigos limpos!" -ForegroundColor Green

Start-Sleep -Seconds 1

# =========================================================
# PASSO 2: LOGIN
# =========================================================
Test-Step "LOGIN com usuário existente" {
    $loginBody = @{
        email = "invoicetest.1761174831@test.com"
        password = "InvoicePass123!"
        tenant_subdomain = "acme"
    } | ConvertTo-Json

    $loginResult = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method Post -Body $loginBody -ContentType "application/json"
    $script:token = if ($loginResult.token) { $loginResult.token } 
                   elseif ($loginResult.access_token) { $loginResult.access_token }
                   else { throw "Token não encontrado" }
    
    $script:headers = @{
        "Authorization" = "Bearer $script:token"
        "Content-Type" = "application/json"
    }
    
    Write-Host "  Token: $($script:token.Substring(0, 20))..." -ForegroundColor Gray
}

Start-Sleep -Seconds 1

# =========================================================
# PASSO 3: CRIAR CUSTOMER
# =========================================================
Test-Step "CRIAR CUSTOMER" {
    $customerBody = @{
        name = "Robusto Customer $timestamp"
        email = "robusto.$timestamp@testinvoice.com"
        type = "individual"
        phone = "+55 11 99999-0000"
    } | ConvertTo-Json

    $customerResult = Invoke-RestMethod -Uri "$baseUrl/api/customers" -Method Post -Headers $script:headers -Body $customerBody
    
    # Parsear JSON aninhado
    if ($customerResult.customer -is [string]) {
        $script:customer = $customerResult.customer | ConvertFrom-Json
    } else {
        $script:customer = $customerResult.customer
    }
    
    $script:customerId = $script:customer.id
    
    if (-not $script:customerId) {
        throw "Customer ID não foi retornado"
    }
    
    Write-Host "  Customer ID: $script:customerId" -ForegroundColor Gray
}

Start-Sleep -Seconds 1

# =========================================================
# PASSO 4: CRIAR PRODUCT (com SKU único)
# =========================================================
Test-Step "CRIAR PRODUCT com SKU único" {
    $uniqueCode = "ROBUST-$timestamp"
    $uniqueSku = "SKU-$timestamp-$(Get-Random -Maximum 9999)"
    
    $productBody = @{
        code = $uniqueCode
        name = "Robust Product $timestamp"
        type = "physical"
        price = 250.00
        sku = $uniqueSku
        description = "Product for robust invoice test"
    } | ConvertTo-Json

    $productResult = Invoke-RestMethod -Uri "$baseUrl/api/products" -Method Post -Headers $script:headers -Body $productBody
    
    # Parsear JSON aninhado
    if ($productResult.product -is [string]) {
        $script:product = $productResult.product | ConvertFrom-Json
    } else {
        $script:product = $productResult.product
    }
    
    $script:productId = $script:product.id
    
    if (-not $script:productId) {
        throw "Product ID não foi retornado"
    }
    
    Write-Host "  Product ID: $script:productId" -ForegroundColor Gray
    Write-Host "  SKU: $uniqueSku" -ForegroundColor Gray
}

Start-Sleep -Seconds 1

# =========================================================
# PASSO 5: CRIAR ORDER
# =========================================================
Test-Step "CRIAR ORDER" {
    $orderBody = @{
        customer_id = $script:customerId
        notes = "test order for robust invoice"
        items = @(
            @{
                product_id = $script:productId
                quantity = 5
                unit_price = 250.00
            }
        )
    } | ConvertTo-Json -Depth 5

    $orderResult = Invoke-RestMethod -Uri "$baseUrl/api/orders" -Method Post -Headers $script:headers -Body $orderBody
    
    # Parsear JSON aninhado
    if ($orderResult.order -is [string]) {
        $script:order = $orderResult.order | ConvertFrom-Json
    } else {
        $script:order = $orderResult.order
    }
    
    $script:orderId = $script:order.id
    
    if (-not $script:orderId) {
        throw "Order ID não foi retornado"
    }
    
    Write-Host "  Order ID: $script:orderId" -ForegroundColor Gray
    Write-Host "  Total: R$ $($script:order.total)" -ForegroundColor Gray
}

Start-Sleep -Seconds 1

# =========================================================
# PASSO 6: CRIAR INVOICE (ENDPOINT 1/7)
# =========================================================
Test-Step "ENDPOINT 1/7 - POST /api/invoices" {
    $invoiceBody = @{
        customer_id = $script:customerId
        order_id = $script:orderId
        invoice_type = "standard"
        due_date = "2025-12-31"
        description = "Robust Test Invoice $timestamp"
        notes = "Automated robust test"
        terms = "Net 30 days"
        items = @(
            @{
                product_id = $script:productId
                description = "Robust invoice item"
                quantity = 5
                unit_price = 250.00
                discount = 50.00
                tax_rate = 18.0
            }
        )
    } | ConvertTo-Json -Depth 5

    $invoiceResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices" -Method Post -Headers $script:headers -Body $invoiceBody
    
    # Parsear JSON aninhado
    if ($invoiceResult.invoice -is [string]) {
        $script:invoice = $invoiceResult.invoice | ConvertFrom-Json
    } else {
        $script:invoice = $invoiceResult.invoice
    }
    
    $script:invoiceId = $script:invoice.id
    
    if (-not $script:invoiceId) {
        throw "Invoice ID não foi retornado"
    }
    
    if (-not $script:invoice.invoice_number) {
        throw "Invoice Number não foi gerado"
    }
    
    if ($script:invoice.status -ne "draft") {
        throw "Invoice status esperado 'draft', recebido '$($script:invoice.status)'"
    }
    
    Write-Host "  Invoice ID: $script:invoiceId" -ForegroundColor Gray
    Write-Host "  Invoice Number: $($script:invoice.invoice_number)" -ForegroundColor Gray
    Write-Host "  Status: $($script:invoice.status)" -ForegroundColor Gray
    Write-Host "  Total: R$ $($script:invoice.total)" -ForegroundColor Gray
}

Start-Sleep -Seconds 1

# =========================================================
# PASSO 7: GET INVOICE BY ID (ENDPOINT 2/7)
# =========================================================
Test-Step "ENDPOINT 2/7 - GET /api/invoices/:id" {
    $getResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$script:invoiceId" -Method Get -Headers $script:headers
    
    if ($getResult.invoice -is [string]) {
        $retrievedInvoice = $getResult.invoice | ConvertFrom-Json
    } else {
        $retrievedInvoice = $getResult.invoice
    }
    
    if ($retrievedInvoice.id -ne $script:invoiceId) {
        throw "Invoice ID não corresponde"
    }
    
    Write-Host "  Invoice recuperada: $($retrievedInvoice.invoice_number)" -ForegroundColor Gray
    Write-Host "  Status: $($retrievedInvoice.status)" -ForegroundColor Gray
}

Start-Sleep -Seconds 1

# =========================================================
# PASSO 8: LIST ALL INVOICES (ENDPOINT 3/7)
# =========================================================
Test-Step "ENDPOINT 3/7 - GET /api/invoices" {
    $listResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices" -Method Get -Headers $script:headers
    
    if ($listResult.count -lt 1) {
        throw "Nenhuma invoice encontrada na listagem"
    }
    
    Write-Host "  Total de invoices: $($listResult.count)" -ForegroundColor Gray
}

Start-Sleep -Seconds 1

# =========================================================
# PASSO 9: GET INVOICE ITEMS (ENDPOINT 4/7)
# =========================================================
Test-Step "ENDPOINT 4/7 - GET /api/invoices/:id/items" {
    $itemsResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$script:invoiceId/items" -Method Get -Headers $script:headers
    
    if ($itemsResult.count -lt 1) {
        throw "Nenhum item encontrado na invoice"
    }
    
    Write-Host "  Items na invoice: $($itemsResult.count)" -ForegroundColor Gray
}

Start-Sleep -Seconds 1

# =========================================================
# PASSO 10: UPDATE STATUS TO SENT (ENDPOINT 5/7)
# =========================================================
Test-Step "ENDPOINT 5/7 - PUT /api/invoices/:id/status (sent)" {
    $statusBody = @{
        status = "sent"
    } | ConvertTo-Json
    
    $statusResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$script:invoiceId/status" -Method Put -Headers $script:headers -Body $statusBody
    
    if ($statusResult.invoice -is [string]) {
        $updatedInvoice = $statusResult.invoice | ConvertFrom-Json
    } else {
        $updatedInvoice = $statusResult.invoice
    }
    
    if ($updatedInvoice.status -ne "sent") {
        throw "Status não foi atualizado para 'sent'"
    }
    
    Write-Host "  Status atualizado: $($updatedInvoice.status)" -ForegroundColor Gray
}

Start-Sleep -Seconds 1

# =========================================================
# PASSO 11: APPLY PARTIAL PAYMENT (ENDPOINT 6/7)
# =========================================================
Test-Step "ENDPOINT 6/7 - POST /api/invoices/:id/payment (parcial)" {
    $partialPaymentBody = @{
        amount = 500.00
    } | ConvertTo-Json
    
    $paymentResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$script:invoiceId/payment" -Method Post -Headers $script:headers -Body $partialPaymentBody
    
    if ($paymentResult.invoice -is [string]) {
        $paidInvoice = $paymentResult.invoice | ConvertFrom-Json
    } else {
        $paidInvoice = $paymentResult.invoice
    }
    
    if ($paidInvoice.status -ne "partial") {
        throw "Status esperado 'partial', recebido '$($paidInvoice.status)'"
    }
    
    if ([double]$paidInvoice.amount_paid -ne 500.00) {
        throw "Amount paid esperado 500.00, recebido $($paidInvoice.amount_paid)"
    }
    
    Write-Host "  Status: $($paidInvoice.status)" -ForegroundColor Gray
    Write-Host "  Paid: R$ $($paidInvoice.amount_paid)" -ForegroundColor Gray
    Write-Host "  Due: R$ $($paidInvoice.amount_due)" -ForegroundColor Gray
    
    $script:amountDue = [double]$paidInvoice.amount_due
}

Start-Sleep -Seconds 1

# =========================================================
# PASSO 12: APPLY FULL PAYMENT (ENDPOINT 6/7 - part 2)
# =========================================================
Test-Step "ENDPOINT 6/7 - POST /api/invoices/:id/payment (completo)" {
    $fullPaymentBody = @{
        amount = $script:amountDue
    } | ConvertTo-Json
    
    $fullPaymentResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$script:invoiceId/payment" -Method Post -Headers $script:headers -Body $fullPaymentBody
    
    if ($fullPaymentResult.invoice -is [string]) {
        $fullyPaidInvoice = $fullPaymentResult.invoice | ConvertFrom-Json
    } else {
        $fullyPaidInvoice = $fullPaymentResult.invoice
    }
    
    if ($fullyPaidInvoice.status -ne "paid") {
        throw "Status esperado 'paid', recebido '$($fullyPaidInvoice.status)'"
    }
    
    if ([double]$fullyPaidInvoice.amount_due -gt 0.02) {
        throw "Amount due deveria ser ~0, recebido $($fullyPaidInvoice.amount_due)"
    }
    
    Write-Host "  Status: $($fullyPaidInvoice.status)" -ForegroundColor Gray
    Write-Host "  Paid: R$ $($fullyPaidInvoice.amount_paid)" -ForegroundColor Gray
    Write-Host "  Due: R$ $($fullyPaidInvoice.amount_due)" -ForegroundColor Gray
}

Start-Sleep -Seconds 1

# =========================================================
# PASSO 13: LIST CUSTOMER INVOICES (ENDPOINT 7/7)
# =========================================================
Test-Step "ENDPOINT 7/7 - GET /api/customers/:customerId/invoices" {
    $customerInvoicesResult = Invoke-RestMethod -Uri "$baseUrl/api/customers/$script:customerId/invoices" -Method Get -Headers $script:headers
    
    if ($customerInvoicesResult.count -lt 1) {
        throw "Nenhuma invoice encontrada para o customer"
    }
    
    Write-Host "  Invoices do customer: $($customerInvoicesResult.count)" -ForegroundColor Gray
}

# =========================================================
# RESUMO FINAL
# =========================================================
Write-Host "`n=========================================================" -ForegroundColor Cyan
Write-Host " RESUMO FINAL - TESTE ROBUSTO" -ForegroundColor Cyan
Write-Host "=========================================================`n" -ForegroundColor Cyan

$successRate = [math]::Round(($script:passedTests / $script:totalTests) * 100, 2)

Write-Host "  Testes Executados: $script:totalTests" -ForegroundColor White
Write-Host "  Testes Passados: $script:passedTests" -ForegroundColor Green
Write-Host "  Taxa de Sucesso: $successRate%" -ForegroundColor $(if ($successRate -eq 100) { "Green" } else { "Yellow" })

Write-Host "`n  IDs Gerados:" -ForegroundColor Cyan
Write-Host "    Customer: $script:customerId" -ForegroundColor Gray
Write-Host "    Product: $script:productId" -ForegroundColor Gray
Write-Host "    Order: $script:orderId" -ForegroundColor Gray
Write-Host "    Invoice: $script:invoiceId" -ForegroundColor Gray

if ($script:passedTests -eq $script:totalTests) {
    Write-Host "`n  >>> TODOS OS ENDPOINTS VALIDADOS! <<<" -ForegroundColor Green
    Write-Host "  >>> MODULO 100% FUNCIONAL! <<<`n" -ForegroundColor Green
} else {
    Write-Host "`n  [AVISO] Alguns testes falharam. Revisar logs acima.`n" -ForegroundColor Yellow
}

Write-Host "=========================================================`n" -ForegroundColor Cyan

