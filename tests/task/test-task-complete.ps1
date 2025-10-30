# ========================================
# TESTE COMPLETO - MÓDULO TASK
# 10 Endpoints + Validações Completas
# ========================================

$baseUrl = "http://localhost:8080"
$testCounter = 0
$passedTests = 0

function Test-Endpoint {
    param (
        [string]$name,
        [scriptblock]$test
    )
    $script:testCounter++
    Write-Host "`n[$script:testCounter] $name" -ForegroundColor Cyan
    try {
        & $test
        $script:passedTests++
        Write-Host "   PASS" -ForegroundColor Green
        return $true
    } catch {
        Write-Host "   FAIL: $_" -ForegroundColor Red
        return $false
    }
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   TESTE COMPLETO - MÓDULO TASK" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Limpar dados antigos
Write-Host "[SETUP] Limpando dados antigos..." -ForegroundColor Yellow
$env:PGPASSWORD="postgre123"
psql -U moneymaker_user -d moneymaker_dev -p 5433 -c "DELETE FROM task_comments WHERE tenant_id = (SELECT id FROM tenants WHERE subdomain = 'acme');" 2>$null | Out-Null
psql -U moneymaker_user -d moneymaker_dev -p 5433 -c "DELETE FROM checklist_items WHERE tenant_id = (SELECT id FROM tenants WHERE subdomain = 'acme');" 2>$null | Out-Null
psql -U moneymaker_user -d moneymaker_dev -p 5433 -c "DELETE FROM task_checklists WHERE tenant_id = (SELECT id FROM tenants WHERE subdomain = 'acme');" 2>$null | Out-Null
psql -U moneymaker_user -d moneymaker_dev -p 5433 -c "DELETE FROM tasks WHERE tenant_id = (SELECT id FROM tenants WHERE subdomain = 'acme');" 2>$null | Out-Null
Write-Host "[SETUP] Dados limpos!`n" -ForegroundColor Green

# ========================================
# 1. REGISTRO E LOGIN
# ========================================

$timestamp = [DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds()
$testEmail = "task.test.$timestamp@test.com"
$testPassword = "Task@Test123"

Test-Endpoint "REGISTER USER" {
    $registerBody = @{
        email = $testEmail
        password = $testPassword
        name = "Task Test User"
        tenant_subdomain = "acme"
    } | ConvertTo-Json

    $registerResult = Invoke-RestMethod -Uri "$baseUrl/api/auth/register" -Method Post -Body $registerBody -ContentType "application/json"
    
    if (-not $registerResult.user.id) {
        throw "User ID not returned"
    }
    
    $script:userId = $registerResult.user.id
    Write-Host "   User ID: $($script:userId)" -ForegroundColor Gray
}

# Adicionar usuário ao tenant
Write-Host "`n[SETUP] Adicionando usuário ao tenant..." -ForegroundColor Yellow
$addToTenantQuery = "INSERT INTO user_tenants (user_id, tenant_id, role) SELECT '$($script:userId)', id, 'admin' FROM tenants WHERE subdomain = 'acme' ON CONFLICT DO NOTHING;"
psql -U moneymaker_user -d moneymaker_dev -p 5433 -c $addToTenantQuery 2>$null | Out-Null
Write-Host "[SETUP] Usuário adicionado!`n" -ForegroundColor Green

Test-Endpoint "LOGIN USER" {
    $loginBody = @{
        email = $testEmail
        password = $testPassword
        tenant_subdomain = "acme"
    } | ConvertTo-Json

    $loginResult = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method Post -Body $loginBody -ContentType "application/json"
    
    if (-not $loginResult.access_token) {
        throw "Access token not returned"
    }
    
    $script:token = $loginResult.access_token
    Write-Host "   Token recebido (primeiros 20 chars): $($script:token.Substring(0, [Math]::Min(20, $script:token.Length)))..." -ForegroundColor Gray
}

$headers = @{
    "Authorization" = "Bearer $($script:token)"
    "Content-Type" = "application/json"
}

# ========================================
# 2. CRIAR TASK
# ========================================

Test-Endpoint "CREATE TASK" {
    $createBody = @{
        title = "Implementar módulo de Tasks"
        description = "Criar todas as funcionalidades do módulo Task"
        task_type = "feature"
        status = "todo"
        priority = "high"
        estimated_hours = 20
        story_points = 8
        tags = @("backend", "ddd", "cpp")
    } | ConvertTo-Json

    $createResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks" -Method Post -Headers $headers -Body $createBody
    
    if (-not $createResult.id) {
        throw "Task ID not returned"
    }
    
    $script:taskId = $createResult.id
    Write-Host "   Task ID: $($script:taskId)" -ForegroundColor Gray
}

# ========================================
# 3. GET TASK BY ID
# ========================================

Test-Endpoint "GET TASK BY ID" {
    $getResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)" -Method Get -Headers $headers
    
    if ($getResult.id -ne $script:taskId) {
        throw "Task ID mismatch"
    }
    
    if ($getResult.title -ne "Implementar módulo de Tasks") {
        throw "Task title mismatch"
    }
    
    Write-Host "   Task: $($getResult.title)" -ForegroundColor Gray
    Write-Host "   Status: $($getResult.status)" -ForegroundColor Gray
    Write-Host "   Priority: $($getResult.priority)" -ForegroundColor Gray
}

# ========================================
# 4. LIST TASKS
# ========================================

Test-Endpoint "LIST TASKS" {
    $listResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks" -Method Get -Headers $headers
    
    if ($listResult.tasks.Count -eq 0) {
        throw "No tasks returned"
    }
    
    $foundTask = $listResult.tasks | Where-Object { $_.id -eq $script:taskId }
    if (-not $foundTask) {
        throw "Created task not found in list"
    }
    
    Write-Host "   Total tasks: $($listResult.tasks.Count)" -ForegroundColor Gray
}

# ========================================
# 5. UPDATE TASK
# ========================================

Test-Endpoint "UPDATE TASK" {
    $updateBody = @{
        title = "Implementar módulo de Tasks (ATUALIZADO)"
        description = "Criar todas as funcionalidades do módulo Task + Testes"
        priority = "urgent"
        actual_hours = 5
    } | ConvertTo-Json

    $updateResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)" -Method Put -Headers $headers -Body $updateBody
    
    if ($updateResult.message -notmatch "updated") {
        throw "Task not updated"
    }
    
    # Verificar atualização
    $getResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)" -Method Get -Headers $headers
    
    if ($getResult.title -ne "Implementar módulo de Tasks (ATUALIZADO)") {
        throw "Title not updated"
    }
    
    if ($getResult.priority -ne "urgent") {
        throw "Priority not updated"
    }
    
    Write-Host "   Task atualizada com sucesso" -ForegroundColor Gray
}

