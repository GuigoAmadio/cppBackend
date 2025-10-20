# Teste usando CURL (mais confiável que Invoke-WebRequest)
Write-Host "=== TESTE COM CURL ===" -ForegroundColor Cyan

Write-Host "`n1. Testando REGISTER..." -ForegroundColor Yellow
curl -X POST http://localhost:8080/api/auth/register `
    -H "Content-Type: application/json" `
    -d '{\"email\":\"curltest@test.com\",\"password\":\"Pass123ABC\",\"name\":\"Curl Test\"}' `
    --verbose

Write-Host "`n`n2. Testando LOGIN..." -ForegroundColor Yellow
curl -X POST http://localhost:8080/api/auth/login `
    -H "Content-Type: application/json" `
    -d '{\"email\":\"curltest@test.com\",\"password\":\"Pass123ABC\"}' `
    --verbose

