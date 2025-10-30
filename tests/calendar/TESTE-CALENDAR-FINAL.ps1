# ====================================================
# TESTE COMPLETO - MODULO CALENDAR
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
Write-Host "   TESTE COMPLETO - MODULO CALENDAR" -ForegroundColor Cyan
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host ""

# ====================================================
# SETUP
# ====================================================

Write-Host "[SETUP] Limpando dados antigos..." -ForegroundColor Yellow
$env:PGPASSWORD="postgre123"
$queries = @(
    "DELETE FROM event_reminders WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')",
    "DELETE FROM event_participants WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')",
    "DELETE FROM events WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')"
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
    $script:email = "calendar$timestamp@test.com"
    $script:password = "Calendar123"
    
    $body = @{
        email = $script:email
        password = $script:password
        name = "Calendar Test User"
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
# 3. CREATE EVENT
# ====================================================

Test-Endpoint "CREATE EVENT" {
    $body = @{
        title = "Reuniao de Sprint Planning"
        event_type = "meeting"
        start_time = "2025-11-15 09:00:00"
        end_time = "2025-11-15 10:30:00"
        description = "Planejamento da proxima sprint"
        location = "Sala de Reunioes 1"
        color = "#4CAF50"
        all_day = "false"
        is_private = "false"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/events" -Method Post -Headers $script:headers -Body $body
    
    if (-not $result.id) {
        throw "Event ID not returned"
    }
    
    $script:eventId = $result.id
    Write-Host "   Event ID: $($script:eventId)" -ForegroundColor Gray
}

# ====================================================
# 4. GET EVENT BY ID
# ====================================================

Test-Endpoint "GET EVENT BY ID" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/events/$($script:eventId)" -Method Get -Headers $script:headers
    
    if ($result.id -ne $script:eventId) {
        throw "Event ID mismatch"
    }
    
    if ($result.title -ne "Reuniao de Sprint Planning") {
        throw "Event title mismatch"
    }
    
    Write-Host "   Title: $($result.title)" -ForegroundColor Gray
    Write-Host "   Type: $($result.event_type)" -ForegroundColor Gray
}

# ====================================================
# 5. LIST EVENTS
# ====================================================

Test-Endpoint "LIST EVENTS" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/events" -Method Get -Headers $script:headers
    
    if ($result.events.Count -eq 0) {
        throw "No events returned"
    }
    
    $foundEvent = $result.events | Where-Object { $_.id -eq $script:eventId }
    if (-not $foundEvent) {
        throw "Created event not found in list"
    }
    
    Write-Host "   Total events: $($result.events.Count)" -ForegroundColor Gray
}

# ====================================================
# 6. UPDATE EVENT
# ====================================================

Test-Endpoint "UPDATE EVENT" {
    $body = @{
        title = "Reuniao de Sprint Planning (ATUALIZADA)"
        description = "Planejamento atualizado com novos requisitos"
        color = "#FF5722"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/events/$($script:eventId)" -Method Put -Headers $script:headers -Body $body
    
    if ($result.message -notmatch "updated") {
        throw "Event not updated"
    }
    
    # Verificar atualizacao
    $getResult = Invoke-RestMethod -Uri "$baseUrl/api/events/$($script:eventId)" -Method Get -Headers $script:headers
    
    if ($getResult.title -ne "Reuniao de Sprint Planning (ATUALIZADA)") {
        throw "Title not updated"
    }
    
    Write-Host "   Event atualizado" -ForegroundColor Gray
}

# ====================================================
# 7. ADD PARTICIPANT
# ====================================================

Test-Endpoint "ADD PARTICIPANT" {
    $body = @{
        user_id = $script:userId
        role = "required"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/events/$($script:eventId)/participants" -Method Post -Headers $script:headers -Body $body
    
    if (-not $result.id) {
        throw "Participant ID not returned"
    }
    
    $script:participantId = $result.id
    Write-Host "   Participant ID: $($script:participantId)" -ForegroundColor Gray
}

# ====================================================
# 8. GET PARTICIPANTS
# ====================================================

Test-Endpoint "GET PARTICIPANTS" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/events/$($script:eventId)/participants" -Method Get -Headers $script:headers
    
    if ($result.participants.Count -eq 0) {
        throw "No participants returned"
    }
    
    $foundParticipant = $result.participants | Where-Object { $_.id -eq $script:participantId }
    if (-not $foundParticipant) {
        throw "Added participant not found"
    }
    
    Write-Host "   Total participants: $($result.participants.Count)" -ForegroundColor Gray
}

# ====================================================
# 9. UPDATE PARTICIPANT STATUS
# ====================================================

Test-Endpoint "UPDATE PARTICIPANT STATUS" {
    $body = @{
        status = "accepted"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/events/participants/$($script:participantId)/status" -Method Put -Headers $script:headers -Body $body
    
    if ($result.message -notmatch "updated") {
        throw "Participant status not updated"
    }
    
    Write-Host "   Status atualizado" -ForegroundColor Gray
}

# ====================================================
# 10. ADD REMINDER
# ====================================================

Test-Endpoint "ADD REMINDER" {
    $body = @{
        reminder_type = "notification"
        minutes_before = 15
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/events/$($script:eventId)/reminders" -Method Post -Headers $script:headers -Body $body
    
    if (-not $result.id) {
        throw "Reminder ID not returned"
    }
    
    $script:reminderId = $result.id
    Write-Host "   Reminder ID: $($script:reminderId)" -ForegroundColor Gray
}

# ====================================================
# 11. GET REMINDERS
# ====================================================

Test-Endpoint "GET REMINDERS" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/events/$($script:eventId)/reminders" -Method Get -Headers $script:headers
    
    if ($result.reminders.Count -eq 0) {
        throw "No reminders returned"
    }
    
    $foundReminder = $result.reminders | Where-Object { $_.id -eq $script:reminderId }
    if (-not $foundReminder) {
        throw "Added reminder not found"
    }
    
    Write-Host "   Total reminders: $($result.reminders.Count)" -ForegroundColor Gray
}

# ====================================================
# 12. DELETE EVENT
# ====================================================

Test-Endpoint "DELETE EVENT" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/events/$($script:eventId)" -Method Delete -Headers $script:headers
    
    if ($result.message -notmatch "deleted") {
        throw "Event not deleted"
    }
    
    # Verificar exclusao
    try {
        $getResult = Invoke-RestMethod -Uri "$baseUrl/api/events/$($script:eventId)" -Method Get -Headers $script:headers
        throw "Event still exists after deletion"
    } catch {
        if ($_.Exception.Message -match "404") {
            Write-Host "   Event removido com sucesso" -ForegroundColor Gray
        } else {
            throw
        }
    }
}

# ====================================================
# SUMMARY
# ====================================================

Write-Host "`n" -NoNewline
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host "   RESULTADO FINAL" -ForegroundColor Cyan
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host ""

$successRate = [math]::Round(($script:passCount / $script:testCount) * 100, 2)

Write-Host "Testes executados: $($script:testCount)" -ForegroundColor White
Write-Host "Testes passados:   $($script:passCount)" -ForegroundColor $(if ($script:passCount -eq $script:testCount) { "Green" } else { "Yellow" })
Write-Host "Testes falhados:   $($script:testCount - $script:passCount)" -ForegroundColor $(if ($script:passCount -eq $script:testCount) { "Green" } else { "Red" })
Write-Host "Taxa de sucesso:   $successRate%" -ForegroundColor $(if ($successRate -eq 100) { "Green" } elseif ($successRate -ge 80) { "Yellow" } else { "Red" })
Write-Host ""
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host ""

if ($script:passCount -eq $script:testCount) {
    Write-Host "MODULO CALENDAR 100% FUNCIONAL!" -ForegroundColor Green
    Write-Host ""
} else {
    Write-Host "Alguns testes falharam. Revise os logs acima." -ForegroundColor Yellow
    Write-Host ""
}

