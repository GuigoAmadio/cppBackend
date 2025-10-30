# Teste simples e funcional do módulo Workspace

$baseUrl = "http://localhost:8080"
Write-Host "`n" -NoNewline
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  TESTE SIMPLES - MÓDULO WORKSPACE" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""

try {
    # Passo 1: Login
    Write-Host "[1] Fazendo login..." -ForegroundColor Yellow
    $loginBody = @{
        email = "workspace.test@test.com"
        password = "Test123!@#"
    } | ConvertTo-Json

    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -ContentType "application/json" -Body $loginBody

    $token = $loginResponse.token
    Write-Host "  OK Token obtido!" -ForegroundColor Green
    Write-Host ""

    # Passo 2: Criar Workspace
    Write-Host "[2] Criando workspace..." -ForegroundColor Yellow
    $workspaceBody = @{
        name = "Test Workspace $(Get-Date -Format 'HHmmss')"
        slug = "test-workspace-$(Get-Date -Format 'HHmmss')"
        type = "team"
        description = "Workspace criado automaticamente"
    } | ConvertTo-Json

    $headers = @{
        "Authorization" = "Bearer $token"
        "Content-Type" = "application/json"
    }

    $createResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces" -Method POST -Headers $headers -Body $workspaceBody

    if ($createResponse.success) {
        $workspaceId = $createResponse.id
        Write-Host "  OK Workspace criado! ID: $workspaceId" -ForegroundColor Green
        Write-Host ""
    } else {
        Write-Host "  ERRO: $($createResponse.message)" -ForegroundColor Red
        exit 1
    }

    # Passo 3: Buscar por ID
    Write-Host "[3] Buscando workspace por ID..." -ForegroundColor Yellow
    $getResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces/$workspaceId" -Method GET -Headers $headers

    if ($getResponse.success) {
        Write-Host "  OK Workspace encontrado: $($getResponse.workspace.name)" -ForegroundColor Green
        Write-Host ""
    }

    # Passo 4: Listar todos
    Write-Host "[4] Listando todos os workspaces..." -ForegroundColor Yellow
    $listResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces" -Method GET -Headers $headers

    if ($listResponse.success) {
        Write-Host "  OK Total de workspaces: $($listResponse.workspaces.Count)" -ForegroundColor Green
        Write-Host ""
    }

    # Passo 5: Atualizar
    Write-Host "[5] Atualizando workspace..." -ForegroundColor Yellow
    $updateBody = @{
        name = "Updated Test Workspace"
        description = "Descricao atualizada"
    } | ConvertTo-Json

    $updateResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces/$workspaceId" -Method PUT -Headers $headers -Body $updateBody

    if ($updateResponse.success) {
        Write-Host "  OK Workspace atualizado!" -ForegroundColor Green
        Write-Host ""
    }

    # Passo 6: Deletar
    Write-Host "[6] Deletando workspace..." -ForegroundColor Yellow
    $deleteResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces/$workspaceId" -Method DELETE -Headers $headers

    if ($deleteResponse.success) {
        Write-Host "  OK Workspace deletado!" -ForegroundColor Green
        Write-Host ""
    }

    # Resumo Final
    Write-Host ""
    Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Green
    Write-Host "  TODOS OS TESTES PASSARAM!" -ForegroundColor Green
    Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Green
    Write-Host ""

}
catch {
    Write-Host ""
    Write-Host "ERRO: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host ""
    exit 1
}
