# Testes Simples C++ Backend v2.0

$baseUrl = "http://localhost:8080"

Write-Host "`n=== TESTANDO C++ BACKEND v2.0 ===`n"

# Teste 1: Health
Write-Host "1. Health Check..."
try {
    $r = Invoke-RestMethod "$baseUrl/health"
    Write-Host "   OK" -ForegroundColor Green
} catch {
    Write-Host "   FALHOU: $($_.Exception.Message)" -ForegroundColor Red
}

# Teste 2: Pool Stats
Write-Host "2. Pool Stats..."
try {
    $r = Invoke-RestMethod "$baseUrl/api/pool/stats"
    Write-Host "   Total: $($r.total_connections), Disponivel: $($r.available_connections)" -ForegroundColor Cyan
    Write-Host "   OK" -ForegroundColor Green
} catch {
    Write-Host "   FALHOU: $($_.Exception.Message)" -ForegroundColor Red
}

# Teste 3: DB Test
Write-Host "3. Database Test..."
try {
    $r = Invoke-RestMethod "$baseUrl/api/db/test"
    Write-Host "   DB: $($r.database), Versao: $($r.version)" -ForegroundColor Cyan
    Write-Host "   OK" -ForegroundColor Green
} catch {
    Write-Host "   FALHOU: $($_.Exception.Message)" -ForegroundColor Red
}

# Teste 4: DB Query
Write-Host "4. Database Query..."
try {
    $r = Invoke-RestMethod "$baseUrl/api/db/query"
    Write-Host "   Tabelas encontradas: $($r.rows)" -ForegroundColor Cyan
    Write-Host "   OK" -ForegroundColor Green
} catch {
    Write-Host "   FALHOU: $($_.Exception.Message)" -ForegroundColor Red
}

# Teste 5: Stress Test
Write-Host "5. Stress Test (3 conexoes)..."
try {
    $r = Invoke-RestMethod "$baseUrl/api/stress/3"
    Write-Host "   Conexoes: $($r.connections_acquired), Queries: $($r.queries_executed)" -ForegroundColor Cyan
    Write-Host "   OK" -ForegroundColor Green
} catch {
    Write-Host "   FALHOU: $($_.Exception.Message)" -ForegroundColor Red
}

# Teste 6: Middlewares (headers)
Write-Host "6. Middlewares (CORS + Timing)..."
try {
    $r = Invoke-WebRequest "$baseUrl/health"
    $cors = $r.Headers["Access-Control-Allow-Origin"]
    $timing = $r.Headers["X-Response-Time"]
    Write-Host "   CORS: $cors, Timing: $timing" -ForegroundColor Cyan
    Write-Host "   OK" -ForegroundColor Green
} catch {
    Write-Host "   FALHOU: $($_.Exception.Message)" -ForegroundColor Red
}

# Teste 7: Performance
Write-Host "7. Performance (50 requests)..."
$start = Get-Date
for ($i = 1; $i -le 50; $i++) {
    Invoke-RestMethod "$baseUrl/health" | Out-Null
}
$duration = ((Get-Date) - $start).TotalMilliseconds
$avg = $duration / 50
Write-Host "   Tempo medio: $([math]::Round($avg, 2))ms/request" -ForegroundColor Cyan
Write-Host "   OK" -ForegroundColor Green

Write-Host "`n=== TESTES CONCLUIDOS ===`n"

