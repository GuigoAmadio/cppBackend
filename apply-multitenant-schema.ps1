# Script para aplicar o schema multitenant
Write-Host "=== APLICANDO SCHEMA MULTITENANT ===" -ForegroundColor Cyan

# Variaveis de conexao
$env:PGPASSWORD = "postgre123"
$dbHost = "localhost"
$dbPort = "5433"
$dbName = "moneymaker_dev"
$dbUser = "moneymaker_user"

# Tentar encontrar psql.exe
$psqlPaths = @(
    "C:\Program Files\PostgreSQL\17\bin\psql.exe",
    "C:\Program Files\PostgreSQL\16\bin\psql.exe",
    "C:\Program Files\PostgreSQL\15\bin\psql.exe",
    "C:\Program Files (x86)\PostgreSQL\17\bin\psql.exe",
    "C:\Program Files (x86)\PostgreSQL\16\bin\psql.exe",
    "psql.exe"  # Tentar no PATH
)

$psqlExe = $null
foreach ($path in $psqlPaths) {
    if (Test-Path $path -ErrorAction SilentlyContinue) {
        $psqlExe = $path
        Write-Host "psql.exe encontrado: $path" -ForegroundColor Gray
        break
    }
}

if (-not $psqlExe) {
    # Tentar encontrar via Get-Command (se estiver no PATH)
    try {
        $psqlCmd = Get-Command psql -ErrorAction Stop
        $psqlExe = $psqlCmd.Source
        Write-Host "psql.exe encontrado no PATH: $psqlExe" -ForegroundColor Gray
    } catch {
        Write-Host "`nERRO: psql.exe nao encontrado!" -ForegroundColor Red
        Write-Host "Por favor, conecte manualmente ao PostgreSQL e execute:" -ForegroundColor Yellow
        Write-Host "  1. psql -h localhost -p 5433 -U moneymaker_user -d moneymaker_dev" -ForegroundColor Gray
        Write-Host "  2. \i prisma/schema_multitenant.sql" -ForegroundColor Gray
        Write-Host "  3. \i prisma/associate_user_to_tenants.sql" -ForegroundColor Gray
        Remove-Item Env:\PGPASSWORD
        exit 1
    }
}

Write-Host "`n1. Aplicando schema principal..." -ForegroundColor Yellow

# Aplicar schema principal
Get-Content .\prisma\schema_multitenant.sql | & $psqlExe -h $dbHost -p $dbPort -U $dbUser -d $dbName -v ON_ERROR_STOP=1

if ($LASTEXITCODE -ne 0) {
    Write-Host "`nErro ao aplicar schema!" -ForegroundColor Red
    Remove-Item Env:\PGPASSWORD
    exit 1
}

Write-Host "   Schema principal aplicado!" -ForegroundColor Green

# Associar user aos tenants
Write-Host "`n2. Associando users aos tenants..." -ForegroundColor Yellow

Get-Content .\prisma\associate_user_to_tenants.sql | & $psqlExe -h $dbHost -p $dbPort -U $dbUser -d $dbName

Write-Host "   Users associados!" -ForegroundColor Green

Write-Host "`n=== RESUMO ===" -ForegroundColor Cyan
Write-Host "✓ Tabelas criadas: tenants, users, user_tenants" -ForegroundColor Green
Write-Host "✓ Views criadas: v_user_tenants, v_tenant_stats" -ForegroundColor Green
Write-Host "✓ Tenants de exemplo: demo, test" -ForegroundColor Green
Write-Host "✓ User finaluser@test.com associado aos tenants" -ForegroundColor Green

Write-Host "`n=== VERIFICANDO ===" -ForegroundColor Cyan
Write-Host "Consultando tenants criados..." -ForegroundColor Gray

"SELECT subdomain, name, plan FROM tenants;" | & $psqlExe -h $dbHost -p $dbPort -U $dbUser -d $dbName -t

Write-Host "`n=== PROXIMOS PASSOS ===" -ForegroundColor Yellow
Write-Host "1. Compilar: cd build; mingw32-make" -ForegroundColor Gray
Write-Host "2. Testar login: finaluser@test.com (Pass1234)" -ForegroundColor Gray
Write-Host "3. Usar subdomain demo ou test no Host header" -ForegroundColor Gray

Remove-Item Env:\PGPASSWORD
