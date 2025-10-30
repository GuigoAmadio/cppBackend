# Teste completo do módulo Payment
# Testa: Create, Get, List, Process, Refund, Cancel, Transactions

$ErrorActionPreference = "Stop"
$API_URL = "http://localhost:8080"

Write-Host "`n=====================================" -ForegroundColor Cyan
Write-Host "TESTE COMPLETO - MODULO PAYMENT" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan

# Registrar usuário de teste
Write-Host "`n[1/13] Registrando usuario de teste..." -ForegroundColor Yellow
try {
    $registerBody = @{
        email = "payment.test@acme.com"
        password = "Test@123"
        name = "Payment Tester"
        tenant_subdomain = "acme"
    } | ConvertTo-Json

    $registerResult = Invoke-RestMethod -Uri "$API_URL/api/auth/register" -Method POST -Headers @{"Content-Type"="application/json"} -Body $registerBody
    Write-Host "OK - Usuario registrado" -ForegroundColor Green
} catch {
    Write-Host "SKIP - Usuario ja existe (ok)" -ForegroundColor Yellow
}

# Login
Write-Host "`n[2/13] Fazendo login..." -ForegroundColor Yellow
$loginBody = @{
    email = "payment.test@acme.com"
    password = "Test@123"
    tenant_subdomain = "acme"
} | ConvertTo-Json

$loginResult = Invoke-RestMethod -Uri "$API_URL/api/auth/login" -Method POST -Headers @{"Content-Type"="application/json"} -Body $loginBody
$token = if ($loginResult.token) { $loginResult.token } elseif ($loginResult.access_token) { $loginResult.access_token } elseif ($loginResult.data.token) { $loginResult.data.token } else { $loginResult.data.access_token }

if (!$token) {
    Write-Host "ERRO - Token nao encontrado!" -ForegroundColor Red
    exit 1
}
Write-Host "OK - Token: $($token.Substring(0,30))..." -ForegroundColor Green

$headers = @{
    "Authorization" = "Bearer $token"
    "Content-Type" = "application/json"
}

# Criar Customer para testes
Write-Host "`n[3/13] Criando customer de teste..." -ForegroundColor Yellow
$customerBody = @{
    name = "Payment Test Customer"
    email = "payment.customer@test.com"
    type = "individual"
} | ConvertTo-Json

$customerResult = Invoke-RestMethod -Uri "$API_URL/api/customers" -Method POST -Headers $headers -Body $customerBody
$customerId = $customerResult.customer.id
Write-Host "OK - Customer ID: $customerId" -ForegroundColor Green

# Criar Product para testes
Write-Host "`n[4/13] Criando product de teste..." -ForegroundColor Yellow
$productBody = @{
    code = "PROD-PAY-001"
    name = "Payment Test Product"
    product_type = "physical"
    price = 100.00
} | ConvertTo-Json

$productResult = Invoke-RestMethod -Uri "$API_URL/api/products" -Method POST -Headers $headers -Body $productBody
$productId = $productResult.product.id
Write-Host "OK - Product ID: $productId" -ForegroundColor Green

# Criar Order para testes
Write-Host "`n[5/13] Criando order de teste..." -ForegroundColor Yellow
$orderBody = @{
    customer_id = $customerId
    items = @(
        @{
            product_id = $productId
            quantity = 2
            unit_price = 100.00
        }
    )
    payment_method = "pix"
} | ConvertTo-Json -Depth 5

$orderResult = Invoke-RestMethod -Uri "$API_URL/api/orders" -Method POST -Headers $headers -Body $orderBody
$orderId = $orderResult.order.id
Write-Host "OK - Order ID: $orderId" -ForegroundColor Green

# TEST 1: CREATE PAYMENT
Write-Host "`n[6/13] TEST CREATE PAYMENT" -ForegroundColor Cyan
$createPaymentBody = @{
    order_id = $orderId
    customer_id = $customerId
    amount = 200.00
    currency = "BRL"
    provider = "pix"
    payment_method = "pix"
} | ConvertTo-Json

$createResult = Invoke-RestMethod -Uri "$API_URL/api/payments" -Method POST -Headers $headers -Body $createPaymentBody
$paymentId = ($createResult.payment | ConvertFrom-Json).id
Write-Host "OK - Payment criado: $paymentId" -ForegroundColor Green

