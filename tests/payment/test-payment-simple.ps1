# Teste simples do módulo Payment
# Usa dados já criados (customer e order)

$ErrorActionPreference = "Stop"
$API_URL = "http://localhost:8080"

Write-Host "`n===================================" -ForegroundColor Cyan
Write-Host "TESTE SIMPLES - MODULO PAYMENT" -ForegroundColor Cyan
Write-Host "===================================" -ForegroundColor Cyan

# Login
Write-Host "`n[1/9] Fazendo login..." -ForegroundColor Yellow
$loginBody = @{
    email = "payment.test@acme.com"
    password = "Test@123"
    tenant_subdomain = "acme"
} | ConvertTo-Json

$loginResult = Invoke-RestMethod -Uri "$API_URL/api/auth/login" -Method POST -Headers @{"Content-Type"="application/json"} -Body $loginBody
$token = if ($loginResult.token) { $loginResult.token } elseif ($loginResult.access_token) { $loginResult.access_token } else { $loginResult.data.access_token }

Write-Host "OK - Token obtido" -ForegroundColor Green

$headers = @{
    "Authorization" = "Bearer $token"
    "Content-Type" = "application/json"
}

# Buscar um customer existente
Write-Host "`n[2/9] Buscando customers..." -ForegroundColor Yellow
$customersResult = Invoke-RestMethod -Uri "$API_URL/api/customers" -Method GET -Headers $headers
$customerId = $customersResult.customers[0].id
Write-Host "OK - Customer ID: $customerId" -ForegroundColor Green

# Buscar uma order existente
Write-Host "`n[3/9] Buscando orders..." -ForegroundColor Yellow
$ordersResult = Invoke-RestMethod -Uri "$API_URL/api/orders" -Method GET -Headers $headers
$orderId = $ordersResult.orders[0].id
Write-Host "OK - Order ID: $orderId" -ForegroundColor Green

# TEST 1: CREATE PAYMENT
Write-Host "`n[4/9] TEST CREATE PAYMENT" -ForegroundColor Cyan
$createPaymentBody = "{`"order_id`":`"$orderId`",`"customer_id`":`"$customerId`",`"amount`":250.00,`"currency`":`"BRL`",`"provider`":`"pix`",`"payment_method`":`"pix`"}"

$createResult = Invoke-RestMethod -Uri "$API_URL/api/payments" -Method POST -Headers $headers -Body $createPaymentBody
$payment = $createResult.payment | ConvertFrom-Json
$paymentId = $payment.id
Write-Host "OK - Payment criado: $paymentId" -ForegroundColor Green

# TEST 2: GET PAYMENT
Write-Host "`n[5/9] TEST GET PAYMENT" -ForegroundColor Cyan
$getResult = Invoke-RestMethod -Uri "$API_URL/api/payments/$paymentId" -Method GET -Headers $headers
$payment = $getResult.payment | ConvertFrom-Json
Write-Host "OK - Payment encontrado: Status = $($payment.status)" -ForegroundColor Green

# TEST 3: LIST PAYMENTS
Write-Host "`n[6/9] TEST LIST PAYMENTS" -ForegroundColor Cyan
$listResult = Invoke-RestMethod -Uri "$API_URL/api/payments" -Method GET -Headers $headers
$paymentsCount = $listResult.count
Write-Host "OK - Encontrados $paymentsCount payment(s)" -ForegroundColor Green

# TEST 4: LIST TRANSACTIONS
Write-Host "`n[7/9] TEST LIST TRANSACTIONS" -ForegroundColor Cyan
$transactionsResult = Invoke-RestMethod -Uri "$API_URL/api/payments/$paymentId/transactions" -Method GET -Headers $headers
$transactionsCount = $transactionsResult.count
Write-Host "OK - Encontradas $transactionsCount transacao(es)" -ForegroundColor Green

# TEST 5: PROCESS PAYMENT
Write-Host "`n[8/9] TEST PROCESS PAYMENT" -ForegroundColor Cyan
$guid = [System.Guid]::NewGuid().ToString()
$processBody = @{
    provider_payment_id = "PIX-$guid"
    provider_response = "approved"
} | ConvertTo-Json

$processResult = Invoke-RestMethod -Uri "$API_URL/api/payments/$paymentId/process" -Method POST -Headers $headers -Body $processBody
$processedPayment = $processResult.payment | ConvertFrom-Json
Write-Host "OK - Payment processado: Status = $($processedPayment.status)" -ForegroundColor Green

# TEST 6: REFUND PAYMENT
Write-Host "`n[9/9] TEST REFUND PAYMENT" -ForegroundColor Cyan
$refundBody = @{
    amount = 250.00
    reason = "Teste de reembolso automatizado"
} | ConvertTo-Json

$refundResult = Invoke-RestMethod -Uri "$API_URL/api/payments/$paymentId/refund" -Method POST -Headers $headers -Body $refundBody
$refundedPayment = $refundResult.payment | ConvertFrom-Json
Write-Host "OK - Payment reembolsado: Status = $($refundedPayment.status)" -ForegroundColor Green

Write-Host "`n===================================" -ForegroundColor Green
Write-Host "TODOS OS TESTES PASSARAM! (6/6)" -ForegroundColor Green
Write-Host "===================================" -ForegroundColor Green

