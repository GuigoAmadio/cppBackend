# Script para testar ThreadPool do C++ Backend
# Este script envia múltiplas requisições simultâneas para demonstrar
# que o servidor processa requests em paralelo usando o ThreadPool

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  🧪 TESTE DE THREADING - ThreadPool" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Certifique-se de que o servidor está rodando em http://localhost:8080" -ForegroundColor Yellow
Write-Host ""

# Teste 1: Requisições lentas simultâneas
Write-Host "📊 TESTE 1: Requisições Lentas Simultâneas" -ForegroundColor Green
Write-Host "Enviando 5 requisições que demoram 3 segundos cada..." -ForegroundColor Gray
Write-Host "Se o ThreadPool está funcionando, todas devem completar em ~3 segundos" -ForegroundColor Gray
Write-Host "Se não tivesse threading, demoraria ~15 segundos!" -ForegroundColor Gray
Write-Host ""

$startTime = Get-Date

# Criar 5 jobs que rodam em paralelo
$jobs = 1..5 | ForEach-Object {
    Start-Job -ScriptBlock {
        param($num)
        $result = Invoke-WebRequest -Uri "http://localhost:8080/api/slow/3" -ErrorAction SilentlyContinue
        return @{
            Number = $num
            StatusCode = $result.StatusCode
            Content = $result.Content
        }
    } -ArgumentList $_
}

# Aguardar todos os jobs completarem
$results = $jobs | Wait-Job | Receive-Job

$endTime = Get-Date
$duration = ($endTime - $startTime).TotalSeconds

# Limpar jobs
$jobs | Remove-Job

Write-Host "✅ Resultado:" -ForegroundColor Green
Write-Host "   Tempo total: $([math]::Round($duration, 2)) segundos" -ForegroundColor White
Write-Host "   Requisições completadas: $($results.Count)" -ForegroundColor White

if ($duration -lt 5) {
    Write-Host "   🎉 SUCESSO! ThreadPool está funcionando (processou em paralelo)" -ForegroundColor Green
} else {
    Write-Host "   ⚠️  AVISO: Pode estar processando sequencialmente" -ForegroundColor Yellow
}

Write-Host ""

# Teste 2: Contador thread-safe
Write-Host "📊 TESTE 2: Contador Thread-Safe (Atomic)" -ForegroundColor Green
Write-Host "Enviando 10 requisições simultâneas ao contador..." -ForegroundColor Gray
Write-Host ""

$jobs = 1..10 | ForEach-Object {
    Start-Job -ScriptBlock {
        $result = Invoke-WebRequest -Uri "http://localhost:8080/api/counter" -ErrorAction SilentlyContinue
        return $result.Content | ConvertFrom-Json
    }
}

$counterResults = $jobs | Wait-Job | Receive-Job
$jobs | Remove-Job

Write-Host "✅ Resultado:" -ForegroundColor Green
$counterResults | ForEach-Object {
    Write-Host "   $($_.message)" -ForegroundColor White
}

# Verificar se os números são únicos (thread-safe)
$counts = $counterResults | ForEach-Object { $_.count }
$uniqueCounts = $counts | Select-Object -Unique

if ($counts.Count -eq $uniqueCounts.Count) {
    Write-Host "   🎉 SUCESSO! Contador é thread-safe (sem race conditions)" -ForegroundColor Green
} else {
    Write-Host "   ⚠️  AVISO: Possível race condition detectada" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  ✅ Testes Completos!" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan

