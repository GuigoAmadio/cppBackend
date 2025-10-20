# ===================================
# TESTES DE SEGURANCA E VALIDACAO
# ===================================

$baseUrl = "http://localhost:8080"
$testsPassed = 0
$testsFailed = 0

function Test-Endpoint {
    param(
        [string]$Name,
        [string]$Url,
        [string]$Method = "POST",
        [hashtable]$Body,
        [int]$ExpectedStatus
    )
    
    Write-Host "`n=== $Name ===" -ForegroundColor Cyan
    
    try {
        $jsonBody = ($Body | ConvertTo-Json -Compress)
        Write-Host "Body: $jsonBody" -ForegroundColor Gray
        
        $response = Invoke-WebRequest -Uri $Url `
            -Method $Method `
            -Body $jsonBody `
            -ContentType "application/json; charset=utf-8" `
            -ErrorAction Stop
        
        $statusCode = $response.StatusCode
        $content = $response.Content
        
        if ($statusCode -eq $ExpectedStatus) {
            Write-Host "PASS - Status: $statusCode" -ForegroundColor Green
            Write-Host "Response: $content" -ForegroundColor Gray
            $script:testsPassed++
        } else {
            Write-Host "FAIL - Expected: $ExpectedStatus, Got: $statusCode" -ForegroundColor Red
            $script:testsFailed++
        }
    } catch {
        $statusCode = 0
        $errorBody = ""
        
        if ($_.Exception.Response) {
            $statusCode = [int]$_.Exception.Response.StatusCode
            if ($_.ErrorDetails.Message) {
                $errorBody = $_.ErrorDetails.Message
            }
        }
        
        if ($statusCode -eq $ExpectedStatus) {
            Write-Host "PASS - Status: $statusCode" -ForegroundColor Green
            Write-Host "Response: $errorBody" -ForegroundColor Gray
            $script:testsPassed++
        } else {
            Write-Host "FAIL - Expected: $ExpectedStatus, Got: $statusCode" -ForegroundColor Red
            Write-Host "Error: $errorBody" -ForegroundColor Red
            $script:testsFailed++
        }
    }
}

Write-Host "╔══════════════════════════════════════════╗" -ForegroundColor Yellow
Write-Host "║   TESTES DE SEGURANCA - C++ BACKEND      ║" -ForegroundColor Yellow
Write-Host "╚══════════════════════════════════════════╝" -ForegroundColor Yellow

# ============================================
# CATEGORIA 1: VALIDACAO DE EMAIL
# ============================================
Write-Host "`n### CATEGORIA 1: VALIDACAO DE EMAIL ###" -ForegroundColor Magenta

Test-Endpoint -Name "Email invalido - sem @" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "invalidemail"; password = "SecureP@ss123"; name = "Test" } `
    -ExpectedStatus 400

Test-Endpoint -Name "Email invalido - sem dominio" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "test@"; password = "SecureP@ss123"; name = "Test" } `
    -ExpectedStatus 400

Test-Endpoint -Name "Email invalido - espacos" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "test @example.com"; password = "SecureP@ss123"; name = "Test" } `
    -ExpectedStatus 400

Test-Endpoint -Name "Email invalido - muito longo" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "a" * 300 + "@example.com"; password = "SecureP@ss123"; name = "Test" } `
    -ExpectedStatus 400

Test-Endpoint -Name "Email invalido - caracteres especiais" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "test<>@example.com"; password = "SecureP@ss123"; name = "Test" } `
    -ExpectedStatus 400

# ============================================
# CATEGORIA 2: VALIDACAO DE SENHA
# ============================================
Write-Host "`n### CATEGORIA 2: VALIDACAO DE SENHA ###" -ForegroundColor Magenta

Test-Endpoint -Name "Senha muito curta - menos de 8 chars" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "test2@example.com"; password = "Pass1!"; name = "Test" } `
    -ExpectedStatus 400

Test-Endpoint -Name "Senha sem maiuscula" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "test3@example.com"; password = "password123!"; name = "Test" } `
    -ExpectedStatus 400

Test-Endpoint -Name "Senha sem minuscula" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "test4@example.com"; password = "PASSWORD123!"; name = "Test" } `
    -ExpectedStatus 400

Test-Endpoint -Name "Senha sem numero" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "test5@example.com"; password = "Password!"; name = "Test" } `
    -ExpectedStatus 400

Test-Endpoint -Name "Senha sem caractere especial" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "test6@example.com"; password = "Password123"; name = "Test" } `
    -ExpectedStatus 400

Test-Endpoint -Name "Senha vazia" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "test7@example.com"; password = ""; name = "Test" } `
    -ExpectedStatus 400

# ============================================
# CATEGORIA 3: CAMPOS FALTANDO
# ============================================
Write-Host "`n### CATEGORIA 3: CAMPOS FALTANDO ###" -ForegroundColor Magenta

Test-Endpoint -Name "Sem email" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ password = "SecureP@ss123"; name = "Test" } `
    -ExpectedStatus 400

Test-Endpoint -Name "Sem password" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "test8@example.com"; name = "Test" } `
    -ExpectedStatus 400

Test-Endpoint -Name "Sem name" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "test9@example.com"; password = "SecureP@ss123" } `
    -ExpectedStatus 400

Test-Endpoint -Name "Body vazio" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{} `
    -ExpectedStatus 400

