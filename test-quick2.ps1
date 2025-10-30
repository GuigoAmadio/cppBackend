$baseUrl = "http://localhost:8080"

# Login with existing user
$loginBody = @{
    email = "finance1761237381419@test.com"
    password = "Finance123"
    tenant_subdomain = "acme"
} | ConvertTo-Json

Write-Host "Logging in..." -ForegroundColor Cyan
$loginResult = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method Post -Body $loginBody -ContentType "application/json"
$token = $loginResult.token
Write-Host "Token obtained!" -ForegroundColor Green

# Create Account
$headers = @{
    "Authorization" = "Bearer $token"
    "Content-Type" = "application/json"
}

$accountBody = @{
    name = "Test Account Quick"
    type = "checking"
    currency = "BRL"
    initial_balance = 1000
} | ConvertTo-Json

Write-Host "`nCreating account..." -ForegroundColor Cyan
try {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/accounts" -Method Post -Body $accountBody -Headers $headers
    Write-Host "SUCCESS!" -ForegroundColor Green
    Write-Host ($result | ConvertTo-Json)
} catch {
    Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "Response: $($_.ErrorDetails.Message)" -ForegroundColor Yellow
}

# Check server logs
Write-Host "`n=== SERVER LOGS (Last 20) ===" -ForegroundColor Magenta
Get-Content "server.log" -Tail 20

