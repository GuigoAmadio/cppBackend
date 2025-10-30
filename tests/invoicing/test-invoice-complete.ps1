# ========================================
# TESTE COMPLETO - INVOICE MODULE
# ========================================

$ErrorActionPreference = "Continue"
$baseUrl = "http://localhost:8080"

# Gerar timestamp para valores únicos
$timestamp = [DateTimeOffset]::Now.ToUnixTimeSeconds()

Write-Host "`n============================================" -ForegroundColor Cyan
Write-Host "  TESTE COMPLETO - INVOICE MODULE" -ForegroundColor Cyan
Write-Host "============================================`n" -ForegroundColor Cyan

# ========================================
# 1. SETUP: CRIAR E LOGAR USUÁRIO
# ========================================
Write-Host "`n[1/15] SETUP: Criando usuário de teste..." -ForegroundColor Yellow

$testEmail = "invoice.test.$timestamp@test.com"
$testPassword = "TestPass123!"

$registerBody = @{
    email = $testEmail
    password = $testPassword
    name = "Invoice Test User"
    tenant_subdomain = "acme"
} | ConvertTo-Json

try {
    $registerResult = Invoke-RestMethod -Uri "$baseUrl/api/auth/register" -Method Post -Body $registerBody -ContentType "application/json"
    $userId = $registerResult.user.id
    Write-Host "[OK] Usuário criado: $userId" -ForegroundColor Green
} catch {
    Write-Host "[WARN] Falha ao registrar (pode já existir): $($_.Exception.Message)" -ForegroundColor Yellow
}

Start-Sleep -Seconds 1

# ========================================
# 1.5. ADICIONAR USUÁRIO AO TENANT
# ========================================
Write-Host "`n[1.5/15] Adicionando usuário ao tenant 'acme'..." -ForegroundColor Yellow

# Conectar ao PostgreSQL e adicionar usuário ao tenant
$env:PGPASSWORD = "postgre123"
$sqlCommand = "INSERT INTO user_tenants (user_id, tenant_id, role) VALUES ('$userId', '659eb680-1f6e-47b5-bafd-38100d6cee8e', 'member') ON CONFLICT DO NOTHING;"
$null = psql -U moneymaker_user -d moneymaker_dev -h localhost -p 5433 -c $sqlCommand 2>&1

Write-Host "[OK] Usuário adicionado ao tenant!" -ForegroundColor Green

Start-Sleep -Seconds 1

# ========================================
# 2. LOGIN
# ========================================
Write-Host "`n[2/15] LOGIN: Fazendo login..." -ForegroundColor Yellow

$loginBody = @{
    email = $testEmail
    password = $testPassword
    tenant_subdomain = "acme"
} | ConvertTo-Json

