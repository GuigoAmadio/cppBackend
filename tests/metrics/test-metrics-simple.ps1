# test-metrics-simple.ps1
Write-Host "Testing /health..." -ForegroundColor Yellow
try {
    $response = Invoke-WebRequest -Uri "http://localhost:8080/health" -Method GET -UseBasicParsing
    Write-Host "  Status: $($response.StatusCode)" -ForegroundColor Green
    Write-Host "  Content: $($response.Content)" -ForegroundColor Gray
} catch {
    Write-Host "  Error: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "  Response: $($_.ErrorDetails.Message)" -ForegroundColor Red
}

Write-Host ""
Write-Host "Testing /metrics..." -ForegroundColor Yellow
try {
    $response = Invoke-WebRequest -Uri "http://localhost:8080/metrics" -Method GET -UseBasicParsing
    Write-Host "  Status: $($response.StatusCode)" -ForegroundColor Green
    Write-Host "  Content-Length: $($response.RawContentLength)" -ForegroundColor Gray
    Write-Host "  Content:" -ForegroundColor Cyan
    Write-Host $response.Content -ForegroundColor White
} catch {
    Write-Host "  Error: $($_.Exception.Message)" -ForegroundColor Red
}

