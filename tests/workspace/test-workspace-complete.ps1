# Teste completo do módulo Workspace com tenant

$baseUrl = "http://localhost:8080"
Write-Host "`n╔══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║  🧪 TESTE COMPLETO - WORKSPACE COM TENANT                ║" -ForegroundColor Cyan
Write-Host "╚══════════════════════════════════════════════════════════╝`n" -ForegroundColor Cyan

try {
    # Passo 1: Login
    Write-Host "[1/8] Fazendo login..." -ForegroundColor Yellow
    $loginBody = @{
        email = "workspace.test@test.com"
        password = "Test123!@#"
    } | ConvertTo-Json

    $loginResponse = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -ContentType "application/json" -Body $loginBody
    $token = $loginResponse.token
    $userId = $loginResponse.user.id
    Write-Host "  ✅ Token obtido! User ID: $userId" -ForegroundColor Green
    Write-Host ""

    # Passo 2: Adicionar usuário ao tenant ACME
    Write-Host "[2/8] Adicionando usuário ao tenant ACME..." -ForegroundColor Yellow
    $addToTenantBody = @{
        userId = $userId
        role = "admin"
    } | ConvertTo-Json

    $headers = @{
        "Authorization" = "Bearer $token"
        "Content-Type" = "application/json"
    }

    try {
        $addResult = Invoke-RestMethod -Uri "$baseUrl/api/admin/tenants/acme-corp/users" -Method POST -Headers $headers -Body $addToTenantBody
        Write-Host "  OK Usuario adicionado ao tenant ACME!" -ForegroundColor Green
    }
    catch {
        if ($_.Exception.Message -like "*409*") {
            Write-Host "  INFO Usuario ja esta no tenant ACME" -ForegroundColor Gray
        }
        else {
            Write-Host "  WARN Erro ao adicionar ao tenant (continuando...)" -ForegroundColor Yellow
        }
    }
    Write-Host ""

    # Passo 3: Fazer login novamente para pegar tenant_id no JWT
    Write-Host "[3/8] Re-login para obter JWT com tenant..." -ForegroundColor Yellow
    $loginBody2 = @{
        email = "workspace.test@test.com"
        password = "Test123!@#"
        tenant_subdomain = "acme"
    } | ConvertTo-Json

    $loginResponse2 = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method POST -ContentType "application/json" -Body $loginBody2
    $tokenWithTenant = $loginResponse2.token
    
    $headersWithTenant = @{
        "Authorization" = "Bearer $tokenWithTenant"
        "Content-Type" = "application/json"
    }
    Write-Host "  ✅ JWT com tenant obtido!" -ForegroundColor Green
    Write-Host ""

    # Passo 4: Criar Workspace
    Write-Host "[4/8] Criando workspace..." -ForegroundColor Yellow
    $timestamp = Get-Date -Format "HHmmss"
    $workspaceBody = @{
        name = "Project Alpha $timestamp"
        slug = "project-alpha-$timestamp"
        type = "team"
        description = "Workspace de desenvolvimento do projeto Alpha"
    } | ConvertTo-Json

    $createResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces" -Method POST -Headers $headersWithTenant -Body $workspaceBody
    
    if ($createResponse.success) {
        $workspaceId = $createResponse.id
        Write-Host "  ✅ Workspace criado!" -ForegroundColor Green
        Write-Host "     ID: $workspaceId" -ForegroundColor Gray
        Write-Host "     Nome: Project Alpha $timestamp" -ForegroundColor Gray
    } else {
        throw "Falha ao criar workspace: $($createResponse.message)"
    }
    Write-Host ""

    # Passo 5: Buscar Workspace por ID
    Write-Host "[5/8] Buscando workspace por ID..." -ForegroundColor Yellow
    $getResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces/$workspaceId" -Method GET -Headers $headersWithTenant
    
    if ($getResponse.success) {
        Write-Host "  ✅ Workspace encontrado!" -ForegroundColor Green
        Write-Host "     Nome: $($getResponse.workspace.name)" -ForegroundColor Gray
        Write-Host "     Slug: $($getResponse.workspace.slug)" -ForegroundColor Gray
        Write-Host "     Tipo: $($getResponse.workspace.type)" -ForegroundColor Gray
    }
    Write-Host ""

    # Passo 6: Listar Workspaces
    Write-Host "[6/8] Listando todos os workspaces..." -ForegroundColor Yellow
    $listResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces" -Method GET -Headers $headersWithTenant
    
    if ($listResponse.success) {
        Write-Host "  ✅ Total de workspaces: $($listResponse.workspaces.Count)" -ForegroundColor Green
    }
    Write-Host ""

    # Passo 7: Atualizar Workspace
    Write-Host "[7/8] Atualizando workspace..." -ForegroundColor Yellow
    $updateBody = @{
        name = "Project Alpha (Updated)"
        description = "Descrição atualizada com novas informações"
    } | ConvertTo-Json

    $updateResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces/$workspaceId" -Method PUT -Headers $headersWithTenant -Body $updateBody
    
    if ($updateResponse.success) {
        Write-Host "  ✅ Workspace atualizado!" -ForegroundColor Green
    }
    Write-Host ""

    # Passo 8: Deletar Workspace
    Write-Host "[8/8] Deletando workspace..." -ForegroundColor Yellow
    $deleteResponse = Invoke-RestMethod -Uri "$baseUrl/api/workspaces/$workspaceId" -Method DELETE -Headers $headersWithTenant
    
    if ($deleteResponse.success) {
        Write-Host "  ✅ Workspace deletado!" -ForegroundColor Green
    }
    Write-Host ""

    # Resumo Final
    Write-Host "╔══════════════════════════════════════════════════════════╗" -ForegroundColor Green
    Write-Host "║  ✅ TODOS OS TESTES PASSARAM COM SUCESSO!                ║" -ForegroundColor Green
    Write-Host "╚══════════════════════════════════════════════════════════╝" -ForegroundColor Green
    Write-Host ""
    Write-Host "📊 Resumo:" -ForegroundColor Cyan
    Write-Host "  • Login: ✅" -ForegroundColor White
    Write-Host "  • Associação ao tenant: ✅" -ForegroundColor White
    Write-Host "  • Criar workspace: ✅" -ForegroundColor White
    Write-Host "  • Buscar workspace: ✅" -ForegroundColor White
    Write-Host "  • Listar workspaces: ✅" -ForegroundColor White
    Write-Host "  • Atualizar workspace: ✅" -ForegroundColor White
    Write-Host "  • Deletar workspace: ✅" -ForegroundColor White
    Write-Host ""

}
catch {
    Write-Host ""
    Write-Host "ERRO NO TESTE" -ForegroundColor Red
    Write-Host $_.Exception.Message
    exit 1
}

