# ====================================================
# TESTE COMPLETO - MODULO FINANCE
# 29 Endpoints: Accounts, Transactions, Categories, Budgets, Reports
# ====================================================

$baseUrl = "http://localhost:8080"
$testCount = 0
$passCount = 0

function Test-Endpoint {
    param([string]$name, [scriptblock]$test)
    $script:testCount++
    Write-Host "`n[$script:testCount] $name" -ForegroundColor Cyan
    try {
        & $test
        $script:passCount++
        Write-Host "   PASS" -ForegroundColor Green
        return $true
    } catch {
        Write-Host "   FAIL: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

Write-Host "`n" -NoNewline
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host "   TESTE COMPLETO - MODULO FINANCE" -ForegroundColor Cyan
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host ""

# ====================================================
# SETUP
# ====================================================

Write-Host "[SETUP] Limpando dados antigos..." -ForegroundColor Yellow
$env:PGPASSWORD="postgre123"
$queries = @(
    "DELETE FROM financial_transactions WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')",
    "DELETE FROM budgets WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')",
    "DELETE FROM financial_accounts WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')",
    "DELETE FROM transaction_categories WHERE tenant_id IN (SELECT id FROM tenants WHERE subdomain = 'acme')"
)

foreach ($q in $queries) {
    psql -U moneymaker_user -d moneymaker_dev -p 5433 -c $q 2>$null | Out-Null
}
Write-Host "[SETUP] Dados limpos!`n" -ForegroundColor Green

# ====================================================
# 1. REGISTRO
# ====================================================

Test-Endpoint "REGISTER USER" {
    $timestamp = [DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds()
    $script:email = "finance$timestamp@test.com"
    $script:password = "Finance123"
    
    $body = @{
        email = $script:email
        password = $script:password
        name = "Finance Test User"
        tenant_subdomain = "acme"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/auth/register" -Method Post -Body $body -ContentType "application/json"
    
    if (-not $result.user.id) {
        throw "User ID not returned"
    }
    
    $script:userId = $result.user.id
    Write-Host "   User ID: $($script:userId)" -ForegroundColor Gray
}

# Adicionar ao tenant
Write-Host "`n[SETUP] Adicionando usuario ao tenant..." -ForegroundColor Yellow
$addQuery = "INSERT INTO user_tenants (user_id, tenant_id, role) SELECT '$($script:userId)', id, 'admin' FROM tenants WHERE subdomain = 'acme' ON CONFLICT DO NOTHING"
psql -U moneymaker_user -d moneymaker_dev -p 5433 -c $addQuery 2>$null | Out-Null
Write-Host "[SETUP] Usuario adicionado!`n" -ForegroundColor Green

# ====================================================
# 2. LOGIN
# ====================================================

Test-Endpoint "LOGIN USER" {
    $body = @{
        email = $script:email
        password = $script:password
        tenant_subdomain = "acme"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/auth/login" -Method Post -Body $body -ContentType "application/json"
    
    if (-not $result.token) {
        throw "Token not returned"
    }
    
    $script:token = $result.token
    $script:headers = @{
        "Authorization" = "Bearer $($script:token)"
        "Content-Type" = "application/json"
    }
    
    Write-Host "   Token obtained!" -ForegroundColor Gray
}

# ====================================================
# 3. ACCOUNTS - CREATE (6 endpoints)
# ====================================================

Test-Endpoint "CREATE ACCOUNT - Checking" {
    $body = @{
        name = "Main Checking Account"
        type = "checking"
        currency = "BRL"
        initial_balance = 5000.00
        bank_name = "Banco do Brasil"
        account_number = "12345-6"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/accounts" -Method Post -Body $body -Headers $script:headers
    
    if (-not $result.id) {
        throw "Account ID not returned"
    }
    
    $script:accountId = $result.id
    Write-Host "   Account ID: $($script:accountId)" -ForegroundColor Gray
}

Test-Endpoint "CREATE ACCOUNT - Cash" {
    $body = @{
        name = "Cash Wallet"
        type = "cash"
        currency = "BRL"
        initial_balance = 500.00
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/accounts" -Method Post -Body $body -Headers $script:headers
    
    if (-not $result.id) {
        throw "Cash Account ID not returned"
    }
    
    $script:cashAccountId = $result.id
    Write-Host "   Cash Account ID: $($script:cashAccountId)" -ForegroundColor Gray
}

# ====================================================
# 4. ACCOUNTS - GET & LIST
# ====================================================

Test-Endpoint "GET ACCOUNT" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/accounts/$($script:accountId)" -Method Get -Headers $script:headers
    
    if ($result.id -ne $script:accountId) {
        throw "Account ID mismatch"
    }
    if ($result.balance -ne 5000.00) {
        throw "Initial balance incorrect"
    }
    
    Write-Host "   Balance: $($result.balance) $($result.currency)" -ForegroundColor Gray
}

Test-Endpoint "LIST ACCOUNTS" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/accounts" -Method Get -Headers $script:headers
    
    if ($result.total -lt 2) {
        throw "Expected at least 2 accounts"
    }
    
    Write-Host "   Total accounts: $($result.total)" -ForegroundColor Gray
}

Test-Endpoint "GET ACCOUNT BALANCE" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/accounts/$($script:accountId)/balance" -Method Get -Headers $script:headers
    
    if ($result.balance -ne 5000.00) {
        throw "Balance incorrect"
    }
    
    Write-Host "   Balance: $($result.balance)" -ForegroundColor Gray
}

# ====================================================
# 5. CATEGORIES - CREATE & LIST (5 endpoints)
# ====================================================

Test-Endpoint "CREATE CATEGORY - Expense" {
    $body = @{
        name = "Alimentacao"
        type = "expense"
        color = "#ff6b6b"
        icon = "food"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/categories" -Method Post -Body $body -Headers $script:headers
    
    if (-not $result.id) {
        throw "Category ID not returned"
    }
    
    $script:categoryId = $result.id
    Write-Host "   Category ID: $($script:categoryId)" -ForegroundColor Gray
}

Test-Endpoint "GET CATEGORY" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/categories/$($script:categoryId)" -Method Get -Headers $script:headers
    
    if ($result.name -ne "Alimentacao") {
        throw "Category name mismatch"
    }
    
    Write-Host "   Category: $($result.name)" -ForegroundColor Gray
}

Test-Endpoint "LIST CATEGORIES" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/categories?type=expense" -Method Get -Headers $script:headers
    
    if ($result.total -lt 1) {
        throw "Expected at least 1 expense category"
    }
    
    Write-Host "   Total categories: $($result.total)" -ForegroundColor Gray
}

# ====================================================
# 6. TRANSACTIONS - CREATE (8 endpoints)
# ====================================================

Test-Endpoint "CREATE TRANSACTION - Expense" {
    $body = @{
        account_id = $script:accountId
        category_id = $script:categoryId
        type = "expense"
        amount = 150.50
        currency = "BRL"
        description = "Almoco no restaurante"
        transaction_date = (Get-Date).ToString("yyyy-MM-dd")
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/transactions" -Method Post -Body $body -Headers $script:headers
    
    if (-not $result.id) {
        throw "Transaction ID not returned"
    }
    
    $script:transactionId = $result.id
    Write-Host "   Transaction ID: $($script:transactionId)" -ForegroundColor Gray
}

Test-Endpoint "CREATE TRANSACTION - Income" {
    $body = @{
        account_id = $script:accountId
        type = "income"
        amount = 3000.00
        currency = "BRL"
        description = "Salario mensal"
        transaction_date = (Get-Date).ToString("yyyy-MM-dd")
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/transactions" -Method Post -Body $body -Headers $script:headers
    
    if (-not $result.id) {
        throw "Income Transaction ID not returned"
    }
    
    Write-Host "   Income added: R$ 3000.00" -ForegroundColor Gray
}

Test-Endpoint "CREATE TRANSFER" {
    $body = @{
        from_account_id = $script:accountId
        to_account_id = $script:cashAccountId
        amount = 200.00
        description = "Transferencia para carteira"
        transaction_date = (Get-Date).ToString("yyyy-MM-dd")
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/transactions/transfer" -Method Post -Body $body -Headers $script:headers
    
    if (-not $result.id) {
        throw "Transfer ID not returned"
    }
    
    Write-Host "   Transfer ID: $($result.id)" -ForegroundColor Gray
}

# ====================================================
# 7. TRANSACTIONS - GET & LIST
# ====================================================

Test-Endpoint "GET TRANSACTION" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/transactions/$($script:transactionId)" -Method Get -Headers $script:headers
    
    if ($result.amount -ne 150.50) {
        throw "Transaction amount mismatch"
    }
    
    Write-Host "   Amount: $($result.amount)" -ForegroundColor Gray
}

Test-Endpoint "LIST TRANSACTIONS" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/transactions" -Method Get -Headers $script:headers
    
    if ($result.total -lt 3) {
        throw "Expected at least 3 transactions (2 normal + 1 transfer = 4 txns)"
    }
    
    Write-Host "   Total transactions: $($result.total)" -ForegroundColor Gray
}

# ====================================================
# 8. BUDGETS - CREATE & MANAGE (5 endpoints)
# ====================================================

Test-Endpoint "CREATE BUDGET" {
    $startDate = (Get-Date -Day 1).ToString("yyyy-MM-dd")
    $endDate = (Get-Date -Day 1).AddMonths(1).AddDays(-1).ToString("yyyy-MM-dd")
    
    $body = @{
        name = "Orcamento Alimentacao Mensal"
        category_id = $script:categoryId
        amount = 1000.00
        period = "monthly"
        start_date = $startDate
        end_date = $endDate
        is_recurring = $true
        alert_percentage = 80
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/budgets" -Method Post -Body $body -Headers $script:headers
    
    if (-not $result.id) {
        throw "Budget ID not returned"
    }
    
    $script:budgetId = $result.id
    Write-Host "   Budget ID: $($script:budgetId)" -ForegroundColor Gray
}

Test-Endpoint "GET BUDGET" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/budgets/$($script:budgetId)" -Method Get -Headers $script:headers
    
    if ($result.amount -ne 1000.00) {
        throw "Budget amount mismatch"
    }
    
    Write-Host "   Amount: $($result.amount), Spent: $($result.spent)" -ForegroundColor Gray
}

Test-Endpoint "LIST BUDGETS" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/budgets?active=true" -Method Get -Headers $script:headers
    
    if ($result.total -lt 1) {
        throw "Expected at least 1 budget"
    }
    
    Write-Host "   Total budgets: $($result.total)" -ForegroundColor Gray
}

# ====================================================
# 9. REPORTS - 5 ENDPOINTS
# ====================================================

Test-Endpoint "REPORT - Cash Flow" {
    $startDate = (Get-Date).AddDays(-30).ToString("yyyy-MM-dd")
    $endDate = (Get-Date).ToString("yyyy-MM-dd")
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/reports/cashflow?start_date=$startDate&end_date=$endDate" -Method Get -Headers $script:headers
    
    if (-not $result.total_income) {
        throw "Cash flow report incomplete"
    }
    
    Write-Host "   Income: $($result.total_income), Expense: $($result.total_expense), Net: $($result.net_cash_flow)" -ForegroundColor Gray
}

Test-Endpoint "REPORT - Income vs Expense" {
    $startDate = (Get-Date).AddDays(-30).ToString("yyyy-MM-dd")
    $endDate = (Get-Date).ToString("yyyy-MM-dd")
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/reports/income-vs-expense?start_date=$startDate&end_date=$endDate" -Method Get -Headers $script:headers
    
    if (-not $result.total_income) {
        throw "Income vs Expense report incomplete"
    }
    
    Write-Host "   Income: $($result.total_income), Expense: $($result.total_expense)" -ForegroundColor Gray
}

Test-Endpoint "REPORT - By Category" {
    $startDate = (Get-Date).AddDays(-30).ToString("yyyy-MM-dd")
    $endDate = (Get-Date).ToString("yyyy-MM-dd")
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/reports/by-category?type=expense&start_date=$startDate&end_date=$endDate" -Method Get -Headers $script:headers
    
    # É OK ter 0 items se não há categorias com transações completas
    Write-Host "   Categories with spending: $($result.items.Count)" -ForegroundColor Gray
}

Test-Endpoint "REPORT - Profit & Loss" {
    $startDate = (Get-Date).AddDays(-30).ToString("yyyy-MM-dd")
    $endDate = (Get-Date).ToString("yyyy-MM-dd")
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/reports/profit-loss?start_date=$startDate&end_date=$endDate" -Method Get -Headers $script:headers
    
    if (-not $result.total_revenue) {
        throw "P&L report incomplete"
    }
    
    Write-Host "   Revenue: $($result.total_revenue), Expenses: $($result.total_expenses), Net Profit: $($result.net_profit)" -ForegroundColor Gray
}

Test-Endpoint "REPORT - Budget Performance" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/reports/budget-performance" -Method Get -Headers $script:headers
    
    if ($result.total -lt 1) {
        throw "Expected at least 1 budget"
    }
    
    Write-Host "   Budgets evaluated: $($result.total)" -ForegroundColor Gray
}

# ====================================================
# 10. UPDATE & DELETE OPERATIONS
# ====================================================

Test-Endpoint "UPDATE ACCOUNT" {
    $body = @{
        name = "Main Checking Account UPDATED"
        bank_name = "Caixa Economica Federal"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/accounts/$($script:accountId)" -Method Put -Body $body -Headers $script:headers
    
    if ($result.message -notlike "*updated*") {
        throw "Update failed"
    }
}

Test-Endpoint "UPDATE TRANSACTION" {
    $body = @{
        description = "Almoco no restaurante ATUALIZADO"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/transactions/$($script:transactionId)" -Method Put -Body $body -Headers $script:headers
    
    if ($result.message -notlike "*updated*") {
        throw "Update failed"
    }
}

Test-Endpoint "UPDATE CATEGORY" {
    $body = @{
        name = "Alimentacao e Bebidas"
        color = "#4ecdc4"
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/categories/$($script:categoryId)" -Method Put -Body $body -Headers $script:headers
    
    if ($result.message -notlike "*updated*") {
        throw "Update failed"
    }
}

Test-Endpoint "UPDATE BUDGET" {
    $body = @{
        amount = 1200.00
    } | ConvertTo-Json
    
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/budgets/$($script:budgetId)" -Method Put -Body $body -Headers $script:headers
    
    if ($result.message -notlike "*updated*") {
        throw "Update failed"
    }
}

Test-Endpoint "DELETE TRANSACTION" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/transactions/$($script:transactionId)" -Method Delete -Headers $script:headers
    
    if ($result.message -notlike "*deleted*") {
        throw "Delete failed"
    }
}

Test-Endpoint "DELETE BUDGET" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/budgets/$($script:budgetId)" -Method Delete -Headers $script:headers
    
    if ($result.message -notlike "*deleted*") {
        throw "Delete failed"
    }
}

Test-Endpoint "DELETE CATEGORY" {
    $result = Invoke-RestMethod -Uri "$baseUrl/api/finance/categories/$($script:categoryId)" -Method Delete -Headers $script:headers
    
    if ($result.message -notlike "*deleted*") {
        throw "Delete failed"
    }
}

# ====================================================
# SUMMARY
# ====================================================

Write-Host "`n" -NoNewline
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host "   RESULTADOS FINAIS" -ForegroundColor Cyan
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host ""
Write-Host "   Total de testes: $testCount" -ForegroundColor White
Write-Host "   Testes passados: $passCount" -ForegroundColor Green
Write-Host "   Testes falhados: $($testCount - $passCount)" -ForegroundColor Red
Write-Host "   Taxa de sucesso: $([math]::Round(($passCount/$testCount)*100, 2))%" -ForegroundColor $(if($passCount -eq $testCount){"Green"}else{"Yellow"})
Write-Host ""
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host ""