# ========================================
# 6. UPDATE TASK STATUS
# ========================================

Test-Endpoint "UPDATE TASK STATUS" {
    $statusBody = @{
        status = "in_progress"
    } | ConvertTo-Json

    $statusResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)/status" -Method Put -Headers $headers -Body $statusBody
    
    if ($statusResult.message -notmatch "updated") {
        throw "Status not updated"
    }
    
    # Verificar status
    $getResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)" -Method Get -Headers $headers
    
    if ($getResult.status -ne "in_progress") {
        throw "Status not changed"
    }
    
    Write-Host "   Status: $($getResult.status)" -ForegroundColor Gray
}

# ========================================
# 7. ADD COMMENT
# ========================================

Test-Endpoint "ADD COMMENT" {
    $commentBody = @{
        content = "Progresso: 50% completo"
    } | ConvertTo-Json

    $commentResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)/comments" -Method Post -Headers $headers -Body $commentBody
    
    if ($commentResult.message -notmatch "created|added") {
        throw "Comment not added"
    }
    
    Write-Host "   Comentário adicionado" -ForegroundColor Gray
}

# ========================================
# 8. GET COMMENTS
# ========================================

Test-Endpoint "GET COMMENTS" {
    $commentsResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)/comments" -Method Get -Headers $headers
    
    if ($commentsResult.comments.Count -eq 0) {
        throw "No comments returned"
    }
    
    $foundComment = $commentsResult.comments | Where-Object { $_.content -match "Progresso" }
    if (-not $foundComment) {
        throw "Added comment not found"
    }
    
    Write-Host "   Total comments: $($commentsResult.comments.Count)" -ForegroundColor Gray
}

# ========================================
# 9. ADD CHECKLIST
# ========================================

Test-Endpoint "ADD CHECKLIST" {
    $checklistBody = @{
        title = "Requisitos de Implementação"
    } | ConvertTo-Json

    $checklistResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)/checklists" -Method Post -Headers $headers -Body $checklistBody
    
    if ($checklistResult.message -notmatch "created|added") {
        throw "Checklist not added"
    }
    
    Write-Host "   Checklist adicionada" -ForegroundColor Gray
}

# ========================================
# 10. GET CHECKLISTS
# ========================================

Test-Endpoint "GET CHECKLISTS" {
    $checklistsResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)/checklists" -Method Get -Headers $headers
    
    if ($checklistsResult.checklists.Count -eq 0) {
        throw "No checklists returned"
    }
    
    $foundChecklist = $checklistsResult.checklists | Where-Object { $_.title -match "Requisitos" }
    if (-not $foundChecklist) {
        throw "Added checklist not found"
    }
    
    Write-Host "   Total checklists: $($checklistsResult.checklists.Count)" -ForegroundColor Gray
}

# ========================================
# 11. DELETE TASK
# ========================================

Test-Endpoint "DELETE TASK" {
    $deleteResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)" -Method Delete -Headers $headers
    
    if ($deleteResult.message -notmatch "deleted") {
        throw "Task not deleted"
    }
    
    # Verificar que a task foi deletada
    try {
        $getResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)" -Method Get -Headers $headers
        throw "Task still exists after deletion"
    } catch {
        if ($_.Exception.Response.StatusCode -eq 404) {
            Write-Host "   Task deletada com sucesso" -ForegroundColor Gray
        } else {
            throw $_
        }
    }
}

# ========================================
# RESULTADO FINAL
# ========================================

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   RESULTADO FINAL" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   Testes executados: $testCounter" -ForegroundColor White
Write-Host "   Testes passou: $passedTests" -ForegroundColor Green
Write-Host "   Testes falhou: $($testCounter - $passedTests)" -ForegroundColor Red
Write-Host "   Taxa de sucesso: $([math]::Round(($passedTests / $testCounter) * 100, 2))%" -ForegroundColor Yellow
Write-Host "========================================`n" -ForegroundColor Cyan

if ($passedTests -eq $testCounter) {
    Write-Host "MÓDULO TASK: 100% FUNCIONAL!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "MÓDULO TASK: ALGUNS TESTES FALHARAM" -ForegroundColor Red
    exit 1
}

