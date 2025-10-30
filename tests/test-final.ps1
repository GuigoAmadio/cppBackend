# Teste Final - Registro e Login com Logs de Debug
Write-Host "=== TESTE FINAL: REGISTRO + LOGIN ===" -ForegroundColor Cyan

# 1. REGISTRO
Write-Host "`n1. REGISTRANDO USUARIO..." -ForegroundColor Yellow
$registerBody = @{
    email = "debuguser@test.com"
    password = "TestPass123"
    name = "Debug User"
} | ConvertTo-Json

try {
    $registerResponse = Invoke-WebRequest -Uri "http://localhost:8080/api/auth/register" `
        -Method POST `
        -Body $registerBody `
        -ContentType "application/json" `
        -UseBasicParsing
    
    Write-Host "✓ REGISTRO OK - Status: $($registerResponse.StatusCode)" -ForegroundColor Green
    Write-Host ($registerResponse.Content | ConvertFrom-Json | ConvertTo-Json -Depth 5) -ForegroundColor Gray
} catch {
    Write-Host "✗ REGISTRO FALHOU" -ForegroundColor Red
    if ($_.ErrorDetails.Message) {
        Write-Host $_.ErrorDetails.Message -ForegroundColor Red
    }
    exit 1
}

Start-Sleep -Seconds 2

# 2. LOGIN
Write-Host "`n2. FAZENDO LOGIN..." -ForegroundColor Yellow
$loginBody = @{
    email = "debuguser@test.com"
    password = "TestPass123"
} | ConvertTo-Json

try {
    $loginResponse = Invoke-WebRequest -Uri "http://localhost:8080/api/auth/login" `
        -Method POST `
        -Body $loginBody `
        -ContentType "application/json" `
        -UseBasicParsing
    
    Write-Host "✓ LOGIN OK - Status: $($loginResponse.StatusCode)" -ForegroundColor Green
    $json = $loginResponse.Content | ConvertFrom-Json
    
    Write-Host "`n=== TOKEN JWT ===" -ForegroundColor Cyan
    Write-Host $json.token -ForegroundColor Green
    
    Write-Host "`n=== REFRESH TOKEN ===" -ForegroundColor Cyan
    Write-Host $json.refreshToken -ForegroundColor Green
    
    Write-Host "`n=== USER DATA ===" -ForegroundColor Cyan
    Write-Host ($json.user | ConvertTo-Json -Depth 5) -ForegroundColor Gray
    
} catch {
    Write-Host "✗ LOGIN FALHOU" -ForegroundColor Red
    if ($_.ErrorDetails.Message) {
        Write-Host $_.ErrorDetails.Message -ForegroundColor Red
    }
}

Write-Host "`n=== TESTE CONCLUIDO ===" -ForegroundColor Cyan
Write-Host "Verifique a janela do servidor para ver os logs de DEBUG do Bcrypt!" -ForegroundColor Yellow

