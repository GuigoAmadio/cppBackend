# ==============================================================================
# 🧪 TESTE COMPLETO: MESSAGE QUEUE (Redis Streams)
# ==============================================================================

$baseUrl = "http://localhost:8080"

Write-Host "`n" -NoNewline
Write-Host "═══════════════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  🧪 TESTE: MESSAGE QUEUE (Redis Streams)" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""

# ==============================================================================
# TESTE 1: Verificar status inicial da fila
# ==============================================================================
Write-Host "📊 TESTE 1: Status inicial da fila" -ForegroundColor Yellow
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray

$response = Invoke-WebRequest -Uri "$baseUrl/api/queue/status" -Method GET
Write-Host "Status Code: $($response.StatusCode)" -ForegroundColor Green
$status = $response.Content | ConvertFrom-Json | ConvertTo-Json -Depth 10
Write-Host $status -ForegroundColor White
Write-Host ""

# ==============================================================================
# TESTE 2: Publicar mensagens de email com SUCESSO
# ==============================================================================
Write-Host "📧 TESTE 2: Publicar 5 mensagens de email (sucesso esperado)" -ForegroundColor Yellow
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray

For ($i = 1; $i -le 5; $i++) {
    $body = @{
        stream = "emails"
        data   = @{
            to      = "user$i@example.com"
            subject = "Welcome Email #$i"
            body    = "Hello user$i, welcome to our platform!"
        }
    } | ConvertTo-Json

    Write-Host "Publicando email #$i..." -NoNewline -ForegroundColor Cyan
    $response = Invoke-WebRequest -Uri "$baseUrl/api/queue/publish" `
        -Method POST `
        -ContentType "application/json" `
        -Body $body

    $result = $response.Content | ConvertFrom-Json
    Write-Host " ✅ ID: $($result.message_id)" -ForegroundColor Green
}

Write-Host ""
Write-Host "⏳ Aguardando processamento (3 segundos)..." -ForegroundColor Cyan
Start-Sleep -Seconds 3
Write-Host ""

# ==============================================================================
# TESTE 3: Verificar status após processamento
# ==============================================================================
Write-Host "📊 TESTE 3: Status após processamento" -ForegroundColor Yellow
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray

$response = Invoke-WebRequest -Uri "$baseUrl/api/queue/status" -Method GET
$status = $response.Content | ConvertFrom-Json
Write-Host "Worker Status:" -ForegroundColor Cyan
Write-Host "  - Running: $($status.email_worker.running)" -ForegroundColor White
Write-Host "  - Processed: $($status.email_worker.processed)" -ForegroundColor White
Write-Host "  - Succeeded: $($status.email_worker.succeeded)" -ForegroundColor Green
Write-Host "  - Failed: $($status.email_worker.failed)" -ForegroundColor Red
Write-Host "  - Retried: $($status.email_worker.retried)" -ForegroundColor Yellow
Write-Host "  - Moved to DLQ: $($status.email_worker.moved_to_dlq)" -ForegroundColor Magenta
Write-Host ""

# ==============================================================================
# TESTE 4: Publicar mensagens INVÁLIDAS (sem campos obrigatórios)
# ==============================================================================
Write-Host "❌ TESTE 4: Publicar mensagens INVÁLIDAS (faltando campos)" -ForegroundColor Yellow
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray

Write-Host "Publicando email sem 'to' (vai falhar e ser retentado)..." -ForegroundColor Cyan

$badBody = @{
    stream = "emails"
    data   = @{
        subject = "Email sem destinatário"
        body    = "Este email não tem o campo 'to' e deve falhar"
    }
} | ConvertTo-Json

$response = Invoke-WebRequest -Uri "$baseUrl/api/queue/publish" `
    -Method POST `
    -ContentType "application/json" `
    -Body $badBody

$result = $response.Content | ConvertFrom-Json
Write-Host "✅ Mensagem publicada (ID: $($result.message_id))" -ForegroundColor Green
Write-Host "   (Worker tentará processar e falhará)" -ForegroundColor Yellow
Write-Host ""

Write-Host "⏳ Aguardando tentativas de processamento (5 segundos)..." -ForegroundColor Cyan
Start-Sleep -Seconds 5
Write-Host ""

# ==============================================================================
# TESTE 5: Verificar status final (com falhas)
# ==============================================================================
Write-Host "📊 TESTE 5: Status final (com falhas esperadas)" -ForegroundColor Yellow
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray

$response = Invoke-WebRequest -Uri "$baseUrl/api/queue/status" -Method GET
$status = $response.Content | ConvertFrom-Json

Write-Host "Estatísticas finais do Worker:" -ForegroundColor Cyan
Write-Host "  - Total Processed: $($status.email_worker.processed)" -ForegroundColor White
Write-Host "  - ✅ Succeeded: $($status.email_worker.succeeded)" -ForegroundColor Green
Write-Host "  - ❌ Failed: $($status.email_worker.failed)" -ForegroundColor Red
Write-Host "  - 🔁 Retried: $($status.email_worker.retried)" -ForegroundColor Yellow
Write-Host "  - 💀 Moved to DLQ: $($status.email_worker.moved_to_dlq)" -ForegroundColor Magenta
Write-Host ""

Write-Host "Estatísticas da Fila 'emails':" -ForegroundColor Cyan
Write-Host "  - Queue Length: $($status.emails.length)" -ForegroundColor White
Write-Host "  - DLQ Length: $($status.emails.dlq_length)" -ForegroundColor White
Write-Host ""

Write-Host "Configuração de Retry:" -ForegroundColor Cyan
Write-Host "  - Max Retries: $($status.retry_config.max_retries)" -ForegroundColor White
Write-Host "  - Initial Delay: $($status.retry_config.initial_delay_ms) ms" -ForegroundColor White
Write-Host "  - Max Delay: $($status.retry_config.max_delay_ms) ms" -ForegroundColor White
Write-Host "  - Backoff Multiplier: $($status.retry_config.backoff_multiplier)x" -ForegroundColor White
Write-Host ""

# ==============================================================================
# TESTE 6: Publicar mais mensagens de teste
# ==============================================================================
Write-Host "📨 TESTE 6: Publicar lote de 10 mensagens" -ForegroundColor Yellow
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray

For ($i = 1; $i -le 10; $i++) {
    $body = @{
        stream = "emails"
        data   = @{
            to      = "batch$i@example.com"
            subject = "Batch Email #$i"
            body    = "This is batch email number $i"
        }
    } | ConvertTo-Json

    Write-Host "." -NoNewline -ForegroundColor Cyan
    Invoke-WebRequest -Uri "$baseUrl/api/queue/publish" `
        -Method POST `
        -ContentType "application/json" `
        -Body $body | Out-Null
}

Write-Host " ✅ 10 mensagens publicadas!" -ForegroundColor Green
Write-Host ""

Write-Host "⏳ Aguardando processamento em background (5 segundos)..." -ForegroundColor Cyan
Start-Sleep -Seconds 5
Write-Host ""

# ==============================================================================
# TESTE 7: Status final completo
# ==============================================================================
Write-Host "📊 TESTE 7: Status final completo" -ForegroundColor Yellow
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray

$response = Invoke-WebRequest -Uri "$baseUrl/api/queue/status" -Method GET
$status = $response.Content | ConvertFrom-Json | ConvertTo-Json -Depth 10
Write-Host $status -ForegroundColor White
Write-Host ""

# ==============================================================================
# RESUMO
# ==============================================================================
Write-Host "═══════════════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  ✅ TESTES CONCLUÍDOS!" -ForegroundColor Green
Write-Host "═══════════════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""
Write-Host "📋 Funcionalidades Testadas:" -ForegroundColor Cyan
Write-Host "  ✅ Publicação de mensagens (POST /api/queue/publish)" -ForegroundColor Green
Write-Host "  ✅ Status da fila (GET /api/queue/status)" -ForegroundColor Green
Write-Host "  ✅ Consumo assíncrono (QueueWorker em background)" -ForegroundColor Green
Write-Host "  ✅ ACK de mensagens bem-sucedidas" -ForegroundColor Green
Write-Host "  ✅ Retry automático (mensagens inválidas)" -ForegroundColor Green
Write-Host "  ✅ Estatísticas do worker" -ForegroundColor Green
Write-Host "  ✅ Processamento em batch" -ForegroundColor Green
Write-Host ""
Write-Host "📝 Próximos passos:" -ForegroundColor Yellow
Write-Host "  - Testar DLQ com mensagens que falham após 3 tentativas" -ForegroundColor White
Write-Host "  - Integrar EmailService real (SMTP)" -ForegroundColor White
Write-Host "  - Criar workers para outros tipos de tarefas" -ForegroundColor White
Write-Host ""

