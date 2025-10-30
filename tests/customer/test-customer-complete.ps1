# ==============================================================================
# TEST CUSTOMER MODULE - Complete Flow
# ==============================================================================

$ErrorActionPreference = "Stop"
$baseUrl = "http://localhost:8080"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   CUSTOMER MODULE - COMPLETE TESTS" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Track results
$testsRun = 0
$testsPassed = 0
$testsFailed = 0

function Test-Endpoint {
    param(
        [string]$Name,
        [string]$Method,
        [string]$Url,
        [object]$Body = $null,
        [hashtable]$Headers = @{},
        [int]$ExpectedStatus = 200
    )
    
    $script:testsRun++
    Write-Host "[$script:testsRun] $Name" -ForegroundColor Yellow
    
    try {
        $params = @{
            Uri = $Url
            Method = $Method
            Headers = $Headers
            ContentType = "application/json"
        }
        
        if ($Body) {
            $params.Body = ($Body | ConvertTo-Json -Depth 10)
        }
        
        $response = Invoke-WebRequest @params -UseBasicParsing
        $result = $response.Content | ConvertFrom-Json
        
        if ($response.StatusCode -eq $ExpectedStatus) {
            Write-Host "   OK - Status $($response.StatusCode)" -ForegroundColor Green
            $script:testsPassed++
            return $result
        } else {
            Write-Host "   FALHOU - Expected $ExpectedStatus, got $($response.StatusCode)" -ForegroundColor Red
            $script:testsFailed++
            return $null
        }
    } catch {
        $statusCode = $_.Exception.Response.StatusCode.value__
        if ($statusCode -eq $ExpectedStatus) {
            Write-Host "   OK - Status $statusCode (expected)" -ForegroundColor Green
            $script:testsPassed++
            
            try {
                $errorBody = $_.ErrorDetails.Message | ConvertFrom-Json
                return $errorBody
            } catch {
                return $null
            }
        } else {
            Write-Host "   FALHOU - Expected $ExpectedStatus, got $statusCode" -ForegroundColor Red
            Write-Host "   Error: $($_.Exception.Message)" -ForegroundColor Red
            $script:testsFailed++
            return $null
        }
    }
}

# ==============================================================================
# 1. REGISTER NEW USER
# ==============================================================================
Write-Host "`n1. REGISTER NEW USER" -ForegroundColor Cyan

$testEmail = "customer.test.$(Get-Random -Maximum 9999)@acme.com"
$testPassword = "Test@123456"

$registerBody = @{
    email = $testEmail
    password = $testPassword
    name = "Customer Test User"
    tenant_subdomain = "acme"
}

$registerResult = Test-Endpoint `
    -Name "Register User" `
    -Method "POST" `
    -Url "$baseUrl/api/auth/register" `
    -Body $registerBody `
    -ExpectedStatus 201

if (-not $registerResult) {
    Write-Host "`nREGISTER FALHOU! Abortando testes." -ForegroundColor Red
    exit 1
}

$userId = $registerResult.user.id
Write-Host "   User ID: $userId" -ForegroundColor Green
Write-Host "   Email: $testEmail" -ForegroundColor White

# Add user to tenant via SQL
Write-Host "   Adding user to acme tenant..." -ForegroundColor Yellow
$env:PGPASSWORD='postgre123'
$sql = "INSERT INTO user_tenants (user_id, tenant_id, role) VALUES ('$userId', (SELECT id FROM tenants WHERE subdomain='acme'), 'admin') ON CONFLICT DO NOTHING;"
psql -h localhost -p 5433 -U moneymaker_user -d moneymaker_dev -c $sql 2>&1 | Out-Null
Write-Host "   User added to tenant!" -ForegroundColor Green

Start-Sleep -Seconds 1

# ==============================================================================
# 2. LOGIN
# ==============================================================================
Write-Host "`n2. LOGIN" -ForegroundColor Cyan

$loginBody = @{
    email = $testEmail
    password = $testPassword
    tenant_subdomain = "acme"
}

$loginResult = Test-Endpoint `
    -Name "Login with new user" `
    -Method "POST" `
    -Url "$baseUrl/api/auth/login" `
    -Body $loginBody

