# ==================== TESTE COMPLETO DO AUDIT LOG ====================
# Script para testar toda a funcionalidade de Audit Log
# ======================================================================

$baseUrl = "http://localhost:8080"
$tenantSubdomain = "acme"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  TESTE COMPLETO DO AUDIT LOG" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# ==================== 1. REGISTRAR UM NOVO USUÁRIO ====================
Write-Host "[1/5] Testando USER_REGISTERED no Audit Log..." -ForegroundColor Yellow

$registerBody = @{
    name = "Teste Audit User"
    email = "audit.test@example.com"
    password = "SecurePass123!"
} | ConvertTo-Json

try {
    $registerResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/register" `
        -Method POST `
        -Headers @{
            "Content-Type" = "application/json"
            "Host" = "$tenantSubdomain.localhost:8080"
        } `
        -Body $registerBody

    Write-Host "✅ Usuário registrado com sucesso!" -ForegroundColor Green
    Write-Host "   User ID: $($registerResponse.user.id)" -ForegroundColor Gray
    $userId = $registerResponse.user.id
} catch {
    Write-Host "❌ Erro ao registrar usuário:" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# ==================== 2. LOGIN DO USUÁRIO ====================
Write-Host ""
Write-Host "[2/5] Testando USER_LOGIN no Audit Log..." -ForegroundColor Yellow

$loginBody = @{
    email = "audit.test@example.com"
    password = "SecurePass123!"
} | ConvertTo-Json

try {
    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" `
        -Method POST `
        -Headers @{
            "Content-Type" = "application/json"
            "Host" = "$tenantSubdomain.localhost:8080"
        } `
        -Body $loginBody

    Write-Host "✅ Login realizado com sucesso!" -ForegroundColor Green
    Write-Host "   Token: $($loginResponse.token.Substring(0, 30))..." -ForegroundColor Gray
    $token = $loginResponse.token
    $tenantId = $loginResponse.user.tenant_id
} catch {
    Write-Host "❌ Erro ao fazer login:" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}

Start-Sleep -Seconds 1

# ==================== 3. TROCAR SENHA ====================
Write-Host ""
Write-Host "[3/5] Testando PASSWORD_CHANGED no Audit Log..." -ForegroundColor Yellow

$changePasswordBody = @{
    old_password = "SecurePass123!"
    new_password = "NewSecurePass456!"
} | ConvertTo-Json

try {
    $changePasswordResponse = Invoke-RestMethod -Uri "$baseUrl/api/users/$userId/password" `
        -Method PUT `
        -Headers @{
            "Content-Type" = "application/json"
            "Authorization" = "Bearer $token"
            "Host" = "$tenantSubdomain.localhost:8080"
        } `
        -Body $changePasswordBody

    Write-Host "✅ Senha alterada com sucesso!" -ForegroundColor Green
} catch {
    Write-Host "❌ Erro ao trocar senha:" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    # Não sair aqui, pois ainda podemos testar a consulta de logs
}

Start-Sleep -Seconds 1

# ==================== 4. FAZER LOGIN COM OWNER PARA VER LOGS ====================
Write-Host ""
Write-Host "[4/5] Fazendo login como OWNER para acessar audit logs..." -ForegroundColor Yellow

$ownerLoginBody = @{
    email = "owner@acme.com"
    password = "SecurePass123!"
} | ConvertTo-Json

try {
    $ownerLoginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" `
        -Method POST `
        -Headers @{
            "Content-Type" = "application/json"
            "Host" = "$tenantSubdomain.localhost:8080"
        } `
        -Body $ownerLoginBody

    Write-Host "✅ Login como OWNER realizado!" -ForegroundColor Green
    $ownerToken = $ownerLoginResponse.token
    $ownerTenantId = $ownerLoginResponse.user.tenant_id
} catch {
    Write-Host "⚠️  Não foi possível fazer login como owner (usuário pode não existir)" -ForegroundColor Yellow
    Write-Host "   Tentando consultar logs com o usuário de teste..." -ForegroundColor Yellow
    $ownerToken = $token
    $ownerTenantId = $tenantId
}

Start-Sleep -Seconds 1

# ==================== 5. CONSULTAR AUDIT LOGS ====================
Write-Host ""
Write-Host "[5/5] Consultando Audit Logs..." -ForegroundColor Yellow

try {
    $auditLogsResponse = Invoke-RestMethod -Uri "$baseUrl/api/admin/audit-logs" `
        -Method GET `
        -Headers @{
            "Authorization" = "Bearer $ownerToken"
            "Host" = "$tenantSubdomain.localhost:8080"
        }

    Write-Host "✅ Audit Logs recuperados com sucesso!" -ForegroundColor Green
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "  AUDIT LOGS ENCONTRADOS" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "Total: $($auditLogsResponse.total)" -ForegroundColor White
    Write-Host "Limit: $($auditLogsResponse.limit)" -ForegroundColor White
    Write-Host "Offset: $($auditLogsResponse.offset)" -ForegroundColor White
    Write-Host ""

    if ($auditLogsResponse.logs -and $auditLogsResponse.logs.Count -gt 0) {
        $counter = 1
        foreach ($log in $auditLogsResponse.logs) {
            Write-Host "[$counter] ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Gray
            Write-Host "  ID:          $($log.id)" -ForegroundColor White
            Write-Host "  Action:      $($log.action)" -ForegroundColor Cyan
            Write-Host "  Resource:    $($log.resource)" -ForegroundColor Yellow
            if ($log.user_id) {
                Write-Host "  User ID:     $($log.user_id)" -ForegroundColor Magenta
            }
            if ($log.tenant_id) {
                Write-Host "  Tenant ID:   $($log.tenant_id)" -ForegroundColor Blue
            }
            if ($log.resource_id) {
                Write-Host "  Resource ID: $($log.resource_id)" -ForegroundColor Green
            }
            Write-Host "  Details:     $($log.details)" -ForegroundColor Gray
            Write-Host "  IP:          $($log.ip_address)" -ForegroundColor DarkGray
            Write-Host "  User Agent:  $($log.user_agent)" -ForegroundColor DarkGray
            Write-Host ""
            $counter++
        }
    } else {
        Write-Host "⚠️  Nenhum log encontrado para este tenant" -ForegroundColor Yellow
    }

} catch {
    Write-Host "❌ Erro ao consultar audit logs:" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
    Write-Host "Status Code: $($_.Exception.Response.StatusCode.value__)" -ForegroundColor Red
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  TESTE COMPLETO" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Ações esperadas no Audit Log:" -ForegroundColor White
Write-Host "  1. USER_REGISTERED - Registro do novo usuário" -ForegroundColor Gray
Write-Host "  2. USER_LOGIN (x2) - Login do usuário de teste + owner" -ForegroundColor Gray
Write-Host "  3. PASSWORD_CHANGED - Troca de senha" -ForegroundColor Gray
Write-Host ""

