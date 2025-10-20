Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   TESTE AUTH MIDDLEWARE" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$baseUrl = "http://localhost:8080"

# PASSO 1: Login
Write-Host "`nPASSO 1: Fazendo login..." -ForegroundColor Yellow
$credentials = @{
    email = "finaluser@test.com"
    password = "Pass1234"
} | ConvertTo-Json

$loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -Body $credentials -ContentType "application/json" -Headers @{ "Host" = "demo.localhost:8080" }
$token = $loginResponse.token
Write-Host "✓ Login OK! Token obtido." -ForegroundColor Green

# PASSO 2: GET /api/me COM token
Write-Host "`nPASSO 2: GET /api/me COM token..." -ForegroundColor Yellow
try {
    $meResponse = Invoke-RestMethod -Uri "$baseUrl/api/me" -Method GET -Headers @{ "Authorization" = "Bearer $token" }
    Write-Host "✓ Sucesso! User: $($meResponse.name)" -ForegroundColor Green
    $meResponse | ConvertTo-Json | Write-Host -ForegroundColor Gray
} catch {
    Write-Host "✗ Erro: $($_.Exception.Message)" -ForegroundColor Red
}

# PASSO 3: GET /api/me SEM token
Write-Host "`nPASSO 3: GET /api/me SEM token (deve dar 401)..." -ForegroundColor Yellow
try {
    $meResponseNoAuth = Invoke-RestMethod -Uri "$baseUrl/api/me" -Method GET
    Write-Host "✗ NAO DEVERIA FUNCIONAR!" -ForegroundColor Red
} catch {
    $statusCode = $_.Exception.Response.StatusCode.Value__
    if ($statusCode -eq 401) {
        Write-Host "✓ Correto! 401 Unauthorized" -ForegroundColor Green
    } else {
        Write-Host "✗ Status code errado: $statusCode" -ForegroundColor Red
    }
}

# PASSO 4: GET /api/me token invalido
Write-Host "`nPASSO 4: GET /api/me token invalido (deve dar 401)..." -ForegroundColor Yellow
try {
    $badResponse = Invoke-RestMethod -Uri "$baseUrl/api/me" -Method GET -Headers @{ "Authorization" = "Bearer fake-token-123" }
    Write-Host "✗ NAO DEVERIA FUNCIONAR!" -ForegroundColor Red
} catch {
    $statusCode = $_.Exception.Response.StatusCode.Value__
    if ($statusCode -eq 401) {
        Write-Host "✓ Correto! 401 Unauthorized" -ForegroundColor Green
    } else {
        Write-Host "✗ Status code errado: $statusCode" -ForegroundColor Red
    }
}

# PASSO 5: GET /api/users/outro-id (deve dar 403)
Write-Host "`nPASSO 5: GET /api/users/outro-id (deve dar 403)..." -ForegroundColor Yellow
try {
    $otherResponse = Invoke-RestMethod -Uri "$baseUrl/api/users/outro-id-fake" -Method GET -Headers @{ "Authorization" = "Bearer $token" }
    Write-Host "✗ NAO DEVERIA FUNCIONAR!" -ForegroundColor Red
} catch {
    $statusCode = $_.Exception.Response.StatusCode.Value__
    if ($statusCode -eq 403) {
        Write-Host "✓ Correto! 403 Forbidden" -ForegroundColor Green
    } else {
        Write-Host "✗ Status code errado: $statusCode" -ForegroundColor Red
    }
}

# PASSO 6: GET /api/users/proprio-id (deve funcionar)
$userId = $loginResponse.user.id
Write-Host "`nPASSO 6: GET /api/users/$userId (proprio id)..." -ForegroundColor Yellow
try {
    $selfResponse = Invoke-RestMethod -Uri "$baseUrl/api/users/$userId" -Method GET -Headers @{ "Authorization" = "Bearer $token" }
    Write-Host "✓ Sucesso! User: $($selfResponse.name)" -ForegroundColor Green
} catch {
    Write-Host "✗ Erro: $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   TESTES CONCLUIDOS" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