try {
    $loginResult = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method Post -Body $loginBody -ContentType "application/json"
    $token = if ($loginResult.token) { $loginResult.token } 
            elseif ($loginResult.access_token) { $loginResult.access_token }
            elseif ($loginResult.data.token) { $loginResult.data.token }
            elseif ($loginResult.data.access_token) { $loginResult.data.access_token }
            else { $null }
    
    if (-not $token) {
        throw "Token não encontrado na resposta de login"
    }
    
    Write-Host "[OK] Login realizado! Token: $($token.Substring(0, [Math]::Min(20, $token.Length)))..." -ForegroundColor Green
} catch {
    Write-Host "[ERRO] Falha no login: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# Headers com autenticação
$headers = @{
    "Authorization" = "Bearer $token"
    "Content-Type" = "application/json"
}

Start-Sleep -Seconds 1

# ========================================
# 3. CRIAR CUSTOMER (pré-requisito)
# ========================================
Write-Host "`n[3/15] CRIAR CUSTOMER..." -ForegroundColor Yellow

$customerBody = @{
    name = "Invoice Test Customer $timestamp"
    email = "customer.$timestamp@test.com"
    type = "individual"
} | ConvertTo-Json

try {
    $customerResult = Invoke-RestMethod -Uri "$baseUrl/api/customers" -Method Post -Headers $headers -Body $customerBody
    $customerId = $customerResult.customer.id
    Write-Host "[OK] Customer criado: $customerId" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] Falha ao criar customer: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# ========================================
# 4. CRIAR PRODUCT (pré-requisito)
# ========================================
Write-Host "`n[4/15] CRIAR PRODUCT..." -ForegroundColor Yellow

$productBody = @{
    code = "PROD-INV-$timestamp"
    name = "Product for Invoice $timestamp"
    type = "physical"
    price = 100.00
} | ConvertTo-Json

try {
    $productResult = Invoke-RestMethod -Uri "$baseUrl/api/products" -Method Post -Headers $headers -Body $productBody
    $productId = $productResult.product.id
    Write-Host "[OK] Product criado: $productId" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] Falha ao criar product: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# ========================================
# 5. CRIAR ORDER (pré-requisito)
# ========================================
Write-Host "`n[5/15] CRIAR ORDER..." -ForegroundColor Yellow

$orderBody = @{
    customer_id = $customerId
    items = @(
        @{
            product_id = $productId
            quantity = 2
            unit_price = 100.00
        }
    )
} | ConvertTo-Json -Depth 5

try {
    $orderResult = Invoke-RestMethod -Uri "$baseUrl/api/orders" -Method Post -Headers $headers -Body $orderBody
    $orderId = $orderResult.order.id
    Write-Host "[OK] Order criada: $orderId" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] Falha ao criar order: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# ========================================
# 6. CRIAR INVOICE (principal)
# ========================================
Write-Host "`n[6/15] CRIAR INVOICE..." -ForegroundColor Yellow

$invoiceBody = @{
    customer_id = $customerId
    order_id = $orderId
    invoice_type = "standard"
    due_date = "2025-12-31"
    description = "Test Invoice $timestamp"
    notes = "Test notes"
    terms = "Net 30"
    items = @(
        @{
            product_id = $productId
            description = "Product line item"
            quantity = 2
            unit_price = 100.00
            discount = 10.00
            tax_rate = 10.0
        }
    )
} | ConvertTo-Json -Depth 5

try {
    $invoiceResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices" -Method Post -Headers $headers -Body $invoiceBody
    $invoiceId = $invoiceResult.invoice.id
    Write-Host "[OK] Invoice criada: $invoiceId" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] Falha ao criar invoice: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "Body: $invoiceBody" -ForegroundColor Gray
    exit 1
}

Start-Sleep -Seconds 1

# ========================================
# 7. GET INVOICE
# ========================================
Write-Host "`n[7/15] GET INVOICE..." -ForegroundColor Yellow

