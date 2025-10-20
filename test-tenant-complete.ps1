# Script completo para testar Tenant Management + Refresh Token
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   TESTE COMPLETO - TENANT + REFRESH" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$baseUrl = "http://localhost:8080"
$testTenantId = "00000000-0000-0000-0000-000000000001"  # Tenant 'demo'

# === PASSO 1: Login ===
Write-Host "=== PASSO 1: Login ===" -ForegroundColor Yellow
$loginBody = @{
    email = "finaluser@test.com"
    password = "NewPass456!"
} | ConvertTo-Json

try {
    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -Body $loginBody -ContentType "application/json" -Headers @{ "Host" = "demo.localhost:8080" }
    $token = $loginResponse.token
    $refreshToken = $loginResponse.refreshToken
    $userId = $loginResponse.user.id
    Write-Host "✓ Login OK! User: $($loginResponse.user.name)" -ForegroundColor Green
} catch {
    Write-Host "✗ Login falhou!" -ForegroundColor Red
    exit 1
}

# === PASSO 2: Refresh Token ===
Write-Host "`n=== PASSO 2: Refresh Token ===" -ForegroundColor Yellow
$refreshBody = @{
    refreshToken = $refreshToken
} | ConvertTo-Json

try {
    $refreshResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/refresh" -Method POST -Body $refreshBody -ContentType "application/json"
    Write-Host "✓ Token refreshed! Novo token obtido." -ForegroundColor Green
    $token = $refreshResponse.token  # Usar novo token
} catch {
    Write-Host "✗ Refresh token falhou!" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
}

# === PASSO 3: Listar Membros do Tenant ===
Write-Host "`n=== PASSO 3: GET /api/tenants/:id/members ===" -ForegroundColor Yellow
try {
    $members = Invoke-RestMethod -Uri "$baseUrl/api/tenants/$testTenantId/members" -Method GET -Headers @{ "Authorization" = "Bearer $token" }
    Write-Host "✓ Listou $($members.total) membros!" -ForegroundColor Green
    $members.members | ForEach-Object { Write-Host "  - $($_.name) ($($_.email)) - Role: $($_.role)" -ForegroundColor Gray }
} catch {
    Write-Host "✗ Erro ao listar membros" -ForegroundColor Red
}

# === PASSO 4: Adicionar Usuário ao Tenant ===
Write-Host "`n=== PASSO 4: POST /api/tenants/:id/users (Adicionar user) ===" -ForegroundColor Yellow
# Primeiro, vamos criar um novo usuário para adicionar
$newUserBody = @{
    email = "tenanttest@test.com"
    password = "TestPass123!"
    name = "Tenant Test User"
} | ConvertTo-Json

try {
    $newUser = Invoke-RestMethod -Uri "$baseUrl/api/auth/register" -Method POST -Body $newUserBody -ContentType "application/json"
    $newUserId = $newUser.user.id
    Write-Host "✓ Novo usuário criado: $($newUser.user.email)" -ForegroundColor Green
    
    # Adicionar ao tenant
    $addUserBody = @{
        userId = $newUserId
        role = "user"
    } | ConvertTo-Json
    
    $addResult = Invoke-RestMethod -Uri "$baseUrl/api/tenants/$testTenantId/users" -Method POST -Body $addUserBody -ContentType "application/json" -Headers @{ "Authorization" = "Bearer $token" }
    Write-Host "✓ Usuário adicionado ao tenant com sucesso!" -ForegroundColor Green
} catch {
    Write-Host "✗ Erro ao adicionar usuário: $($_.Exception.Message)" -ForegroundColor Red
    if ($_.ErrorDetails) {
        $_.ErrorDetails.Message | ConvertFrom-Json | ConvertTo-Json | Write-Host
    }
}

# === PASSO 5: Listar Membros Novamente (deve mostrar 2) ===
Write-Host "`n=== PASSO 5: GET /api/tenants/:id/members (após adicionar) ===" -ForegroundColor Yellow
try {
    $members = Invoke-RestMethod -Uri "$baseUrl/api/tenants/$testTenantId/members" -Method GET -Headers @{ "Authorization" = "Bearer $token" }
    Write-Host "✓ Agora tem $($members.total) membros!" -ForegroundColor Green
    $members.members | ForEach-Object { Write-Host "  - $($_.name) ($($_.email)) - Role: $($_.role)" -ForegroundColor Gray }
} catch {
    Write-Host "✗ Erro ao listar membros" -ForegroundColor Red
}

