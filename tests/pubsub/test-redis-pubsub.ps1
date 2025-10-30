# ==============================================================================
# 🧪 TESTE: Redis Pub/Sub para WebSocket Multi-Instance
# ==============================================================================

$baseUrl = "http://localhost:8080"

Write-Host "`n" -NoNewline
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host "  🧪 TESTE: REDIS PUB/SUB + WEBSOCKET BROADCASTING" -ForegroundColor Cyan
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host ""

# ==============================================================================
# TESTE 1: Conectar WebSocket e aguardar mensagens
# ==============================================================================
Write-Host "📋 INSTRUÇÕES:" -ForegroundColor Yellow
Write-Host ""
Write-Host "  1. Abra o arquivo 'websocket-test.html' no navegador" -ForegroundColor White
Write-Host "  2. Clique em 'Conectar'" -ForegroundColor White
Write-Host "  3. Aguarde este script enviar mensagens via Pub/Sub" -ForegroundColor White
Write-Host "  4. Verifique se as mensagens aparecem no navegador" -ForegroundColor White
Write-Host ""
Write-Host "Pressione ENTER quando estiver pronto..." -ForegroundColor Yellow
Read-Host

Write-Host ""
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host "  🔌 PUBLICANDO MENSAGENS VIA REDIS PUB/SUB" -ForegroundColor Cyan
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host ""

# ==============================================================================
# Usar redis-cli para publicar diretamente
# ==============================================================================
Write-Host "📡 Publicando 5 mensagens no canal 'websocket:broadcast'..." -ForegroundColor Cyan
Write-Host ""

For ($i = 1; $i -le 5; $i++) {
    $message = "Mensagem de teste #$i via Redis Pub/Sub - " + (Get-Date -Format "HH:mm:ss")
    
    # Publicar usando redis-cli (assumindo que está no PATH)
    try {
        $result = & redis-cli PUBLISH "websocket:broadcast" $message 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  ✅ Mensagem $i publicada - Receivers: $result" -ForegroundColor Green
        } else {
            Write-Host "  ❌ Falha ao publicar mensagem $i" -ForegroundColor Red
        }
    } catch {
        Write-Host "  ❌ Erro: $_" -ForegroundColor Red
        Write-Host ""
        Write-Host "⚠️  redis-cli não encontrado!" -ForegroundColor Yellow
        Write-Host "    Instalando redis-cli:" -ForegroundColor White
        Write-Host "    - Windows: https://github.com/microsoftarchive/redis/releases" -ForegroundColor White
        Write-Host "    - Ou use Redis Desktop Manager" -ForegroundColor White
        Write-Host ""
        Write-Host "    Alternativa: Use PowerShell para testar via API REST:" -ForegroundColor Cyan
        break
    }
    
    Start-Sleep -Milliseconds 500
}

Write-Host ""
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host "  ✅ TESTE CONCLUÍDO!" -ForegroundColor Green
Write-Host "================================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "📊 O que deve ter acontecido:" -ForegroundColor Yellow
Write-Host "  1. ✅ Mensagens foram publicadas no Redis" -ForegroundColor White
Write-Host "  2. ✅ Servidor recebeu via Pub/Sub listener" -ForegroundColor White
Write-Host "  3. ✅ Servidor fez broadcast para WebSockets locais" -ForegroundColor White
Write-Host "  4. ✅ Mensagens apareceram no navegador" -ForegroundColor White
Write-Host ""
Write-Host "🔍 Verifique na janela do servidor os logs:" -ForegroundColor Cyan
Write-Host "   - '[Pub/Sub] Broadcasting message to local WebSocket clients'" -ForegroundColor White
Write-Host ""

