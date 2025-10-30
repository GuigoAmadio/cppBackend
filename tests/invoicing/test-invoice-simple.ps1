# ========================================
# TESTE SIMPLIFICADO - INVOICE MODULE
# Usando usuário existente
# ========================================

$ErrorActionPreference = "Continue"
$baseUrl = "http://localhost:8080"
$timestamp = [DateTimeOffset]::Now.ToUnixTimeSeconds()

Write-Host "`n=========================================" -ForegroundColor Cyan
Write-Host " TESTE INVOICE - USUÁRIO EXISTENTE" -ForegroundColor Cyan
Write-Host "=========================================`n" -ForegroundColor Cyan

# ========================================
# 1. LOGIN com usuário existente
# ========================================
Write-Host "[1/12] LOGIN..." -ForegroundColor Yellow

$loginBody = @{
    email = "invoicetest.1761174831@test.com"
    password = "InvoicePass123!"
    tenant_subdomain = "acme"
} | ConvertTo-Json

try {
    $loginResult = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method Post -Body $loginBody -ContentType "application/json"
    $token = if ($loginResult.token) { $loginResult.token } 
            elseif ($loginResult.access_token) { $loginResult.access_token }
            else { $null }
    
    if (-not $token) { throw "Token não encontrado" }
    Write-Host "[OK] Login realizado!" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] Login falhou: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

$headers = @{
    "Authorization" = "Bearer $token"
    "Content-Type" = "application/json"
}

Start-Sleep -Seconds 1

# ========================================
# 2. CRIAR CUSTOMER
# ========================================
Write-Host "[2/12] CRIAR CUSTOMER..." -ForegroundColor Yellow

$customerBody = @{
    name = "Invoice Customer $timestamp"
    email = "invoice.customer.$timestamp@test.com"
    type = "individual"
} | ConvertTo-Json

try {
    $customerResult = Invoke-RestMethod -Uri "$baseUrl/api/customers" -Method Post -Headers $headers -Body $customerBody
    $customerId = $customerResult.customer.id
    Write-Host "[OK] Customer: $customerId" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# ========================================
# 3. CRIAR PRODUCT
# ========================================
Write-Host "[3/12] CRIAR PRODUCT..." -ForegroundColor Yellow

$productBody = @{
    code = "INV-PROD-$timestamp"
    name = "Invoice Product $timestamp"
    type = "physical"
    price = 150.00
} | ConvertTo-Json

try {
    $productResult = Invoke-RestMethod -Uri "$baseUrl/api/products" -Method Post -Headers $headers -Body $productBody
    $productId = $productResult.product.id
    Write-Host "[OK] Product: $productId" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# ========================================
# 4. CRIAR ORDER
# ========================================
Write-Host "[4/12] CRIAR ORDER..." -ForegroundColor Yellow

$orderBody = @{
    customer_id = $customerId
    items = @(
        @{
            product_id = $productId
            quantity = 3
            unit_price = 150.00
        }
    )
} | ConvertTo-Json -Depth 5

try {
    $orderResult = Invoke-RestMethod -Uri "$baseUrl/api/orders" -Method Post -Headers $headers -Body $orderBody
    $orderId = $orderResult.order.id
    Write-Host "[OK] Order: $orderId" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# ========================================
# 5. CRIAR INVOICE
# ========================================
Write-Host "[5/12] CRIAR INVOICE..." -ForegroundColor Yellow

$invoiceBody = @{
    customer_id = $customerId
    order_id = $orderId
    invoice_type = "standard"
    due_date = "2025-12-31"
    description = "Test Invoice $timestamp"
    items = @(
        @{
            product_id = $productId
            description = "Invoice Item"
            quantity = 3
            unit_price = 150.00
            discount = 20.00
            tax_rate = 15.0
        }
    )
} | ConvertTo-Json -Depth 5

try {
    $invoiceResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices" -Method Post -Headers $headers -Body $invoiceBody
    $invoiceData = $invoiceResult.invoice | ConvertFrom-Json
    $invoiceId = $invoiceData.id
    Write-Host "[OK] Invoice criada: $invoiceId" -ForegroundColor Green
    Write-Host "  Number: $($invoiceData.invoice_number)" -ForegroundColor Gray
    Write-Host "  Total: $($invoiceData.total)" -ForegroundColor Gray
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# ========================================
# 6. GET INVOICE
# ========================================
Write-Host "[6/12] GET INVOICE..." -ForegroundColor Yellow

try {
    $getResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId" -Method Get -Headers $headers
    $inv = $getResult.invoice | ConvertFrom-Json
    Write-Host "[OK] Status: $($inv.status), Total: $($inv.total)" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 7. LIST INVOICES
# ========================================
Write-Host "[7/12] LIST INVOICES..." -ForegroundColor Yellow

try {
    $listResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices" -Method Get -Headers $headers
    Write-Host "[OK] Total invoices: $($listResult.count)" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 8. GET INVOICE ITEMS
# ========================================
Write-Host "[8/12] GET ITEMS..." -ForegroundColor Yellow

try {
    $itemsResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId/items" -Method Get -Headers $headers
    Write-Host "[OK] Items: $($itemsResult.count)" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 9. UPDATE STATUS (sent)
# ========================================
Write-Host "[9/12] UPDATE STATUS -> sent..." -ForegroundColor Yellow

$statusBody = @{
    status = "sent"
} | ConvertTo-Json

try {
    $updateResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId/status" -Method Put -Headers $headers -Body $statusBody
    $invUpd = $updateResult.invoice | ConvertFrom-Json
    Write-Host "[OK] Status: $($invUpd.status)" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 10. APPLY PARTIAL PAYMENT
# ========================================
Write-Host "[10/12] APPLY PAYMENT (100.00)..." -ForegroundColor Yellow

$paymentBody = @{
    amount = 100.00
} | ConvertTo-Json

try {
    $payResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId/payment" -Method Post -Headers $headers -Body $paymentBody
    $invPay = $payResult.invoice | ConvertFrom-Json
    Write-Host "[OK] Status: $($invPay.status), Paid: $($invPay.amount_paid), Due: $($invPay.amount_due)" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 11. APPLY FULL PAYMENT
# ========================================
Write-Host "[11/12] APPLY FULL PAYMENT..." -ForegroundColor Yellow

try {
    $currentInv = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId" -Method Get -Headers $headers
    $invCurrent = $currentInv.invoice | ConvertFrom-Json
    $amountDue = $invCurrent.amount_due
    
    $fullPayBody = @{
        amount = $amountDue
    } | ConvertTo-Json
    
    $fullPayResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId/payment" -Method Post -Headers $headers -Body $fullPayBody
    $invFull = $fullPayResult.invoice | ConvertFrom-Json
    Write-Host "[OK] Status: $($invFull.status), Due: $($invFull.amount_due)" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 12. LIST CUSTOMER INVOICES
# ========================================
Write-Host "[12/12] LIST CUSTOMER INVOICES..." -ForegroundColor Yellow

try {
    $custInvResult = Invoke-RestMethod -Uri "$baseUrl/api/customers/$customerId/invoices" -Method Get -Headers $headers
    Write-Host "[OK] Customer invoices: $($custInvResult.count)" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host "`n==========================================" -ForegroundColor Cyan
Write-Host " RESUMO" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host " Invoice ID: $invoiceId" -ForegroundColor White
Write-Host " Customer ID: $customerId" -ForegroundColor White
Write-Host " Product ID: $productId" -ForegroundColor White
Write-Host " Order ID: $orderId" -ForegroundColor White
Write-Host "==========================================`n" -ForegroundColor Cyan

