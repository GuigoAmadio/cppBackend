# ====================================================
# TESTE COMPLETO - MODULO TASK
# 10 Endpoints com Rate Limiting Aumentado
# ====================================================

$baseUrl = "http://localhost:8080"
$testCount = 0
$passCount = 0

function Test-Endpoint {
    param([string]$name, [scriptblock]$test)
    $script:testCount++
    Write-Host "`n[$script:testCount] $name" -ForegroundColor Cyan
    try {
        & $test
        $script:passCount++
        Write-Host "   PASS" -ForegroundColor Green
        return $true
    } catch {
        Write-Host "   FAIL: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

Write-Host "`n" -NoNewline
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host "   TESTE COMPLETO - MODULO TASK" -ForegroundColor Cyan
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host ""

# ====================================================
# SETUP
# ====================================================

Write-Host "[SETUP] Limpando dados antigos..." -ForegroundColor Yellow
$env:PGPASSWORD="postgre123"
$queries = @(
    "DELETE FROM task_comments WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')",
    "DELETE FROM checklist_items WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')",
    "DELETE FROM task_checklists WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')",
    "DELETE FROM tasks WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')"
)

foreach ($q in $queries) {
    psql -U moneymaker_user -d moneymaker_dev -p 5433 -c $q 2>$null | Out-Null
}
Write-Host "[SETUP] Dados limpos!`n" -ForegroundColor Green

# ====================================================
# 1. REGISTRO
# ====================================================

Test-Endpoint "REGISTER USER" {
    $timestamp = [DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds()
    $script:email = "taskfinal$timestamp@test.com"
    $script:password = "TaskFinal123"
    
    $body = @{
        email = $script:email
        password = $script:password
        name = "Task Final Test"
        tenant_subdomain = "acme"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/auth/register" -Method Post -Body $body -ContentType "application/json"
    
    if (-not $result.user.id) {
        throw "User ID not returned"
    }
    
    $script:userId = $result.user.id
    Write-Host "   User ID: $($script:userId)" -ForegroundColor Gray
}

# Adicionar ao tenant
Write-Host "`n[SETUP] Adicionando usuario ao tenant..." -ForegroundColor Yellow
$addQuery = "INSERT INTO user_tenants (user_id, tenant_id, role) SELECT '$($script:userId)', id, 'admin' FROM tenants WHERE subdomain = 'acme' ON CONFLICT DO NOTHING"
psql -U moneymaker_user -d moneymaker_dev -p 5433 -c $addQuery 2>$null | Out-Null
Write-Host "[SETUP] Usuario adicionado!`n" -ForegroundColor Green

# ====================================================
# 2. LOGIN
# ====================================================

Test-Endpoint "LOGIN USER" {
    $body = @{
        email = $script:email
        password = $script:password
        tenant_subdomain = "acme"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method Post -Body $body -ContentType "application/json"
    
    # O backend retorna "token", não "access_token"
    if (-not $result.token) {
        throw "Token not returned"
    }
    
    $script:token = $result.token
    $script:headers = @{
        "Authorization" = "Bearer $($script:token)"
        "Content-Type" = "application/json"
    }
    
    Write-Host "   Token: OK ($($script:token.Length) chars)" -ForegroundColor Gray
}

# ====================================================
# 3. CREATE TASK
# ====================================================

Test-Endpoint "CREATE TASK" {
    $body = @{
        title = "Implementar modulo de Tasks completo"
        description = "Criar todas as funcionalidades do modulo Task com testes"
        task_type = "feature"
        status = "todo"
        priority = "high"
        estimated_hours = 20
        story_points = 8
        tags = @("backend", "ddd", "cpp", "test")
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/tasks" -Method Post -Headers $script:headers -Body $body
    
    if (-not $result.id) {
        throw "Task ID not returned"
    }
    
    $script:taskId = $result.id
    Write-Host "   Task ID: $($script:taskId)" -ForegroundColor Gray
}

# ====================================================
# 4. GET TASK BY ID
# ====================================================

Test-Endpoint "GET TASK BY ID" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)" -Method Get -Headers $script:headers
    
    if ($result.id -ne $script:taskId) {
        throw "Task ID mismatch"
    }
    
    if ($result.title -ne "Implementar modulo de Tasks completo") {
        throw "Task title mismatch"
    }
    
    Write-Host "   Title: $($result.title)" -ForegroundColor Gray
    Write-Host "   Status: $($result.status)" -ForegroundColor Gray
}

# ====================================================
# 5. LIST TASKS
# ====================================================

Test-Endpoint "LIST TASKS" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/tasks" -Method Get -Headers $script:headers
    
    if ($result.tasks.Count -eq 0) {
        throw "No tasks returned"
    }
    
    $foundTask = $result.tasks | Where-Object { $_.id -eq $script:taskId }
    if (-not $foundTask) {
        throw "Created task not found in list"
    }
    
    Write-Host "   Total tasks: $($result.tasks.Count)" -ForegroundColor Gray
}

# ====================================================
# 6. UPDATE TASK
# ====================================================

Test-Endpoint "UPDATE TASK" {
    $body = @{
        title = "Implementar modulo de Tasks completo (ATUALIZADO)"
        description = "Task atualizada com sucesso"
        priority = "urgent"
        actual_hours = 10
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)" -Method Put -Headers $script:headers -Body $body
    
    if ($result.message -notmatch "updated") {
        throw "Task not updated"
    }
    
    # Verificar atualizacao
    $getResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)" -Method Get -Headers $script:headers
    
    if ($getResult.title -ne "Implementar modulo de Tasks completo (ATUALIZADO)") {
        throw "Title not updated"
    }
    
    Write-Host "   Task atualizada" -ForegroundColor Gray
}

# ====================================================
# 7. UPDATE STATUS
# ====================================================

Test-Endpoint "UPDATE STATUS" {
    $body = @{
        status = "in_progress"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)/status" -Method Put -Headers $script:headers -Body $body
    
    if ($result.message -notmatch "updated") {
        throw "Status not updated"
    }
    
    # Verificar status
    $getResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)" -Method Get -Headers $script:headers
    
    if ($getResult.status -ne "in_progress") {
        throw "Status not changed"
    }
    
    Write-Host "   Status: $($getResult.status)" -ForegroundColor Gray
}

# ====================================================
# 8. ADD COMMENT
# ====================================================

Test-Endpoint "ADD COMMENT" {
    $body = @{
        content = "Progresso: modulo Task 100% funcional!"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)/comments" -Method Post -Headers $script:headers -Body $body
    
    if ($result.message -notmatch "created|added") {
        throw "Comment not added"
    }
    
    Write-Host "   Comentario adicionado" -ForegroundColor Gray
}

# ====================================================
# 9. GET COMMENTS
# ====================================================

Test-Endpoint "GET COMMENTS" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)/comments" -Method Get -Headers $script:headers
    
    if ($result.comments.Count -eq 0) {
        throw "No comments returned"
    }
    
    $foundComment = $result.comments | Where-Object { $_.content -match "Progresso" }
    if (-not $foundComment) {
        throw "Added comment not found"
    }
    
    Write-Host "   Total comments: $($result.comments.Count)" -ForegroundColor Gray
}

