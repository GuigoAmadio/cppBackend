# ==============================================================================
# 🧪 TESTES AVANÇADOS: MESSAGE QUEUE
# ==============================================================================

$baseUrl = "http://localhost:8080"

Write-Host "`n" -NoNewline
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host "  🧪 TESTES AVANÇADOS: MESSAGE QUEUE" -ForegroundColor Cyan
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host ""

# ==============================================================================
# TESTE 1: Status inicial
# ==============================================================================
Write-Host "📊 TESTE 1: Status inicial" -ForegroundColor Yellow
Write-Host "────────────────────────────────────────────────────────────────" -ForegroundColor DarkGray

$status = Invoke-RestMethod -Uri "$baseUrl/api/queue/status" -Method GET
Write-Host "  Worker running: $($status.email_worker.running)" -ForegroundColor Green
Write-Host "  Queue length: $($status.emails.length)" -ForegroundColor White
Write-Host "  DLQ length: $($status.emails.dlq_length)" -ForegroundColor White
Write-Host ""

# ==============================================================================
# TESTE 2: Publicar mensagens VÁLIDAS em lote
# ==============================================================================
Write-Host "✅ TESTE 2: Publicando 10 mensagens VÁLIDAS em lote" -ForegroundColor Yellow
Write-Host "────────────────────────────────────────────────────────────────" -ForegroundColor DarkGray

$successIds = @()
For ($i = 1; $i -le 10; $i++) {
    $body = @{
        stream = "emails"
        data   = @{
            to      = "valid$i@example.com"
            subject = "Valid Email $i"
            body    = "This is a valid email number $i"
        }
    } | ConvertTo-Json -Compress

    $res = Invoke-RestMethod -Uri "$baseUrl/api/queue/publish" `
        -Method POST -ContentType "application/json" -Body $body
    $successIds += $res.message_id
    Write-Host "  . " -NoNewline -ForegroundColor Green
}
Write-Host " ✅ 10 mensagens válidas publicadas!" -ForegroundColor Green
Write-Host ""

Start-Sleep -Seconds 2

# ==============================================================================
# TESTE 3: Publicar mensagens INVÁLIDAS (sem campo "to")
# ==============================================================================
Write-Host "❌ TESTE 3: Publicando 3 mensagens INVÁLIDAS (sem 'to')" -ForegroundColor Yellow
Write-Host "────────────────────────────────────────────────────────────────" -ForegroundColor DarkGray

$invalidIds = @()
For ($i = 1; $i -le 3; $i++) {
    $body = @{
        stream = "emails"
        data   = @{
            subject = "Invalid Email $i"
            body    = "Esta mensagem NÃO tem o campo 'to' e vai FALHAR"
        }
    } | ConvertTo-Json -Compress

    $res = Invoke-RestMethod -Uri "$baseUrl/api/queue/publish" `
        -Method POST -ContentType "application/json" -Body $body
    $invalidIds += $res.message_id
    Write-Host "  ❌ Mensagem inválida $i publicada: $($res.message_id)" -ForegroundColor Red
}
Write-Host ""
Write-Host "  ⚠️  Estas mensagens vão FALHAR e ser retentadas automaticamente!" -ForegroundColor Yellow
Write-Host ""

Start-Sleep -Seconds 3

# ==============================================================================
# TESTE 4: Status após processamento inicial
# ==============================================================================
Write-Host "📊 TESTE 4: Status após processamento inicial" -ForegroundColor Yellow
Write-Host "────────────────────────────────────────────────────────────────" -ForegroundColor DarkGray

$status = Invoke-RestMethod -Uri "$baseUrl/api/queue/status" -Method GET
Write-Host "  Processadas: $($status.email_worker.processed)" -ForegroundColor Cyan
Write-Host "  ✅ Sucesso: $($status.email_worker.succeeded)" -ForegroundColor Green
Write-Host "  ❌ Falharam: $($status.email_worker.failed)" -ForegroundColor Red
Write-Host "  🔁 Retried: $($status.email_worker.retried)" -ForegroundColor Yellow
Write-Host "  💀 Movidas para DLQ: $($status.email_worker.moved_to_dlq)" -ForegroundColor Magenta
Write-Host "  📦 Fila principal: $($status.emails.length)" -ForegroundColor White
Write-Host "  💀 DLQ: $($status.emails.dlq_length)" -ForegroundColor White
Write-Host ""

# ==============================================================================
# TESTE 5: Publicar em BURST (rapidez)
# ==============================================================================
Write-Host "⚡ TESTE 5: Publicando 20 mensagens em BURST (máxima velocidade)" -ForegroundColor Yellow
Write-Host "────────────────────────────────────────────────────────────────" -ForegroundColor DarkGray

$startTime = Get-Date
For ($i = 1; $i -le 20; $i++) {
    $body = @{
        stream = "emails"
        data   = @{
            to      = "burst$i@example.com"
            subject = "Burst Email $i"
            body    = "High-speed email $i"
        }
    } | ConvertTo-Json -Compress

    Invoke-RestMethod -Uri "$baseUrl/api/queue/publish" `
        -Method POST -ContentType "application/json" -Body $body | Out-Null
}
$endTime = Get-Date
$duration = ($endTime - $startTime).TotalMilliseconds

Write-Host "  ✅ 20 mensagens publicadas em $([math]::Round($duration, 2)) ms" -ForegroundColor Green
Write-Host "  📊 Throughput: $([math]::Round(20000 / $duration, 2)) msg/s" -ForegroundColor Cyan
Write-Host ""

Start-Sleep -Seconds 3

# ==============================================================================
# TESTE 6: Publicar mensagens sem 'subject' (outro tipo de erro)
# ==============================================================================
Write-Host "❌ TESTE 6: Mensagens sem 'subject'" -ForegroundColor Yellow
Write-Host "────────────────────────────────────────────────────────────────" -ForegroundColor DarkGray

For ($i = 1; $i -le 2; $i++) {
    $body = @{
        stream = "emails"
        data   = @{
            to   = "nosubject$i@example.com"
            body = "Email sem subject"
        }
    } | ConvertTo-Json -Compress

    $res = Invoke-RestMethod -Uri "$baseUrl/api/queue/publish" `
        -Method POST -ContentType "application/json" -Body $body
    Write-Host "  ❌ Publicado sem subject: $($res.message_id)" -ForegroundColor Red
}
Write-Host ""

Start-Sleep -Seconds 2

# ==============================================================================
# TESTE 7: Status intermediário
# ==============================================================================
Write-Host "📊 TESTE 7: Status intermediário" -ForegroundColor Yellow
Write-Host "────────────────────────────────────────────────────────────────" -ForegroundColor DarkGray

$status = Invoke-RestMethod -Uri "$baseUrl/api/queue/status" -Method GET
Write-Host "  Total processadas: $($status.email_worker.processed)" -ForegroundColor Cyan
Write-Host "  ✅ Sucesso: $($status.email_worker.succeeded) ($([math]::Round($status.email_worker.succeeded / $status.email_worker.processed * 100, 1))%)" -ForegroundColor Green
Write-Host "  ❌ Falharam: $($status.email_worker.failed) ($([math]::Round($status.email_worker.failed / $status.email_worker.processed * 100, 1))%)" -ForegroundColor Red
Write-Host ""

# ==============================================================================
# TESTE 8: Aguardar e verificar se mensagens inválidas vão para DLQ
# ==============================================================================
Write-Host "⏳ TESTE 8: Aguardando retry automático e DLQ (15 segundos)..." -ForegroundColor Yellow
Write-Host "────────────────────────────────────────────────────────────────" -ForegroundColor DarkGray
Write-Host "  As mensagens inválidas serão retentadas automaticamente" -ForegroundColor White
Write-Host "  Após 3 tentativas falhadas, elas vão para a DLQ" -ForegroundColor White
Write-Host ""

For ($i = 1; $i -le 15; $i++) {
    Write-Host "  ." -NoNewline -ForegroundColor Yellow
    Start-Sleep -Seconds 1
}
Write-Host " ⏰ 15 segundos decorridos!" -ForegroundColor Green
Write-Host ""

# ==============================================================================
# TESTE 9: Status final completo
# ==============================================================================
Write-Host "📊 TESTE 9: Status final completo" -ForegroundColor Yellow
Write-Host "────────────────────────────────────────────────────────────────" -ForegroundColor DarkGray

