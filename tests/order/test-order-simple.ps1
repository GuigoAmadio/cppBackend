# ==============================================================================
# TEST ORDER MODULE - Simple Test (sem rate limit)
# ==============================================================================

$ErrorActionPreference = "Stop"
$baseUrl = "http://localhost:8080"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   ORDER MODULE - SIMPLE TEST" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$testsRun = 0
$testsPassed = 0

function Test-Endpoint {
    param(
        [string]$Name,
        [string]$Method,
        [string]$Url,
        [object]$Body = $null,
        [hashtable]$Headers = @{},
        [int]$ExpectedStatus = 200
    )
    
    $script:testsRun++
    Write-Host "[$script:testsRun] $Name" -ForegroundColor Yellow
    
    try {
        $params = @{
            Uri = $Url
            Method = $Method
            Headers = $Headers
            ContentType = "application/json"
            TimeoutSec = 10
        }
        
        if ($Body) {
            $params.Body = ($Body | ConvertTo-Json -Depth 10)
        }
        
        $response = Invoke-WebRequest @params -UseBasicParsing
        $result = $response.Content | ConvertFrom-Json
        
        if ($response.StatusCode -eq $ExpectedStatus) {
            Write-Host "   OK - Status $($response.StatusCode)" -ForegroundColor Green
            $script:testsPassed++
            return $result
        }
    } catch {
        $statusCode = $_.Exception.Response.StatusCode.value__
        if ($statusCode -eq $ExpectedStatus) {
            Write-Host "   OK - Status $statusCode (expected)" -ForegroundColor Green
            $script:testsPassed++
            return $null
        } else {
            Write-Host "   FALHOU - Expected $ExpectedStatus, got $statusCode" -ForegroundColor Red
            if ($_.ErrorDetails.Message) {
                try {
                    $errorJson = $_.ErrorDetails.Message | ConvertFrom-Json
                    Write-Host "   Error: $($errorJson.error)" -ForegroundColor Yellow
                } catch {
                    Write-Host "   Error: $($_.ErrorDetails.Message)" -ForegroundColor Yellow
                }
            } else {
                Write-Host "   Error: $($_.Exception.Message)" -ForegroundColor Yellow
            }
            return $null
        }
    }
}

# ==============================================================================
# 1. LOGIN (usuário pré-criado)
# ==============================================================================
Write-Host "`n1. LOGIN (usuário existente)" -ForegroundColor Cyan

$loginBody = @{
    email = "ordertest@acme.com"
    password = "Test@123456"
    tenant_subdomain = "acme"
}

$loginResult = Test-Endpoint -Name "Login" -Method "POST" -Url "$baseUrl/api/auth/login" -Body $loginBody

if (-not $loginResult -or -not $loginResult.token) {
    Write-Host "❌ LOGIN FALHOU! Tentando criar usuário via API..." -ForegroundColor Red
    
    # Tentar registrar
    $registerBody = @{
        email = "ordertest@acme.com"
        password = "Test@123456"
        name = "Order Test User"
        tenant_subdomain = "acme"
    }
    
    try {
        $registerResult = Test-Endpoint -Name "Register User" -Method "POST" -Url "$baseUrl/api/auth/register" -Body $registerBody -ExpectedStatus 201
        if ($registerResult) {
            $userId = $registerResult.user.id
            $env:PGPASSWORD='postgre123'
            $sql = "INSERT INTO user_tenants (user_id, tenant_id, role) VALUES ('$userId', (SELECT id FROM tenants WHERE subdomain='acme'), 'admin') ON CONFLICT DO NOTHING;"
            psql -h localhost -p 5433 -U moneymaker_user -d moneymaker_dev -c $sql 2>&1 | Out-Null
            Start-Sleep -Seconds 2
            
            # Tentar login novamente
            $loginResult = Test-Endpoint -Name "Login (after register)" -Method "POST" -Url "$baseUrl/api/auth/login" -Body $loginBody
        }
    } catch {
        Write-Host "❌ Não foi possível criar usuário. Abortando." -ForegroundColor Red
        exit 1
    }
}

if (-not $loginResult -or -not $loginResult.token) {
    Write-Host "❌ LOGIN FALHOU! Abortando testes." -ForegroundColor Red
    exit 1
}

$token = $loginResult.token
$authHeaders = @{
    "Authorization" = "Bearer $token"
}

Write-Host "✅ Autenticado com sucesso!" -ForegroundColor Green

# ==============================================================================
# 2. CREATE CUSTOMER
# ==============================================================================
Write-Host "`n2. CREATE CUSTOMER" -ForegroundColor Cyan

$customerBody = @{
    name = "Simple Test Customer"
    email = "simplecust@test.com"
    document_number = "98765432109"
    type = "individual"
}

$customerResult = Test-Endpoint -Name "Create Customer" -Method "POST" -Url "$baseUrl/api/customers" -Body $customerBody -Headers $authHeaders -ExpectedStatus 201

if (-not $customerResult) {
    Write-Host "❌ Falha ao criar customer. Abortando." -ForegroundColor Red
    exit 1
}

$customerId = $customerResult.id
Write-Host "   Customer ID: $customerId" -ForegroundColor White

# ==============================================================================
# 3. CREATE PRODUCT
# ==============================================================================
Write-Host "`n3. CREATE PRODUCT" -ForegroundColor Cyan

