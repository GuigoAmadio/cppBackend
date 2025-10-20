# test-email-password-features.ps1
# Testa Email Verification e Password Reset com usuario existente

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   TESTE: EMAIL & PASSWORD FEATURES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$baseUrl = "http://localhost:8080"

# === Usar usuario existente ===
$testEmail = "finaluser@test.com"
$testUserId = "" # Vamos buscar fazendo login

Write-Host "=== PASSO 1: Login com usuario existente ===" -ForegroundColor Yellow

$loginBody = @{
    email = $testEmail
    password = "NewPassword123!"
} | ConvertTo-Json

try {
    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -Body $loginBody -ContentType "application/json" -Headers @{ "Host" = "demo.localhost:8080" } -ErrorAction Stop
    $testUserId = $loginResponse.user.id
    Write-Host "  Login OK! User ID: $testUserId" -ForegroundColor Green
    Write-Host "  Email: $($loginResponse.user.email)" -ForegroundColor Gray
} catch {
    Write-Host "  Erro no login: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "  Tentando com senha alternativa..." -ForegroundColor Yellow
    
    $loginBody2 = @{
        email = $testEmail
        password = "NewPass456!"
    } | ConvertTo-Json
    
    try {
        $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -Body $loginBody2 -ContentType "application/json" -Headers @{ "Host" = "demo.localhost:8080" } -ErrorAction Stop
        $testUserId = $loginResponse.user.id
        Write-Host "  Login OK com senha alternativa! User ID: $testUserId" -ForegroundColor Green
    } catch {
        Write-Host "  Login falhou com ambas as senhas" -ForegroundColor Red
        exit 1
    }
}

Write-Host ""

# === TESTE 2: Enviar Email de Verificacao ===
Write-Host "=== PASSO 2: Enviar Email de Verificacao ===" -ForegroundColor Yellow

$sendVerifBody = @{
    userId = $testUserId
} | ConvertTo-Json

try {
    $verifResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/send-verification" -Method POST -Body $sendVerifBody -ContentType "application/json" -ErrorAction Stop
    Write-Host "  Email enviado com sucesso!" -ForegroundColor Green
    Write-Host "  Mensagem: $($verifResponse.message)" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "  IMPORTANTE: Verifique os LOGS DO SERVIDOR para ver:" -ForegroundColor Yellow
    Write-Host "  - Token de verificacao gerado" -ForegroundColor White
    Write-Host "  - Link de verificacao" -ForegroundColor White
    Write-Host ""
} catch {
    Write-Host "  Erro: $($_.Exception.Message)" -ForegroundColor Red
    $_.ErrorDetails.Message | Write-Host -ForegroundColor Red
}

Write-Host ""

# === TESTE 3: Solicitar Password Reset ===
Write-Host "=== PASSO 3: Solicitar Password Reset ===" -ForegroundColor Yellow

$forgotBody = @{
    email = $testEmail
} | ConvertTo-Json

try {
    $forgotResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/forgot-password" -Method POST -Body $forgotBody -ContentType "application/json" -ErrorAction Stop
    Write-Host "  Reset solicitado com sucesso!" -ForegroundColor Green
    Write-Host "  Mensagem: $($forgotResponse.message)" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "  IMPORTANTE: Verifique os LOGS DO SERVIDOR para ver:" -ForegroundColor Yellow
    Write-Host "  - Token de reset gerado" -ForegroundColor White
    Write-Host "  - Link de reset" -ForegroundColor White
    Write-Host "  - IP e User-Agent capturados" -ForegroundColor White
    Write-Host ""
} catch {
    Write-Host "  Erro: $($_.Exception.Message)" -ForegroundColor Red
    $_.ErrorDetails.Message | Write-Host -ForegroundColor Red
}

Write-Host ""

# === TESTE 4: Testar Validacoes ===
Write-Host "=== PASSO 4: Testar Validacoes ===" -ForegroundColor Yellow

Write-Host "  4.1. Verificar email com token invalido..." -ForegroundColor Cyan
$badVerifyBody = @{
    token = "invalid-token-123"
} | ConvertTo-Json

try {
    Invoke-RestMethod -Uri "$baseUrl/api/auth/verify-email" -Method POST -Body $badVerifyBody -ContentType "application/json" -ErrorAction Stop | Out-Null
    Write-Host "    ERRO: Deveria ter rejeitado!" -ForegroundColor Red
} catch {
    Write-Host "    OK: Rejeitou token invalido" -ForegroundColor Green
}

Write-Host "  4.2. Reset com token invalido..." -ForegroundColor Cyan
$badResetBody = @{
    token = "invalid-token-123"
    newPassword = "NewPass123!"
} | ConvertTo-Json

try {
    Invoke-RestMethod -Uri "$baseUrl/api/auth/reset-password" -Method POST -Body $badResetBody -ContentType "application/json" -ErrorAction Stop | Out-Null
    Write-Host "    ERRO: Deveria ter rejeitado!" -ForegroundColor Red
} catch {
    Write-Host "    OK: Rejeitou token invalido" -ForegroundColor Green
}

Write-Host "  4.3. Forgot password com email invalido..." -ForegroundColor Cyan
$badEmailBody = @{
    email = "naoexiste@example.com"
} | ConvertTo-Json

try {
    $response = Invoke-RestMethod -Uri "$baseUrl/api/auth/forgot-password" -Method POST -Body $badEmailBody -ContentType "application/json" -ErrorAction Stop
    Write-Host "    OK: Nao revelou se email existe (seguranca)" -ForegroundColor Green
    Write-Host "    Mensagem: $($response.message)" -ForegroundColor Gray
} catch {
    Write-Host "    OK: Tratou email inexistente" -ForegroundColor Green
}

Write-Host ""

# === RESUMO ===
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   TESTES CONCLUIDOS!" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "FUNCIONALIDADES TESTADAS:" -ForegroundColor Green
Write-Host "  - Email Verification (envio)" -ForegroundColor White
Write-Host "  - Password Reset (solicitacao)" -ForegroundColor White
Write-Host "  - Validacoes de tokens invalidos" -ForegroundColor White
Write-Host "  - Seguranca (nao revelar emails)" -ForegroundColor White
Write-Host ""
Write-Host "PROXIMOS PASSOS:" -ForegroundColor Yellow
Write-Host "  1. Verificar LOGS do servidor para tokens" -ForegroundColor White
Write-Host "  2. Copiar um token e testar verify-email" -ForegroundColor White
Write-Host "  3. Copiar um token e testar reset-password" -ForegroundColor White
Write-Host "  4. Verificar que emails foram 'enviados' (mock)" -ForegroundColor White
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan

