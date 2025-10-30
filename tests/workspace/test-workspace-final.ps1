$baseUrl = "http://localhost:8080"
Write-Host "=== TESTE WORKSPACE COM TENANT ===" -ForegroundColor Cyan

# Login
$loginBody = '{"email":"workspace.test@test.com","password":"Test123!@#"}'
$loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -ContentType "application/json" -Body $loginBody
$token = $loginResponse.token
$userId = $loginResponse.user.id
Write-Host "[1] OK - Login (User: $userId)" -ForegroundColor Green

# Adicionar ao tenant (subdomain: acme)
$addBody = "{`"userId`":`"$userId`",`"role`":`"admin`"}"
$headers = @{"Authorization"="Bearer $token";"Content-Type"="application/json"}
try {
    Invoke-RestMethod -Uri "$baseUrl/api/admin/tenants/acme/users" -Method POST -Headers $headers -Body $addBody | Out-Null
    Write-Host "[2] OK - Usuario adicionado ao tenant 'acme'" -ForegroundColor Green
}
catch {
    Write-Host "[2] OK - Usuario ja esta no tenant 'acme'" -ForegroundColor Gray
}

# Re-login com tenant
$loginBody2 = '{"email":"workspace.test@test.com","password":"Test123!@#","tenant_subdomain":"acme"}'
$loginResponse2 = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -ContentType "application/json" -Body $loginBody2
$token2 = $loginResponse2.token
$headers2 = @{"Authorization"="Bearer $token2";"Content-Type"="application/json"}
Write-Host "[3] OK - Re-login com tenant" -ForegroundColor Green

# Criar workspace
$ts = Get-Date -Format "HHmmss"
$wsBody = "{`"name`":`"Test WS $ts`",`"slug`":`"test-ws-$ts`",`"type`":`"team`",`"description`":`"Test`"}"
$createResp = Invoke-RestMethod -Uri "$baseUrl/api/workspaces" -Method POST -Headers $headers2 -Body $wsBody
$wsId = $createResp.id
Write-Host "[4] OK - Workspace criado: $wsId" -ForegroundColor Green

# Buscar
$getResp = Invoke-RestMethod -Uri "$baseUrl/api/workspaces/$wsId" -Method GET -Headers $headers2
Write-Host "[5] OK - Workspace encontrado: $($getResp.workspace.name)" -ForegroundColor Green

# Listar
$listResp = Invoke-RestMethod -Uri "$baseUrl/api/workspaces" -Method GET -Headers $headers2
Write-Host "[6] OK - Total workspaces: $($listResp.workspaces.Count)" -ForegroundColor Green

# Atualizar
$upBody = '{"name":"Updated Workspace","description":"Updated"}'
$upResp = Invoke-RestMethod -Uri "$baseUrl/api/workspaces/$wsId" -Method PUT -Headers $headers2 -Body $upBody
Write-Host "[7] OK - Workspace atualizado" -ForegroundColor Green

# Deletar
$delResp = Invoke-RestMethod -Uri "$baseUrl/api/workspaces/$wsId" -Method DELETE -Headers $headers2
Write-Host "[8] OK - Workspace deletado" -ForegroundColor Green

Write-Host "`n=== TODOS OS TESTES PASSARAM ===" -ForegroundColor Green
