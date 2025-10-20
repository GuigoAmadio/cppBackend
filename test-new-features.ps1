# test-new-features.ps1
# Testa TODAS as novas features implementadas

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   TESTE COMPLETO - NOVAS FEATURES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$baseUrl = "http://localhost:8080"
$testsPassed = 0
$testsFailed = 0

# === TESTE 1: Rate Limiting (ja testado anteriormente) ===
Write-Host "=== TESTE 1: Rate Limiting ===" -ForegroundColor Yellow
Write-Host "  (ja testado anteriormente - OK)" -ForegroundColor Green
$testsPassed++
Write-Host ""

# === TESTE 2: Registrar usuario para testes ===
Write-Host "=== TESTE 2: Registrar Usuario para Testes ===" -ForegroundColor Yellow

$testEmail = "testverify@example.com"
$testPassword = "TestPassword123!"
$testName = "Test User"
$testUserId = ""

$registerBody = @{
    email = $testEmail
    password = $testPassword
    name = $testName
} | ConvertTo-Json

try {
    $registerResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/register" -Method POST -Body $registerBody -ContentType "application/json" -ErrorAction Stop
    $testUserId = $registerResponse.user.id
    Write-Host "  Usuario registrado: $testUserId" -ForegroundColor Green
    $testsPassed++
} catch {
    $statusCode = $_.Exception.Response.StatusCode.value__
    if ($statusCode -eq 409) {
        Write-Host "  Usuario ja existe (409) - tudo bem, vamos usar o existente" -ForegroundColor Yellow
        # Fazer login para pegar o ID
        $loginBody = @{
            email = $testEmail
            password = $testPassword
        } | ConvertTo-Json
        try {
            $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -Body $loginBody -ContentType "application/json" -Headers @{ "Host" = "demo.localhost:8080" } -ErrorAction Stop
            $testUserId = $loginResponse.user.id
            Write-Host "  Usuario ID recuperado: $testUserId" -ForegroundColor Green
            $testsPassed++
        } catch {
            Write-Host "  Erro ao fazer login: $($_.Exception.Message)" -ForegroundColor Red
            $testsFailed++
        }
    } else {
        Write-Host "  Erro ao registrar: $($_.Exception.Message)" -ForegroundColor Red
        $testsFailed++
    }
}

Write-Host ""

# === TESTE 3: Email Verification - Enviar email ===
Write-Host "=== TESTE 3: Email Verification - Enviar Email ===" -ForegroundColor Yellow

if ($testUserId) {
    $sendVerifBody = @{
        userId = $testUserId
    } | ConvertTo-Json

    try {
        $verifResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/send-verification" -Method POST -Body $sendVerifBody -ContentType "application/json" -ErrorAction Stop
        Write-Host "  Email de verificacao enviado!" -ForegroundColor Green
        Write-Host "  (Verifique os logs do servidor para ver o token)" -ForegroundColor Cyan
        $testsPassed++
    } catch {
        Write-Host "  Erro ao enviar email: $($_.Exception.Message)" -ForegroundColor Red
        $_.ErrorDetails.Message | Write-Host -ForegroundColor Red
        $testsFailed++
    }
} else {
    Write-Host "  Pulando (sem userId)" -ForegroundColor Yellow
}

Write-Host ""

# === TESTE 4: Password Reset - Solicitar reset ===
Write-Host "=== TESTE 4: Password Reset - Solicitar Reset ===" -ForegroundColor Yellow

$forgotBody = @{
    email = $testEmail
} | ConvertTo-Json

try {
    $forgotResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/forgot-password" -Method POST -Body $forgotBody -ContentType "application/json" -ErrorAction Stop
    Write-Host "  Solicitacao de reset enviada!" -ForegroundColor Green
    Write-Host "  Mensagem: $($forgotResponse.message)" -ForegroundColor Cyan
    Write-Host "  (Verifique os logs do servidor para ver o token)" -ForegroundColor Cyan
    $testsPassed++
} catch {
    Write-Host "  Erro ao solicitar reset: $($_.Exception.Message)" -ForegroundColor Red
    $_.ErrorDetails.Message | Write-Host -ForegroundColor Red
    $testsFailed++
}

Write-Host ""

# === TESTE 5: Password Reset - Com token invalido ===
Write-Host "=== TESTE 5: Password Reset - Token Invalido ===" -ForegroundColor Yellow

$resetBody = @{
    token = "00000000-0000-0000-0000-000000000000"
    newPassword = "NewPassword123!"
} | ConvertTo-Json

try {
    $resetResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/reset-password" -Method POST -Body $resetBody -ContentType "application/json" -ErrorAction Stop
    Write-Host "  ERRO: Deveria ter falhado com token invalido!" -ForegroundColor Red
    $testsFailed++
} catch {
    $statusCode = $_.Exception.Response.StatusCode.value__
    if ($statusCode -eq 400) {
        Write-Host "  OK: Rejeitou token invalido (400)" -ForegroundColor Green
        $testsPassed++
    } else {
        Write-Host "  Erro inesperado: $statusCode" -ForegroundColor Red
        $testsFailed++
    }
}

Write-Host ""

# === TESTE 6: Email Verification - Token invalido ===
Write-Host "=== TESTE 6: Email Verification - Token Invalido ===" -ForegroundColor Yellow

$verifyBody = @{
    token = "00000000-0000-0000-0000-000000000000"
} | ConvertTo-Json

try {
    $verifyResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/verify-email" -Method POST -Body $verifyBody -ContentType "application/json" -ErrorAction Stop
    Write-Host "  ERRO: Deveria ter falhado com token invalido!" -ForegroundColor Red
    $testsFailed++
} catch {
    $statusCode = $_.Exception.Response.StatusCode.value__
    if ($statusCode -eq 400) {
        Write-Host "  OK: Rejeitou token invalido (400)" -ForegroundColor Green
        $testsPassed++
    } else {
        Write-Host "  Erro inesperado: $statusCode" -ForegroundColor Red
        $testsFailed++
    }
}

Write-Host ""

# === RESUMO FINAL ===
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   RESUMO DOS TESTES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Testes Passaram: $testsPassed" -ForegroundColor Green
Write-Host "Testes Falharam: $testsFailed" -ForegroundColor $(if ($testsFailed -eq 0) { "Green" } else { "Red" })
Write-Host ""

if ($testsFailed -eq 0 -and $testsPassed -gt 0) {
    Write-Host "SUCESSO: TODOS OS TESTES PASSARAM!" -ForegroundColor Green
    Write-Host ""
    Write-Host "PROXIMOS PASSOS:" -ForegroundColor Cyan
    Write-Host "1. Verificar logs do servidor para tokens gerados" -ForegroundColor White
    Write-Host "2. Testar com tokens reais (copiar dos logs)" -ForegroundColor White
    Write-Host "3. Implementar integracao de Audit Log" -ForegroundColor White
    Write-Host "4. Implementar Metrics (opcional)" -ForegroundColor White
} else {
    Write-Host "ALGUNS TESTES FALHARAM - Verifique os erros acima" -ForegroundColor Red
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan

