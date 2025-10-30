# ==============================================================================
# TEST CATEGORY MODULE - Complete Flow with Hierarchy
# ==============================================================================

$ErrorActionPreference = "Stop"
$baseUrl = "http://localhost:8080"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "   CATEGORY MODULE - COMPLETE TESTS" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Track results
$testsRun = 0
$testsPassed = 0

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
            if ($env:DEBUG_TESTS) {
                Write-Host "   DEBUG Body: $($params.Body)" -ForegroundColor Gray
            }
        }
        
        $response = Invoke-WebRequest @params -UseBasicParsing
        $result = $response.Content | ConvertFrom-Json
        
        if ($response.StatusCode -eq $ExpectedStatus) {
            Write-Host "   OK - Status $($response.StatusCode)" -ForegroundColor Green
            $script:testsPassed++
            return $result
        }
    } catch {
        $statusCode = $_.Exception.Response.StatusCode.value__
        if ($statusCode -eq $ExpectedStatus) {
            Write-Host "   OK - Status $statusCode (expected)" -ForegroundColor Green
            $script:testsPassed++
            return $null
        } else {
            Write-Host "   FALHOU - Expected $ExpectedStatus, got $statusCode" -ForegroundColor Red
            if ($_.ErrorDetails.Message) {
                try {
                    $errorJson = $_.ErrorDetails.Message | ConvertFrom-Json
                    Write-Host "   Error: $($errorJson.error)" -ForegroundColor Yellow
                } catch {
                    Write-Host "   Error: $($_.ErrorDetails.Message)" -ForegroundColor Yellow
                }
            }
            return $null
        }
    }
}

# ==============================================================================
# 1. REGISTER + LOGIN
# ==============================================================================
Write-Host "`n1. REGISTER + LOGIN" -ForegroundColor Cyan

$testEmail = "category.test.$(Get-Random -Maximum 9999)@acme.com"
$testPassword = "Test@123456"

$registerBody = @{
    email = $testEmail
    password = $testPassword
    name = "Category Test User"
    tenant_subdomain = "acme"
}

$registerResult = Test-Endpoint -Name "Register User" -Method "POST" -Url "$baseUrl/api/auth/register" -Body $registerBody -ExpectedStatus 201

if (-not $registerResult) {
    Write-Host "REGISTER FALHOU!" -ForegroundColor Red
    exit 1
}

$userId = $registerResult.user.id
$env:PGPASSWORD='postgre123'
$sql = "INSERT INTO user_tenants (user_id, tenant_id, role) VALUES ('$userId', (SELECT id FROM tenants WHERE subdomain='acme'), 'admin') ON CONFLICT DO NOTHING;"
psql -h localhost -p 5433 -U moneymaker_user -d moneymaker_dev -c $sql 2>&1 | Out-Null

Start-Sleep -Seconds 1

$loginBody = @{
    email = $testEmail
    password = $testPassword
    tenant_subdomain = "acme"
}

$loginResult = Test-Endpoint -Name "Login" -Method "POST" -Url "$baseUrl/api/auth/login" -Body $loginBody

$token = $loginResult.token
$authHeaders = @{
    "Authorization" = "Bearer $token"
}

# ==============================================================================
# 2. CREATE ROOT CATEGORY
# ==============================================================================
Write-Host "`n2. CREATE ROOT CATEGORY (Electronics)" -ForegroundColor Cyan

$categoryBody = @{
    name = "Electronics"
    slug = "electronics"
    description = "Electronic devices and accessories"
    display_order = 1
}

$createResult = Test-Endpoint -Name "Create Root Category" -Method "POST" -Url "$baseUrl/api/categories" -Body $categoryBody -Headers $authHeaders -ExpectedStatus 201

$rootCategoryId = $createResult.id
Write-Host "   Category ID: $rootCategoryId" -ForegroundColor White

# ==============================================================================
# 3. CREATE CHILD CATEGORY
# ==============================================================================
Write-Host "`n3. CREATE CHILD CATEGORY (Smartphones)" -ForegroundColor Cyan

$childBody = @{
    name = "Smartphones"
    slug = "smartphones"
    parent_id = $rootCategoryId
    description = "Mobile phones and accessories"
    display_order = 1
}

$childResult = Test-Endpoint -Name "Create Child Category" -Method "POST" -Url "$baseUrl/api/categories" -Body $childBody -Headers $authHeaders -ExpectedStatus 201

$childCategoryId = $childResult.id
Write-Host "   Child Category ID: $childCategoryId" -ForegroundColor White

