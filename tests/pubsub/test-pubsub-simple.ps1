# ==============================================================================
# 🧪 TESTE SIMPLES: Redis Pub/Sub
# ==============================================================================

$baseUrl = "http://localhost:8080"

Write-Host "`n================================================================" -ForegroundColor Cyan
Write-Host "  🧪 TESTE: REDIS PUB/SUB + WEBSOCKET" -ForegroundColor Cyan
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "📋 PASSOS:" -ForegroundColor Yellow
Write-Host "  1. Abra websocket-test.html no navegador" -ForegroundColor White
Write-Host "  2. Clique em 'Conectar'" -ForegroundColor White
Write-Host "  3. Aguarde as mensagens aparecerem" -ForegroundColor White
Write-Host ""
Write-Host "Pressione ENTER quando estiver pronto..." -ForegroundColor Yellow
Read-Host

Write-Host ""
Write-Host "📡 Publicando 5 mensagens via API..." -ForegroundColor Cyan
Write-Host ""

For ($i = 1; $i -le 5; $i++) {
    $message = "Teste #$i via Redis Pub/Sub - " + (Get-Date -Format "HH:mm:ss")
    $body = @{message = $message} | ConvertTo-Json
    
    try {
        $response = Invoke-RestMethod -Uri "$baseUrl/api/pubsub/broadcast" `
            -Method POST `
            -ContentType "application/json" `
            -Body $body
        
        Write-Host "  ✅ Mensagem $i enviada" -ForegroundColor Green
        Write-Host "     Receivers: $($response.receivers)" -ForegroundColor Cyan
        Write-Host "     Message: $($response.sent_message)" -ForegroundColor Gray
        
    } catch {
        Write-Host "  ❌ Erro: $_" -ForegroundColor Red
    }
    
    Start-Sleep -Milliseconds 800
}

Write-Host ""
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host "  ✅ TESTE CONCLUÍDO!" -ForegroundColor Green
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "📊 O que aconteceu:" -ForegroundColor Yellow
Write-Host "  1. API publicou mensagens no Redis (canal 'websocket:broadcast')" -ForegroundColor White
Write-Host "  2. Servidor recebeu via RedisPubSub listener" -ForegroundColor White
Write-Host "  3. Servidor fez broadcast para WebSockets locais" -ForegroundColor White
Write-Host "  4. Mensagens apareceram no navegador" -ForegroundColor White
Write-Host ""
Write-Host "🔍 Verifique os logs do servidor para ver:" -ForegroundColor Cyan
Write-Host "   - '[Pub/Sub] Broadcasting message to local WebSocket clients'" -ForegroundColor White
Write-Host ""

