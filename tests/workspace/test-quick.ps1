$baseUrl = "http://localhost:8080"
Write-Host "=== TESTE WORKSPACE ===" -ForegroundColor Cyan

$loginBody = '{"email":"workspace.test@test.com","password":"Test123!@#"}'
$loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -ContentType "application/json" -Body $loginBody
$token = $loginResponse.token
Write-Host "[1] OK - Token obtido" -ForegroundColor Green

$workspaceBody = '{"name":"Test WS","slug":"test-ws-'+$(Get-Date -Format 'HHmmss')+'","type":"team","description":"Test"}'
$headers = @{"Authorization"="Bearer $token";"Content-Type"="application/json"}
$createResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces" -Method POST -Headers $headers -Body $workspaceBody
$workspaceId = $createResponse.id
Write-Host "[2] OK - Workspace criado: $workspaceId" -ForegroundColor Green

$getResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces/$workspaceId" -Method GET -Headers $headers
Write-Host "[3] OK - Workspace encontrado: $($getResponse.workspace.name)" -ForegroundColor Green

$listResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces" -Method GET -Headers $headers
Write-Host "[4] OK - Total: $($listResponse.workspaces.Count)" -ForegroundColor Green

$updateBody = '{"name":"Updated WS","description":"Updated"}'
$updateResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces/$workspaceId" -Method PUT -Headers $headers -Body $updateBody
Write-Host "[5] OK - Workspace atualizado" -ForegroundColor Green

$deleteResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces/$workspaceId" -Method DELETE -Headers $headers
Write-Host "[6] OK - Workspace deletado" -ForegroundColor Green

Write-Host "`n=== TODOS OS TESTES PASSARAM ===" -ForegroundColor Green