# ============================================
# CATEGORIA 4: SQL INJECTION
# ============================================
Write-Host "`n### CATEGORIA 4: SQL INJECTION ###" -ForegroundColor Magenta

Test-Endpoint -Name "SQL Injection - email com OR 1=1" `
    -Url "$baseUrl/api/auth/login" `
    -Body @{ email = "admin' OR '1'='1"; password = "anything" } `
    -ExpectedStatus 400

Test-Endpoint -Name "SQL Injection - password com DROP TABLE" `
    -Url "$baseUrl/api/auth/login" `
    -Body @{ email = "test@example.com"; password = "'; DROP TABLE users; --" } `
    -ExpectedStatus 401

Test-Endpoint -Name "SQL Injection - email com UNION SELECT" `
    -Url "$baseUrl/api/auth/login" `
    -Body @{ email = "' UNION SELECT * FROM users--"; password = "anything" } `
    -ExpectedStatus 400

# ============================================
# CATEGORIA 5: AUTENTICACAO
# ============================================
Write-Host "`n### CATEGORIA 5: AUTENTICACAO ###" -ForegroundColor Magenta

Test-Endpoint -Name "Login - email invalido" `
    -Url "$baseUrl/api/auth/login" `
    -Body @{ email = "notfound@example.com"; password = "SecureP@ss123" } `
    -ExpectedStatus 401

Test-Endpoint -Name "Login - senha incorreta" `
    -Url "$baseUrl/api/auth/login" `
    -Body @{ email = "user@example.com"; password = "WrongPassword123!" } `
    -ExpectedStatus 401

Test-Endpoint -Name "Login - email vazio" `
    -Url "$baseUrl/api/auth/login" `
    -Body @{ email = ""; password = "SecureP@ss123" } `
    -ExpectedStatus 400

# ============================================
# CATEGORIA 6: XSS (Cross-Site Scripting)
# ============================================
Write-Host "`n### CATEGORIA 6: XSS (Cross-Site Scripting) ###" -ForegroundColor Magenta

Test-Endpoint -Name "XSS - script tag no name" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "xss1@example.com"; password = "SecureP@ss123"; name = "<script>alert('xss')</script>" } `
    -ExpectedStatus 400

Test-Endpoint -Name "XSS - HTML no name" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "xss2@example.com"; password = "SecureP@ss123"; name = "<img src=x onerror=alert(1)>" } `
    -ExpectedStatus 400

# ============================================
# CATEGORIA 7: DUPLICACAO
# ============================================
Write-Host "`n### CATEGORIA 7: DUPLICACAO ###" -ForegroundColor Magenta

# Primeiro, criar um usuario valido
Test-Endpoint -Name "Criar usuario valido" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "duplicate@example.com"; password = "SecureP@ss123"; name = "Duplicate Test" } `
    -ExpectedStatus 201

# Tentar criar novamente com mesmo email
Test-Endpoint -Name "Email duplicado" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "duplicate@example.com"; password = "SecureP@ss123"; name = "Duplicate Test 2" } `
    -ExpectedStatus 409

# ============================================
# CATEGORIA 8: TAMANHOS EXTREMOS
# ============================================
Write-Host "`n### CATEGORIA 8: TAMANHOS EXTREMOS ###" -ForegroundColor Magenta

Test-Endpoint -Name "Name muito longo (>100 chars)" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "long@example.com"; password = "SecureP@ss123"; name = "A" * 200 } `
    -ExpectedStatus 400

Test-Endpoint -Name "Password muito longo (>128 chars)" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "long2@example.com"; password = "A" * 150; name = "Test" } `
    -ExpectedStatus 400

# ============================================
# CATEGORIA 9: UNICODE E CARACTERES ESPECIAIS
# ============================================
Write-Host "`n### CATEGORIA 9: UNICODE E CARACTERES ESPECIAIS ###" -ForegroundColor Magenta

Test-Endpoint -Name "Nome com emojis" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "emoji@example.com"; password = "SecureP@ss123"; name = "Test 😀🎉" } `
    -ExpectedStatus 400

Test-Endpoint -Name "Email com unicode" `
    -Url "$baseUrl/api/auth/register" `
    -Body @{ email = "tëst@éxample.com"; password = "SecureP@ss123"; name = "Test" } `
    -ExpectedStatus 400

# ============================================
# RESUMO
# ============================================
Write-Host "`n╔══════════════════════════════════════════╗" -ForegroundColor Yellow
Write-Host "║            RESUMO DOS TESTES             ║" -ForegroundColor Yellow
Write-Host "╚══════════════════════════════════════════╝" -ForegroundColor Yellow
Write-Host "✓ Testes Passados: $testsPassed" -ForegroundColor Green
Write-Host "✗ Testes Falhados: $testsFailed" -ForegroundColor Red
Write-Host "Total: $($testsPassed + $testsFailed)" -ForegroundColor Cyan

if ($testsFailed -eq 0) {
    Write-Host "`n🎉 TODOS OS TESTES PASSARAM! 🎉" -ForegroundColor Green
} else {
    Write-Host "`n⚠️  ALGUNS TESTES FALHARAM" -ForegroundColor Yellow
}

