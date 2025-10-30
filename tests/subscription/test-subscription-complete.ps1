# ================================================
# TEST: Subscription Module - COMPLETE TEST
# ================================================

$baseUrl = "http://localhost:8080"
$testResults = @()
$testCounter = 0

function Test-Endpoint {
    param(
        [string]$Name,
        [string]$Method,
        [string]$Url,
        [hashtable]$Headers = @{},
        [string]$Body = $null,
        [int]$ExpectedStatus = 200
    )
    
    $script:testCounter++
    Write-Host "`n[$script:testCounter] $Name..." -ForegroundColor Cyan
    
    try {
        $params = @{
            Uri = $Url
            Method = $Method
            Headers = $Headers
            ContentType = "application/json"
            ErrorAction = "Stop"
        }
        
        if ($Body) {
            $params.Body = $Body
        }
        
        $response = Invoke-WebRequest @params
        $content = $response.Content | ConvertFrom-Json
        
        if ($response.StatusCode -eq $ExpectedStatus) {
            Write-Host "✓ PASSED ($($response.StatusCode))" -ForegroundColor Green
            $script:testResults += @{Name=$Name; Status="PASSED"}
            return $content
        } else {
            Write-Host "✗ FAILED (Expected $ExpectedStatus, got $($response.StatusCode))" -ForegroundColor Red
            $script:testResults += @{Name=$Name; Status="FAILED"}
            return $null
        }
    } catch {
        Write-Host "✗ FAILED ($($_.Exception.Message))" -ForegroundColor Red
        $script:testResults += @{Name=$Name; Status="FAILED"}
        return $null
    }
}

Write-Host "`n========================================" -ForegroundColor Yellow
Write-Host "  SUBSCRIPTION MODULE - COMPLETE TEST" -ForegroundColor Yellow
Write-Host "========================================`n" -ForegroundColor Yellow

# ===== STEP 1: REGISTER & LOGIN =====
Write-Host "`n===== STEP 1: Authentication =====" -ForegroundColor Magenta

$registerBody = @{
    email = "test-sub@test.com"
    password = "Test@123456"
    full_name = "Subscription Test User"
    role = "admin"
} | ConvertTo-Json

$registerResult = Test-Endpoint -Name "Register User" -Method POST -Url "$baseUrl/api/auth/register" -Body $registerBody -ExpectedStatus 201

if (!$registerResult) {
    Write-Host "`nAuthentication failed! Exiting..." -ForegroundColor Red
    exit 1
}

$loginBody = @{
    email = "test-sub@test.com"
    password = "Test@123456"
    tenant_subdomain = "acme"
} | ConvertTo-Json

$loginResult = Test-Endpoint -Name "Login User" -Method POST -Url "$baseUrl/api/auth/login" -Body $loginBody

if (!$loginResult -or !$loginResult.token) {
    Write-Host "`nLogin failed! Exiting..." -ForegroundColor Red
    exit 1
}

$token = $loginResult.token
$authHeaders = @{
    "Authorization" = "Bearer $token"
}

Write-Host "`n✓ Authentication successful!" -ForegroundColor Green

# ===== STEP 2: CREATE PLANS =====
Write-Host "`n===== STEP 2: Create Subscription Plans =====" -ForegroundColor Magenta

$basicPlanBody = @{
    name = "Basic Plan"
    description = "Perfect for individuals"
    plan_type = "basic"
    price = 29.90
    currency = "BRL"
    billing_cycle = "monthly"
    trial_days = 7
    features = '{"max_users": 1, "storage_gb": 10}'
    limits = '{"projects": 5}'
    is_public = "true"
} | ConvertTo-Json

$basicPlan = Test-Endpoint -Name "Create Basic Plan" -Method POST -Url "$baseUrl/api/subscription/plans" -Headers $authHeaders -Body $basicPlanBody -ExpectedStatus 201

$proPlanBody = @{
    name = "Pro Plan"
    description = "For growing teams"
    plan_type = "pro"
    price = 99.90
    currency = "BRL"
    billing_cycle = "monthly"
    trial_days = 14
    features = '{"max_users": 10, "storage_gb": 100}'
    limits = '{"projects": 50}'
    is_public = "true"
} | ConvertTo-Json

$proPlan = Test-Endpoint -Name "Create Pro Plan" -Method POST -Url "$baseUrl/api/subscription/plans" -Headers $authHeaders -Body $proPlanBody -ExpectedStatus 201

# ===== STEP 3: LIST PLANS =====
Write-Host "`n===== STEP 3: List Plans =====" -ForegroundColor Magenta

$allPlans = Test-Endpoint -Name "List All Plans" -Method GET -Url "$baseUrl/api/subscription/plans" -Headers $authHeaders

$activePlans = Test-Endpoint -Name "List Active Plans" -Method GET -Url "$baseUrl/api/subscription/plans?active=true" -Headers $authHeaders

$publicPlans = Test-Endpoint -Name "List Public Plans" -Method GET -Url "$baseUrl/api/subscription/plans?active=true&public=true" -Headers $authHeaders

# ===== STEP 4: CREATE CUSTOMER =====
Write-Host "`n===== STEP 4: Create Customer =====" -ForegroundColor Magenta