# ====================================================
# 10. ADD CHECKLIST
# ====================================================

Test-Endpoint "ADD CHECKLIST" {
    $body = @{
        title = "Requisitos de Implementacao"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)/checklists" -Method Post -Headers $script:headers -Body $body
    
    if ($result.message -notmatch "created|added") {
        throw "Checklist not added"
    }
    
    Write-Host "   Checklist adicionada" -ForegroundColor Gray
}

# ====================================================
# 11. GET CHECKLISTS
# ====================================================

Test-Endpoint "GET CHECKLISTS" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)/checklists" -Method Get -Headers $script:headers
    
    if ($result.checklists.Count -eq 0) {
        throw "No checklists returned"
    }
    
    $foundChecklist = $result.checklists | Where-Object { $_.title -match "Requisitos" }
    if (-not $foundChecklist) {
        throw "Added checklist not found"
    }
    
    Write-Host "   Total checklists: $($result.checklists.Count)" -ForegroundColor Gray
}

# ====================================================
# 12. DELETE TASK
# ====================================================

Test-Endpoint "DELETE TASK" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)" -Method Delete -Headers $script:headers
    
    if ($result.message -notmatch "deleted") {
        throw "Task not deleted"
    }
    
    # Verificar que foi deletada
    try {
        $getResult = Invoke-RestMethod -Uri "$baseUrl/api/tasks/$($script:taskId)" -Method Get -Headers $script:headers
        throw "Task still exists after deletion"
    } catch {
        if ($_.Exception.Response.StatusCode -eq 404) {
            Write-Host "   Task deletada" -ForegroundColor Gray
        } else {
            throw $_
        }
    }
}

# ====================================================
# RESULTADO FINAL
# ====================================================

Write-Host ""
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host "   RESULTADO FINAL" -ForegroundColor Cyan
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host "   Testes executados: $testCount" -ForegroundColor White
Write-Host "   Testes passou: $passCount" -ForegroundColor Green
Write-Host "   Testes falhou: $($testCount - $passCount)" -ForegroundColor Red
Write-Host "   Taxa de sucesso: $([math]::Round(($passCount / $testCount) * 100, 2))%" -ForegroundColor Yellow
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host ""

if ($passCount -eq $testCount) {
    Write-Host "MODULO TASK: 100% FUNCIONAL!" -ForegroundColor Green -BackgroundColor DarkGreen
    Write-Host ""
    exit 0
} else {
    Write-Host "MODULO TASK: ALGUNS TESTES FALHARAM" -ForegroundColor Red
    Write-Host ""
    exit 1
}

