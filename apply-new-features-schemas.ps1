# apply-new-features-schemas.ps1
# Aplica schemas SQL para Email Verification, Password Reset e Audit Logs

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   APLICANDO SCHEMAS - NEW FEATURES" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# --- Configurações do Banco ---
$dbHost = "localhost"
$dbPort = "5433"
$dbName = "moneymaker_dev"
$dbUser = "moneymaker_user"
$env:PGPASSWORD = "postgre123"

# --- Encontrar psql.exe ---
$psqlPaths = @(
    "C:\Program Files\PostgreSQL\17\bin\psql.exe",
    "C:\Program Files\PostgreSQL\16\bin\psql.exe",
    "C:\Program Files\PostgreSQL\15\bin\psql.exe"
)

$psqlExe = $null
foreach ($path in $psqlPaths) {
    if (Test-Path $path) {
        $psqlExe = $path
        break
    }
}

if (-not $psqlExe) {
    Write-Host "ERROR: psql.exe nao encontrado!" -ForegroundColor Red
    exit 1
}

Write-Host "psql.exe encontrado: $psqlExe" -ForegroundColor Green
Write-Host ""

# --- Aplicar Schema 1: Email + Password ---
Write-Host "=== Aplicando: Email Verification + Password Reset ===" -ForegroundColor Yellow
$sql1 = "prisma\schema_email_and_password.sql"

if (Test-Path $sql1) {
    & $psqlExe -h $dbHost -p $dbPort -U $dbUser -d $dbName -f $sql1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "OK: Email + Password schemas aplicados" -ForegroundColor Green
    }
}

Write-Host ""

# --- Aplicar Schema 2: Audit Logs ---
Write-Host "=== Aplicando: Audit Logs ===" -ForegroundColor Yellow
$sql2 = "prisma\schema_audit_logs.sql"

if (Test-Path $sql2) {
    & $psqlExe -h $dbHost -p $dbPort -U $dbUser -d $dbName -f $sql2
    if ($LASTEXITCODE -eq 0) {
        Write-Host "OK: Audit Logs schema aplicado" -ForegroundColor Green
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   SCHEMAS APLICADOS!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
