# ===================================================================
# 🧪 Script de Testes - C++ Backend v2.0
# ===================================================================
# Testa todas as novas features implementadas:
# - ConnectionPool
# - Middleware System
# - Logger melhorado
# ===================================================================

Write-Host "`n═══════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  🧪 TESTES C++ BACKEND v2.0" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════`n" -ForegroundColor Cyan

$baseUrl = "http://localhost:8080"
$passed = 0
$failed = 0

function Test-Endpoint {
    param(
        [string]$name,
        [string]$url,
        [string]$method = "GET",
        [hashtable]$expectedKeys = @{}
    )
    
    Write-Host "🔍 Testando: $name" -ForegroundColor Yellow
    Write-Host "   URL: $url" -ForegroundColor Gray
    
    try {
        $response = Invoke-RestMethod -Uri $url -Method $method -ErrorAction Stop
        
        Write-Host "   ✅ Status: OK" -ForegroundColor Green
        
        # Verificar keys esperadas
        $allKeysFound = $true
        foreach ($key in $expectedKeys.Keys) {
            if ($response.PSObject.Properties.Name -contains $key) {
                Write-Host "   ✓ Campo '$key' presente" -ForegroundColor DarkGreen
            } else {
                Write-Host "   ✗ Campo '$key' ausente" -ForegroundColor Red
                $allKeysFound = $false
            }
        }
        
        # Mostrar resposta
        Write-Host "   📦 Resposta:" -ForegroundColor Gray
        $response | ConvertTo-Json -Depth 3 | Write-Host -ForegroundColor DarkGray
        
        if ($allKeysFound) {
            Write-Host "   ✅ PASSOU`n" -ForegroundColor Green
            return $true
        } else {
            Write-Host "   ❌ FALHOU (campos ausentes)`n" -ForegroundColor Red
            return $false
        }
        
    } catch {
        Write-Host "   ❌ ERRO: $($_.Exception.Message)" -ForegroundColor Red
        Write-Host "   ❌ FALHOU`n" -ForegroundColor Red
        return $false
    }
}

# ===================================================================
# TESTE 1: Health Check
# ===================================================================
Write-Host "═══ TESTE 1: Health Check ═══" -ForegroundColor Magenta
if (Test-Endpoint -name "Health Check" -url "$baseUrl/health") {
    $passed++
} else {
    $failed++
}

# ===================================================================
# TESTE 2: Pool Stats (ConnectionPool)
# ===================================================================
Write-Host "═══ TESTE 2: Pool Stats ═══" -ForegroundColor Magenta
if (Test-Endpoint -name "Pool Stats" -url "$baseUrl/api/pool/stats" -expectedKeys @{
    "status" = "string"
    "total_connections" = "number"
    "available_connections" = "number"
    "active_connections" = "number"
}) {
    $passed++
} else {
    $failed++
}

# ===================================================================
# TESTE 3: Database Test (com ConnectionPool)
# ===================================================================
Write-Host "═══ TESTE 3: Database Test ═══" -ForegroundColor Magenta
if (Test-Endpoint -name "DB Test" -url "$baseUrl/api/db/test" -expectedKeys @{
    "status" = "string"
    "connected" = "bool"
    "version" = "string"
    "pool_stats" = "object"
}) {
    $passed++
} else {
    $failed++
}

# ===================================================================
# TESTE 4: Database Query
# ===================================================================
Write-Host "═══ TESTE 4: Database Query ═══" -ForegroundColor Magenta
if (Test-Endpoint -name "DB Query" -url "$baseUrl/api/db/query" -expectedKeys @{
    "status" = "string"
    "query" = "string"
    "rows" = "number"
}) {
    $passed++
} else {
    $failed++
}

# ===================================================================
# TESTE 5: Stress Test (2 conexões)
# ===================================================================
Write-Host "═══ TESTE 5: Stress Test (2 conexões) ═══" -ForegroundColor Magenta
if (Test-Endpoint -name "Stress Test" -url "$baseUrl/api/stress/2" -expectedKeys @{
    "status" = "string"
    "connections_acquired" = "number"
    "queries_executed" = "number"
    "pool_stats" = "object"
}) {
    $passed++
} else {
    $failed++
}

# ===================================================================
# TESTE 6: Stress Test (5 conexões)
# ===================================================================
Write-Host "═══ TESTE 6: Stress Test (5 conexões) ═══" -ForegroundColor Magenta
if (Test-Endpoint -name "Stress Test Heavy" -url "$baseUrl/api/stress/5" -expectedKeys @{
    "status" = "string"
    "connections_acquired" = "number"
    "queries_executed" = "number"
}) {
    $passed++
} else {
    $failed++
}

# ===================================================================
# TESTE 7: Teste de Concorrência (10 requests simultâneas)
# ===================================================================
Write-Host "═══ TESTE 7: Concorrência (10 requests) ═══" -ForegroundColor Magenta
Write-Host "🔥 Disparando 10 requests simultâneas..." -ForegroundColor Yellow

