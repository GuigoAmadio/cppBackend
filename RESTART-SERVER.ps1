# Script para reiniciar o servidor C++ Backend

Write-Host "=== REINICIANDO SERVIDOR C++ BACKEND ===" -ForegroundColor Cyan

# 1. Parar todos os processos cppBackend
Write-Host "`nParando processos antigos..." -ForegroundColor Yellow
Get-Process cppBackend -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Seconds 2

# 2. Verificar se a porta está livre
$port = 8080
$listener = Get-NetTCPConnection -LocalPort $port -ErrorAction SilentlyContinue
if ($listener) {
    Write-Host "Porta $port ainda em uso, aguardando..." -ForegroundColor Yellow
    Start-Sleep -Seconds 3
}

# 3. Iniciar o novo servidor
Write-Host "`nIniciando novo servidor..." -ForegroundColor Green
cd c:\Users\Guillermo\Desktop\cppBackend\build
Start-Process powershell -ArgumentList "-NoExit", "-Command", ".\cppBackend.exe"

# 4. Aguardar inicialização
Write-Host "`nAguardando servidor iniciar..." -ForegroundColor Cyan
Start-Sleep -Seconds 5

# 5. Testar se está rodando
try {
    $response = Invoke-WebRequest -Uri "http://localhost:8080/" -Method GET -TimeoutSec 5 -ErrorAction Stop
    Write-Host "`n✓ Servidor iniciado com sucesso!" -ForegroundColor Green
    Write-Host "Status: $($response.StatusCode)" -ForegroundColor Green
} catch {
    Write-Host "`n✗ Falha ao conectar no servidor" -ForegroundColor Red
    Write-Host "Verifique a janela do servidor para erros" -ForegroundColor Yellow
}

Write-Host "`n=== PRONTO ===" -ForegroundColor Cyan
Write-Host "Servidor rodando em http://localhost:8080" -ForegroundColor Green
Write-Host "`nTeste com:" -ForegroundColor Yellow
Write-Host 'Invoke-WebRequest -Uri "http://localhost:8080/api/auth/register" -Method POST -Body ''{"email":"test@example.com","password":"Secure123!","name":"Test"}'' -ContentType "application/json" -UseBasicParsing' -ForegroundColor Gray

