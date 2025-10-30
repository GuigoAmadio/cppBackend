# =========================================================
# TESTE FINAL - INVOICE MODULE (7 ENDPOINTS)
# Extração correta de IDs + Validação completa
# =========================================================

$ErrorActionPreference = "Continue"
$baseUrl = "http://localhost:8080"
$timestamp = [DateTimeOffset]::Now.ToUnixTimeSeconds()

Write-Host "`n=========================================" -ForegroundColor Cyan
Write-Host " TESTE FINAL - INVOICE (7 ENDPOINTS)" -ForegroundColor Cyan
Write-Host "=========================================`n" -ForegroundColor Cyan

$passedTests = 0
$totalTests = 0

# =========================================================
# 1. LOGIN
# =========================================================
$totalTests++
Write-Host "[$totalTests] LOGIN..." -ForegroundColor Yellow

$loginBody = @{
    email = "invoicetest.1761174831@test.com"
    password = "InvoicePass123!"
    tenant_subdomain = "acme"
} | ConvertTo-Json

try {
    $loginResult = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method Post -Body $loginBody -ContentType "application/json"
    $token = if ($loginResult.token) { $loginResult.token } else { $loginResult.access_token }
    
    if (-not $token) { throw "Token não encontrado" }
    
    $headers = @{
        "Authorization" = "Bearer $token"
        "Content-Type" = "application/json"
    }
    
    $passedTests++
    Write-Host "  [OK] Token: $($token.Substring(0,20))...`n" -ForegroundColor Green
} catch {
    Write-Host "  [ERRO] $($_.Exception.Message)`n" -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# =========================================================
# 2. CRIAR CUSTOMER (pre-requisito)
# =========================================================
$totalTests++
Write-Host "[$totalTests] CRIAR CUSTOMER..." -ForegroundColor Yellow

$customerBody = @{
    name = "Invoice Customer $timestamp"
    email = "invoice.$timestamp@test.com"
    type = "individual"
} | ConvertTo-Json

try {
    $customerResult = Invoke-RestMethod -Uri "$baseUrl/api/customers" -Method Post -Headers $headers -Body $customerBody
    
    # CORREÇÃO: ID vem direto na raiz!
    $customerId = $customerResult.id
    
    if (-not $customerId) { throw "Customer ID não retornado" }
    
    $passedTests++
    Write-Host "  [OK] Customer ID: $customerId`n" -ForegroundColor Green
} catch {
    Write-Host "  [ERRO] $($_.Exception.Message)`n" -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# =========================================================
# 3. CRIAR PRODUCT (pre-requisito)
# =========================================================
$totalTests++
Write-Host "[$totalTests] CRIAR PRODUCT..." -ForegroundColor Yellow

$productBody = @{
    code = "INV-PROD-$timestamp"
    name = "Invoice Product $timestamp"
    type = "physical"
    price = 250.00
    sku = "INV-SKU-$timestamp"
} | ConvertTo-Json

try {
    $productResult = Invoke-RestMethod -Uri "$baseUrl/api/products" -Method Post -Headers $headers -Body $productBody
    
    # CORREÇÃO: ID vem direto na raiz!
    $productId = $productResult.id
    
    if (-not $productId) { throw "Product ID não retornado" }
    
    $passedTests++
    Write-Host "  [OK] Product ID: $productId`n" -ForegroundColor Green
} catch {
    Write-Host "  [ERRO] $($_.Exception.Message)`n" -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# =========================================================
# 4. CRIAR ORDER (pre-requisito)
# =========================================================
$totalTests++
Write-Host "[$totalTests] CRIAR ORDER..." -ForegroundColor Yellow

$orderBody = @{
    customer_id = $customerId
    items = @(
        @{
            product_id = $productId
            quantity = 4
            unit_price = 250.00
        }
    )
} | ConvertTo-Json -Depth 5

try {
    $orderResult = Invoke-RestMethod -Uri "$baseUrl/api/orders" -Method Post -Headers $headers -Body $orderBody
    
    # CORREÇÃO: ID vem direto na raiz!
    $orderId = $orderResult.id
    
    if (-not $orderId) { throw "Order ID não retornado" }
    
    $passedTests++
    Write-Host "  [OK] Order ID: $orderId`n" -ForegroundColor Green
} catch {
    Write-Host "  [ERRO] $($_.Exception.Message)`n" -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# =========================================================
# 5. ENDPOINT 1/7 - POST /api/invoices
# =========================================================
$totalTests++
Write-Host "[$totalTests] ENDPOINT 1/7 - POST /api/invoices..." -ForegroundColor Yellow

$invoiceBody = @{
    customer_id = $customerId
    order_id = $orderId
    invoice_type = "standard"
    due_date = "2025-12-31"
    description = "Test Invoice $timestamp"
    notes = "Automated test"
    terms = "Net 30"
    items = @(
        @{
            product_id = $productId
            description = "Invoice line item"
            quantity = 4
            unit_price = 250.00
            discount = 50.00
            tax_rate = 15.0
        }
    )
} | ConvertTo-Json -Depth 5

try {
    $invoiceResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices" -Method Post -Headers $headers -Body $invoiceBody
    
    # Backend retorna id, invoice_number, status, total diretamente na raiz
    $invoiceId = $invoiceResult.id
    
    if (-not $invoiceId) { throw "Invoice ID não retornado" }
    if ($invoiceResult.status -ne "draft") { throw "Status esperado 'draft', recebido '$($invoiceResult.status)'" }
    
    $passedTests++
    Write-Host "  [OK] Invoice ID: $invoiceId" -ForegroundColor Green
    Write-Host "      Number: $($invoiceResult.invoice_number)" -ForegroundColor Gray
    Write-Host "      Status: $($invoiceResult.status)" -ForegroundColor Gray
    Write-Host "      Total: $($invoiceResult.total)`n" -ForegroundColor Gray
} catch {
    Write-Host "  [ERRO] $($_.Exception.Message)`n" -ForegroundColor Red
    Write-Host "  Response: $($invoiceResult | ConvertTo-Json -Compress)`n" -ForegroundColor Gray
}

Start-Sleep -Seconds 1

# =========================================================
# 6. ENDPOINT 2/7 - GET /api/invoices/:id
# =========================================================
$totalTests++
Write-Host "[$totalTests] ENDPOINT 2/7 - GET /api/invoices/:id..." -ForegroundColor Yellow

if ($invoiceId) {
    try {
        $getResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId" -Method Get -Headers $headers
        
        if ($getResult.invoice -is [string]) {
            $retrievedInvoice = $getResult.invoice | ConvertFrom-Json
        } else {
            $retrievedInvoice = $getResult.invoice
        }
        
        if ($retrievedInvoice.id -ne $invoiceId) { throw "Invoice ID não corresponde" }
        
        $passedTests++
        Write-Host "  [OK] Invoice recuperada: $($retrievedInvoice.invoice_number)`n" -ForegroundColor Green
    } catch {
        Write-Host "  [ERRO] $($_.Exception.Message)`n" -ForegroundColor Red
    }
} else {
    Write-Host "  [SKIP] Invoice não criada no passo anterior`n" -ForegroundColor Yellow
}

Start-Sleep -Seconds 1

# =========================================================
# 7. ENDPOINT 3/7 - GET /api/invoices
# =========================================================
$totalTests++
Write-Host "[$totalTests] ENDPOINT 3/7 - GET /api/invoices..." -ForegroundColor Yellow

try {
    $listResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices" -Method Get -Headers $headers
    
    Write-Host "  [OK] Total invoices: $($listResult.count)`n" -ForegroundColor Green
    $passedTests++
} catch {
    Write-Host "  [ERRO] $($_.Exception.Message)`n" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# =========================================================
# 8. ENDPOINT 4/7 - GET /api/invoices/:id/items
# =========================================================
$totalTests++
Write-Host "[$totalTests] ENDPOINT 4/7 - GET /api/invoices/:id/items..." -ForegroundColor Yellow

if ($invoiceId) {
    try {
        $itemsResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId/items" -Method Get -Headers $headers
        
        if ($itemsResult.count -lt 1) { throw "Nenhum item encontrado" }
        
        $passedTests++
        Write-Host "  [OK] Items: $($itemsResult.count)`n" -ForegroundColor Green
    } catch {
        Write-Host "  [ERRO] $($_.Exception.Message)`n" -ForegroundColor Red
    }
} else {
    Write-Host "  [SKIP] Invoice não criada`n" -ForegroundColor Yellow
}

Start-Sleep -Seconds 1

# =========================================================
# 9. ENDPOINT 5/7 - PUT /api/invoices/:id/status
# =========================================================
$totalTests++
Write-Host "[$totalTests] ENDPOINT 5/7 - PUT /api/invoices/:id/status..." -ForegroundColor Yellow

if ($invoiceId) {
    try {
        $statusBody = @{ status = "sent" } | ConvertTo-Json
        
        $statusResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId/status" -Method Put -Headers $headers -Body $statusBody
        
        if ($statusResult.invoice -is [string]) {
            $updatedInvoice = $statusResult.invoice | ConvertFrom-Json
        } else {
            $updatedInvoice = $statusResult.invoice
        }
        
        if ($updatedInvoice.status -ne "sent") { throw "Status não atualizado para 'sent'" }
        
        $passedTests++
        Write-Host "  [OK] Status: $($updatedInvoice.status)`n" -ForegroundColor Green
    } catch {
        Write-Host "  [ERRO] $($_.Exception.Message)`n" -ForegroundColor Red
    }
} else {
    Write-Host "  [SKIP] Invoice não criada`n" -ForegroundColor Yellow
}

Start-Sleep -Seconds 1

# =========================================================
# 10. ENDPOINT 6/7 - POST /api/invoices/:id/payment (PARCIAL)
# =========================================================
$totalTests++
Write-Host "[$totalTests] ENDPOINT 6/7 - POST /api/invoices/:id/payment (parcial)..." -ForegroundColor Yellow

if ($invoiceId) {
    try {
        $paymentBody = @{ amount = 500.00 } | ConvertTo-Json
        
        $payResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId/payment" -Method Post -Headers $headers -Body $paymentBody
        
        if ($payResult.invoice -is [string]) {
            $paidInvoice = $payResult.invoice | ConvertFrom-Json
        } else {
            $paidInvoice = $payResult.invoice
        }
        
        if ($paidInvoice.status -ne "partial") { throw "Status esperado 'partial', recebido '$($paidInvoice.status)'" }
        
        $script:amountDue = [double]$paidInvoice.amount_due
        
        $passedTests++
        Write-Host "  [OK] Status: $($paidInvoice.status)" -ForegroundColor Green
        Write-Host "      Paid: R$ $($paidInvoice.amount_paid)" -ForegroundColor Gray
        Write-Host "      Due: R$ $($paidInvoice.amount_due)`n" -ForegroundColor Gray
    } catch {
        Write-Host "  [ERRO] $($_.Exception.Message)`n" -ForegroundColor Red
    }
} else {
    Write-Host "  [SKIP] Invoice não criada`n" -ForegroundColor Yellow
}

Start-Sleep -Seconds 1

# =========================================================
# 11. ENDPOINT 6/7 - POST /api/invoices/:id/payment (COMPLETO)
# =========================================================
$totalTests++
Write-Host "[$totalTests] ENDPOINT 6/7 - POST /api/invoices/:id/payment (completo)..." -ForegroundColor Yellow

if ($invoiceId -and $script:amountDue) {
    try {
        $fullPaymentBody = @{ amount = $script:amountDue } | ConvertTo-Json
        
        $fullPayResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId/payment" -Method Post -Headers $headers -Body $fullPaymentBody
        
        if ($fullPayResult.invoice -is [string]) {
            $fullyPaidInvoice = $fullPayResult.invoice | ConvertFrom-Json
        } else {
            $fullyPaidInvoice = $fullPayResult.invoice
        }
        
        if ($fullyPaidInvoice.status -ne "paid") { throw "Status esperado 'paid', recebido '$($fullyPaidInvoice.status)'" }
        
        $passedTests++
        Write-Host "  [OK] Status: $($fullyPaidInvoice.status)" -ForegroundColor Green
        Write-Host "      Paid: R$ $($fullyPaidInvoice.amount_paid)" -ForegroundColor Gray
        Write-Host "      Due: R$ $($fullyPaidInvoice.amount_due)`n" -ForegroundColor Gray
    } catch {
        Write-Host "  [ERRO] $($_.Exception.Message)`n" -ForegroundColor Red
    }
} else {
    Write-Host "  [SKIP] Invoice não criada ou pagamento parcial não feito`n" -ForegroundColor Yellow
}

Start-Sleep -Seconds 1

# =========================================================
# 12. ENDPOINT 7/7 - GET /api/customers/:customerId/invoices
# =========================================================
$totalTests++
Write-Host "[$totalTests] ENDPOINT 7/7 - GET /api/customers/:customerId/invoices..." -ForegroundColor Yellow

if ($customerId) {
    try {
        $custInvResult = Invoke-RestMethod -Uri "$baseUrl/api/customers/$customerId/invoices" -Method Get -Headers $headers
        
        if ($custInvResult.count -lt 1) { throw "Nenhuma invoice encontrada para o customer" }
        
        $passedTests++
        Write-Host "  [OK] Customer invoices: $($custInvResult.count)`n" -ForegroundColor Green
    } catch {
        Write-Host "  [ERRO] $($_.Exception.Message)`n" -ForegroundColor Red
    }
} else {
    Write-Host "  [SKIP] Customer não criado`n" -ForegroundColor Yellow
}

# =========================================================
# RESUMO FINAL
# =========================================================
Write-Host "`n=========================================" -ForegroundColor Cyan
Write-Host " RESUMO FINAL" -ForegroundColor Cyan
Write-Host "=========================================`n" -ForegroundColor Cyan

$successRate = [math]::Round(($passedTests / $totalTests) * 100, 2)

Write-Host "  Testes Executados: $totalTests" -ForegroundColor White
Write-Host "  Testes Passados: $passedTests" -ForegroundColor Green
Write-Host "  Taxa de Sucesso: $successRate%" -ForegroundColor $(if ($successRate -ge 90) { "Green" } elseif ($successRate -ge 70) { "Yellow" } else { "Red" })

Write-Host "`n  IDs Gerados:" -ForegroundColor Cyan
Write-Host "    Customer: $customerId" -ForegroundColor White
Write-Host "    Product: $productId" -ForegroundColor White
Write-Host "    Order: $orderId" -ForegroundColor White
Write-Host "    Invoice: $invoiceId" -ForegroundColor White

if ($passedTests -eq $totalTests) {
    Write-Host "`n  >>> TODOS OS ENDPOINTS VALIDADOS! <<<" -ForegroundColor Green
    Write-Host "  >>> MODULO INVOICE 100% FUNCIONAL! <<<`n" -ForegroundColor Green
} elseif ($successRate -ge 70) {
    Write-Host "`n  [AVISO] Maioria dos testes passou, revisar falhas.`n" -ForegroundColor Yellow
} else {
    Write-Host "`n  [ERRO] Muitos testes falharam, revisar logs.`n" -ForegroundColor Red
}

Write-Host "=========================================`n" -ForegroundColor Cyan

