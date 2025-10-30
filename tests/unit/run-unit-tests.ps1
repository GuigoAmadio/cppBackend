$ErrorActionPreference = 'Stop'

Write-Host "[UNIT] Compilando e executando" -ForegroundColor Cyan

$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $root 'build-unit'
if (-not (Test-Path $build)) { New-Item -ItemType Directory -Path $build | Out-Null }

function Run-One($src, $out) {
  $cmd = "g++ -std=c++17 -I $root/src $src -o $build/$out"
  Write-Host $cmd
  cmd /c $cmd
  & "$build/$out"
}

# 1) clamp
Run-One "$PSScriptRoot/test-accept-offer-clamp.cpp" "test-accept-offer-clamp.exe"

# 2) ranking
Run-One "$PSScriptRoot/test-ranking.cpp" "test-ranking.exe"

# 3) validators (precisa linkar o .cpp do use-case)
$cmd3 = "g++ -std=c++17 -I $root/src `"$PSScriptRoot/test-create-validators.cpp`" `"$root/src/domains/leiloes/use_cases/CreateLeilaoUseCase.cpp`" -o `"$build/test-create-validators.exe`""
Write-Host $cmd3
cmd /c $cmd3
& "$build/test-create-validators.exe"

# 4) create-offer validators
$cmd4 = "g++ -std=c++17 -I $root/src `"$PSScriptRoot/test-create-offer-validators.cpp`" `"$root/src/domains/offers/use_cases/CreateOfferUseCase.cpp`" -o `"$build/test-create-offer-validators.exe`""
Write-Host $cmd4
cmd /c $cmd4
& "$build/test-create-offer-validators.exe"

Write-Host "[UNIT] Todos OK" -ForegroundColor Green


