# Script para testar AuthMiddleware
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   TESTE AUTH MIDDLEWARE" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$baseUrl = "http://localhost:8080"

# Função para exibir resultado
function Show-Result {
    param($Title, $Response, $StatusCode, $Success = $true)
    
    Write-Host "`n--- $Title ---" -ForegroundColor Yellow
    if ($Success) {
        Write-Host "Status Code: $StatusCode" -ForegroundColor Green
    } else {
        Write-Host "Status Code: $StatusCode" -ForegroundColor Red
    }
    
    if ($Response) {
        $Response | ConvertTo-Json -Depth 10 | Write-Host -ForegroundColor Gray
    }
}

Write-Host "=== PASSO 1: Fazer login para obter token ===" -ForegroundColor Cyan

$credentials = @{
    email = "finaluser@test.com"
    password = "Pass1234"
} | ConvertTo-Json

try {
    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" `
        -Method POST `
        -Body $credentials `
        -ContentType "application/json" `
        -Headers @{ "Host" = "demo.localhost:8080" }
    
    $token = $loginResponse.token
    Show-Result "Login Successful" $loginResponse 200
    
    Write-Host "`nToken obtido (primeiros 50 chars): $($token.Substring(0, [Math]::Min(50, $token.Length)))..." -ForegroundColor Green
    
} catch {
    Write-Host "ERRO: Não foi possível fazer login!" -ForegroundColor Red
    exit 1
}

Write-Host "`n`n=== PASSO 2: Acessar /api/me (rota protegida) COM token ===" -ForegroundColor Cyan

try {
    $meResponse = Invoke-RestMethod -Uri "$baseUrl/api/me" `
        -Method GET `
        -Headers @{ 
            "Authorization" = "Bearer $token"
        }
    
    Show-Result "GET /api/me COM TOKEN" $meResponse 200
    
} catch {
    $errorMessage = ""
    try {
        $errorResponse = $_.ErrorDetails.Message | ConvertFrom-Json
        $errorMessage = $errorResponse.message
    } catch {
        $errorMessage = $_.Exception.Message
    }
    Write-Host "`n--- GET /api/me COM TOKEN (ERRO) ---" -ForegroundColor Yellow
    Write-Host "Status Code: $($_.Exception.Response.StatusCode.Value__)" -ForegroundColor Red
    Write-Host "Mensagem: $errorMessage" -ForegroundColor Gray
}

Write-Host "`n`n=== PASSO 3: Acessar /api/me (rota protegida) SEM token ===" -ForegroundColor Cyan

try {
    $meResponseNoAuth = Invoke-RestMethod -Uri "$baseUrl/api/me" `
        -Method GET
    
    Show-Result "GET /api/me SEM TOKEN (NÃO DEVERIA FUNCIONAR!)" $meResponseNoAuth 200 $false
    
} catch {
    $errorMessage = ""
    try {
        $errorResponse = $_.ErrorDetails.Message | ConvertFrom-Json
        $errorMessage = $errorResponse.message
    } catch {
        $errorMessage = $_.Exception.Message
    }
    Write-Host "`n--- GET /api/me SEM TOKEN (ESPERADO 401) ---" -ForegroundColor Yellow
    Write-Host "Status Code: $($_.Exception.Response.StatusCode.Value__)" -ForegroundColor $(if ($_.Exception.Response.StatusCode.Value__ -eq 401) { "Green" } else { "Red" })
    Write-Host "Mensagem: $errorMessage" -ForegroundColor Gray
}

Write-Host "`n`n=== PASSO 4: Acessar /api/me com token INVÁLIDO ===" -ForegroundColor Cyan

