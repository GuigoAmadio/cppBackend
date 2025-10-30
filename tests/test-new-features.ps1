# ========================================
# TESTE COMPLETO - NOVAS FEATURES
# ========================================
# Data: 24/10/2025
# Features: Health Checks, Metrics, Database/Admin
# ========================================

$ErrorActionPreference = "Continue"
$baseUrl = "http://localhost:8080"
$testsPassed = 0
$testsFailed = 0

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  TESTE DAS NOVAS FEATURES" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# ========================================
# 1. HEALTH CHECKS
# ========================================
Write-Host "[CATEGORIA 1] HEALTH CHECKS" -ForegroundColor Yellow

# Test 1.1: /health (Liveness)
try {
    $response = Invoke-WebRequest -Uri "$baseUrl/health" -Method Get -UseBasicParsing
    if ($response.StatusCode -eq 200 -and $response.Content -like "*Server is running*") {
        Write-Host "  [OK] GET /health - Liveness probe" -ForegroundColor Green
        $testsPassed++
    } else {
        Write-Host "  [FAIL] GET /health - Resposta inesperada" -ForegroundColor Red
        $testsFailed++
    }
} catch {
    Write-Host "  [FAIL] GET /health - Erro: $_" -ForegroundColor Red
    $testsFailed++
}

# Test 1.2: /ready (Readiness)
try {
    $response = Invoke-WebRequest -Uri "$baseUrl/ready" -Method Get -UseBasicParsing
    $json = $response.Content | ConvertFrom-Json
    if ($response.StatusCode -eq 200 -and $json.status -eq "ready") {
        Write-Host "  [OK] GET /ready - Readiness probe (DB: $($json.database), Redis: $($json.redis))" -ForegroundColor Green
        $testsPassed++
    } else {
        Write-Host "  [FAIL] GET /ready - Status: $($json.status)" -ForegroundColor Red
        $testsFailed++
    }
} catch {
    Write-Host "  [FAIL] GET /ready - Erro: $_" -ForegroundColor Red
    $testsFailed++
}

# Test 1.3: /live (Deep Health Check)
try {
    $response = Invoke-WebRequest -Uri "$baseUrl/live" -Method Get -UseBasicParsing
    $json = $response.Content | ConvertFrom-Json
    if ($response.StatusCode -eq 200 -and $json.status -eq "healthy") {
        Write-Host "  [OK] GET /live - Deep check (${$json.response_time_ms}ms)" -ForegroundColor Green
        $testsPassed++
    } else {
        Write-Host "  [FAIL] GET /live - Status: $($json.status)" -ForegroundColor Red
        $testsFailed++
    }
} catch {
    Write-Host "  [FAIL] GET /live - Erro: $_" -ForegroundColor Red
    $testsFailed++
}

# ========================================
# 2. PROMETHEUS METRICS
# ========================================
Write-Host "`n[CATEGORIA 2] PROMETHEUS METRICS" -ForegroundColor Yellow

# Test 2.1: /metrics
try {
    $response = Invoke-WebRequest -Uri "$baseUrl/metrics" -Method Get -UseBasicParsing
    if ($response.StatusCode -eq 200 -and $response.Content -like "*http_requests_total*") {
        $lines = ($response.Content -split "`n" | Where-Object { $_ -notmatch "^#" -and $_ -ne "" }).Count
        Write-Host "  [OK] GET /metrics - $lines métricas exportadas" -ForegroundColor Green
        $testsPassed++
    } else {
        Write-Host "  [FAIL] GET /metrics - Formato inválido" -ForegroundColor Red
        $testsFailed++
    }
} catch {
    Write-Host "  [FAIL] GET /metrics - Erro: $_" -ForegroundColor Red
    $testsFailed++
}

# ========================================
# 3. AUTENTICAÇÃO (Para Database/Admin)
# ========================================
Write-Host "`n[CATEGORIA 3] AUTENTICAÇÃO" -ForegroundColor Yellow

$loginBody = @{
    email = "dbadmin@test.com"
    password = "Admin@123"
    tenant_subdomain = "acme"
} | ConvertTo-Json

try {
    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method Post -Body $loginBody -ContentType "application/json"
    
    if ($loginResponse.token -or $loginResponse.access_token) {
        $token = if ($loginResponse.token) { $loginResponse.token } else { $loginResponse.access_token }
        Write-Host "  [OK] Login bem-sucedido (Token: $($token.Substring(0,20))...)" -ForegroundColor Green
        $testsPassed++
        
        $headers = @{
            "Authorization" = "Bearer $token"
        }
    } else {
        Write-Host "  [FAIL] Login não retornou token" -ForegroundColor Red
        Write-Host "  Response: $($loginResponse | ConvertTo-Json)" -ForegroundColor Gray
        $testsFailed++
        $headers = $null
    }
} catch {
    Write-Host "  [FAIL] Login falhou: $_" -ForegroundColor Red
    $testsFailed++
    $headers = $null
}

# ========================================
# 4. DATABASE/ADMIN ENDPOINTS
# ========================================
Write-Host "`n[CATEGORIA 4] DATABASE/ADMIN" -ForegroundColor Yellow

