# Teste SIMPLES de Login
Write-Host "=== TESTE DE LOGIN ===" -ForegroundColor Cyan

# Registrar usuário novo
Write-Host "`n1. Registrando usuário..." -ForegroundColor Yellow
$registerBody = @{
    email = "logintest@test.com"
    password = "Pass123"
    name = "Login Test"
} | ConvertTo-Json

try {
    $registerResponse = Invoke-WebRequest -Uri "http://localhost:8080/api/auth/register" `
        -Method POST `
        -Body $registerBody `
        -ContentType "application/json" `
        -UseBasicParsing
    
    Write-Host "✓ Registro OK - Status: $($registerResponse.StatusCode)" -ForegroundColor Green
    Write-Host $registerResponse.Content -ForegroundColor Gray
} catch {
    Write-Host "✗ Registro FALHOU" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 2

# Fazer login com o mesmo usuário
Write-Host "`n2. Fazendo login..." -ForegroundColor Yellow
$loginBody = @{
    email = "logintest@test.com"
    password = "Pass123"
} | ConvertTo-Json

Write-Host "Login Body: $loginBody" -ForegroundColor Gray

try {
    $loginResponse = Invoke-WebRequest -Uri "http://localhost:8080/api/auth/login" `
        -Method POST `
        -Body $loginBody `
        -ContentType "application/json" `
        -UseBasicParsing
    
    Write-Host "✓ Login OK - Status: $($loginResponse.StatusCode)" -ForegroundColor Green
    Write-Host $loginResponse.Content -ForegroundColor Gray
    
    # Parse e mostrar token
    $json = $loginResponse.Content | ConvertFrom-Json
    Write-Host "`n=== TOKEN JWT ===" -ForegroundColor Cyan
    Write-Host $json.token -ForegroundColor Green
    
} catch {
    Write-Host "✗ Login FALHOU" -ForegroundColor Red
    if ($_.ErrorDetails.Message) {
        Write-Host $_.ErrorDetails.Message -ForegroundColor Red
    } else {
        Write-Host $_.Exception.Message -ForegroundColor Red
    }
}

