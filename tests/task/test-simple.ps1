$baseUrl = "http://localhost:8080"
$testCount = 0
$passCount = 0

Write-Host "`n========== TESTE MODULO TASK ==========" -ForegroundColor Cyan

# Limpar dados
Write-Host "`n[1/11] Limpando dados..." -ForegroundColor Yellow
$env:PGPASSWORD="postgre123"
$deleteQueries = @(
    "DELETE FROM task_comments WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')",
    "DELETE FROM checklist_items WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')",
    "DELETE FROM task_checklists WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')",
    "DELETE FROM tasks WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')"
)

foreach ($query in $deleteQueries) {
    psql -U moneymaker_user -d moneymaker_dev -p 5433 -c $query 2>$null | Out-Null
}
Write-Host "   Dados limpos!" -ForegroundColor Green

# Registrar usuario
Write-Host "`n[2/11] Registrando usuario..." -ForegroundColor Yellow
$testCount++
$timestamp = [DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds()
$email = "tasktest$timestamp@test.com"
$password = "Test123456"

$registerJson = @{
    email = $email
    password = $password
    name = "Task Test User"
    tenant_subdomain = "acme"
} | ConvertTo-Json

try {
    $registerResult = Invoke-RestMethod -Uri "$baseUrl/api/auth/register" -Method Post -Body $registerJson -ContentType "application/json"
    $userId = $registerResult.user.id
    Write-Host "   User ID: $userId" -ForegroundColor Gray
    $passCount++
} catch {
    Write-Host "   FALHOU: $_" -ForegroundColor Red
}

# Adicionar ao tenant
Write-Host "`n[3/11] Adicionando ao tenant..." -ForegroundColor Yellow
$addQuery = "INSERT INTO user_tenants (user_id, tenant_id, role) SELECT '$userId', id, 'admin' FROM tenants WHERE subdomain = 'acme' ON CONFLICT DO NOTHING"
psql -U moneymaker_user -d moneymaker_dev -p 5433 -c $addQuery 2>$null | Out-Null
Write-Host "   Adicionado!" -ForegroundColor Green

# Login
Write-Host "`n[4/11] Fazendo login..." -ForegroundColor Yellow
$testCount++
$loginJson = @{
    email = $email
    password = $password
    tenant_subdomain = "acme"
} | ConvertTo-Json

try {
    $loginResult = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method Post -Body $loginJson -ContentType "application/json"
    $token = $loginResult.access_token
    Write-Host "   Token recebido!" -ForegroundColor Gray
    $passCount++
} catch {
    Write-Host "   FALHOU: $_" -ForegroundColor Red
    exit 1
}

$headers = @{
    "Authorization" = "Bearer $token"
    "Content-Type" = "application/json"
}

# Criar Task
Write-Host "`n[5/11] Criando task..." -ForegroundColor Yellow
$testCount++
$taskJson = @{
    title = "Testar modulo Task"
    description = "Validar todos os endpoints"
    task_type = "task"
    status = "todo"
    priority = "high"
    estimated_hours = 10
    tags = @("test", "backend")
} | ConvertTo-Json

try {
    $createResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks" -Method Post -Headers $headers -Body $taskJson
    $taskId = $createResult.id
    Write-Host "   Task ID: $taskId" -ForegroundColor Gray
    $passCount++
} catch {
    Write-Host "   FALHOU: $_" -ForegroundColor Red
}

# Get Task
Write-Host "`n[6/11] Buscando task..." -ForegroundColor Yellow
$testCount++
try {
    $getResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$taskId" -Method Get -Headers $headers
    Write-Host "   Title: $($getResult.title)" -ForegroundColor Gray
    Write-Host "   Status: $($getResult.status)" -ForegroundColor Gray
    $passCount++
} catch {
    Write-Host "   FALHOU: $_" -ForegroundColor Red
}

# List Tasks
Write-Host "`n[7/11] Listando tasks..." -ForegroundColor Yellow
$testCount++
try {
    $listResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks" -Method Get -Headers $headers
    Write-Host "   Total: $($listResult.tasks.Count)" -ForegroundColor Gray
    $passCount++
} catch {
    Write-Host "   FALHOU: $_" -ForegroundColor Red
}

# Update Task
Write-Host "`n[8/11] Atualizando task..." -ForegroundColor Yellow
$testCount++
$updateJson = @{
    title = "Testar modulo Task (ATUALIZADO)"
    priority = "urgent"
} | ConvertTo-Json

try {
    $updateResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$taskId" -Method Put -Headers $headers -Body $updateJson
    Write-Host "   Atualizada!" -ForegroundColor Gray
    $passCount++
} catch {
    Write-Host "   FALHOU: $_" -ForegroundColor Red
}

# Update Status
Write-Host "`n[9/11] Atualizando status..." -ForegroundColor Yellow
$testCount++
$statusJson = @{
    status = "in_progress"
} | ConvertTo-Json

try {
    $statusResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$taskId/status" -Method Put -Headers $headers -Body $statusJson
    Write-Host "   Status atualizado!" -ForegroundColor Gray
    $passCount++
} catch {
    Write-Host "   FALHOU: $_" -ForegroundColor Red
}

# Add Comment
Write-Host "`n[10/11] Adicionando comentario..." -ForegroundColor Yellow
$testCount++
$commentJson = @{
    content = "Teste de comentario funcional"
} | ConvertTo-Json

try {
    $commentResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$taskId/comments" -Method Post -Headers $headers -Body $commentJson
    Write-Host "   Comentario adicionado!" -ForegroundColor Gray
    $passCount++
} catch {
    Write-Host "   FALHOU: $_" -ForegroundColor Red
}

# Get Comments
Write-Host "`n[11/11] Listando comentarios..." -ForegroundColor Yellow
$testCount++
try {
    $commentsResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$taskId/comments" -Method Get -Headers $headers
    Write-Host "   Total: $($commentsResult.comments.Count)" -ForegroundColor Gray
    $passCount++
} catch {
    Write-Host "   FALHOU: $_" -ForegroundColor Red
}

# Resultado
Write-Host "`n=======================================" -ForegroundColor Cyan
Write-Host "RESULTADO:" -ForegroundColor Yellow
Write-Host "  Testes: $testCount" -ForegroundColor White
Write-Host "  Passou: $passCount" -ForegroundColor Green
Write-Host "  Falhou: $($testCount - $passCount)" -ForegroundColor Red
Write-Host "  Taxa: $([math]::Round(($passCount / $testCount) * 100, 2))%" -ForegroundColor Yellow
Write-Host "=======================================" -ForegroundColor Cyan

if ($passCount -eq $testCount) {
    Write-Host "`nMODULO TASK: 100% FUNCIONAL!`n" -ForegroundColor Green -BackgroundColor DarkGreen
    exit 0
} else {
    Write-Host "`nMODULO TASK: ALGUNS TESTES FALHARAM`n" -ForegroundColor Red
    exit 1
}

