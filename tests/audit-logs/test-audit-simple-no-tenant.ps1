# Teste Audit Log SEM tenant (mais simples)

$baseUrl = "http://localhost:8080"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  TESTE AUDIT LOG (SEM TENANT)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 1. REGISTRAR
Write-Host "[1/3] Registrando usuário..." -ForegroundColor Yellow

$email = "audit$(Get-Random)@test.com"
$registerBody = @{
    name = "Audit Test User"
    email = $email
    password = "SecurePass123!"
} | ConvertTo-Json

try {
    $registerResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/register" `
        -Method POST `
        -Headers @{"Content-Type" = "application/json"; "Host" = "localhost:8080"} `
        -Body $registerBody

    Write-Host "✅ Usuário registrado!" -ForegroundColor Green
    Write-Host "   Email: $email" -ForegroundColor Gray
    Write-Host "   User ID: $($registerResponse.user.id)" -ForegroundColor Gray
    $userId = $registerResponse.user.id
} catch {
    Write-Host "❌ Erro: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# 2. LOGIN
Write-Host ""
Write-Host "[2/3] Fazendo login..." -ForegroundColor Yellow

$loginBody = @{
    email = $email
    password = "SecurePass123!"
} | ConvertTo-Json

try {
    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" `
        -Method POST `
        -Headers @{"Content-Type" = "application/json"; "Host" = "localhost:8080"} `
        -Body $loginBody

    Write-Host "✅ Login realizado!" -ForegroundColor Green
    Write-Host "   Token: $($loginResponse.token.Substring(0, 30))..." -ForegroundColor Gray
    $token = $loginResponse.token
} catch {
    Write-Host "❌ Erro: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "   (Normal se login sem tenant não for permitido)" -ForegroundColor Yellow
}

Start-Sleep -Seconds 1

# 3. TROCAR SENHA
Write-Host ""
Write-Host "[3/3] Testando troca de senha..." -ForegroundColor Yellow

$changePasswordBody = @{
    old_password = "SecurePass123!"
    new_password = "NewSecurePass456!"
} | ConvertTo-Json

try {
    Invoke-RestMethod -Uri "$baseUrl/api/users/$userId/password" `
        -Method PUT `
        -Headers @{
            "Content-Type" = "application/json"
            "Authorization" = "Bearer $token"
            "Host" = "localhost:8080"
        } `
        -Body $changePasswordBody

    Write-Host "✅ Senha alterada!" -ForegroundColor Green
} catch {
    Write-Host "⚠️  Não foi possível trocar senha (token pode ser inválido)" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  VERIFICAR LOGS NO BANCO" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Execute no PostgreSQL:" -ForegroundColor White
Write-Host "`$env:PGPASSWORD='postgre123'" -ForegroundColor Gray
Write-Host "psql -h localhost -p 5433 -U moneymaker_user -d moneymaker_dev" -ForegroundColor Gray
Write-Host ""
Write-Host "SELECT action, resource, details, created_at FROM audit_logs" -ForegroundColor Gray
Write-Host "WHERE user_id = '$userId' ORDER BY created_at DESC;" -ForegroundColor Gray