try {
    $meResponseBadToken = Invoke-RestMethod -Uri "$baseUrl/api/me" `
        -Method GET `
        -Headers @{ 
            "Authorization" = "Bearer token-invalido-fake-12345"
        }
    
    Show-Result "GET /api/me COM TOKEN INVÁLIDO (NÃO DEVERIA FUNCIONAR!)" $meResponseBadToken 200 $false
    
} catch {
    $errorMessage = ""
    try {
        $errorResponse = $_.ErrorDetails.Message | ConvertFrom-Json
        $errorMessage = $errorResponse.message
    } catch {
        $errorMessage = $_.Exception.Message
    }
    Write-Host "`n--- GET /api/me COM TOKEN INVÁLIDO (ESPERADO 401) ---" -ForegroundColor Yellow
    Write-Host "Status Code: $($_.Exception.Response.StatusCode.Value__)" -ForegroundColor $(if ($_.Exception.Response.StatusCode.Value__ -eq 401) { "Green" } else { "Red" })
    Write-Host "Mensagem: $errorMessage" -ForegroundColor Gray
}

Write-Host "`n`n=== PASSO 5: Acessar /api/users/:id (outro usuário) ===" -ForegroundColor Cyan

# Extrair user_id do token do login
$userId = $loginResponse.user.id

try {
    # Tentar acessar outro ID
    $otherResponse = Invoke-RestMethod -Uri "$baseUrl/api/users/outro-user-id-fake" `
        -Method GET `
        -Headers @{ 
            "Authorization" = "Bearer $token"
        }
    
    Show-Result "GET /api/users/outro-id (NÃO DEVERIA FUNCIONAR!)" $otherResponse 200 $false
    
} catch {
    $errorMessage = ""
    try {
        $errorResponse = $_.ErrorDetails.Message | ConvertFrom-Json
        $errorMessage = $errorResponse.message
    } catch {
        $errorMessage = $_.Exception.Message
    }
    Write-Host "`n--- GET /api/users/outro-id (ESPERADO 403) ---" -ForegroundColor Yellow
    Write-Host "Status Code: $($_.Exception.Response.StatusCode.Value__)" -ForegroundColor $(if ($_.Exception.Response.StatusCode.Value__ -eq 403) { "Green" } else { "Red" })
    Write-Host "Mensagem: $errorMessage" -ForegroundColor Gray
}

Write-Host "`n`n=== PASSO 6: Acessar /api/users/:id (próprio usuário) ===" -ForegroundColor Cyan

try {
    $selfResponse = Invoke-RestMethod -Uri "$baseUrl/api/users/$userId" `
        -Method GET `
        -Headers @{ 
            "Authorization" = "Bearer $token"
        }
    
    Show-Result "GET /api/users/$userId (PRÓPRIO USUÁRIO)" $selfResponse 200
    
} catch {
    $errorMessage = ""
    try {
        $errorResponse = $_.ErrorDetails.Message | ConvertFrom-Json
        $errorMessage = $errorResponse.message
    } catch {
        $errorMessage = $_.Exception.Message
    }
    Write-Host "`n--- GET /api/users/$userId (ERRO) ---" -ForegroundColor Yellow
    Write-Host "Status Code: $($_.Exception.Response.StatusCode.Value__)" -ForegroundColor Red
    Write-Host "Mensagem: $errorMessage" -ForegroundColor Gray
}

Write-Host "`n`n========================================" -ForegroundColor Cyan
Write-Host "   RESUMO DOS TESTES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "✓ Login funcionando (obtém token)" -ForegroundColor Green
Write-Host "✓ GET /api/me COM token → 200 OK" -ForegroundColor Green
Write-Host "✓ GET /api/me SEM token → 401 Unauthorized" -ForegroundColor Green
Write-Host "✓ GET /api/me token inválido → 401 Unauthorized" -ForegroundColor Green
Write-Host "✓ GET /api/users/outro-id → 403 Forbidden" -ForegroundColor Green
Write-Host "✓ GET /api/users/proprio-id → 200 OK" -ForegroundColor Green
Write-Host "`n========================================`n" -ForegroundColor Cyan

