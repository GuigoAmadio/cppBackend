# ==============================================================================
# TEST ORDER MODULE - Complete E-commerce Flow
# ==============================================================================

$ErrorActionPreference = "Stop"
$baseUrl = "http://localhost:8080"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   ORDER MODULE - COMPLETE TESTS" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Track results
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
            }
            return $null
        }
    }
}

# ==============================================================================
# 1. REGISTER + LOGIN
# ==============================================================================
Write-Host "`n1. REGISTER + LOGIN" -ForegroundColor Cyan

$testEmail = "order.test.$(Get-Random -Maximum 9999)@acme.com"
$testPassword = "Test@123456"

$registerBody = @{
    email = $testEmail
    password = $testPassword
    name = "Order Test User"
    tenant_subdomain = "acme"
}

$registerResult = Test-Endpoint -Name "Register User" -Method "POST" -Url "$baseUrl/api/auth/register" -Body $registerBody -ExpectedStatus 201

if (-not $registerResult) {
    Write-Host "REGISTER FALHOU!" -ForegroundColor Red
    exit 1
}

$userId = $registerResult.user.id
$env:PGPASSWORD='postgre123'
$sql = "INSERT INTO user_tenants (user_id, tenant_id, role) VALUES ('$userId', (SELECT id FROM tenants WHERE subdomain='acme'), 'admin') ON CONFLICT DO NOTHING;"
psql -h localhost -p 5433 -U moneymaker_user -d moneymaker_dev -c $sql 2>&1 | Out-Null

Start-Sleep -Seconds 1

$loginBody = @{
    email = $testEmail
    password = $testPassword
    tenant_subdomain = "acme"
}

$loginResult = Test-Endpoint -Name "Login" -Method "POST" -Url "$baseUrl/api/auth/login" -Body $loginBody

$token = $loginResult.token
$authHeaders = @{
    "Authorization" = "Bearer $token"
}

# ==============================================================================
# 2. CREATE CUSTOMER
# ==============================================================================
Write-Host "`n2. CREATE CUSTOMER (for orders)" -ForegroundColor Cyan

$customerBody = @{
    name = "Test Customer for Orders"
    email = "ordercust@test.com"
    document_number = "12345678901"
    type = "individual"
    phone = "11999887766"
}

$customerResult = Test-Endpoint -Name "Create Customer" -Method "POST" -Url "$baseUrl/api/customers" -Body $customerBody -Headers $authHeaders -ExpectedStatus 201

$customerId = $customerResult.id
Write-Host "   Customer ID: $customerId" -ForegroundColor White

# ==============================================================================
# 3. CREATE PRODUCT
# ==============================================================================
Write-Host "`n3. CREATE PRODUCT (for order items)" -ForegroundColor Cyan

$productBody = @{
    name = "Laptop Dell XPS"
    code = "DELL-XPS-001"
    price = 4999.99
    stock_quantity = 10
    type = "physical"
}

$productResult = Test-Endpoint -Name "Create Product" -Method "POST" -Url "$baseUrl/api/products" -Body $productBody -Headers $authHeaders -ExpectedStatus 201

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
            product_name = "Laptop Dell XPS"
            product_code = "DELL-XPS-001"
            quantity = 2
            unit_price = 4999.99
        }
    )
    shipping_cost = 50.00
    discount = 100.00
    tax = 200.00
    shipping_address = "Rua Teste 123"
    shipping_city = "Sao Paulo"
    shipping_state = "SP"
    shipping_zip_code = "01234-567"
    customer_notes = "Entregar na portaria"
}

$createResult = Test-Endpoint -Name "Create Order" -Method "POST" -Url "$baseUrl/api/orders" -Body $orderBody -Headers $authHeaders -ExpectedStatus 201

$orderId = $createResult.order_id
$orderNumber = $createResult.order_number
Write-Host "   Order ID: $orderId" -ForegroundColor White
Write-Host "   Order Number: $orderNumber" -ForegroundColor White

# ==============================================================================
# 5. GET ORDER
# ==============================================================================
Write-Host "`n5. GET ORDER BY ID" -ForegroundColor Cyan

$getResult = Test-Endpoint -Name "Get Order" -Method "GET" -Url "$baseUrl/api/orders/$orderId" -Headers $authHeaders

if ($getResult) {
    Write-Host "   Order Number: $($getResult.order_number)" -ForegroundColor White
    Write-Host "   Status: $($getResult.status)" -ForegroundColor White
    Write-Host "   Payment Status: $($getResult.payment_status)" -ForegroundColor White
    Write-Host "   Total: $($getResult.total)" -ForegroundColor White
    Write-Host "   Items Count: $($getResult.items.Count)" -ForegroundColor White
}

# ==============================================================================
# 6. LIST ORDERS
# ==============================================================================
Write-Host "`n6. LIST ALL ORDERS" -ForegroundColor Cyan

$listResult = Test-Endpoint -Name "List Orders" -Method "GET" -Url "$baseUrl/api/orders" -Headers $authHeaders

if ($listResult) {
    Write-Host "   Total Orders: $($listResult.total)" -ForegroundColor White
}

# ==============================================================================
# 7. LIST CUSTOMER ORDERS
# ==============================================================================
Write-Host "`n7. LIST CUSTOMER ORDERS" -ForegroundColor Cyan

