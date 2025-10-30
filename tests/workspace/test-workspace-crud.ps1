# ==============================================================================
# Teste completo do módulo WORKSPACE
# ==============================================================================

$baseUrl = "http://localhost:8080/api"
$testsPassed = 0
$testsFailed = 0

Write-Host "`n============================================================" -ForegroundColor Cyan
Write-Host "  TESTES - MODULO WORKSPACE" -ForegroundColor Cyan
Write-Host "============================================================`n" -ForegroundColor Cyan

# Variáveis globais para os testes
$token = ""
$workspaceId = ""
$secondUserId = ""

# Função auxiliar para fazer requests
function Invoke-ApiTest {
    param(
        [string]$Method,
        [string]$Endpoint,
        [string]$Body,
        [string]$Token,
        [string]$TestName
    )
    
    try {
        $headers = @{
            "Content-Type" = "application/json"
        }
        
        if ($Token) {
            $headers["Authorization"] = "Bearer $Token"
        }
        
        $params = @{
            Uri = "$baseUrl$Endpoint"
            Method = $Method
            Headers = $headers
            ErrorAction = "Stop"
        }
        
        if ($Body) {
            $params.Body = $Body
        }
        
        $response = Invoke-RestMethod @params
        return @{ Success = $true; Data = $response }
        
    } catch {
        return @{ Success = $false; Error = $_.Exception.Message }
    }
}

# ==============================================================================
# SETUP: Criar usuário e fazer login
# ==============================================================================

Write-Host "`n[SETUP] Criando usuario para testes..." -ForegroundColor Yellow

$registerBody = @{
    name = "Test User Workspace"
    email = "workspace.test@test.com"
    password = "Test123!@#"
} | ConvertTo-Json

$result = Invoke-ApiTest -Method "POST" -Endpoint "/auth/register" -Body $registerBody -TestName "Register"

if ($result.Success) {
    Write-Host "  Usuario criado!" -ForegroundColor Green
    $userId = $result.Data.user_id
} else {
    Write-Host "  Falha ao criar usuario, tentando login..." -ForegroundColor Yellow
}

# Login
Write-Host "`n[SETUP] Fazendo login..." -ForegroundColor Yellow

$loginBody = @{
    email = "workspace.test@test.com"
    password = "Test123!@#"
} | ConvertTo-Json

$result = Invoke-ApiTest -Method "POST" -Endpoint "/auth/login" -Body $loginBody -TestName "Login"

if ($result.Success) {
    $token = $result.Data.access_token
    $userId = $result.Data.user.id
    Write-Host "  Login bem-sucedido! Token obtido." -ForegroundColor Green
} else {
    Write-Host "  ERRO: Nao foi possivel fazer login!" -ForegroundColor Red
    Write-Host "  Verifique se o servidor esta rodando." -ForegroundColor Red
    exit 1
}

# ==============================================================================
# TESTE 1: Criar workspace
# ==============================================================================

Write-Host "`n[TESTE 1] Criando workspace..." -ForegroundColor Cyan

$createBody = @{
    name = "My Test Workspace"
    slug = "my-test-workspace"
    type = "team"
    description = "Workspace de testes"
} | ConvertTo-Json

$result = Invoke-ApiTest -Method "POST" -Endpoint "/workspaces" -Body $createBody -Token $token -TestName "Create Workspace"

if ($result.Success -and $result.Data.success) {
    $workspaceId = $result.Data.workspace_id
    Write-Host "  PASS: Workspace criado - ID: $workspaceId" -ForegroundColor Green
    $testsPassed++
} else {
    Write-Host "  FAIL: $($result.Error)" -ForegroundColor Red
    $testsFailed++
}

# ==============================================================================
# TESTE 2: Buscar workspace por ID
# ==============================================================================

Write-Host "`n[TESTE 2] Buscando workspace por ID..." -ForegroundColor Cyan

$result = Invoke-ApiTest -Method "GET" -Endpoint "/workspaces/$workspaceId" -Token $token -TestName "Get Workspace"