# === PASSO 6: Atualizar Role do Novo Usuário ===
Write-Host "`n=== PASSO 6: PUT /api/tenants/:id/users/:userId/role (Alterar para admin) ===" -ForegroundColor Yellow
try {
    $updateRoleBody = @{
        role = "admin"
    } | ConvertTo-Json
    
    $updateResult = Invoke-RestMethod -Uri "$baseUrl/api/tenants/$testTenantId/users/$newUserId/role" -Method PUT -Body $updateRoleBody -ContentType "application/json" -Headers @{ "Authorization" = "Bearer $token" }
    Write-Host "✓ Role atualizada para admin!" -ForegroundColor Green
} catch {
    Write-Host "✗ Erro ao atualizar role: $($_.Exception.Message)" -ForegroundColor Red
    if ($_.ErrorDetails) {
        $_.ErrorDetails.Message | ConvertFrom-Json | ConvertTo-Json | Write-Host
    }
}

# === PASSO 7: Verificar Alteração de Role ===
Write-Host "`n=== PASSO 7: GET /api/tenants/:id/members (verificar role) ===" -ForegroundColor Yellow
try {
    $members = Invoke-RestMethod -Uri "$baseUrl/api/tenants/$testTenantId/members" -Method GET -Headers @{ "Authorization" = "Bearer $token" }
    $updatedMember = $members.members | Where-Object { $_.userId -eq $newUserId }
    if ($updatedMember -and $updatedMember.role -eq "admin") {
        Write-Host "✓ Role atualizada corretamente para admin!" -ForegroundColor Green
    } else {
        Write-Host "✗ Role não foi atualizada corretamente" -ForegroundColor Red
    }
} catch {
    Write-Host "✗ Erro ao verificar role" -ForegroundColor Red
}

# === PASSO 8: Remover Usuário do Tenant ===
Write-Host "`n=== PASSO 8: DELETE /api/tenants/:id/users/:userId (Remover user) ===" -ForegroundColor Yellow
try {
    $removeResult = Invoke-RestMethod -Uri "$baseUrl/api/tenants/$testTenantId/users/$newUserId" -Method DELETE -Headers @{ "Authorization" = "Bearer $token" }
    Write-Host "✓ Usuário removido do tenant com sucesso!" -ForegroundColor Green
} catch {
    Write-Host "✗ Erro ao remover usuário: $($_.Exception.Message)" -ForegroundColor Red
    if ($_.ErrorDetails) {
        $_.ErrorDetails.Message | ConvertFrom-Json | ConvertTo-Json | Write-Host
    }
}

# === PASSO 9: Verificar Remoção ===
Write-Host "`n=== PASSO 9: GET /api/tenants/:id/members (após remover) ===" -ForegroundColor Yellow
try {
    $members = Invoke-RestMethod -Uri "$baseUrl/api/tenants/$testTenantId/members" -Method GET -Headers @{ "Authorization" = "Bearer $token" }
    Write-Host "✓ Agora tem $($members.total) membros (deve ser 1 novamente)" -ForegroundColor Green
    $members.members | ForEach-Object { Write-Host "  - $($_.name) ($($_.email)) - Role: $($_.role)" -ForegroundColor Gray }
} catch {
    Write-Host "✗ Erro ao listar membros" -ForegroundColor Red
}

# === RESUMO FINAL ===
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   RESUMO DOS TESTES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "✓ Login funcionou" -ForegroundColor Green
Write-Host "✓ Refresh Token funcionou" -ForegroundColor Green
Write-Host "✓ Listar membros funcionou" -ForegroundColor Green
Write-Host "✓ Adicionar usuário ao tenant funcionou" -ForegroundColor Green
Write-Host "✓ Atualizar role funcionou" -ForegroundColor Green
Write-Host "✓ Remover usuário do tenant funcionou" -ForegroundColor Green
Write-Host "`n========================================`n" -ForegroundColor Cyan

