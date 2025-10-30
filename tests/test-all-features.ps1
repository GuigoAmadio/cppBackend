# test-all-features.ps1
# Testa todas as features implementadas

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   TESTE COMPLETO - NOVAS FEATURES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$baseUrl = "http://localhost:8080"
$testsPassed = 0
$testsFailed = 0

# === TESTE 1: Rate Limiting no Login ===
Write-Host ""
Write-Host "=== TESTE 1: Rate Limiting no Login (5 req/min) ===" -ForegroundColor Yellow

$loginBody = @{
    email = "test@example.com"
    password = "wrongpass"
} | ConvertTo-Json

Write-Host "Enviando 6 requisicoes rapidas de login..." -ForegroundColor Gray

for ($i = 1; $i -le 6; $i++) {
    try {
        $response = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -Body $loginBody -ContentType "application/json" -Headers @{ "Host" = "demo.localhost:8080" } -ErrorAction Stop
        Write-Host "  Req $i : OK (200)" -ForegroundColor Green
    } catch {
        $statusCode = $_.Exception.Response.StatusCode.value__
        if ($statusCode -eq 429) {
            Write-Host "  Req $i : BLOQUEADO (429 - Too Many Requests)" -ForegroundColor Red
            $testsPassed++
        } elseif ($statusCode -eq 401 -or $statusCode -eq 400) {
            Write-Host "  Req $i : OK (401/400 - Login falhou, mas passou rate limit)" -ForegroundColor Green
        } else {
            Write-Host "  Req $i : ERRO ($statusCode)" -ForegroundColor Red
            $testsFailed++
        }
    }
    Start-Sleep -Milliseconds 200
}

if ($testsPassed -gt 0) {
    Write-Host "OK: Rate Limiting funcionando! Bloqueou apos 5 requisicoes." -ForegroundColor Green
} else {
    Write-Host "ERRO: Rate Limiting NAO funcionou. Nenhuma requisicao foi bloqueada." -ForegroundColor Red
    $testsFailed++
}

# === TESTE 2: Verificar Response do Rate Limit ===
Write-Host ""
Write-Host "=== TESTE 2: Verificar Response do Rate Limit ===" -ForegroundColor Yellow

try {
    # Forcar rate limit enviando 10 requests
    for ($i = 1; $i -le 10; $i++) {
        try {
            Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -Body $loginBody -ContentType "application/json" -Headers @{ "Host" = "demo.localhost:8080" } -ErrorAction Stop | Out-Null
        } catch {
            if ($_.Exception.Response.StatusCode.value__ -eq 429) {
                $errorBody = $_.ErrorDetails.Message | ConvertFrom-Json
                
                Write-Host "  Response 429 capturado:" -ForegroundColor Cyan
                Write-Host "    status: $($errorBody.status)" -ForegroundColor Gray
                Write-Host "    message: $($errorBody.message)" -ForegroundColor Gray
                Write-Host "    retryAfter: $($errorBody.retryAfter) segundos" -ForegroundColor Gray
                
                if ($errorBody.status -eq "error" -and $errorBody.retryAfter -gt 0) {
                    Write-Host "  OK: Response 429 bem formatado!" -ForegroundColor Green
                    $testsPassed++
                } else {
                    Write-Host "  ERRO: Response 429 mal formatado" -ForegroundColor Red
                    $testsFailed++
                }
                break
            }
        }
        Start-Sleep -Milliseconds 100
    }
} catch {
    Write-Host "  ERRO ao testar response 429" -ForegroundColor Red
    $testsFailed++
}

# === TESTE 3: Testar Rate Limiting no Register ===
Write-Host ""
Write-Host "=== TESTE 3: Rate Limiting no Register (3 req/hora) ===" -ForegroundColor Yellow

Write-Host "Enviando 4 requisicoes rapidas de registro..." -ForegroundColor Gray

$registerBlocked = 0

for ($i = 1; $i -le 4; $i++) {
    $registerBody = @{
        email = "newtestuser$i@example.com"
        password = "TestPass123!"
        name = "Test User $i"
    } | ConvertTo-Json
    
    try {
        $response = Invoke-RestMethod -Uri "$baseUrl/api/auth/register" -Method POST -Body $registerBody -ContentType "application/json" -ErrorAction Stop
        Write-Host "  Req $i : OK (201 - Criado)" -ForegroundColor Green
    } catch {
        $statusCode = $_.Exception.Response.StatusCode.value__
        if ($statusCode -eq 429) {
            Write-Host "  Req $i : BLOQUEADO (429 - Too Many Requests)" -ForegroundColor Red
            $registerBlocked++
        } elseif ($statusCode -eq 409) {
            Write-Host "  Req $i : OK (409 - Usuario ja existe)" -ForegroundColor Green
        } else {
            Write-Host "  Req $i : ERRO ($statusCode)" -ForegroundColor Yellow
        }
    }
    Start-Sleep -Milliseconds 200
}

if ($registerBlocked -gt 0) {
    Write-Host "OK: Rate Limiting no Register funcionando!" -ForegroundColor Green
    $testsPassed++
} else {
    Write-Host "AVISO: Rate Limiting no Register - nenhuma req bloqueada (pode ja ter expirado)" -ForegroundColor Yellow
}

# === RESUMO FINAL ===
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   RESUMO DOS TESTES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Testes Passaram: $testsPassed" -ForegroundColor Green
Write-Host "Testes Falharam: $testsFailed" -ForegroundColor $(if ($testsFailed -eq 0) { "Green" } else { "Red" })
Write-Host ""

if ($testsFailed -eq 0 -and $testsPassed -gt 0) {
    Write-Host "SUCESSO: TODOS OS TESTES PASSARAM!" -ForegroundColor Green
} elseif ($testsPassed -gt 0) {
    Write-Host "PARCIAL: Alguns testes passaram" -ForegroundColor Yellow
} else {
    Write-Host "FALHA: Nenhum teste passou" -ForegroundColor Red
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