$customerOrdersResult = Test-Endpoint -Name "List Customer Orders" -Method "GET" -Url "$baseUrl/api/customers/$customerId/orders" -Headers $authHeaders

if ($customerOrdersResult) {
    Write-Host "   Customer Orders: $($customerOrdersResult.total)" -ForegroundColor White
}

# ==============================================================================
# 8. UPDATE ORDER STATUS
# ==============================================================================
Write-Host "`n8. UPDATE ORDER STATUS" -ForegroundColor Cyan

$statusBody = @{
    status = "confirmed"
}

$statusResult = Test-Endpoint -Name "Update Order Status to Confirmed" -Method "PUT" -Url "$baseUrl/api/orders/$orderId/status" -Body $statusBody -Headers $authHeaders

# Verify status changed
$verifyResult = Test-Endpoint -Name "Verify Status Changed" -Method "GET" -Url "$baseUrl/api/orders/$orderId" -Headers $authHeaders

if ($verifyResult -and $verifyResult.status -eq "confirmed") {
    Write-Host "   Status correctly updated to confirmed" -ForegroundColor Green
    $script:testsPassed++
}

# ==============================================================================
# 9. UPDATE PAYMENT STATUS
# ==============================================================================
Write-Host "`n9. UPDATE PAYMENT STATUS" -ForegroundColor Cyan

$paymentBody = @{
    payment_status = "paid"
}

$paymentResult = Test-Endpoint -Name "Update Payment Status to Paid" -Method "PUT" -Url "$baseUrl/api/orders/$orderId/payment" -Body $paymentBody -Headers $authHeaders

# ==============================================================================
# 10. UPDATE STATUS TO PROCESSING
# ==============================================================================
Write-Host "`n10. UPDATE TO PROCESSING" -ForegroundColor Cyan

$processingBody = @{
    status = "processing"
}

Test-Endpoint -Name "Update to Processing" -Method "PUT" -Url "$baseUrl/api/orders/$orderId/status" -Body $processingBody -Headers $authHeaders

# ==============================================================================
# 11. UPDATE STATUS TO SHIPPED
# ==============================================================================
Write-Host "`n11. UPDATE TO SHIPPED" -ForegroundColor Cyan

$shippedBody = @{
    status = "shipped"
}

Test-Endpoint -Name "Update to Shipped" -Method "PUT" -Url "$baseUrl/api/orders/$orderId/status" -Body $shippedBody -Headers $authHeaders

# ==============================================================================
# 12. UPDATE STATUS TO DELIVERED
# ==============================================================================
Write-Host "`n12. UPDATE TO DELIVERED" -ForegroundColor Cyan

$deliveredBody = @{
    status = "delivered"
}

Test-Endpoint -Name "Update to Delivered" -Method "PUT" -Url "$baseUrl/api/orders/$orderId/status" -Body $deliveredBody -Headers $authHeaders

# ==============================================================================
# 13. TRY TO CANCEL DELIVERED ORDER (should fail)
# ==============================================================================
Write-Host "`n13. TRY CANCEL DELIVERED ORDER (should fail)" -ForegroundColor Cyan

$cancelBody = @{
    reason = "Test cancellation"
}

Test-Endpoint -Name "Cancel Delivered Order (should fail)" -Method "POST" -Url "$baseUrl/api/orders/$orderId/cancel" -Body $cancelBody -Headers $authHeaders -ExpectedStatus 400

# ==============================================================================
# 14. CREATE ANOTHER ORDER AND CANCEL IT
# ==============================================================================
Write-Host "`n14. CREATE & CANCEL NEW ORDER" -ForegroundColor Cyan

$newOrderBody = @{
    customer_id = $customerId
    payment_method = "credit_card"
    items = @(
        @{
            product_id = $productId
            product_name = "Laptop Dell XPS"
            quantity = 1
            unit_price = 4999.99
        }
    )
}

$newOrderResult = Test-Endpoint -Name "Create New Order" -Method "POST" -Url "$baseUrl/api/orders" -Body $newOrderBody -Headers $authHeaders -ExpectedStatus 201

$newOrderId = $newOrderResult.order_id

$cancelBody2 = @{
    reason = "Changed mind"
}

Test-Endpoint -Name "Cancel New Order" -Method "POST" -Url "$baseUrl/api/orders/$newOrderId/cancel" -Body $cancelBody2 -Headers $authHeaders

# Verify cancelled
$verifyCancelResult = Test-Endpoint -Name "Verify Cancelled" -Method "GET" -Url "$baseUrl/api/orders/$newOrderId" -Headers $authHeaders

if ($verifyCancelResult -and $verifyCancelResult.status -eq "cancelled") {
    Write-Host "   Order correctly cancelled" -ForegroundColor Green
    $script:testsPassed++
}

# ==============================================================================
# SUMMARY
# ==============================================================================
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "           TEST SUMMARY" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Tests Run:    $testsRun" -ForegroundColor White
Write-Host "Tests Passed: $testsPassed" -ForegroundColor Green
Write-Host "Success Rate: $([math]::Round(($testsPassed/$testsRun)*100, 2))%" -ForegroundColor Yellow
Write-Host "========================================`n" -ForegroundColor Cyan