# ==============================================================================
# 4. GET CATEGORY BY ID
# ==============================================================================
Write-Host "`n4. GET CATEGORY BY ID" -ForegroundColor Cyan

$getResult = Test-Endpoint -Name "Get Root Category" -Method "GET" -Url "$baseUrl/api/categories/$rootCategoryId" -Headers $authHeaders

if ($getResult) {
    Write-Host "   Name: $($getResult.name)" -ForegroundColor White
    Write-Host "   Slug: $($getResult.slug)" -ForegroundColor White
}

# ==============================================================================
# 5. GET ROOT CATEGORIES
# ==============================================================================
Write-Host "`n5. GET ROOT CATEGORIES" -ForegroundColor Cyan

$rootResult = Test-Endpoint -Name "Get Root Categories" -Method "GET" -Url "$baseUrl/api/categories/root" -Headers $authHeaders

if ($rootResult) {
    Write-Host "   Total Root Categories: $($rootResult.total)" -ForegroundColor White
}

# ==============================================================================
# 6. GET CATEGORY CHILDREN
# ==============================================================================
Write-Host "`n6. GET CATEGORY CHILDREN" -ForegroundColor Cyan

$childrenResult = Test-Endpoint -Name "Get Children of Electronics" -Method "GET" -Url "$baseUrl/api/categories/$rootCategoryId/children" -Headers $authHeaders

if ($childrenResult) {
    Write-Host "   Children Count: $($childrenResult.total)" -ForegroundColor White
}

# ==============================================================================
# 7. UPDATE CATEGORY
# ==============================================================================
Write-Host "`n7. UPDATE CATEGORY" -ForegroundColor Cyan

$updateBody = @{
    name = "Electronics and Gadgets"
    description = "All electronic devices, gadgets and accessories"
}

$updateResult = Test-Endpoint -Name "Update Root Category" -Method "PUT" -Url "$baseUrl/api/categories/$rootCategoryId" -Body $updateBody -Headers $authHeaders

# ==============================================================================
# 8. LIST ALL CATEGORIES
# ==============================================================================
Write-Host "`n8. LIST ALL CATEGORIES" -ForegroundColor Cyan

$listResult = Test-Endpoint -Name "List All Categories" -Method "GET" -Url "$baseUrl/api/categories" -Headers $authHeaders

if ($listResult) {
    Write-Host "   Total Categories: $($listResult.total)" -ForegroundColor White
}

# ==============================================================================
# 9. TRY DELETE CATEGORY WITH CHILDREN
# ==============================================================================
Write-Host "`n9. TRY DELETE CATEGORY WITH CHILDREN (should fail)" -ForegroundColor Cyan

$deleteFailResult = Test-Endpoint -Name "Delete Root (should fail)" -Method "DELETE" -Url "$baseUrl/api/categories/$rootCategoryId" -Headers $authHeaders -ExpectedStatus 400

# ==============================================================================
# 10. DELETE CHILD CATEGORY
# ==============================================================================
Write-Host "`n10. DELETE CHILD CATEGORY" -ForegroundColor Cyan

$deleteChildResult = Test-Endpoint -Name "Delete Child Category" -Method "DELETE" -Url "$baseUrl/api/categories/$childCategoryId" -Headers $authHeaders

# ==============================================================================
# 11. DELETE ROOT CATEGORY
# ==============================================================================
Write-Host "`n11. DELETE ROOT CATEGORY (now it should work)" -ForegroundColor Cyan

$deleteRootResult = Test-Endpoint -Name "Delete Root Category" -Method "DELETE" -Url "$baseUrl/api/categories/$rootCategoryId" -Headers $authHeaders

# ==============================================================================
# 12. VERIFY CATEGORY IS ARCHIVED
# ==============================================================================
Write-Host "`n12. VERIFY CATEGORY IS ARCHIVED" -ForegroundColor Cyan

$verifyResult = Test-Endpoint -Name "Get Deleted Category" -Method "GET" -Url "$baseUrl/api/categories/$rootCategoryId" -Headers $authHeaders

if ($verifyResult -and $verifyResult.status -eq "archived") {
    Write-Host "   Status correctly set to archived" -ForegroundColor Green
    $script:testsPassed++
}

# ==============================================================================
# SUMMARY
# ==============================================================================
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "           TEST SUMMARY" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Tests Run:    $testsRun" -ForegroundColor White
Write-Host "Tests Passed: $testsPassed" -ForegroundColor Green
Write-Host "Success Rate: $([math]::Round(($testsPassed/$testsRun)*100, 2))%" -ForegroundColor Yellow
Write-Host "========================================`n" -ForegroundColor Cyan

