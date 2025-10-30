$baseUrl = "http://localhost:8080"

Write-Host "========== SUBSCRIPTION MODULE TEST ==========" -ForegroundColor Cyan

# Register
$registerBody = '{"email":"testsub@test.com","password":"Test@123","full_name":"Sub Test","role":"admin"}'
Write-Host "`n[1] Registering user..." -ForegroundColor Yellow
$registerResp = Invoke-RestMethod -Uri "$baseUrl/api/auth/register" -Method POST -Body $registerBody -ContentType "application/json" -ErrorAction SilentlyContinue

# Login
$loginBody = '{"email":"testsub@test.com","password":"Test@123","tenant_subdomain":"acme"}'
Write-Host "[2] Logging in..." -ForegroundColor Yellow
$loginResp = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -Body $loginBody -ContentType "application/json"
$token = $loginResp.token
$headers = @{"Authorization" = "Bearer $token"}

Write-Host "Token: $($token.Substring(0, 20))..." -ForegroundColor Green

# Create Plan
$planBody = '{"name":"Basic Plan","description":"Test plan","plan_type":"basic","price":29.90,"currency":"BRL","billing_cycle":"monthly","trial_days":7,"is_public":"true"}'
Write-Host "`n[3] Creating plan..." -ForegroundColor Yellow
$planResp = Invoke-RestMethod -Uri "$baseUrl/api/subscription/plans" -Method POST -Headers $headers -Body $planBody -ContentType "application/json"
Write-Host "Plan created!" -ForegroundColor Green

# List Plans
Write-Host "`n[4] Listing plans..." -ForegroundColor Yellow
$plansResp = Invoke-RestMethod -Uri "$baseUrl/api/subscription/plans" -Method GET -Headers $headers
Write-Host "Found $($plansResp.count) plan(s)" -ForegroundColor Green

# Create Customer
$customerBody = '{"name":"John Doe","type":"individual","email":"john@test.com"}'
Write-Host "`n[5] Creating customer..." -ForegroundColor Yellow
$customerResp = Invoke-RestMethod -Uri "$baseUrl/api/customers" -Method POST -Headers $headers -Body $customerBody -ContentType "application/json"
$customerId = ($customerResp.customer | ConvertFrom-Json).id
$planId = ($planResp.plan | ConvertFrom-Json).id
Write-Host "Customer ID: $customerId" -ForegroundColor Green
Write-Host "Plan ID: $planId" -ForegroundColor Green

# Create Subscription
$subBody = "{`"customer_id`":`"$customerId`",`"plan_id`":`"$planId`"}"
Write-Host "`n[6] Creating subscription..." -ForegroundColor Yellow
$subResp = Invoke-RestMethod -Uri "$baseUrl/api/subscription/subscriptions" -Method POST -Headers $headers -Body $subBody -ContentType "application/json"
$subscriptionId = ($subResp.subscription | ConvertFrom-Json).id
Write-Host "Subscription ID: $subscriptionId" -ForegroundColor Green

# Get Subscription
Write-Host "`n[7] Getting subscription..." -ForegroundColor Yellow
$getSubResp = Invoke-RestMethod -Uri "$baseUrl/api/subscription/subscriptions/$subscriptionId" -Method GET -Headers $headers
Write-Host "Subscription status: $(($getSubResp.subscription | ConvertFrom-Json).status)" -ForegroundColor Green

# Get Stats
Write-Host "`n[8] Getting stats..." -ForegroundColor Yellow
$statsResp = Invoke-RestMethod -Uri "$baseUrl/api/subscription/stats" -Method GET -Headers $headers
Write-Host "Active subscriptions: $($statsResp.active_subscriptions)" -ForegroundColor Green
Write-Host "Total plans: $($statsResp.total_plans)" -ForegroundColor Green

# Get Billing History
Write-Host "`n[9] Getting billing history..." -ForegroundColor Yellow
$billingResp = Invoke-RestMethod -Uri "$baseUrl/api/subscription/subscriptions/$subscriptionId/billing" -Method GET -Headers $headers
Write-Host "Found $($billingResp.count) billing record(s)" -ForegroundColor Green

Write-Host "`n========== ALL TESTS COMPLETED! ==========`n" -ForegroundColor Green

