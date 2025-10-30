$baseUrl = "http://localhost:8080"
$timestamp = [DateTimeOffset]::Now.ToUnixTimeMilliseconds()
$passed = 0
$total = 13

Write-Host "`n" -NoNewline
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  SUBSCRIPTION - TESTE COMPLETO FINAL" -ForegroundColor Cyan
Write-Host "  Timestamp: $timestamp" -ForegroundColor Gray
Write-Host "========================================" -ForegroundColor Cyan

# LOGIN
Write-Host "`n[1/$total] LOGIN..." -ForegroundColor Yellow
$loginBody = @{email="subscriptiontest@acme.com";password="Sub@2024Test";tenant_subdomain="acme"} | ConvertTo-Json
$login = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -Body $loginBody -ContentType "application/json"
$token = $login.token
$headers = @{"Authorization" = "Bearer $token"}
Write-Host "    OK" -ForegroundColor Green
$passed++

# CREATE PLAN 1
Write-Host "`n[2/$total] CREATE PLAN - Basic ($timestamp)..." -ForegroundColor Yellow
$plan1Body = @{name="Basic-$timestamp";plan_type="basic";price=29.90;currency="BRL";billing_cycle="monthly";trial_days=7;is_public="true"} | ConvertTo-Json
$plan1 = Invoke-RestMethod -Uri "$baseUrl/api/subscription/plans" -Method POST -Headers $headers -Body $plan1Body -ContentType "application/json"
$plan1Id = ($plan1.plan | ConvertFrom-Json).id
Write-Host "    OK - ID: $plan1Id" -ForegroundColor Green
$passed++

# CREATE PLAN 2
Write-Host "`n[3/$total] CREATE PLAN - Pro ($timestamp)..." -ForegroundColor Yellow
$plan2Body = @{name="Pro-$timestamp";plan_type="pro";price=99.90;currency="BRL";billing_cycle="monthly";trial_days=14;is_public="true"} | ConvertTo-Json
$plan2 = Invoke-RestMethod -Uri "$baseUrl/api/subscription/plans" -Method POST -Headers $headers -Body $plan2Body -ContentType "application/json"
$plan2Id = ($plan2.plan | ConvertFrom-Json).id
Write-Host "    OK - ID: $plan2Id" -ForegroundColor Green
$passed++

# LIST PLANS
Write-Host "`n[4/$total] LIST PLANS..." -ForegroundColor Yellow
$plans = Invoke-RestMethod -Uri "$baseUrl/api/subscription/plans" -Method GET -Headers $headers
Write-Host "    OK - Total: $($plans.count)" -ForegroundColor Green
$passed++

# GET PLAN
Write-Host "`n[5/$total] GET PLAN BY ID..." -ForegroundColor Yellow
$getPlan = Invoke-RestMethod -Uri "$baseUrl/api/subscription/plans/$plan1Id" -Method GET -Headers $headers
Write-Host "    OK" -ForegroundColor Green
$passed++

# CREATE CUSTOMER
Write-Host "`n[6/$total] CREATE CUSTOMER..." -ForegroundColor Yellow
$customerBody = @{name="Customer-$timestamp";type="individual";email="cust$timestamp@test.com"} | ConvertTo-Json
$customer = Invoke-RestMethod -Uri "$baseUrl/api/customers" -Method POST -Headers $headers -Body $customerBody -ContentType "application/json"
$customerId = $customer.id
Write-Host "    OK - ID: $customerId" -ForegroundColor Green
$passed++

# CREATE SUBSCRIPTION
Write-Host "`n[7/$total] CREATE SUBSCRIPTION..." -ForegroundColor Yellow
$subBody = @{customer_id=$customerId;plan_id=$plan1Id} | ConvertTo-Json
$sub = Invoke-RestMethod -Uri "$baseUrl/api/subscription/subscriptions" -Method POST -Headers $headers -Body $subBody -ContentType "application/json"
$subId = ($sub.subscription | ConvertFrom-Json).id
$subData = $sub.subscription | ConvertFrom-Json
Write-Host "    OK - ID: $subId, Status: $($subData.status)" -ForegroundColor Green
$passed++

# GET SUBSCRIPTION
Write-Host "`n[8/$total] GET SUBSCRIPTION..." -ForegroundColor Yellow
$getSub = Invoke-RestMethod -Uri "$baseUrl/api/subscription/subscriptions/$subId" -Method GET -Headers $headers
Write-Host "    OK" -ForegroundColor Green
$passed++

# LIST SUBSCRIPTIONS
Write-Host "`n[9/$total] LIST SUBSCRIPTIONS..." -ForegroundColor Yellow
$subs = Invoke-RestMethod -Uri "$baseUrl/api/subscription/subscriptions" -Method GET -Headers $headers
Write-Host "    OK - Total: $($subs.count)" -ForegroundColor Green
$passed++

# BILLING HISTORY
Write-Host "`n[10/$total] GET BILLING HISTORY..." -ForegroundColor Yellow
$billing = Invoke-RestMethod -Uri "$baseUrl/api/subscription/subscriptions/$subId/billing" -Method GET -Headers $headers
Write-Host "    OK - Records: $($billing.count)" -ForegroundColor Green
$passed++

# STATS
Write-Host "`n[11/$total] GET STATS..." -ForegroundColor Yellow
$stats = Invoke-RestMethod -Uri "$baseUrl/api/subscription/stats" -Method GET -Headers $headers
Write-Host "    OK - Plans: $($stats.total_plans), Subs: $($stats.total_subscriptions)" -ForegroundColor Green
$passed++

# PROCESS RENEWAL
Write-Host "`n[12/$total] PROCESS RENEWAL (Payment Integration)..." -ForegroundColor Yellow
try {
    $renewal = Invoke-RestMethod -Uri "$baseUrl/api/subscription/subscriptions/$subId/renew" -Method POST -Headers $headers
    Write-Host "    OK - Renewal processed" -ForegroundColor Green
    $passed++
} catch {
    Write-Host "    FAILED: $($_.Exception.Message)" -ForegroundColor Red
}

# CANCEL SUBSCRIPTION
Write-Host "`n[13/$total] CANCEL SUBSCRIPTION..." -ForegroundColor Yellow
$cancelBody = @{reason="Teste finalizado";immediate="false"} | ConvertTo-Json
$cancel = Invoke-RestMethod -Uri "$baseUrl/api/subscription/subscriptions/$subId/cancel" -Method POST -Headers $headers -Body $cancelBody -ContentType "application/json"
Write-Host "    OK - Cancelled" -ForegroundColor Green
$passed++

# RESULTADO
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  RESULTADO: $passed/$total PASSED" -ForegroundColor White
if ($passed -eq $total) {
    Write-Host ""
    Write-Host "  PERFEITO! TODOS OS TESTES PASSARAM!" -ForegroundColor Green
    Write-Host "  MODULO SUBSCRIPTION 100% FUNCIONAL!" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "  $($total - $passed) teste(s) falharam" -ForegroundColor Yellow
}
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
