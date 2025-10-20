# test-audit-metrics.ps1
# Testa Audit Logs e Metrics

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   TESTE: AUDIT LOGS + METRICS" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$baseUrl = "http://localhost:8080"

# === TESTE 1: Verificar endpoint /metrics ===
Write-Host "=== TESTE 1: Endpoint /metrics ===" -ForegroundColor Yellow

try {
    $metricsResponse = Invoke-WebRequest -Uri "$baseUrl/metrics" -Method GET -ErrorAction Stop
    Write-Host "  Status: $($metricsResponse.StatusCode)" -ForegroundColor Green
    Write-Host "  Content-Type: $($metricsResponse.Headers['Content-Type'])" -ForegroundColor Gray
    Write-Host ""
    Write-Host "  Primeiras linhas das metricas:" -ForegroundColor Cyan
    $lines = $metricsResponse.Content -split "`n" | Select-Object -First 15
    foreach ($line in $lines) {
        if ($line.Trim() -ne "") {
            Write-Host "    $line" -ForegroundColor White
        }
    }
    Write-Host ""
} catch {
    Write-Host "  Erro ao acessar /metrics: $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# === TESTE 2: Fazer alguns requests para gerar métricas ===
Write-Host "=== TESTE 2: Gerar Metricas (fazendo requests) ===" -ForegroundColor Yellow

try {
    # Health check
    $null = Invoke-WebRequest -Uri "$baseUrl/health" -Method GET -ErrorAction Stop
    Write-Host "  GET /health" -ForegroundColor Green
    
    # Pool stats
    $null = Invoke-WebRequest -Uri "$baseUrl/api/pool/stats" -Method GET -ErrorAction Stop
    Write-Host "  GET /api/pool/stats" -ForegroundColor Green
    
    # DB test
    $null = Invoke-WebRequest -Uri "$baseUrl/api/db/test" -Method GET -ErrorAction Stop
    Write-Host "  GET /api/db/test" -ForegroundColor Green
    
    Write-Host "  Requests executados com sucesso!" -ForegroundColor Green
} catch {
    Write-Host "  Erro: $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# === TESTE 3: Verificar métricas novamente ===
Write-Host "=== TESTE 3: Metricas Atualizadas ===" -ForegroundColor Yellow

try {
    $metricsResponse2 = Invoke-WebRequest -Uri "$baseUrl/metrics" -Method GET -ErrorAction Stop
    Write-Host "  Metricas coletadas!" -ForegroundColor Green
    Write-Host ""
    
    # Filtrar métricas interessantes
    $content = $metricsResponse2.Content
    
    Write-Host "  Contadores (http_requests_total):" -ForegroundColor Cyan
    $counters = $content -split "`n" | Where-Object { $_ -match "^http_requests_total" }
    foreach ($counter in $counters) {
        if ($counter.Trim() -ne "") {
            Write-Host "    $counter" -ForegroundColor White
        }
    }
    Write-Host ""
    
    Write-Host "  Requisicoes Ativas (http_requests_in_flight):" -ForegroundColor Cyan
    $inFlight = $content -split "`n" | Where-Object { $_ -match "^http_requests_in_flight" }
    foreach ($line in $inFlight) {
        if ($line.Trim() -ne "") {
            Write-Host "    $line" -ForegroundColor White
        }
    }
    Write-Host ""
    
    Write-Host "  Latencias (http_request_duration_seconds):" -ForegroundColor Cyan
    $durations = $content -split "`n" | Where-Object { $_ -match "^http_request_duration_seconds_(sum|count)" }
    foreach ($line in $durations | Select-Object -First 6) {
        if ($line.Trim() -ne "") {
            Write-Host "    $line" -ForegroundColor White
        }
    }
    
} catch {
    Write-Host "  Erro: $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# === TESTE 4: Testar Audit Logs (requer autenticacao) ===
Write-Host "=== TESTE 4: Audit Logs (Endpoint Protegido) ===" -ForegroundColor Yellow

# Fazer login para pegar token
$loginBody = @{
    email = "finaluser@test.com"
    password = "NewPass456!"
} | ConvertTo-Json

try {
    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -Body $loginBody -ContentType "application/json" -Headers @{ "Host" = "demo.localhost:8080" } -ErrorAction Stop
    $token = $loginResponse.token
    Write-Host "  Login OK! Token obtido" -ForegroundColor Green
    
    # Tentar acessar audit logs
    try {
        $auditResponse = Invoke-RestMethod -Uri "$baseUrl/api/admin/audit-logs" -Method GET -Headers @{ "Authorization" = "Bearer $token" } -ErrorAction Stop
        Write-Host "  Audit logs acessados:" -ForegroundColor Green
        Write-Host "    Total: $($auditResponse.total)" -ForegroundColor White
        Write-Host "    Limit: $($auditResponse.limit)" -ForegroundColor White
        Write-Host "    Logs retornados: $($auditResponse.logs.Count)" -ForegroundColor White
        
        if ($auditResponse.logs.Count -gt 0) {
            Write-Host ""
            Write-Host "    Exemplo de log:" -ForegroundColor Cyan
            $firstLog = $auditResponse.logs[0]
            $firstLog.PSObject.Properties | ForEach-Object {
                Write-Host "      $($_.Name): $($_.Value)" -ForegroundColor Gray
            }
        }
    } catch {
        $statusCode = $_.Exception.Response.StatusCode.value__
        if ($statusCode -eq 403) {
            Write-Host "  Usuario nao tem permissao (403) - esperado se nao for admin" -ForegroundColor Yellow
        } else {
            Write-Host "  Erro ao acessar audit logs: $($_.Exception.Message)" -ForegroundColor Red
        }
    }
    
} catch {
    Write-Host "  Erro no login: $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""

# === RESUMO ===
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   TESTES CONCLUIDOS!" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "FUNCIONALIDADES TESTADAS:" -ForegroundColor Green
Write-Host "  - Endpoint /metrics (Prometheus)" -ForegroundColor White
Write-Host "  - Coleta automatica de metricas" -ForegroundColor White
Write-Host "  - Endpoint /api/admin/audit-logs" -ForegroundColor White
Write-Host ""
Write-Host "METRICAS DISPONIVEIS:" -ForegroundColor Yellow
Write-Host "  - http_requests_total (contador)" -ForegroundColor White
Write-Host "  - http_requests_in_flight (gauge)" -ForegroundColor White
Write-Host "  - http_request_duration_seconds (histogram)" -ForegroundColor White
Write-Host ""
Write-Host "INTEGRACAO PROMETHEUS:" -ForegroundColor Cyan
Write-Host "  curl http://localhost:8080/metrics" -ForegroundColor Gray
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan

