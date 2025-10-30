# Teste simples de registro sem tenant

$body = @{
    name = "Test User Simple"
    email = "simple@test.com"
    password = "SecurePass123!"
} | ConvertTo-Json

Write-Host "Enviando requisição para http://localhost:8080/api/auth/register" -ForegroundColor Cyan
Write-Host "Body: $body" -ForegroundColor Gray
Write-Host ""

try {
    $response = Invoke-RestMethod -Uri "http://localhost:8080/api/auth/register" `
        -Method POST `
        -Headers @{
            "Content-Type" = "application/json"
            "Host" = "localhost:8080"
        } `
        -Body $body `
        -TimeoutSec 10

    Write-Host "✅ Sucesso!" -ForegroundColor Green
    Write-Host ($response | ConvertTo-Json -Depth 10) -ForegroundColor White
} catch {
    Write-Host "❌ Erro:" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    if ($_.Exception.Response) {
        Write-Host "Status Code: $($_.Exception.Response.StatusCode.value__)" -ForegroundColor Yellow
    }
}

