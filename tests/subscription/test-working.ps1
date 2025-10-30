$baseUrl = "http://localhost:8080"

Write-Host "`n========== SUBSCRIPTION MODULE TEST ==========" -ForegroundColor Cyan

# Login com user existente
$loginBody = '{"email":"workspace.test@test.com","password":"Test@123","tenant_subdomain":"acme"}'
Write-Host "`n[1] Logging in..." -ForegroundColor Yellow
try {
    $loginResp = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -Body $loginBody -ContentType "application/json"
    $token = $loginResp.token
    $headers = @{"Authorization" = "Bearer $token"}
    Write-Host "✓ Logged in successfully!" -ForegroundColor Green
} catch {
    Write-Host "✗ Login failed!" -ForegroundColor Red
    exit 1
}

# Create Plan
Write-Host "`n[2] Creating Basic Plan..." -ForegroundColor Yellow
$planBody = '{"name":"Test Basic Plan","description":"Perfect for individuals","plan_type":"basic","price":29.90,"currency":"BRL","billing_cycle":"monthly","trial_days":7,"is_public":"true"}'
try {
    $planResp = Invoke-RestMethod -Uri "$baseUrl/api/subscription/plans" -Method POST -Headers $headers -Body $planBody -ContentType "application/json"
    Write-Host "✓ Plan created successfully!" -ForegroundColor Green
    $planId = ($planResp.plan | ConvertFrom-Json).id
    Write-Host "  Plan ID: $planId" -ForegroundColor Gray
} catch {
    Write-Host "✗ Failed: $_" -ForegroundColor Red
}

# List Plans
Write-Host "`n[3] Listing all plans..." -ForegroundColor Yellow
try {
    $plansResp = Invoke-RestMethod -Uri "$baseUrl/api/subscription/plans" -Method GET -Headers $headers
    Write-Host "✓ Found $($plansResp.count) plan(s)" -ForegroundColor Green
} catch {
    Write-Host "✗ Failed: $_" -ForegroundColor Red
}

# Create Customer
Write-Host "`n[4] Creating customer..." -ForegroundColor Yellow
$customerBody = '{"name":"John Test Subscriber","type":"individual","email":"john.sub@test.com","phone":"+5511987654321"}'
try {
    $customerResp = Invoke-RestMethod -Uri "$baseUrl/api/customers" -Method POST -Headers $headers -Body $customerBody -ContentType "application/json"
    Write-Host "✓ Customer created successfully!" -ForegroundColor Green
    $customerId = ($customerResp.customer | ConvertFrom-Json).id
    Write-Host "  Customer ID: $customerId" -ForegroundColor Gray
} catch {
    Write-Host "✗ Failed: $_" -ForegroundColor Red
}

if ($planId -and $customerId) {
    # Create Subscription
    Write-Host "`n[5] Creating subscription..." -ForegroundColor Yellow
    $subBody = "{`"customer_id`":`"$customerId`",`"plan_id`":`"$planId`"}"
    try {
        $subResp = Invoke-RestMethod -Uri "$baseUrl/api/subscription/subscriptions" -Method POST -Headers $headers -Body $subBody -ContentType "application/json"
        Write-Host "✓ Subscription created successfully!" -ForegroundColor Green
        $subscriptionId = ($subResp.subscription | ConvertFrom-Json).id
        Write-Host "  Subscription ID: $subscriptionId" -ForegroundColor Gray
        
        # Get Subscription
        Write-Host "`n[6] Getting subscription details..." -ForegroundColor Yellow
        try {
            $getSubResp = Invoke-RestMethod -Uri "$baseUrl/api/subscription/subscriptions/$subscriptionId" -Method GET -Headers $headers
            $subDetails = $getSubResp.subscription | ConvertFrom-Json
            Write-Host "✓ Subscription retrieved!" -ForegroundColor Green
            Write-Host "  Status: $($subDetails.status)" -ForegroundColor Gray
            Write-Host "  Amount: $($subDetails.billing_amount) $($subDetails.currency)" -ForegroundColor Gray
        } catch {
            Write-Host "✗ Failed: $_" -ForegroundColor Red
        }
        
        # Get Billing History
        Write-Host "`n[7] Getting billing history..." -ForegroundColor Yellow
        try {
            $billingResp = Invoke-RestMethod -Uri "$baseUrl/api/subscription/subscriptions/$subscriptionId/billing" -Method GET -Headers $headers
            Write-Host "✓ Found $($billingResp.count) billing record(s)" -ForegroundColor Green
        } catch {
            Write-Host "✗ Failed: $_" -ForegroundColor Red
        }
    } catch {
        Write-Host "✗ Failed: $_" -ForegroundColor Red
    }
}

# Get Stats
Write-Host "`n[8] Getting subscription statistics..." -ForegroundColor Yellow
try {
    $statsResp = Invoke-RestMethod -Uri "$baseUrl/api/subscription/stats" -Method GET -Headers $headers
    Write-Host "✓ Statistics retrieved!" -ForegroundColor Green
    Write-Host "  Total subscriptions: $($statsResp.total_subscriptions)" -ForegroundColor Gray
    Write-Host "  Active subscriptions: $($statsResp.active_subscriptions)" -ForegroundColor Gray
    Write-Host "  Total plans: $($statsResp.total_plans)" -ForegroundColor Gray
    Write-Host "  Active plans: $($statsResp.active_plans)" -ForegroundColor Gray
} catch {
    Write-Host "✗ Failed: $_" -ForegroundColor Red
}

Write-Host "`n========== TESTS COMPLETED ==========`n" -ForegroundColor Green

