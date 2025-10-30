$ErrorActionPreference = 'Stop'

Write-Host "[TEST] Scheduler Close (manual trigger placeholder)" -ForegroundColor Cyan

# Neste projeto, o scheduler é um serviço interno. Este script documenta o passo:
# 1) Avançar o relógio (ou criar leilão com data_limite passada)
# 2) Chamar endpoint auxiliar (se existir) ou iniciar o binário com flag que executa runOnce()

Write-Host "INFO: Executar LeilaoSchedulerService.runOnce() em ambiente de teste" -ForegroundColor Yellow


