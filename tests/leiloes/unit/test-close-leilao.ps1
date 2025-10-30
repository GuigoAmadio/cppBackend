$ErrorActionPreference = 'Stop'

param([string]$Id)
if (-not $Id) { throw "Passe -Id <leilao_id>" }

$resp = Invoke-RestMethod -Method Put -Uri ("http://localhost:8080/leiloes/{0}/fechar" -f $Id)
if ($resp.status -ne 'fechado') { throw "Fechamento falhou" }
Write-Host "OK - Leilao fechado" -ForegroundColor Green