try {
    $getInvoiceResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId" -Method Get -Headers $headers
    Write-Host "[OK] Invoice recuperada: $($getInvoiceResult.invoice.invoice_number)" -ForegroundColor Green
    Write-Host "  Status: $($getInvoiceResult.invoice.status)" -ForegroundColor Gray
    Write-Host "  Total: $($getInvoiceResult.invoice.total)" -ForegroundColor Gray
} catch {
    Write-Host "[ERRO] Falha ao buscar invoice: $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 8. LIST INVOICES
# ========================================
Write-Host "`n[8/15] LIST INVOICES..." -ForegroundColor Yellow

try {
    $listResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices" -Method Get -Headers $headers
    Write-Host "[OK] Invoices listadas: $($listResult.count)" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] Falha ao listar invoices: $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 9. LIST INVOICES BY STATUS
# ========================================
Write-Host "`n[9/15] LIST INVOICES BY STATUS (draft)..." -ForegroundColor Yellow

try {
    $listStatusResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices?status=draft" -Method Get -Headers $headers
    Write-Host "[OK] Invoices 'draft': $($listStatusResult.count)" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] Falha ao listar por status: $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 10. GET INVOICE ITEMS
# ========================================
Write-Host "`n[10/15] GET INVOICE ITEMS..." -ForegroundColor Yellow

try {
    $itemsResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId/items" -Method Get -Headers $headers
    Write-Host "[OK] Invoice items: $($itemsResult.count)" -ForegroundColor Green
    if ($itemsResult.count -gt 0) {
        Write-Host "  Primeiro item: $($itemsResult.items[0].description)" -ForegroundColor Gray
    }
} catch {
    Write-Host "[ERRO] Falha ao buscar items: $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 11. LIST CUSTOMER INVOICES
# ========================================
Write-Host "`n[11/15] LIST CUSTOMER INVOICES..." -ForegroundColor Yellow

try {
    $customerInvoicesResult = Invoke-RestMethod -Uri "$baseUrl/api/customers/$customerId/invoices" -Method Get -Headers $headers
    Write-Host "[OK] Invoices do customer: $($customerInvoicesResult.count)" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] Falha ao listar invoices do customer: $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 12. UPDATE INVOICE STATUS (draft -> sent)
# ========================================
Write-Host "`n[12/15] UPDATE STATUS (draft -> sent)..." -ForegroundColor Yellow

$updateStatusBody = @{
    status = "sent"
} | ConvertTo-Json

try {
    $updateStatusResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId/status" -Method Put -Headers $headers -Body $updateStatusBody
    Write-Host "[OK] Status atualizado: $($updateStatusResult.invoice.status)" -ForegroundColor Green
} catch {
    Write-Host "[ERRO] Falha ao atualizar status: $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 13. APPLY PARTIAL PAYMENT
# ========================================
Write-Host "`n[13/15] APPLY PARTIAL PAYMENT (50.00)..." -ForegroundColor Yellow

$paymentBody = @{
    amount = 50.00
} | ConvertTo-Json

try {
    $paymentResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId/payment" -Method Post -Headers $headers -Body $paymentBody
    Write-Host "[OK] Pagamento aplicado!" -ForegroundColor Green
    Write-Host "  Status: $($paymentResult.invoice.status)" -ForegroundColor Gray
    Write-Host "  Amount Paid: $($paymentResult.invoice.amount_paid)" -ForegroundColor Gray
    Write-Host "  Amount Due: $($paymentResult.invoice.amount_due)" -ForegroundColor Gray
} catch {
    Write-Host "[ERRO] Falha ao aplicar pagamento: $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 14. APPLY FULL PAYMENT
# ========================================
Write-Host "`n[14/15] APPLY FULL PAYMENT (restante)..." -ForegroundColor Yellow

# Buscar o amount_due atual
try {
    $currentInvoice = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId" -Method Get -Headers $headers
    $amountDue = $currentInvoice.invoice.amount_due
    
    $fullPaymentBody = @{
        amount = $amountDue
    } | ConvertTo-Json
    
    $fullPaymentResult = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId/payment" -Method Post -Headers $headers -Body $fullPaymentBody
    Write-Host "[OK] Pagamento total aplicado!" -ForegroundColor Green
    Write-Host "  Status: $($fullPaymentResult.invoice.status)" -ForegroundColor Gray
    Write-Host "  Amount Paid: $($fullPaymentResult.invoice.amount_paid)" -ForegroundColor Gray
    Write-Host "  Amount Due: $($fullPaymentResult.invoice.amount_due)" -ForegroundColor Gray
} catch {
    Write-Host "[ERRO] Falha ao aplicar pagamento total: $($_.Exception.Message)" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# ========================================
# 15. VERIFICAR INVOICE FINAL
# ========================================
Write-Host "`n[15/15] VERIFICAR INVOICE FINAL..." -ForegroundColor Yellow

try {
    $finalInvoice = Invoke-RestMethod -Uri "$baseUrl/api/invoices/$invoiceId" -Method Get -Headers $headers
    Write-Host "[OK] Invoice final:" -ForegroundColor Green
    Write-Host "  ID: $($finalInvoice.invoice.id)" -ForegroundColor Gray
    Write-Host "  Number: $($finalInvoice.invoice.invoice_number)" -ForegroundColor Gray
    Write-Host "  Status: $($finalInvoice.invoice.status)" -ForegroundColor Gray
    Write-Host "  Total: $($finalInvoice.invoice.total)" -ForegroundColor Gray
    Write-Host "  Amount Paid: $($finalInvoice.invoice.amount_paid)" -ForegroundColor Gray
    Write-Host "  Amount Due: $($finalInvoice.invoice.amount_due)" -ForegroundColor Gray
} catch {
    Write-Host "[ERRO] Falha ao verificar invoice final: $($_.Exception.Message)" -ForegroundColor Red
}

# ========================================
# RESUMO
# ========================================
Write-Host "`n============================================" -ForegroundColor Cyan
Write-Host "  RESUMO DOS TESTES" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  User ID: $userId" -ForegroundColor White
Write-Host "  Customer ID: $customerId" -ForegroundColor White
Write-Host "  Product ID: $productId" -ForegroundColor White
Write-Host "  Order ID: $orderId" -ForegroundColor White
Write-Host "  Invoice ID: $invoiceId" -ForegroundColor White
Write-Host "============================================`n" -ForegroundColor Cyan