if ($headers) {
    # Test 4.1: GET /api/admin/database/tables
    try {
        $response = Invoke-RestMethod -Uri "$baseUrl/api/admin/database/tables" -Method Get -Headers $headers
        if ($response.tables -and $response.count -gt 0) {
            Write-Host "  [OK] GET /api/admin/database/tables - $($response.count) tabelas encontradas" -ForegroundColor Green
            $testsPassed++
            
            # Guardar primeira tabela para próximos testes
            $testTable = $response.tables[0].name
        } else {
            Write-Host "  [FAIL] GET /api/admin/database/tables - Nenhuma tabela encontrada" -ForegroundColor Red
            $testsFailed++
        }
    } catch {
        Write-Host "  [FAIL] GET /api/admin/database/tables - Erro: $_" -ForegroundColor Red
        $testsFailed++
    }
    
    # Test 4.2: GET /api/admin/database/stats
    try {
        $response = Invoke-RestMethod -Uri "$baseUrl/api/admin/database/stats" -Method Get -Headers $headers
        if ($response.table_count -ge 0) {
            $sizeMB = [math]::Round($response.database_size_mb, 2)
            Write-Host "  [OK] GET /api/admin/database/stats - DB: ${sizeMB}MB, Tabelas: $($response.table_count), Rows: $($response.total_rows)" -ForegroundColor Green
            $testsPassed++
        } else {
            Write-Host "  [FAIL] GET /api/admin/database/stats - Dados inválidos" -ForegroundColor Red
            $testsFailed++
        }
    } catch {
        Write-Host "  [FAIL] GET /api/admin/database/stats - Erro: $_" -ForegroundColor Red
        $testsFailed++
    }
    
    # Test 4.3: GET /api/admin/database/tables/:name/schema
    if ($testTable) {
        try {
            $response = Invoke-RestMethod -Uri "$baseUrl/api/admin/database/tables/$testTable/schema" -Method Get -Headers $headers
            if ($response.columns -and $response.column_count -gt 0) {
                Write-Host "  [OK] GET /api/admin/database/tables/$testTable/schema - $($response.column_count) colunas" -ForegroundColor Green
                $testsPassed++
            } else {
                Write-Host "  [FAIL] GET /api/admin/database/tables/$testTable/schema - Sem colunas" -ForegroundColor Red
                $testsFailed++
            }
        } catch {
            Write-Host "  [FAIL] GET /api/admin/database/tables/$testTable/schema - Erro: $_" -ForegroundColor Red
            $testsFailed++
        }
    }
    
    # Test 4.4: GET /api/admin/database/tables/:name/count
    if ($testTable) {
        try {
            $response = Invoke-RestMethod -Uri "$baseUrl/api/admin/database/tables/$testTable/count" -Method Get -Headers $headers
            if ($response.PSObject.Properties.Name -contains "row_count") {
                Write-Host "  [OK] GET /api/admin/database/tables/$testTable/count - $($response.row_count) registros" -ForegroundColor Green
                $testsPassed++
            } else {
                Write-Host "  [FAIL] GET /api/admin/database/tables/$testTable/count - Sem contagem" -ForegroundColor Red
                $testsFailed++
            }
        } catch {
            Write-Host "  [FAIL] GET /api/admin/database/tables/$testTable/count - Erro: $_" -ForegroundColor Red
            $testsFailed++
        }
    }
    
    # Test 4.5: POST /api/admin/database/query (SELECT seguro)
    try {
        $queryBody = @{
            query = "SELECT COUNT(*) FROM users"
            read_only = $true
        } | ConvertTo-Json
        
        $response = Invoke-RestMethod -Uri "$baseUrl/api/admin/database/query" -Method Post -Body $queryBody -Headers $headers -ContentType "application/json"
        if ($response.success -and $response.row_count -ge 0) {
            Write-Host "  [OK] POST /api/admin/database/query - Query executada ($($response.row_count) rows, $($response.execution_time_ms)ms)" -ForegroundColor Green
            $testsPassed++
        } else {
            Write-Host "  [FAIL] POST /api/admin/database/query - Falha na execução" -ForegroundColor Red
            $testsFailed++
        }
    } catch {
        Write-Host "  [FAIL] POST /api/admin/database/query - Erro: $_" -ForegroundColor Red
        $testsFailed++
    }
    
    # Test 4.6: POST /api/admin/database/query (Query perigosa deve ser bloqueada)
    try {
        $dangerousQuery = @{
            query = "DROP TABLE users"
            read_only = $true
        } | ConvertTo-Json
        
        $response = Invoke-RestMethod -Uri "$baseUrl/api/admin/database/query" -Method Post -Body $dangerousQuery -Headers $headers -ContentType "application/json" -ErrorAction Stop
        Write-Host "  [FAIL] POST /api/admin/database/query - Query perigosa NÃO foi bloqueada!" -ForegroundColor Red
        $testsFailed++
    } catch {
        if ($_.Exception.Response.StatusCode.value__ -eq 400) {
            Write-Host "  [OK] POST /api/admin/database/query - Query perigosa foi BLOQUEADA (validação OK)" -ForegroundColor Green
            $testsPassed++
        } else {
            Write-Host "  [FAIL] POST /api/admin/database/query - Erro inesperado: $_" -ForegroundColor Red
            $testsFailed++
        }
    }
    
} else {
    Write-Host "  [SKIP] Testes do Database/Admin pulados (sem autenticação)" -ForegroundColor Yellow
    $testsFailed += 6
}

# ========================================
# RESUMO FINAL
# ========================================
$total = $testsPassed + $testsFailed
$percentage = if ($total -gt 0) { [math]::Round(($testsPassed / $total) * 100, 2) } else { 0 }

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  RESUMO FINAL" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Total de testes: $total" -ForegroundColor White
Write-Host "  Passou: $testsPassed" -ForegroundColor Green
Write-Host "  Falhou: $testsFailed" -ForegroundColor Red
Write-Host "  Taxa de sucesso: $percentage%" -ForegroundColor $(if ($percentage -ge 80) { "Green" } elseif ($percentage -ge 50) { "Yellow" } else { "Red" })
Write-Host "========================================`n" -ForegroundColor Cyan

# Exit code
if ($testsFailed -eq 0) {
    exit 0
} else {
    exit 1
}

