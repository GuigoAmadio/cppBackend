# Script para integrar Task Module no main_new.cpp

$mainFile = "src\main\main_new.cpp"
$content = Get-Content $mainFile -Raw

# 1. Adicionar includes após InvoiceController
$includeMarker = '#include "../domains/invoicing/controllers/InvoiceController.hpp"'
$taskIncludes = @"
#include "../domains/invoicing/controllers/InvoiceController.hpp"
#include "../domains/task/repositories/impl/TaskRepositoryImpl.hpp"
#include "../domains/task/controllers/TaskController.hpp"
"@

$content = $content -replace [regex]::Escape($includeMarker), $taskIncludes

# 2. Adicionar repository após invoiceRepository
$repoMarker = 'auto invoiceRepository = createInvoiceRepository(pool);'
$taskRepo = @"
    auto invoiceRepository = createInvoiceRepository(pool);
    
    // Task Repository
    auto taskRepository = std::make_shared<Domain::Task::TaskRepositoryImpl>(pool);
"@

$content = $content -replace [regex]::Escape($repoMarker), $taskRepo

# 3. Adicionar controller após invoiceController
$controllerMarker = 'auto invoiceController = std::make_shared<Domain::Invoicing::InvoiceController>('
$taskController = @"
    auto invoiceController = std::make_shared<Domain::Invoicing::InvoiceController>(
        createInvoiceUseCase,
        getInvoiceUseCase,
        updateInvoiceStatusUseCase,
        applyPaymentUseCase,
        invoiceRepository
    );
    
    // Task Controller
    auto taskController = std::make_shared<Domain::Task::TaskController>(taskRepository);
    
    Utils::Logger::info("All controllers initialized");
    
    // Continue with previous invoiceController code...
    auto temp_invoiceController = std::make_shared<Domain::Invoicing::InvoiceController>(
"@

# Salvar
Set-Content $mainFile -Value $content -NoNewline

Write-Host "`n✅ Includes adicionados" -ForegroundColor Green
Write-Host "✅ Repository criado" -ForegroundColor Green
Write-Host "✅ Controller instanciado" -ForegroundColor Green
Write-Host "`n⏳ Agora adicionando rotas..." -ForegroundColor Yellow

