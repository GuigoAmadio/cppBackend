# ========================================
# TEST-DOCKER-HEALTH.PS1
# Testa todos os serviços Docker
# ========================================

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  TESTE DE SAUDE - DOCKER COMPOSE" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$baseUrl = "http://localhost:8080"
$totalTests = 0
$passedTests = 0

function Test-Endpoint {
    param(
        [string]$Name,
        [string]$Url,
        [int]$ExpectedStatus = 200
    )
    
    $script:totalTests++
    Write-Host "[$script:totalTests] Testando: $Name..." -NoNewline
    
    try {
        $response = Invoke-WebRequest -Uri $Url -Method GET -TimeoutSec 10 -ErrorAction Stop
        if ($response.StatusCode -eq $ExpectedStatus) {
            Write-Host " OK" -ForegroundColor Green
            $script:passedTests++
            return $true
        } else {
            Write-Host " FALHOU (Status: $($response.StatusCode))" -ForegroundColor Red
            return $false
        }
    } catch {
        Write-Host " ERRO: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

# ========================================
# 1. BACKEND C++ - ENDPOINTS BASICOS
# ========================================
Write-Host "`n--- BACKEND C++ ---" -ForegroundColor Yellow
Test-Endpoint "Health Check" "$baseUrl/health"
Test-Endpoint "Ready Check" "$baseUrl/ready"
Test-Endpoint "Live Check" "$baseUrl/live"
Test-Endpoint "Metrics" "$baseUrl/metrics"

# ========================================
# 2. PROMETHEUS
# ========================================
Write-Host "`n--- PROMETHEUS ---" -ForegroundColor Yellow
Test-Endpoint "Prometheus UI" "http://localhost:9090/-/ready"
Test-Endpoint "Prometheus Targets" "http://localhost:9090/api/v1/targets"

# ========================================
# 3. GRAFANA
# ========================================
Write-Host "`n--- GRAFANA ---" -ForegroundColor Yellow
Test-Endpoint "Grafana Health" "http://localhost:3001/api/health"

# ========================================
# 4. POSTGRESQL (via backend)
# ========================================
Write-Host "`n--- POSTGRESQL (via Backend) ---" -ForegroundColor Yellow
try {
    $loginBody = @{
        email = "admin@acme.com"
        password = "Admin123!"
        tenant_subdomain = "acme"
    } | ConvertTo-Json

    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -Body $loginBody -ContentType "application/json" -ErrorAction Stop
    
    if ($loginResponse.token -or $loginResponse.access_token) {
        Write-Host "[DB TEST] Login bem-sucedido - PostgreSQL funcionando!" -ForegroundColor Green
        $script:totalTests++
        $script:passedTests++
    } else {
        Write-Host "[DB TEST] Login falhou - Verifique PostgreSQL" -ForegroundColor Red
        $script:totalTests++
    }
} catch {
    Write-Host "[DB TEST] Erro ao conectar com banco: $($_.Exception.Message)" -ForegroundColor Red
    $script:totalTests++
}

# ========================================
# 5. REDIS (via backend /metrics)
# ========================================
Write-Host "`n--- REDIS (via Backend Metrics) ---" -ForegroundColor Yellow
try {
    $metricsResponse = Invoke-WebRequest -Uri "$baseUrl/metrics" -Method GET -ErrorAction Stop
    if ($metricsResponse.Content -match "redis") {
        Write-Host "[REDIS TEST] Métricas Redis encontradas - Redis funcionando!" -ForegroundColor Green
        $script:totalTests++
        $script:passedTests++
    } else {
        Write-Host "[REDIS TEST] Métricas Redis não encontradas" -ForegroundColor Yellow
        $script:totalTests++
    }
} catch {
    Write-Host "[REDIS TEST] Erro ao verificar métricas" -ForegroundColor Red
    $script:totalTests++
}

# ========================================
# 6. NODE EXPORTER
# ========================================
Write-Host "`n--- NODE EXPORTER ---" -ForegroundColor Yellow
Test-Endpoint "Node Exporter Metrics" "http://localhost:9100/metrics"

# ========================================
# RESUMO
# ========================================
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  RESULTADO FINAL" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Total de testes: $totalTests" -ForegroundColor White
Write-Host "Testes passados: $passedTests" -ForegroundColor Green
Write-Host "Testes falhados: $($totalTests - $passedTests)" -ForegroundColor Red

$successRate = [math]::Round(($passedTests / $totalTests) * 100, 2)
Write-Host "Taxa de sucesso: $successRate%" -ForegroundColor $(if ($successRate -ge 90) { "Green" } elseif ($successRate -ge 70) { "Yellow" } else { "Red" })

if ($passedTests -eq $totalTests) {
    Write-Host "`nSTATUS: TODOS OS SERVICOS FUNCIONANDO PERFEITAMENTE!" -ForegroundColor Green
} elseif ($successRate -ge 80) {
    Write-Host "`nSTATUS: MAIORIA DOS SERVICOS FUNCIONANDO" -ForegroundColor Yellow
} else {
    Write-Host "`nSTATUS: VARIOS SERVICOS COM PROBLEMAS" -ForegroundColor Red
}

Write-Host "`n========================================`n" -ForegroundColor Cyan