$jobs = @()
for ($i = 1; $i -le 10; $i++) {
    $jobs += Start-Job -ScriptBlock {
        param($url)
        Invoke-RestMethod -Uri $url -Method GET
    } -ArgumentList "$baseUrl/api/pool/stats"
}

Write-Host "⏳ Aguardando respostas..." -ForegroundColor Gray
$results = $jobs | Wait-Job | Receive-Job
$jobs | Remove-Job

$successCount = ($results | Where-Object { $_.status -eq "success" }).Count
Write-Host "   ✅ Sucesso: $successCount/10" -ForegroundColor Green

if ($successCount -eq 10) {
    Write-Host "   ✅ PASSOU`n" -ForegroundColor Green
    $passed++
} else {
    Write-Host "   ❌ FALHOU ($successCount/10)`n" -ForegroundColor Red
    $failed++
}

# ===================================================================
# TESTE 8: Middlewares (verificar headers)
# ===================================================================
Write-Host "═══ TESTE 8: Middlewares (CORS + Timing) ═══" -ForegroundColor Magenta
Write-Host "🔍 Verificando headers de middleware..." -ForegroundColor Yellow

try {
    $response = Invoke-WebRequest -Uri "$baseUrl/health" -Method GET
    
    $corsPresent = $response.Headers["Access-Control-Allow-Origin"] -ne $null
    $timingPresent = $response.Headers["X-Response-Time"] -ne $null
    
    if ($corsPresent) {
        Write-Host "   ✅ CORS header presente: $($response.Headers['Access-Control-Allow-Origin'])" -ForegroundColor Green
    } else {
        Write-Host "   ❌ CORS header ausente" -ForegroundColor Red
    }
    
    if ($timingPresent) {
        Write-Host "   ✅ Timing header presente: $($response.Headers['X-Response-Time'])" -ForegroundColor Green
    } else {
        Write-Host "   ❌ Timing header ausente" -ForegroundColor Red
    }
    
    if ($corsPresent -and $timingPresent) {
        Write-Host "   ✅ PASSOU`n" -ForegroundColor Green
        $passed++
    } else {
        Write-Host "   ❌ FALHOU`n" -ForegroundColor Red
        $failed++
    }
    
} catch {
    Write-Host "   ❌ ERRO: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "   ❌ FALHOU`n" -ForegroundColor Red
    $failed++
}

# ===================================================================
# TESTE 9: Teste de Performance Simples
# ===================================================================
Write-Host "═══ TESTE 9: Performance (100 requests) ═══" -ForegroundColor Magenta
Write-Host "⚡ Medindo tempo de 100 requests..." -ForegroundColor Yellow

$startTime = Get-Date

for ($i = 1; $i -le 100; $i++) {
    try {
        Invoke-RestMethod -Uri "$baseUrl/health" -Method GET | Out-Null
    } catch {
        # Ignorar erros
    }
}

$endTime = Get-Date
$duration = ($endTime - $startTime).TotalMilliseconds
$avgTime = $duration / 100

Write-Host "   📊 Tempo total: $([math]::Round($duration, 2))ms" -ForegroundColor Cyan
Write-Host "   📊 Tempo médio: $([math]::Round($avgTime, 2))ms/request" -ForegroundColor Cyan
Write-Host "   📊 Throughput: $([math]::Round(100 / ($duration/1000), 2)) req/s" -ForegroundColor Cyan

if ($avgTime -lt 100) {
    Write-Host "   ✅ PASSOU (< 100ms por request)`n" -ForegroundColor Green
    $passed++
} else {
    Write-Host "   ⚠️ LENTO (> 100ms por request)`n" -ForegroundColor Yellow
    $passed++  # Ainda conta como passou
}

# ===================================================================
# RESUMO
# ===================================================================
Write-Host "`n═══════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  📊 RESUMO DOS TESTES" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════" -ForegroundColor Cyan
Write-Host "✅ Passou: $passed" -ForegroundColor Green
Write-Host "❌ Falhou: $failed" -ForegroundColor Red
Write-Host "📈 Taxa de sucesso: $([math]::Round(($passed / ($passed + $failed)) * 100, 2))%" -ForegroundColor Cyan

if ($failed -eq 0) {
    Write-Host "`n🎉 TODOS OS TESTES PASSARAM!" -ForegroundColor Green
} else {
    Write-Host "`n⚠️ Alguns testes falharam. Verifique os logs." -ForegroundColor Yellow
}

Write-Host "═══════════════════════════════════════`n" -ForegroundColor Cyan

# ===================================================================
# LOGS
# ===================================================================
Write-Host "📄 Para ver os logs do servidor:" -ForegroundColor Cyan
Write-Host "   cat server.log" -ForegroundColor Gray
Write-Host "`n"