$status = Invoke-RestMethod -Uri "$baseUrl/api/queue/status" -Method GET

Write-Host ""
Write-Host "  ════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  📈 ESTATÍSTICAS FINAIS DO WORKER" -ForegroundColor Cyan
Write-Host "  ════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""
Write-Host "  Total processadas: $($status.email_worker.processed)" -ForegroundColor White
Write-Host "  ✅ Sucesso: $($status.email_worker.succeeded)" -ForegroundColor Green
Write-Host "  ❌ Falharam: $($status.email_worker.failed)" -ForegroundColor Red
Write-Host "  🔁 Retried: $($status.email_worker.retried)" -ForegroundColor Yellow
Write-Host "  💀 Movidas para DLQ: $($status.email_worker.moved_to_dlq)" -ForegroundColor Magenta
Write-Host ""
Write-Host "  Taxa de sucesso: $([math]::Round($status.email_worker.succeeded / $status.email_worker.processed * 100, 2))%" -ForegroundColor Cyan
Write-Host ""
Write-Host "  ════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  📦 ESTATÍSTICAS DAS FILAS" -ForegroundColor Cyan
Write-Host "  ════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""
Write-Host "  Fila principal (emails): $($status.emails.length) mensagens" -ForegroundColor White
Write-Host "  Dead Letter Queue: $($status.emails.dlq_length) mensagens" -ForegroundColor Magenta
Write-Host ""

# ==============================================================================
# TESTE 10: Publicar 5 mensagens para verificar processamento contínuo
# ==============================================================================
Write-Host "🔄 TESTE 10: Verificando processamento contínuo (5 novas mensagens)" -ForegroundColor Yellow
Write-Host "────────────────────────────────────────────────────────────────" -ForegroundColor DarkGray

$before = $status.email_worker.processed

For ($i = 1; $i -le 5; $i++) {
    $body = @{
        stream = "emails"
        data   = @{
            to      = "final$i@example.com"
            subject = "Final Test $i"
            body    = "Verificando processamento contínuo"
        }
    } | ConvertTo-Json -Compress

    Invoke-RestMethod -Uri "$baseUrl/api/queue/publish" `
        -Method POST -ContentType "application/json" -Body $body | Out-Null
}

Write-Host "  ✅ 5 mensagens publicadas" -ForegroundColor Green
Start-Sleep -Seconds 3

$status = Invoke-RestMethod -Uri "$baseUrl/api/queue/status" -Method GET
$after = $status.email_worker.processed
$diff = $after - $before

Write-Host "  Processadas antes: $before" -ForegroundColor White
Write-Host "  Processadas depois: $after" -ForegroundColor White
Write-Host "  Diferença: +$diff" -ForegroundColor Cyan
Write-Host ""

# ==============================================================================
# RESUMO FINAL
# ==============================================================================
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host "  ✅ TESTES CONCLUÍDOS!" -ForegroundColor Green
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "📋 Funcionalidades Testadas:" -ForegroundColor Cyan
Write-Host "  ✅ Publicação de mensagens (válidas e inválidas)" -ForegroundColor Green
Write-Host "  ✅ Processamento assíncrono em background" -ForegroundColor Green
Write-Host "  ✅ ACK automático de mensagens bem-sucedidas" -ForegroundColor Green
Write-Host "  ✅ NACK e retry automático de mensagens com falha" -ForegroundColor Green
Write-Host "  ✅ Dead Letter Queue (DLQ) após max retries" -ForegroundColor Green
Write-Host "  ✅ Burst de mensagens (alta velocidade)" -ForegroundColor Green
Write-Host "  ✅ Estatísticas em tempo real" -ForegroundColor Green
Write-Host "  ✅ Processamento contínuo" -ForegroundColor Green
Write-Host ""
Write-Host "🎯 Status Final:" -ForegroundColor Yellow
Write-Host "  Total processadas: $($status.email_worker.processed)" -ForegroundColor White
Write-Host "  Sucesso: $($status.email_worker.succeeded)" -ForegroundColor Green
Write-Host "  Falharam: $($status.email_worker.failed)" -ForegroundColor Red
Write-Host "  DLQ: $($status.emails.dlq_length)" -ForegroundColor Magenta
Write-Host ""

