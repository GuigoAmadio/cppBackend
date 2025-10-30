# Setup test user for Customer tests
$baseUrl = "http://localhost:8080"

Write-Host "Creating test user for Customer tests..." -ForegroundColor Cyan

# Register
$registerBody = @{
    email = "customer.test@acme.com"
    password = "Test@123"
    full_name = "Customer Test User"
    tenant_subdomain = "acme"
} | ConvertTo-Json

try {
    $response = Invoke-WebRequest -Uri "$baseUrl/api/auth/register" -Method POST -Body $registerBody -ContentType "application/json" -UseBasicParsing 2>&1
    Write-Host "User registered!" -ForegroundColor Green
    
    $result = $response.Content | ConvertFrom-Json
    $userId = $result.user_id
    
    Write-Host "User ID: $userId" -ForegroundColor White
    
    # Add to tenant via SQL
    Write-Host "`nAdding user to 'acme' tenant..." -ForegroundColor Yellow
    $env:PGPASSWORD='postgre123'
    $sql = "INSERT INTO user_tenants (user_id, tenant_id, role) VALUES ('$userId', (SELECT id FROM tenants WHERE subdomain='acme'), 'admin') ON CONFLICT DO NOTHING;"
    psql -h localhost -p 5433 -U moneymaker_user -d moneymaker_dev -c $sql | Out-Null
    Write-Host "User added to tenant!" -ForegroundColor Green
    
} catch {
    if ($_.Exception.Response.StatusCode.value__ -eq 409) {
        Write-Host "User already exists!" -ForegroundColor Yellow
    } else {
        Write-Host "Error: $_" -ForegroundColor Red
    }
}

Write-Host "`nSetup complete!" -ForegroundColor Green