if (-not $loginResult) {
    Write-Host "`nLOGIN FALHOU! Abortando testes." -ForegroundColor Red
    exit 1
}

# Debug: Ver estrutura do response
Write-Host "   Login response:" -ForegroundColor Gray
Write-Host "   $($loginResult | ConvertTo-Json -Compress)" -ForegroundColor Gray

# Extrair token (pode ser 'access_token' ou 'token')
$token = if ($loginResult.PSObject.Properties['access_token']) { 
    $loginResult.access_token 
} elseif ($loginResult.PSObject.Properties['token']) { 
    $loginResult.token 
} else { 
    $null 
}

if (-not $token) {
    Write-Host "`nTOKEN NÃO ENCONTRADO NO RESPONSE! Abortando." -ForegroundColor Red
    exit 1
}

$authHeaders = @{
    "Authorization" = "Bearer $token"
}

Write-Host "   Token obtained: $($token.Substring(0, 30))..." -ForegroundColor Green

# ==============================================================================
# 3. CREATE CUSTOMER (Individual - CPF)
# ==============================================================================
Write-Host "`n3. CREATE CUSTOMER (Individual)" -ForegroundColor Cyan

$customerBody = @{
    name = "Joao Silva"
    type = "individual"
    email = "joao.silva@email.com"
    phone = "+55 11 99999-1111"
    document = "123.456.789-00"
    address = '{"street":"Rua A","city":"Sao Paulo","state":"SP","zip":"01000-000"}'
    notes = "Cliente VIP"
}

$createResult = Test-Endpoint `
    -Name "Create Individual Customer" `
    -Method "POST" `
    -Url "$baseUrl/api/customers" `
    -Body $customerBody `
    -Headers $authHeaders `
    -ExpectedStatus 201

if (-not $createResult) {
    Write-Host "   ERRO: Falha ao criar customer" -ForegroundColor Red
    exit 1
}

$customerId1 = $createResult.id
Write-Host "   Customer ID: $customerId1" -ForegroundColor Green

# ==============================================================================
# 4. CREATE CUSTOMER (Business - CNPJ)
# ==============================================================================
Write-Host "`n4. CREATE CUSTOMER (Business)" -ForegroundColor Cyan

$businessBody = @{
    name = "Empresa XPTO Ltda"
    type = "business"
    email = "contato@empresaxpto.com"
    phone = "+55 11 3333-4444"
    document = "12.345.678/0001-90"
    address = '{"street":"Av. Paulista 1000","city":"Sao Paulo","state":"SP","zip":"01310-100"}'
    notes = "Parceiro comercial"
}

$createResult2 = Test-Endpoint `
    -Name "Create Business Customer" `
    -Method "POST" `
    -Url "$baseUrl/api/customers" `
    -Body $businessBody `
    -Headers $authHeaders `
    -ExpectedStatus 201

if ($createResult2) {
    $customerId2 = $createResult2.id
    Write-Host "   Business ID: $customerId2" -ForegroundColor Green
}

# ==============================================================================
# 5. GET CUSTOMER BY ID
# ==============================================================================
Write-Host "`n5. GET CUSTOMER BY ID" -ForegroundColor Cyan

$getResult = Test-Endpoint `
    -Name "Get Customer $customerId1" `
    -Method "GET" `
    -Url "$baseUrl/api/customers/$customerId1" `
    -Headers $authHeaders

if ($getResult) {
    Write-Host "   Name: $($getResult.name)" -ForegroundColor White
    Write-Host "   Email: $($getResult.email)" -ForegroundColor White
    Write-Host "   Type: $($getResult.type)" -ForegroundColor White
    Write-Host "   Status: $($getResult.status)" -ForegroundColor White
}

# ==============================================================================
# 6. UPDATE CUSTOMER
# ==============================================================================
Write-Host "`n6. UPDATE CUSTOMER" -ForegroundColor Cyan

$updateBody = @{
    name = "Joao Silva Santos"
    phone = "+55 11 98888-2222"
    notes = "Cliente VIP Premium"
}

$updateResult = Test-Endpoint `
    -Name "Update Customer" `
    -Method "PUT" `
    -Url "$baseUrl/api/customers/$customerId1" `
    -Body $updateBody `
    -Headers $authHeaders

# ==============================================================================
# 7. GET UPDATED CUSTOMER
# ==============================================================================
Write-Host "`n7. GET UPDATED CUSTOMER" -ForegroundColor Cyan

$getUpdatedResult = Test-Endpoint `
    -Name "Get Updated Customer" `
    -Method "GET" `
    -Url "$baseUrl/api/customers/$customerId1" `
    -Headers $authHeaders

if ($getUpdatedResult) {
    Write-Host "   Updated Name: $($getUpdatedResult.name)" -ForegroundColor White
    Write-Host "   Updated Phone: $($getUpdatedResult.phone)" -ForegroundColor White
}

# ==============================================================================
# 8. LIST ALL CUSTOMERS
# ==============================================================================
Write-Host "`n8. LIST ALL CUSTOMERS" -ForegroundColor Cyan

$listResult = Test-Endpoint `
    -Name "List Customers" `
    -Method "GET" `
    -Url "$baseUrl/api/customers" `
    -Headers $authHeaders

if ($listResult) {
    Write-Host "   Total: $($listResult.total)" -ForegroundColor White
}

# ==============================================================================
# 9. SEARCH CUSTOMERS
# ==============================================================================
Write-Host "`n9. SEARCH CUSTOMERS" -ForegroundColor Cyan

$searchResult = Test-Endpoint `
    -Name "Search 'Silva'" `
    -Method "GET" `
    -Url "$baseUrl/api/customers/search?q=Silva" `
    -Headers $authHeaders

if ($searchResult) {
    Write-Host "   Found: $($searchResult.total)" -ForegroundColor White
}

# ==============================================================================
# 10. DUPLICATE EMAIL VALIDATION
# ==============================================================================
Write-Host "`n10. DUPLICATE EMAIL VALIDATION" -ForegroundColor Cyan

$duplicateBody = @{
    name = "Outro Cliente"
    type = "individual"
    email = "joao.silva@email.com"
}

$duplicateResult = Test-Endpoint `
    -Name "Try Duplicate Email" `
    -Method "POST" `
    -Url "$baseUrl/api/customers" `
    -Body $duplicateBody `
    -Headers $authHeaders `
    -ExpectedStatus 400

# ==============================================================================
# 11. DELETE CUSTOMER (Soft Delete)
# ==============================================================================
Write-Host "`n11. DELETE CUSTOMER" -ForegroundColor Cyan

$deleteResult = Test-Endpoint `
    -Name "Delete Customer $customerId1" `
    -Method "DELETE" `
    -Url "$baseUrl/api/customers/$customerId1" `
    -Headers $authHeaders

# ==============================================================================
# 12. VERIFY CUSTOMER IS INACTIVE
# ==============================================================================
Write-Host "`n12. VERIFY CUSTOMER STATUS" -ForegroundColor Cyan

$verifyResult = Test-Endpoint `
    -Name "Get Deleted Customer" `
    -Method "GET" `
    -Url "$baseUrl/api/customers/$customerId1" `
    -Headers $authHeaders

if ($verifyResult -and $verifyResult.status -eq "inactive") {
    Write-Host "   Status correctly set to inactive" -ForegroundColor Green
    $script:testsPassed++
} else {
    Write-Host "   ERROR: Status not set to inactive" -ForegroundColor Red
    $script:testsFailed++
}

# ==============================================================================
# SUMMARY
# ==============================================================================
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "           TEST SUMMARY" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Tests Run:    $testsRun" -ForegroundColor White
Write-Host "Tests Passed: $testsPassed" -ForegroundColor Green
Write-Host "Tests Failed: $testsFailed" -ForegroundColor $(if ($testsFailed -gt 0) { "Red" } else { "Green" })
Write-Host "Success Rate: $([math]::Round(($testsPassed/$testsRun)*100, 2))%" -ForegroundColor Yellow
Write-Host "========================================`n" -ForegroundColor Cyan

if ($testsFailed -gt 0) {
    exit 1
}

