$ErrorActionPreference = 'Stop'

Write-Host "[OFFERS][FLOW] start" -ForegroundColor Cyan

# Resolver restaurante e freelancer existentes (exige seed prévio)
$pgUser = "moneymaker_user"
$pgHost = "localhost"
$pgPort = 5433
$pgDb   = "moneymaker_dev"
$restId = & psql -U $pgUser -h $pgHost -p $pgPort -d $pgDb -t -A -c "SELECT id FROM restaurante_profiles LIMIT 1" 2>$null
if (-not $restId) { throw "Nenhum restaurante_profiles encontrado. Crie um e rode novamente." }
$freelaId = & psql -U $pgUser -h $pgHost -p $pgPort -d $pgDb -t -A -c "SELECT id FROM freelancer_profiles LIMIT 1" 2>$null
if (-not $freelaId) { throw "Nenhum freelancer_profiles encontrado. Crie um e rode novamente." }

# 0) Criar um leilão auxiliar
$leilaoBody = @{
  restaurante_id   = "$restId"
  titulo           = "Evento ofertas"
  categoria        = "garcom"
  valor_ideal      = 120
  valor_max_quero  = 180
  data_trabalho    = "2025-12-20T20:00:00Z"
  data_limite_offers = "2025-12-19T20:00:00Z"
} | ConvertTo-Json
$leilao = Invoke-RestMethod -Method Post -Uri http://localhost:8080/leiloes -ContentType 'application/json' -Body $leilaoBody
if (-not $leilao.id) { throw "fail create leilao" }
$leilaoId = $leilao.id

# 1) Criar offer
$offerBody = @{ user_id="$freelaId"; restaurante_id="$restId"; leilao_id=$leilaoId; valor_oferecido=130; valor_minimo_aceito=120; mensagem="posso" } | ConvertTo-Json
$offerResp = Invoke-RestMethod -Method Post -Uri http://localhost:8080/offers -ContentType 'application/json' -Body $offerBody
Write-Host "   offer criada" -ForegroundColor Green

# 2) List offers do leilao
$list = Invoke-RestMethod -Method Get -Uri ("http://localhost:8080/leiloes/{0}/offers" -f $leilaoId)
Write-Host "   list ok" -ForegroundColor Green

# 3) Shortlist (precisamos de um id real quando o endpoint retornar)
# Para fluxo real, este script deve capturar o ID retornado pelo create; quando conectado ao DB, ajuste aqui.
# Por enquanto, apenas exercita o endpoint com um placeholder se necessário.

# 4) Accept (idem)

Write-Host "[OFFERS][FLOW] ok (create/list exercitados)" -ForegroundColor Green