$customerBody = @{
    name = "John Doe Subscriber"
    type = "individual"
    email = "john.subscriber@test.com"
    phone = "+55 11 98765-4321"
} | ConvertTo-Json

$customer = Test-Endpoint -Name "Create Customer" -Method POST -Url "$baseUrl/api/customers" -Headers $authHeaders -Body $customerBody -ExpectedStatus 201

if (!$customer) {
    Write-Host "`nFailed to create customer! Exiting..." -ForegroundColor Red
    exit 1
}

$customerId = ($customer.customer | ConvertFrom-Json).id
$basicPlanId = ($basicPlan.plan | ConvertFrom-Json).id
$proPlanId = ($proPlan.plan | ConvertFrom-Json).id

Write-Host "`nCustomer ID: $customerId" -ForegroundColor White
Write-Host "Basic Plan ID: $basicPlanId" -ForegroundColor White
Write-Host "Pro Plan ID: $proPlanId" -ForegroundColor White

# ===== STEP 5: CREATE SUBSCRIPTION =====
Write-Host "`n===== STEP 5: Create Subscription =====" -ForegroundColor Magenta

$subscriptionBody = @{
    customer_id = $customerId
    plan_id = $basicPlanId
} | ConvertTo-Json

$subscription = Test-Endpoint -Name "Create Subscription" -Method POST -Url "$baseUrl/api/subscription/subscriptions" -Headers $authHeaders -Body $subscriptionBody -ExpectedStatus 201

if (!$subscription) {
    Write-Host "`nFailed to create subscription! Exiting..." -ForegroundColor Red
    exit 1
}

$subscriptionId = ($subscription.subscription | ConvertFrom-Json).id
Write-Host "`nSubscription ID: $subscriptionId" -ForegroundColor White

# ===== STEP 6: GET SUBSCRIPTION =====
Write-Host "`n===== STEP 6: Get Subscription =====" -ForegroundColor Magenta

$getSubscription = Test-Endpoint -Name "Get Subscription by ID" -Method GET -Url "$baseUrl/api/subscription/subscriptions/$subscriptionId" -Headers $authHeaders

# ===== STEP 7: LIST SUBSCRIPTIONS =====
Write-Host "`n===== STEP 7: List Subscriptions =====" -ForegroundColor Magenta

$allSubscriptions = Test-Endpoint -Name "List All Subscriptions" -Method GET -Url "$baseUrl/api/subscription/subscriptions" -Headers $authHeaders

$customerSubscriptions = Test-Endpoint -Name "List Customer Subscriptions" -Method GET -Url "$baseUrl/api/subscription/subscriptions?customer_id=$customerId" -Headers $authHeaders

# ===== STEP 8: GET SUBSCRIPTION STATS =====
Write-Host "`n===== STEP 8: Get Statistics =====" -ForegroundColor Magenta

$stats = Test-Endpoint -Name "Get Subscription Stats" -Method GET -Url "$baseUrl/api/subscription/stats" -Headers $authHeaders

# ===== STEP 9: GET BILLING HISTORY =====
Write-Host "`n===== STEP 9: Get Billing History =====" -ForegroundColor Magenta

$billingHistory = Test-Endpoint -Name "Get Billing History" -Method GET -Url "$baseUrl/api/subscription/subscriptions/$subscriptionId/billing" -Headers $authHeaders

# ===== STEP 10: PROCESS RENEWAL =====
Write-Host "`n===== STEP 10: Process Renewal (Payment Integration) =====" -ForegroundColor Magenta

$renewalResult = Test-Endpoint -Name "Process Subscription Renewal" -Method POST -Url "$baseUrl/api/subscription/subscriptions/$subscriptionId/renew" -Headers $authHeaders

# ===== STEP 11: CANCEL SUBSCRIPTION =====
Write-Host "`n===== STEP 11: Cancel Subscription =====" -ForegroundColor Magenta

$cancelBody = @{
    reason = "Customer requested cancellation"
    immediate = "false"
} | ConvertTo-Json

$cancelResult = Test-Endpoint -Name "Cancel Subscription (End of Period)" -Method POST -Url "$baseUrl/api/subscription/subscriptions/$subscriptionId/cancel" -Headers $authHeaders -Body $cancelBody

# ===== FINAL RESULTS =====
Write-Host "`n========================================" -ForegroundColor Yellow
Write-Host "  FINAL RESULTS" -ForegroundColor Yellow
Write-Host "========================================`n" -ForegroundColor Yellow

$passed = ($testResults | Where-Object { $_.Status -eq "PASSED" }).Count
$failed = ($testResults | Where-Object { $_.Status -eq "FAILED" }).Count
$total = $testResults.Count

Write-Host "PASSED: $passed/$total" -ForegroundColor Green
Write-Host "FAILED: $failed/$total" -ForegroundColor Red

if ($failed -eq 0) {
    Write-Host "`n🎉 ALL TESTS PASSED! SUBSCRIPTION MODULE IS 100% FUNCTIONAL! 🎉" -ForegroundColor Green
} else {
    Write-Host "`n⚠️  Some tests failed. Review the logs above." -ForegroundColor Yellow
}

Write-Host "`n========================================`n" -ForegroundColor Yellow