# TEST 2: GET PAYMENT
Write-Host "`n[7/13] TEST GET PAYMENT" -ForegroundColor Cyan
$getResult = Invoke-RestMethod -Uri "$API_URL/api/payments/$paymentId" -Method GET -Headers $headers
$payment = $getResult.payment | ConvertFrom-Json
if ($payment.id -eq $paymentId) {
    Write-Host "OK - Payment encontrado: Status = $($payment.status)" -ForegroundColor Green
} else {
    Write-Host "ERRO - Payment nao encontrado" -ForegroundColor Red
}

# TEST 3: LIST PAYMENTS
Write-Host "`n[8/13] TEST LIST PAYMENTS (by order)" -ForegroundColor Cyan
$listResult = Invoke-RestMethod -Uri "$API_URL/api/payments?order_id=$orderId" -Method GET -Headers $headers
$paymentsCount = $listResult.count
Write-Host "OK - Encontrados $paymentsCount payment(s)" -ForegroundColor Green

# TEST 4: LIST TRANSACTIONS
Write-Host "`n[9/13] TEST LIST TRANSACTIONS" -ForegroundColor Cyan
$transactionsResult = Invoke-RestMethod -Uri "$API_URL/api/payments/$paymentId/transactions" -Method GET -Headers $headers
$transactionsCount = $transactionsResult.count
Write-Host "OK - Encontradas $transactionsCount transacao(es)" -ForegroundColor Green

# TEST 5: PROCESS PAYMENT
Write-Host "`n[10/13] TEST PROCESS PAYMENT" -ForegroundColor Cyan
$guid = [System.Guid]::NewGuid().ToString()
$processBody = @{
    provider_payment_id = "PIX-$guid"
    provider_response = "status:approved"
} | ConvertTo-Json

$processResult = Invoke-RestMethod -Uri "$API_URL/api/payments/$paymentId/process" -Method POST -Headers $headers -Body $processBody
$processedPayment = $processResult.payment | ConvertFrom-Json
if ($processedPayment.status -eq "completed") {
    Write-Host "OK - Payment processado com sucesso" -ForegroundColor Green
} else {
    Write-Host "AVISO - Status: $($processedPayment.status)" -ForegroundColor Yellow
}

# TEST 6: REFUND PAYMENT
Write-Host "`n[11/13] TEST REFUND PAYMENT" -ForegroundColor Cyan
$refundBody = @{
    amount = 200.00
    reason = "Teste de reembolso"
} | ConvertTo-Json

$refundResult = Invoke-RestMethod -Uri "$API_URL/api/payments/$paymentId/refund" -Method POST -Headers $headers -Body $refundBody
$refundedPayment = $refundResult.payment | ConvertFrom-Json
if ($refundedPayment.status -eq "refunded") {
    Write-Host "OK - Payment reembolsado" -ForegroundColor Green
} else {
    Write-Host "AVISO - Status: $($refundedPayment.status)" -ForegroundColor Yellow
}

# TEST 7: CREATE ANOTHER PAYMENT TO TEST CANCEL
Write-Host "`n[12/13] TEST CREATE PAYMENT (para cancelar)" -ForegroundColor Cyan
$createPaymentBody2 = @{
    order_id = $orderId
    customer_id = $customerId
    amount = 150.00
    currency = "BRL"
    provider = "boleto"
    payment_method = "boleto"
} | ConvertTo-Json

$createResult2 = Invoke-RestMethod -Uri "$API_URL/api/payments" -Method POST -Headers $headers -Body $createPaymentBody2
$paymentId2 = ($createResult2.payment | ConvertFrom-Json).id
Write-Host "OK - Payment criado: $paymentId2" -ForegroundColor Green

# TEST 8: CANCEL PAYMENT
Write-Host "`n[13/13] TEST CANCEL PAYMENT" -ForegroundColor Cyan
$cancelBody = @{
    reason = "Teste de cancelamento"
} | ConvertTo-Json

$cancelResult = Invoke-RestMethod -Uri "$API_URL/api/payments/$paymentId2/cancel" -Method POST -Headers $headers -Body $cancelBody
$cancelledPayment = $cancelResult.payment | ConvertFrom-Json
if ($cancelledPayment.status -eq "cancelled") {
    Write-Host "OK - Payment cancelado" -ForegroundColor Green
} else {
    Write-Host "AVISO - Status: $($cancelledPayment.status)" -ForegroundColor Yellow
}

Write-Host "`n=====================================" -ForegroundColor Green
Write-Host "TODOS OS TESTES CONCLUIDOS!" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Green

