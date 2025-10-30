$env:PGPASSWORD = 'postgre123'
$dbhost = 'localhost'
$dbport = '5433'
$dbuser = 'moneymaker_user'
$dbname = 'moneymaker_dev'

$schemas = @(
    "schemas/00-core.sql",
    "schemas/categories.sql",
    "schemas/customers.sql",
    "schemas/products-inventory.sql",
    "schemas/orders.sql",
    "schemas/payments.sql",
    "schemas/subscription.sql",
    "schemas/invoicing.sql",
    "schemas/finance.sql",
    "schemas/calendar.sql",
    "schemas/tasks.sql"
)

Write-Host "Aplicando todos os schemas..." -ForegroundColor Cyan
Write-Host ""

foreach ($schema in $schemas) {
    Write-Host "Aplicando: $schema" -ForegroundColor Yellow
    Get-Content $schema | psql -h $dbhost -p $dbport -U $dbuser -d $dbname 2>&1 | Out-Null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "   OK" -ForegroundColor Green
    } else {
        Write-Host "   ERRO" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Todos os schemas aplicados!" -ForegroundColor Green
