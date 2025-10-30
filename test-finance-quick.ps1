$baseUrl = "http://localhost:8080"

# 1. Register
$regBody = @{
    email = "quicktest@test.com"
    password = "Test123"
    name = "Quick Test"
    tenant_subdomain = "acme"
} | ConvertTo-Json

$regResult = Invoke-RestMethod -Uri "$baseUrl/api/auth/register" -Method Post -Body $regBody -ContentType "application/json"
Write-Host "User ID: $($regResult.user.id)"

# 2. Add to tenant
$env:PGPASSWORD="postgre123"
$addQuery = "INSERT INTO user_tenants (user_id, tenant_id, role) SELECT '$($regResult.user.id)', id, 'admin' FROM tenants WHERE subdomain = 'acme' ON CONFLICT DO NOTHING"
psql -U moneymaker_user -d moneymaker_dev -p 5433 -c $addQuery 2>$null | Out-Null

# 3. Login
$loginBody = @{
    email = "quicktest@test.com"
    password = "Test123"
    tenant_subdomain = "acme"
} | ConvertTo-Json

$loginResult = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method Post -Body $loginBody -ContentType "application/json"
$token = $loginResult.token
Write-Host "Token: $token"

# 4. Create Account
$headers = @{
    "Authorization" = "Bearer $token"
    "Content-Type" = "application/json"
}

$accountBody = @{
    name = "Test Account"
    type = "checking"
    currency = "BRL"
    initial_balance = 1000
} | ConvertTo-Json

Write-Host "`nTrying to create account..."
try {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/accounts" -Method Post -Body $accountBody -Headers $headers
    Write-Host "SUCCESS: $($result | ConvertTo-Json)"
} catch {
    Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
}