$productBody = @{
    name = "Simple Test Product"
    code = "TEST-001"
    price = 100.00
    stock_quantity = 50
    type = "physical"
}

$productResult = Test-Endpoint -Name "Create Product" -Method "POST" -Url "$baseUrl/api/products" -Body $productBody -Headers $authHeaders -ExpectedStatus 201

if (-not $productResult) {
    Write-Host "❌ Falha ao criar product. Abortando." -ForegroundColor Red
    exit 1
}

$productId = $productResult.id
Write-Host "   Product ID: $productId" -ForegroundColor White

# ==============================================================================
# 4. CREATE ORDER
# ==============================================================================
Write-Host "`n4. CREATE ORDER" -ForegroundColor Cyan

$orderBody = @{
    customer_id = $customerId
    payment_method = "pix"
    items = @(
        @{
            product_id = $productId
            product_name = "Simple Test Product"
            product_code = "TEST-001"
            quantity = 2
            unit_price = 100.00
        }
    )
    shipping_cost = 10.00
}

Write-Host "   Request Body:" -ForegroundColor Gray
Write-Host "   $(($orderBody | ConvertTo-Json -Compress))" -ForegroundColor Gray

$createResult = Test-Endpoint -Name "Create Order" -Method "POST" -Url "$baseUrl/api/orders" -Body $orderBody -Headers $authHeaders -ExpectedStatus 201

if (-not $createResult) {
    Write-Host "❌ Falha ao criar order. Abortando." -ForegroundColor Red
    exit 1
}

$orderId = $createResult.order_id
$orderNumber = $createResult.order_number
Write-Host "   Order ID: $orderId" -ForegroundColor White
Write-Host "   Order Number: $orderNumber" -ForegroundColor White

# ==============================================================================
# 5. GET ORDER
# ==============================================================================
Write-Host "`n5. GET ORDER" -ForegroundColor Cyan

$getResult = Test-Endpoint -Name "Get Order by ID" -Method "GET" -Url "$baseUrl/api/orders/$orderId" -Headers $authHeaders

if ($getResult) {
    Write-Host "   Order Number: $($getResult.order_number)" -ForegroundColor White
    Write-Host "   Status: $($getResult.status)" -ForegroundColor White
    Write-Host "   Total: $($getResult.total)" -ForegroundColor White
    Write-Host "   Items: $($getResult.items.Count)" -ForegroundColor White
}

# ==============================================================================
# 6. LIST ORDERS
# ==============================================================================
Write-Host "`n6. LIST ORDERS" -ForegroundColor Cyan

$listResult = Test-Endpoint -Name "List Orders" -Method "GET" -Url "$baseUrl/api/orders" -Headers $authHeaders

if ($listResult) {
    Write-Host "   Total: $($listResult.total)" -ForegroundColor White
}

# ==============================================================================
# 7. UPDATE STATUS
# ==============================================================================
Write-Host "`n7. UPDATE ORDER STATUS" -ForegroundColor Cyan

$statusBody = @{
    status = "confirmed"
}

Test-Endpoint -Name "Update to Confirmed" -Method "PUT" -Url "$baseUrl/api/orders/$orderId/status" -Body $statusBody -Headers $authHeaders

# ==============================================================================
# 8. UPDATE PAYMENT
# ==============================================================================
Write-Host "`n8. UPDATE PAYMENT STATUS" -ForegroundColor Cyan

$paymentBody = @{
    payment_status = "paid"
}

Test-Endpoint -Name "Update to Paid" -Method "PUT" -Url "$baseUrl/api/orders/$orderId/payment" -Body $paymentBody -Headers $authHeaders

# ==============================================================================
# 9. CANCEL ORDER
# ==============================================================================
Write-Host "`n9. CREATE & CANCEL ORDER" -ForegroundColor Cyan

$newOrderBody = @{
    customer_id = $customerId
    items = @(
        @{
            product_id = $productId
            product_name = "Simple Test Product"
            quantity = 1
            unit_price = 100.00
        }
    )
}

$newOrderResult = Test-Endpoint -Name "Create Order to Cancel" -Method "POST" -Url "$baseUrl/api/orders" -Body $newOrderBody -Headers $authHeaders -ExpectedStatus 201

if ($newOrderResult) {
    $cancelOrderId = $newOrderResult.order_id
    
    $cancelBody = @{
        reason = "Test cancellation"
    }
    
    Test-Endpoint -Name "Cancel Order" -Method "POST" -Url "$baseUrl/api/orders/$cancelOrderId/cancel" -Body $cancelBody -Headers $authHeaders
}

# ==============================================================================
# SUMMARY
# ==============================================================================
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "           TEST SUMMARY" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Tests Run:    $testsRun" -ForegroundColor White
Write-Host "Tests Passed: $testsPassed" -ForegroundColor Green
Write-Host "Success Rate: $([math]::Round(($testsPassed/$testsRun)*100, 2))%" -ForegroundColor $(if ($testsPassed -eq $testsRun) { "Green" } else { "Yellow" })
Write-Host "========================================`n" -ForegroundColor Cyan

if ($testsPassed -eq $testsRun) {
    Write-Host "🎉 TODOS OS TESTES PASSARAM!" -ForegroundColor Green
} else {
    Write-Host "⚠️  Alguns testes falharam" -ForegroundColor Yellow
}