if ($result.Success -and $result.Data.workspace) {
    $ws = $result.Data.workspace
    Write-Host "  PASS: Workspace encontrado" -ForegroundColor Green
    Write-Host "    Nome: $($ws.name)" -ForegroundColor Gray
    Write-Host "    Slug: $($ws.slug)" -ForegroundColor Gray
    Write-Host "    Type: $($ws.type)" -ForegroundColor Gray
    $testsPassed++
} else {
    Write-Host "  FAIL: $($result.Error)" -ForegroundColor Red
    $testsFailed++
}

# ==============================================================================
# TESTE 3: Listar workspaces
# ==============================================================================

Write-Host "`n[TESTE 3] Listando workspaces..." -ForegroundColor Cyan

$result = Invoke-ApiTest -Method "GET" -Endpoint "/workspaces" -Token $token -TestName "List Workspaces"

if ($result.Success -and $result.Data.workspaces) {
    Write-Host "  PASS: Workspaces listados - Total: $($result.Data.total)" -ForegroundColor Green
    foreach ($ws in $result.Data.workspaces) {
        Write-Host "    - $($ws.name) ($($ws.slug))" -ForegroundColor Gray
    }
    $testsPassed++
} else {
    Write-Host "  FAIL: $($result.Error)" -ForegroundColor Red
    $testsFailed++
}

# ==============================================================================
# TESTE 4: Atualizar workspace
# ==============================================================================

Write-Host "`n[TESTE 4] Atualizando workspace..." -ForegroundColor Cyan

$updateBody = @{
    name = "Updated Workspace Name"
    description = "Descricao atualizada"
} | ConvertTo-Json

$result = Invoke-ApiTest -Method "PUT" -Endpoint "/workspaces/$workspaceId" -Body $updateBody -Token $token -TestName "Update Workspace"

if ($result.Success -and $result.Data.success) {
    Write-Host "  PASS: Workspace atualizado" -ForegroundColor Green
    $testsPassed++
} else {
    Write-Host "  FAIL: $($result.Error)" -ForegroundColor Red
    $testsFailed++
}

# ==============================================================================
# TESTE 5: Convidar membro (simulado - precisa de outro usuário)
# ==============================================================================

Write-Host "`n[TESTE 5] Testando convite de membro..." -ForegroundColor Cyan
Write-Host "  SKIP: Requer segundo usuario (nao implementado neste teste)" -ForegroundColor Yellow

# ==============================================================================
# TESTE 6: Listar membros do workspace
# ==============================================================================

Write-Host "`n[TESTE 6] Listando membros do workspace..." -ForegroundColor Cyan

$result = Invoke-ApiTest -Method "GET" -Endpoint "/workspaces/$workspaceId/members" -Token $token -TestName "List Members"

if ($result.Success) {
    Write-Host "  PASS: Membros listados - Total: $($result.Data.total)" -ForegroundColor Green
    foreach ($member in $result.Data.members) {
        Write-Host "    - User: $($member.user_id) - Role: $($member.role)" -ForegroundColor Gray
    }
    $testsPassed++
} else {
    Write-Host "  FAIL: $($result.Error)" -ForegroundColor Red
    $testsFailed++
}

# ==============================================================================
# TESTE 7: Deletar workspace (descomente se quiser testar)
# ==============================================================================

Write-Host "`n[TESTE 7] Deletando workspace..." -ForegroundColor Cyan

$result = Invoke-ApiTest -Method "DELETE" -Endpoint "/workspaces/$workspaceId" -Token $token -TestName "Delete Workspace"

if ($result.Success -and $result.Data.success) {
    Write-Host "  PASS: Workspace deletado" -ForegroundColor Green
    $testsPassed++
} else {
    Write-Host "  FAIL: $($result.Error)" -ForegroundColor Red
    $testsFailed++
}

# ==============================================================================
# RESUMO
# ==============================================================================

Write-Host "`n============================================================" -ForegroundColor Cyan
Write-Host "  RESUMO DOS TESTES" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan

$total = $testsPassed + $testsFailed
Write-Host "`nTotal de testes: $total" -ForegroundColor White
Write-Host "Passou: $testsPassed" -ForegroundColor Green
Write-Host "Falhou: $testsFailed" -ForegroundColor Red

if ($testsFailed -eq 0) {
    Write-Host "`n✅ TODOS OS TESTES PASSARAM!" -ForegroundColor Green
} else {
    Write-Host "`n❌ ALGUNS TESTES FALHARAM" -ForegroundColor Red
}

Write-Host ""

