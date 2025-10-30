# Test 2 Transactions
$baseUrl = "http://localhost:8080"

# Register and Login
$registerBody = @{ email = "txtest@test.com"; password = "Test@123"; name = "TX Test" } | ConvertTo-Json
try { Invoke-RestMethod -Uri "$baseUrl/api/auth/register" -Method POST -ContentType "application/json" -Body $registerBody | Out-Null } catch {}

# Add to tenant
$env:PGPASSWORD='postgre123'
psql -h localhost -p 5433 -U moneymaker_user -d moneymaker_dev -c "INSERT INTO tenant_members (tenant_id, user_id, role, joined_at) SELECT '659eb680-1f6e-47b5-bafd-38100d6cee8e', id, 'admin', NOW() FROM users WHERE email='txtest@test.com' ON CONFLICT DO NOTHING" | Out-Null

# Login
$loginBody = @{ email = "txtest@test.com"; password = "Test@123"; tenant_subdomain = "acme" } | ConvertTo-Json
$loginResult = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -ContentType "application/json" -Body $loginBody
$token = $loginResult.token
$headers = @{ Authorization = "Bearer $token"; "Content-Type" = "application/json" }

Write-Host "[TEST] Token obtained" -ForegroundColor Green

# Create Account
$accountBody = '{"name":"Test Account","type":"checking","currency":"BRL","initial_balance":10000}'
$accountResult = Invoke-RestMethod -Uri "$baseUrl/api/finance/accounts" -Method POST -Headers $headers -Body $accountBody
$accountId = $accountResult.id
Write-Host "[TEST] Account created: $accountId" -ForegroundColor Green

# Create Category
$categoryBody = '{"name":"Test Category","type":"expense","color":"#ff0000"}'
$categoryResult = Invoke-RestMethod -Uri "$baseUrl/api/finance/categories" -Method POST -Headers $headers -Body $categoryBody
$categoryId = $categoryResult.id
Write-Host "[TEST] Category created: $categoryId" -ForegroundColor Green

# Transaction 1 - Expense (with category)
Write-Host "`n[TEST] Creating Transaction #1 (EXPENSE with category)..." -ForegroundColor Cyan
$tx1Body = "{`"account_id`":`"$accountId`",`"category_id`":`"$categoryId`",`"type`":`"expense`",`"amount`":100,`"currency`":`"BRL`",`"description`":`"Test expense`",`"transaction_date`":`"2025-10-23`"}"
try {
    $tx1Result = Invoke-RestMethod -Uri "$baseUrl/api/finance/transactions" -Method POST -Headers $headers -Body $tx1Body
    Write-Host "[SUCCESS] TX1 created: $($tx1Result.id)" -ForegroundColor Green
} catch {
    Write-Host "[FAIL] TX1 failed: $_" -ForegroundColor Red
}

Start-Sleep -Seconds 1

# Transaction 2 - Income (NO category)
Write-Host "`n[TEST] Creating Transaction #2 (INCOME without category)..." -ForegroundColor Cyan
$tx2Body = "{`"account_id`":`"$accountId`",`"type`":`"income`",`"amount`":5000,`"currency`":`"BRL`",`"description`":`"Test income`",`"transaction_date`":`"2025-10-23`"}"
try {
    $tx2Result = Invoke-RestMethod -Uri "$baseUrl/api/finance/transactions" -Method POST -Headers $headers -Body $tx2Body
    Write-Host "[SUCCESS] TX2 created: $($tx2Result.id)" -ForegroundColor Green
} catch {
    Write-Host "[FAIL] TX2 failed: $_" -ForegroundColor Red
}

Write-Host "`n[TEST] Check server.log for details" -ForegroundColor Yellow

