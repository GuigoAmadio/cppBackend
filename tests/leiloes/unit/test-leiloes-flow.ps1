$ErrorActionPreference = 'Stop'

Write-Host "[LEILOES][FLOW] start" -ForegroundColor Cyan

# Resolve restaurante existente (exige seed prévio)
$pgUser = "moneymaker_user"
$pgHost = "localhost"
$pgPort = 5433
$pgDb   = "moneymaker_dev"
$restId = & psql -U $pgUser -h $pgHost -p $pgPort -d $pgDb -t -A -c "SELECT id FROM restaurante_profiles LIMIT 1" 2>$null
if (-not $restId) { throw "Nenhum restaurante_profiles encontrado. Crie um e rode novamente." }

# 1) Create
$createBody = @{
  restaurante_id   = "$restId"
  titulo           = "Evento teste"
  categoria        = "garcom"
  valor_ideal      = 120
  valor_max_quero  = 180
  data_trabalho    = "2025-12-20T20:00:00Z"
  data_limite_offers = "2025-12-19T20:00:00Z"
} | ConvertTo-Json
$create = Invoke-RestMethod -Method Post -Uri http://localhost:8080/leiloes -ContentType 'application/json' -Body $createBody
if (-not $create.id) { throw "Create leilao failed" }
$leilaoId = $create.id
Write-Host "   created: $leilaoId" -ForegroundColor Green

# 2) List
$list = Invoke-RestMethod -Method Get -Uri http://localhost:8080/leiloes
Write-Host "   list ok (count=$($list.Length))" -ForegroundColor Green

# 3) Get by id
$get = Invoke-RestMethod -Method Get -Uri ("http://localhost:8080/leiloes/{0}" -f $leilaoId)
Write-Host "   getById ok" -ForegroundColor Green

# 4) Close
$close = Invoke-RestMethod -Method Put -Uri ("http://localhost:8080/leiloes/{0}/fechar" -f $leilaoId)
if ($close.status -ne 'fechado') { throw "Close failed" }
Write-Host "   closed" -ForegroundColor Green

Write-Host "[LEILOES][FLOW] ok" -ForegroundColor Green


