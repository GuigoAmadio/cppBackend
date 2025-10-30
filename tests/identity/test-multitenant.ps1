# Script para testar fluxo multitenant completo
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   TESTE MULTITENANT - C++ BACKEND" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$baseUrl = "http://localhost:8080"

# Função para exibir resultado
function Show-Result {
    param($Title, $Response, $StatusCode)
    
    Write-Host "`n--- $Title ---" -ForegroundColor Yellow
    Write-Host "Status Code: $StatusCode" -ForegroundColor $(if ($StatusCode -lt 400) { "Green" } else { "Red" })
    
    if ($Response) {
        $Response | ConvertTo-Json -Depth 10 | Write-Host -ForegroundColor Gray
    }
}

# Credenciais do usuário que já existe e está associado aos tenants
$credentials = @{
    email = "finaluser@test.com"
    password = "Pass1234"
} | ConvertTo-Json

Write-Host "=== TESTE 1: Login no tenant 'demo' ===" -ForegroundColor Cyan
try {
    $response1 = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" `
        -Method POST `
        -Body $credentials `
        -ContentType "application/json" `
        -Headers @{ "Host" = "demo.localhost:8080" }
    
    Show-Result "Login Demo" $response1 200
    
    # Decodificar JWT para ver o payload
    if ($response1.token) {
        $tokenParts = $response1.token -split '\.'
        if ($tokenParts.Length -eq 3) {
            $payload = $tokenParts[1]
            # Adicionar padding se necessário
            $padding = 4 - ($payload.Length % 4)
            if ($padding -ne 4) {
                $payload += "=" * $padding
            }
            $payload = $payload.Replace('-', '+').Replace('_', '/')
            
            try {
                $decoded = [System.Text.Encoding]::UTF8.GetString([Convert]::FromBase64String($payload))
                Write-Host "`nPayload do JWT (Demo):" -ForegroundColor Magenta
                $decoded | Write-Host -ForegroundColor Gray
            } catch {
                Write-Host "Erro ao decodificar JWT" -ForegroundColor Red
            }
        }
    }
} catch {
    Show-Result "Login Demo (ERRO)" $_.Exception.Message $_.Exception.Response.StatusCode.Value__
}

Write-Host "`n`n=== TESTE 2: Login no tenant 'test' ===" -ForegroundColor Cyan
try {
    $response2 = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" `
        -Method POST `
        -Body $credentials `
        -ContentType "application/json" `
        -Headers @{ "Host" = "test.localhost:8080" }
    
    Show-Result "Login Test" $response2 200
    
    # Decodificar JWT para ver o payload
    if ($response2.token) {
        $tokenParts = $response2.token -split '\.'
        if ($tokenParts.Length -eq 3) {
            $payload = $tokenParts[1]
            # Adicionar padding se necessário
            $padding = 4 - ($payload.Length % 4)
            if ($padding -ne 4) {
                $payload += "=" * $padding
            }
            $payload = $payload.Replace('-', '+').Replace('_', '/')
            
            try {
                $decoded = [System.Text.Encoding]::UTF8.GetString([Convert]::FromBase64String($payload))
                Write-Host "`nPayload do JWT (Test):" -ForegroundColor Magenta
                $decoded | Write-Host -ForegroundColor Gray
            } catch {
                Write-Host "Erro ao decodificar JWT" -ForegroundColor Red
            }
        }
    }
} catch {
    Show-Result "Login Test (ERRO)" $_.Exception.Message $_.Exception.Response.StatusCode.Value__
}

Write-Host "`n`n=== TESTE 3: Login sem subdomain (localhost) ===" -ForegroundColor Cyan
try {
    $response3 = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" `
        -Method POST `
        -Body $credentials `
        -ContentType "application/json" `
        -Headers @{ "Host" = "localhost:8080" }
    
    Show-Result "Login Localhost (sem tenant)" $response3 200
} catch {
    Show-Result "Login Localhost (ERRO)" $_.Exception.Message $_.Exception.Response.StatusCode.Value__
}

Write-Host "`n`n=== TESTE 4: Login com subdomain inexistente ===" -ForegroundColor Cyan
try {
    $response4 = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" `
        -Method POST `
        -Body $credentials `
        -ContentType "application/json" `
        -Headers @{ "Host" = "inexistente.localhost:8080" }
    
    Show-Result "Login Subdomain Inexistente" $response4 200
} catch {
    $errorMessage = ""
    try {
        $errorResponse = $_.ErrorDetails.Message | ConvertFrom-Json
        $errorMessage = $errorResponse.message
    } catch {
        $errorMessage = $_.Exception.Message
    }
    Write-Host "`n--- Login Subdomain Inexistente (ESPERADO ERRO 404) ---" -ForegroundColor Yellow
    Write-Host "Status Code: $($_.Exception.Response.StatusCode.Value__)" -ForegroundColor $(if ($_.Exception.Response.StatusCode.Value__ -eq 404) { "Green" } else { "Red" })
    Write-Host "Mensagem: $errorMessage" -ForegroundColor Gray
}

Write-Host "`n`n========================================" -ForegroundColor Cyan
Write-Host "   RESUMO DOS TESTES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "✓ Login com tenant 'demo' deve retornar JWT com tenant_id" -ForegroundColor Green
Write-Host "✓ Login com tenant 'test' deve retornar JWT com tenant_id diferente" -ForegroundColor Green
Write-Host "✓ Login sem subdomain deve funcionar (fallback)" -ForegroundColor Green
Write-Host "✓ Login com subdomain inexistente deve retornar 404" -ForegroundColor Green
Write-Host "`n========================================`n" -ForegroundColor Cyan

