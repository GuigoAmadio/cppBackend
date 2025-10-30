# Criar Estrutura dos 13 Novos Modulos

Write-Host ""
Write-Host "Criando estrutura dos 13 novos modulos..." -ForegroundColor Cyan
Write-Host ""

$baseDir = "src\domains"

$modules = @(
    "workspace",
    "task",
    "calendar",
    "timetracking",
    "finance",
    "invoicing",
    "subscription",
    "product",
    "order",
    "payment",
    "inventory",
    "customer",
    "crm"
)

$subdirs = @(
    "entities",
    "value_objects",
    "repositories",
    "use_cases",
    "controllers"
)

$totalDirs = 0

foreach ($module in $modules) {
    Write-Host "Criando modulo: $module" -ForegroundColor Yellow
    
    $modulePath = Join-Path $baseDir $module
    
    foreach ($subdir in $subdirs) {
        $fullPath = Join-Path $modulePath $subdir
        
        if (!(Test-Path $fullPath)) {
            New-Item -ItemType Directory -Path $fullPath -Force | Out-Null
            Write-Host "  OK $subdir" -ForegroundColor Green
            $totalDirs++
        } else {
            Write-Host "  Existe $subdir" -ForegroundColor Gray
        }
    }
    
    if ($module -eq "payment") {
        $servicesPath = Join-Path $modulePath "services"
        if (!(Test-Path $servicesPath)) {
            New-Item -ItemType Directory -Path $servicesPath -Force | Out-Null
            Write-Host "  OK services" -ForegroundColor Green
            $totalDirs++
        }
    }
    
    Write-Host ""
}

Write-Host "Estrutura criada com sucesso!" -ForegroundColor Green
Write-Host ""
Write-Host "Modulos criados: $($modules.Count)" -ForegroundColor White
Write-Host "Diretorios criados: $totalDirs" -ForegroundColor White
Write-Host ""
