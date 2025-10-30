$ErrorActionPreference = 'Stop'

param(
  [string]$LeilaoId = "test-leilao-id"
)

Write-Host "[TEST] Create Offers and Shortlist" -ForegroundColor Cyan

function Create-Offer([string]$userId, [int]$oferecido, [int]$minimo) {
  $body = @{ user_id=$userId; restaurante_id = "rest-0001"; leilao_id=$LeilaoId; valor_oferecido=$oferecido; valor_minimo_aceito=$minimo; mensagem="posso trabalhar" } | ConvertTo-Json
  $resp = Invoke-RestMethod -Method Post -Uri http://localhost:8080/offers -ContentType 'application/json' -Body $body
  return $resp
}

$o1 = Create-Offer -userId "freela-01" -oferecido 140 -minimo 130
$o2 = Create-Offer -userId "freela-02" -oferecido 130 -minimo 120

# Shortlist freela-02
$null = Invoke-RestMethod -Method Put -Uri ("http://localhost:8080/offers/{0}/shortlist" -f $o2.id) -ContentType 'application/json' -Body '{"shortlisted":true}'

Write-Host "OK - Offers criadas e shortlist aplicado" -ForegroundColor Green


