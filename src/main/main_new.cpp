/**
 * 🚀 C++ Backend do ZERO - VERSÃO 2.0
 * 
 * ✅ ConnectionPool - Pool de conexões reutilizáveis
 * ✅ Middleware System - Pipeline de requisições
 * ✅ Logger melhorado - Cores, debug mode, file output
 */

#include <cstdlib>
#include <ctime>
#include "../core/http/Server.hpp"
#include "../core/http/Router.hpp"
#include "../core/http/Request.hpp"
#include "../core/http/Response.hpp"
#include "../core/http/Middleware.hpp"
#include "../infrastructure/middleware/TenantMiddleware.hpp"
#include "../infrastructure/middleware/AuthMiddleware.hpp"
#include "../infrastructure/middleware/RoleMiddleware.hpp"
#include "../infrastructure/middleware/RateLimitMiddleware.hpp"
#include "../infrastructure/middleware/MetricsMiddleware.hpp"
#include "../core/utils/MetricsCollector.hpp"
#include "../core/utils/GlobalMetrics.hpp"
#include "../core/database/Connection.hpp"
#include "../core/database/GlobalPool.hpp"
#include "../domains/leiloes/controllers/LeilaoController.hpp"
#include "../domains/offers/controllers/OfferController.hpp"
#include "../core/database/ConnectionPool.hpp"
#include "../core/cache/RedisPool.hpp"
#include "../core/cache/RedisCacheService.hpp"
#include "../core/cache/RedisPubSub.hpp"
#include "../core/websocket/WebSocketManager.hpp"
#include "../core/websocket/WebSocketHandler.hpp"
#include "../core/queue/RedisMessageQueue.hpp"
#include "../core/queue/QueueWorker.hpp"
#include "../core/utils/Logger.hpp"
#include "../core/utils/LoggerNew.hpp"
#include "../domains/user_management/repositories/UserRepository.hpp"
#include "../domains/tenant_management/repositories/TenantRepository.hpp"
#include "../domains/iam/services/JwtService.hpp"
#include "../domains/iam/services/BcryptService.hpp"
#include "../domains/iam/use_cases/RegisterUserUseCase.hpp"
#include "../domains/iam/use_cases/LoginUserUseCase.hpp"
#include "../domains/user_management/use_cases/GetUserUseCase.hpp"
#include "../domains/user_management/use_cases/UpdateUserUseCase.hpp"
#include "../domains/user_management/use_cases/ChangePasswordUseCase.hpp"
#include "../domains/user_management/use_cases/DeleteUserUseCase.hpp"
#include "../domains/user_management/use_cases/ListUsersUseCase.hpp"
#include "../domains/tenant_management/use_cases/ListTenantMembersUseCase.hpp"
#include "../domains/tenant_management/value_objects/TenantUserRole.hpp"
#include "../domains/tenant_management/use_cases/UpdateUserRoleUseCase.hpp"
#include "../domains/tenant_management/use_cases/RemoveUserFromTenantUseCase.hpp"
#include "../domains/tenant_management/use_cases/AddUserToTenantUseCase.hpp"
#include "../domains/iam/use_cases/RefreshTokenUseCase.hpp"
#include "../domains/notification/use_cases/SendVerificationEmailUseCase.hpp"
#include "../domains/iam/use_cases/VerifyEmailUseCase.hpp"
#include "../domains/notification/use_cases/RequestPasswordResetUseCase.hpp"
#include "../domains/notification/use_cases/ResetPasswordUseCase.hpp"
#include "../domains/notification/services/EmailVerificationRepository.hpp"
#include "../domains/notification/services/PasswordResetRepository.hpp"
#include "../domains/audit/repositories/AuditLogRepository.hpp"
#include "../domains/notification/services/EmailService.hpp"
#include "../domains/iam/controllers/AuthController.hpp"
#include "../domains/user_management/controllers/UsersController.hpp"
#include "../domains/workspace/repositories/WorkspaceRepository.hpp"
#include "../domains/workspace/controllers/WorkspaceController.hpp"
#include "../domains/product/repositories/ProductRepository.hpp"
#include "../domains/product/controllers/ProductController.hpp"
#include "../domains/inventory/repositories/InventoryRepository.hpp"
#include "../domains/inventory/controllers/InventoryController.hpp"
#include "../domains/customer/repositories/CustomerRepository.hpp"
#include "../domains/customer/controllers/CustomerController.hpp"
#include "../domains/category/repositories/CategoryRepository.hpp"
#include "../domains/category/controllers/CategoryController.hpp"
#include "../domains/order/repositories/OrderRepository.hpp"
#include "../domains/order/controllers/OrderController.hpp"
#include "../domains/payment/repositories/PaymentRepositoryImpl.hpp"
#include "../domains/payment/use_cases/CreatePaymentUseCase.hpp"
#include "../domains/payment/use_cases/ProcessPaymentUseCase.hpp"
#include "../domains/payment/use_cases/RefundPaymentUseCase.hpp"
#include "../domains/payment/use_cases/CancelPaymentUseCase.hpp"
#include "../domains/payment/use_cases/GetPaymentUseCase.hpp"
#include "../domains/payment/use_cases/ListPaymentsUseCase.hpp"
#include "../domains/payment/controllers/PaymentController.hpp"

// Subscription Domain
#include "../domains/subscription/repositories/impl/PlanRepositoryImpl.hpp"
#include "../domains/subscription/repositories/impl/SubscriptionRepositoryImpl.hpp"
#include "../domains/subscription/repositories/impl/BillingHistoryRepositoryImpl.hpp"
#include "../domains/subscription/controllers/SubscriptionController.hpp"

// Invoicing Domain
#include "../domains/invoicing/repositories/impl/InvoiceRepositoryImpl.hpp"
#include "../domains/invoicing/use_cases/CreateInvoiceUseCase.hpp"
#include "../domains/invoicing/use_cases/GetInvoiceUseCase.hpp"
#include "../domains/invoicing/use_cases/UpdateInvoiceStatusUseCase.hpp"
#include "../domains/invoicing/use_cases/ApplyPaymentUseCase.hpp"
#include "../domains/invoicing/controllers/InvoiceController.hpp"

// Task Domain
#include "../domains/task/repositories/TaskRepository.hpp"
#include "../domains/task/repositories/impl/TaskRepositoryImpl.hpp"
#include "../domains/task/use_cases/CreateTaskUseCase.hpp"
#include "../domains/task/use_cases/GetTaskUseCase.hpp"
#include "../domains/task/use_cases/ListTasksUseCase.hpp"
#include "../domains/task/use_cases/UpdateTaskUseCase.hpp"
#include "../domains/task/use_cases/UpdateStatusUseCase.hpp"
#include "../domains/task/use_cases/DeleteTaskUseCase.hpp"
#include "../domains/task/use_cases/AddCommentUseCase.hpp"
#include "../domains/task/use_cases/AddChecklistUseCase.hpp"
#include "../domains/task/controllers/TaskController.hpp"

// Database/Admin Domain
#include "../domains/database/controllers/DatabaseController.hpp"
#include "../domains/database/use_cases/ExecuteQueryUseCase.hpp"

// Calendar Domain
#include "../domains/calendar/repositories/EventRepository.hpp"
#include "../domains/calendar/repositories/impl/EventRepositoryImpl.hpp"
#include "../domains/calendar/controllers/EventController.hpp"

// Finance Domain
#include "../domains/finance/repositories/impl/AccountRepositoryImpl.hpp"
#include "../domains/finance/repositories/impl/TransactionRepositoryImpl.hpp"
#include "../domains/finance/repositories/impl/CategoryRepositoryImpl.hpp"
#include "../domains/finance/repositories/impl/BudgetRepositoryImpl.hpp"
#include "../domains/finance/repositories/impl/ReconciliationRepositoryImpl.hpp"
#include "../domains/finance/controllers/FinanceController.hpp"

// ==================== FREELANCER APP MODULES ====================
// Restaurantes Domain
#include "../domains/restaurantes/entities/RestauranteProfile.hpp"
#include "../domains/restaurantes/repositories/IRestauranteRepository.hpp"
#include "../domains/restaurantes/repositories/RestauranteRepository.hpp"
#include "../domains/restaurantes/use_cases/CreateRestauranteUseCase.hpp"
#include "../domains/restaurantes/use_cases/GetRestauranteUseCase.hpp"
#include "../domains/restaurantes/use_cases/UpdateRestauranteUseCase.hpp"
#include "../domains/restaurantes/use_cases/ListRestaurantesUseCase.hpp"
#include "../domains/restaurantes/use_cases/SearchNearbyRestaurantesUseCase.hpp"
#include "../domains/restaurantes/controllers/RestauranteController.hpp"

// Freelances Domain
#include "../domains/freelances/entities/FreelancerProfile.hpp"
#include "../domains/freelances/repositories/IFreelancerRepository.hpp"
#include "../domains/freelances/repositories/FreelancerRepository.hpp"
#include "../domains/freelances/use_cases/CreateFreelancerUseCase.hpp"
#include "../domains/freelances/use_cases/GetFreelancerUseCase.hpp"
#include "../domains/freelances/use_cases/UpdateFreelancerUseCase.hpp"
#include "../domains/freelances/use_cases/ListFreelancersUseCase.hpp"
#include "../domains/freelances/use_cases/SearchNearbyFreelancersUseCase.hpp"
#include "../domains/freelances/controllers/FreelancerController.hpp"

// Vagas Domain
#include "../domains/vagas/entities/Vaga.hpp"
#include "../domains/vagas/repositories/IVagaRepository.hpp"
#include "../domains/vagas/repositories/VagaRepository.hpp"
#include "../domains/vagas/use_cases/CreateVagaUseCase.hpp"
#include "../domains/vagas/use_cases/GetVagaUseCase.hpp"
#include "../domains/vagas/use_cases/UpdateVagaUseCase.hpp"
#include "../domains/vagas/use_cases/ListVagasUseCase.hpp"
#include "../domains/vagas/controllers/VagaController.hpp"

// Candidaturas Domain
#include "../domains/candidaturas/entities/Candidatura.hpp"
#include "../domains/candidaturas/repositories/ICandidaturaRepository.hpp"
#include "../domains/candidaturas/repositories/CandidaturaRepository.hpp"
#include "../domains/candidaturas/use_cases/CreateCandidaturaUseCase.hpp"
#include "../domains/candidaturas/use_cases/AcceptCandidaturaUseCase.hpp"
#include "../domains/candidaturas/use_cases/RejectCandidaturaUseCase.hpp"
#include "../domains/candidaturas/controllers/CandidaturaController.hpp"

#include <iostream>
#include <memory>
#include <csignal>
#include <atomic>

using namespace Core::Http;
using namespace Core::Database;
using namespace Core::Utils;
// Removed: using namespace Domains::Identity;

// ==================== GLOBALS ====================

std::unique_ptr<Server> globalServer;
std::shared_ptr<ConnectionPool> globalPool;
std::shared_ptr<Core::Utils::MetricsCollector> globalMetricsCollector;
std::shared_ptr<Core::Cache::RedisPool> globalRedisPool;
std::shared_ptr<Core::Cache::CacheService> globalCacheService;
std::shared_ptr<Core::WebSocket::WebSocketManager> globalWsManager;
std::shared_ptr<Core::WebSocket::WebSocketHandler> globalWsHandler;
std::shared_ptr<Core::Queue::RedisMessageQueue> globalMessageQueue;
std::shared_ptr<Core::Queue::QueueWorker> globalEmailWorker;
std::shared_ptr<Core::Cache::RedisPubSub> globalRedisPubSub;

// ==================== GLOBAL METRICS FUNCTIONS ====================
namespace GlobalMetrics {

void recordOrderCreated(const std::string& tenantId, double totalAmount) {
    if (globalMetricsCollector) {
        globalMetricsCollector->incrementCounter("orders_created_total", 
            {{"tenant", tenantId}});
        globalMetricsCollector->incrementCounter("orders_revenue_total", 
            {{"tenant", tenantId}}, totalAmount);
    }
}

void recordPaymentProcessed(const std::string& provider, const std::string& status,
                            double amount, const std::string& currency) {
    if (globalMetricsCollector) {
        globalMetricsCollector->incrementCounter("payments_processed_total", 
            {{"provider", provider}, {"status", status}});
        globalMetricsCollector->incrementCounter("payments_amount_total", 
            {{"provider", provider}, {"currency", currency}}, amount);
    }
}

void recordSubscriptionCreated(const std::string& planId, const std::string& tenantId) {
    if (globalMetricsCollector) {
        globalMetricsCollector->incrementCounter("subscriptions_created_total", 
            {{"plan", planId}, {"tenant", tenantId}});
    }
}

void recordEmailSent(const std::string& type) {
    if (globalMetricsCollector) {
        globalMetricsCollector->incrementCounter("emails_sent_total", 
            {{"type", type}});
    }
}

void recordTaskCompleted(const std::string& userId) {
    if (globalMetricsCollector) {
        globalMetricsCollector->incrementCounter("tasks_completed_total", 
            {{"user", userId}});
    }
}

} // namespace GlobalMetrics

/**
 * Signal handler para Ctrl+C
 */
void signalHandler(int signal) {
    std::cout << "\n🛑 Recebido sinal " << signal << ", parando servidor..." << std::endl;
    
    if (globalEmailWorker) {
        std::cout << "Stopping email worker..." << std::endl;
        globalEmailWorker->stop();
    }
    
    if (globalRedisPubSub) {
        std::cout << "Stopping Redis Pub/Sub..." << std::endl;
        globalRedisPubSub->stop();
    }
    
    if (globalServer) {
        globalServer->stop();
    }
    
    if (globalPool) {
        globalPool->shutdown();
    }
    
    LoggerNew::shutdown();
    exit(0);
}

/**
 * Configurar rotas da aplicação
 */
void setupRoutes(Router& router, ConnectionPool& pool, 
                 std::shared_ptr<Finance::FinanceController> financeController,
                 std::shared_ptr<Domains::Restaurantes::Controllers::RestauranteController> restauranteController,
                 std::shared_ptr<Domains::Freelances::Controllers::FreelancerController> freelancerController,
                 std::shared_ptr<Domains::Vagas::Controllers::VagaController> vagaController,
                 std::shared_ptr<Domains::Candidaturas::Controllers::CandidaturaController> candidaturaController) {
    
    Utils::Logger::info("[setupRoutes] INICIO DA FUNCAO!");
    Utils::Logger::info("[setupRoutes] Router endereco recebido: " + std::to_string(reinterpret_cast<uintptr_t>(&router)));
    Utils::Logger::info("[setupRoutes] financeController pointer received: " + std::string(financeController ? "valid" : "NULL"));
    
    // ==================== HEALTH CHECK ====================
    router.get("/health", [](const Request& req) {
        (void)req;
        return Response(StatusCode::OK).text("OK - Server is running!");
    });
    
    // ==================== ROOT ====================
    router.get("/", [](const Request& req) {
        (void)req;
        std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>C++ Backend v2.0</title>
    <style>
        body {
            font-family: 'Segoe UI', Arial, sans-serif;
            max-width: 900px;
            margin: 50px auto;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        .container {
            background: rgba(255,255,255,0.1);
            padding: 30px;
            border-radius: 10px;
            backdrop-filter: blur(10px);
        }
        h1 { font-size: 3em; margin: 0; }
        h2 { color: #ffd700; }
        .endpoint {
            background: rgba(0,0,0,0.2);
            padding: 15px;
            margin: 10px 0;
            border-radius: 5px;
            border-left: 4px solid #ffd700;
        }
        code {
            background: rgba(0,0,0,0.3);
            padding: 2px 8px;
            border-radius: 4px;
        }
        .new { color: #00ff88; font-weight: bold; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 C++ Backend v2.0</h1>
        <p><strong>100% C++ Puro - Agora com ConnectionPool e Middlewares!</strong></p>
        
        <h2>🆕 Novidades v2.0</h2>
        <ul>
            <li class="new">✅ Connection Pool (reutilização de conexões DB)</li>
            <li class="new">✅ Middleware System (pipeline de requisições)</li>
            <li class="new">✅ Logger melhorado (cores, debug mode, file output)</li>
            <li class="new">✅ CORS automático</li>
            <li class="new">✅ Request timing</li>
            <li class="new">✅ Error handling</li>
        </ul>
        
        <h2>📡 Endpoints</h2>
        
        <div class="endpoint">
            <strong>GET /health</strong><br>
            Health check simples
        </div>
        
        <div class="endpoint">
            <strong>GET /api/pool/stats</strong> <span class="new">NOVO</span><br>
            Estatísticas do connection pool
        </div>
        
        <div class="endpoint">
            <strong>GET /api/db/test</strong><br>
            Testa conexão com PostgreSQL (agora usando pool!)
        </div>
        
        <div class="endpoint">
            <strong>GET /api/db/query</strong> <span class="new">NOVO</span><br>
            Executa query de exemplo (múltiplas conexões)
        </div>
        
        <div class="endpoint">
            <strong>GET /api/stress/:connections</strong> <span class="new">NOVO</span><br>
            Teste de stress do pool (adquire N conexões simultâneas)
        </div>
        
        <h2>🧪 Experimente</h2>
        <p>Use curl ou seu navegador:</p>
        <code>curl http://localhost:8080/api/pool/stats</code><br>
        <code>curl http://localhost:8080/api/stress/5</code>
    </div>
</body>
</html>
)";
        return Response(StatusCode::OK).html(html);
    });

    // ==================== SEALED BIDS (LEILÕES & OFFERS) ====================
    router.post("/leiloes", [](const Request& req) { return Domains::Leiloes::Controllers::LeilaoController::create(req); });
    router.get("/leiloes", [](const Request& req) { return Domains::Leiloes::Controllers::LeilaoController::list(req); });
    router.get("/leiloes/:id", [](const Request& req) { return Domains::Leiloes::Controllers::LeilaoController::getById(req); });
    router.put("/leiloes/:id/fechar", [](const Request& req) { return Domains::Leiloes::Controllers::LeilaoController::close(req); });
    router.post("/offers", [](const Request& req) { return Domains::Offers::Controllers::OfferController::create(req); });
    router.get("/leiloes/:id/offers", [](const Request& req) { return Domains::Offers::Controllers::OfferController::listForLeilao(req); });
    router.put("/offers/:id/shortlist", [](const Request& req) { return Domains::Offers::Controllers::OfferController::setShortlist(req); });
    router.put("/offers/:id/aceitar", [](const Request& req) { return Domains::Offers::Controllers::OfferController::accept(req); });
    
    

    auto userRepository = std::make_shared<Domains::UserManagement::Repositories::UserRepository>(globalPool);
    auto tenantRepository = std::make_shared<Domains::TenantManagement::Repositories::TenantRepository>(globalPool);
    
    // Workspace Repository e Controller
    auto workspaceRepository = Domain::Workspace::createWorkspaceRepository(globalPool);
    auto workspaceController = std::make_shared<Domain::Workspace::WorkspaceController>(workspaceRepository);

    // Product Repository e Controller
    auto productRepository = Domain::Product::createProductRepository(globalPool);
    auto productController = std::make_shared<Domain::Product::ProductController>(productRepository);
    
    // Inventory Repository e Controller
    auto inventoryRepository = Domain::Inventory::createInventoryRepository(globalPool);
    auto inventoryController = std::make_shared<Domain::Inventory::InventoryController>(inventoryRepository);
    
    // Customer Repository e Controller
    auto customerRepository = Domain::Customer::createCustomerRepository(globalPool);
    auto customerController = std::make_shared<Domain::Customer::CustomerController>(customerRepository);
    
    // Category Repository e Controller
    auto categoryRepository = Domain::Category::createCategoryRepository(globalPool);
    auto categoryController = std::make_shared<Domain::Category::CategoryController>(categoryRepository);
    
    // Order Repository e Controller
    auto orderRepository = Domain::Order::createOrderRepository(globalPool);
    auto orderController = std::make_shared<Domain::Order::OrderController>(orderRepository);
    
    // Payment Repository, Use Cases e Controller (with OrderRepository integration)
    auto paymentRepository = std::make_shared<Domain::Payment::PaymentRepositoryImpl>(globalPool);
    auto createPaymentUseCase = std::make_shared<Domain::Payment::CreatePaymentUseCase>(paymentRepository);
    auto processPaymentUseCase = std::make_shared<Domain::Payment::ProcessPaymentUseCase>(paymentRepository, orderRepository);
    auto refundPaymentUseCase = std::make_shared<Domain::Payment::RefundPaymentUseCase>(paymentRepository);
    auto cancelPaymentUseCase = std::make_shared<Domain::Payment::CancelPaymentUseCase>(paymentRepository);
    auto getPaymentUseCase = std::make_shared<Domain::Payment::GetPaymentUseCase>(paymentRepository);
    auto listPaymentsUseCase = std::make_shared<Domain::Payment::ListPaymentsUseCase>(paymentRepository);
    auto paymentController = std::make_shared<Domain::Payment::PaymentController>(
        createPaymentUseCase, processPaymentUseCase, refundPaymentUseCase,
        cancelPaymentUseCase, getPaymentUseCase, listPaymentsUseCase, paymentRepository
    );

    // Subscription Module (Plans + Subscriptions + Billing)
    auto planRepository = std::make_shared<Domain::Subscription::PlanRepositoryImpl>(globalPool);
    auto subscriptionRepository = std::make_shared<Domain::Subscription::SubscriptionRepositoryImpl>(globalPool);
    auto billingHistoryRepository = std::make_shared<Domain::Subscription::BillingHistoryRepositoryImpl>(globalPool);
    auto subscriptionController = std::make_shared<Domain::Subscription::SubscriptionController>(
        planRepository, subscriptionRepository, billingHistoryRepository, paymentRepository
    );

    // Invoicing Module (Invoices + Invoice Items)
    auto invoiceRepository = std::make_shared<Domain::Invoicing::InvoiceRepositoryImpl>(globalPool);
    auto createInvoiceUseCase = std::make_shared<Domain::Invoicing::CreateInvoiceUseCase>(invoiceRepository);
    auto getInvoiceUseCase = std::make_shared<Domain::Invoicing::GetInvoiceUseCase>(invoiceRepository);
    auto updateInvoiceStatusUseCase = std::make_shared<Domain::Invoicing::UpdateInvoiceStatusUseCase>(invoiceRepository);
    auto applyPaymentUseCase = std::make_shared<Domain::Invoicing::ApplyPaymentUseCase>(invoiceRepository);
    auto invoiceController = std::make_shared<Domain::Invoicing::InvoiceController>(
        createInvoiceUseCase, getInvoiceUseCase, updateInvoiceStatusUseCase, 
        applyPaymentUseCase, invoiceRepository
    );

    // Task Repository e Controller
    auto taskRepository = std::make_shared<Domain::Task::TaskRepositoryImpl>(globalPool);
    auto taskController = std::make_shared<Domain::Task::TaskController>(taskRepository);

    // Database/Admin Use Case e Controller
    auto executeQueryUseCase = std::make_shared<Domain::Database::ExecuteQueryUseCase>(globalPool);
    auto databaseController = std::make_shared<Domain::Database::DatabaseController>(executeQueryUseCase, globalPool);

    // Calendar Repository e Controller
    auto eventRepository = std::make_shared<Domain::Calendar::EventRepositoryImpl>(globalPool);
    auto eventController = std::make_shared<Domain::Calendar::EventController>(eventRepository);

// 1.5. Criar Services (JWT e Bcrypt)
auto bcryptService = std::make_shared<Domains::IAM::Services::BcryptService>(12); // cost factor 12
auto jwtService = std::make_shared<Domains::IAM::Services::JwtService>(
    "your-super-secret-jwt-key-change-this-in-production",  // Secret key
    60  // Expiration: 60 minutes
);

// 1.6. Criar Audit Log Repository (ANTES dos Use Cases)
auto auditLogRepository = std::make_shared<Domains::Audit::Repositories::AuditLogRepository>(globalPool);

// 2. Criar Use Cases
auto registerUseCase = std::make_shared<Domains::IAM::UseCases::RegisterUserUseCase>(
    userRepository,
    bcryptService,
    jwtService,
    auditLogRepository,
    tenantRepository  // ✅ Adicionado para suporte a multitenancy
);
auto loginUseCase = std::make_shared<Domains::IAM::UseCases::LoginUserUseCase>(
    userRepository,
    tenantRepository,
    bcryptService,
    jwtService,
    auditLogRepository
);
auto getUserUseCase = std::make_shared<Domains::UserManagement::UseCases::GetUserUseCase>(userRepository);

auto updateUserUseCase = std::make_shared<Domains::UserManagement::UseCases::UpdateUserUseCase>(userRepository);
auto changePasswordUseCase = std::make_shared<Domains::UserManagement::UseCases::ChangePasswordUseCase>(
    userRepository,
    bcryptService,
    auditLogRepository
);
auto deleteUserUseCase = std::make_shared<Domains::UserManagement::UseCases::DeleteUserUseCase>(userRepository);
auto listUsersUseCase = std::make_shared<Domains::UserManagement::UseCases::ListUsersUseCase>(userRepository);

// Tenant Management Use Cases
auto listTenantMembersUseCase = std::make_shared<Domains::TenantManagement::UseCases::ListTenantMembersUseCase>(tenantRepository);
auto updateUserRoleUseCase = std::make_shared<Domains::TenantManagement::UseCases::UpdateUserRoleUseCase>(tenantRepository);
auto removeUserFromTenantUseCase = std::make_shared<Domains::TenantManagement::UseCases::RemoveUserFromTenantUseCase>(tenantRepository);
auto addUserToTenantUseCase = std::make_shared<Domains::TenantManagement::UseCases::AddUserToTenantUseCase>(tenantRepository, userRepository);

// Refresh Token Use Case
auto refreshTokenUseCase = std::make_shared<Domains::IAM::UseCases::RefreshTokenUseCase>(jwtService, userRepository, tenantRepository);

// Email Verification & Password Reset
auto emailService = std::make_shared<Domains::Notification::Services::MockEmailService>();
auto emailVerificationRepository = std::make_shared<Domains::Notification::Repositories::EmailVerificationRepository>(globalPool);
auto passwordResetRepository = std::make_shared<Domains::Notification::Repositories::PasswordResetRepository>(globalPool);

auto sendVerificationEmailUseCase = std::make_shared<Domains::Notification::UseCases::SendVerificationEmailUseCase>(
    emailVerificationRepository,
    userRepository,
    emailService
);

auto verifyEmailUseCase = std::make_shared<Domains::IAM::UseCases::VerifyEmailUseCase>(
    emailVerificationRepository,
    userRepository,
    emailService
);

auto requestPasswordResetUseCase = std::make_shared<Domains::Notification::UseCases::RequestPasswordResetUseCase>(
    passwordResetRepository,
    userRepository,
    emailService
);

auto resetPasswordUseCase = std::make_shared<Domains::Notification::UseCases::ResetPasswordUseCase>(
    passwordResetRepository,
    userRepository,
    bcryptService
);

  // 3. Criar Controllers
  auto authController = std::make_shared<Domains::IAM::Controllers::AuthController>(
      registerUseCase,
      loginUseCase
  );

  auto usersController = std::make_shared<Domains::UserManagement::Controllers::UsersController>(
      getUserUseCase,
      updateUserUseCase,
      changePasswordUseCase,
      deleteUserUseCase,
      listUsersUseCase
  );

// 4. Configurar Rate Limiting
// Login: 1000 tentativas por hora (MODO DESENVOLVIMENTO - em prod use 5/60)
auto rateLimitLogin = createRateLimitMiddleware(1000, 3600);

// Register: 1000 tentativas por hora (MODO DESENVOLVIMENTO - em prod use 3/3600)
auto rateLimitRegister = createRateLimitMiddleware(1000, 3600);

// Helper para aplicar rate limit a um handler
auto withRateLimit = [](MiddlewareFunction rateLimit, std::function<Response(const Request&)> handler) {
    return [rateLimit, handler](const Request& req) -> Response {
        Response res;
        bool rateLimitPassed = false;
        
        // Copiar request para poder modificar
        Request mutableReq = req;
        
        // Executar middleware de rate limit
        rateLimit(mutableReq, res, [&rateLimitPassed]() {
            rateLimitPassed = true;
        });
        
        // Se rate limit bloqueou, retornar resposta do middleware
        if (!rateLimitPassed) {
            return res;
        }
        
        // Rate limit OK, executar handler
        return handler(mutableReq);
    };
};

// 5. Registrar rotas com Rate Limiting
router.post("/api/auth/register", withRateLimit(rateLimitRegister, [authController](const Request& req) {
    return authController->register_(req);
}));

router.post("/api/auth/login", withRateLimit(rateLimitLogin, [authController](const Request& req) {
    return authController->login(req);
}));

// POST /api/auth/refresh - Renovar token
router.post("/api/auth/refresh", [refreshTokenUseCase](const Request& req) {
    // Parse body
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("refreshToken")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("refreshToken is required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    Domains::IAM::UseCases::RefreshTokenDto dto;
    dto.refreshToken = obj["refreshToken"]->asString();
    
    try {
        auto result = refreshTokenUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("Token refreshed successfully");
        json->asObject()["token"] = Core::Json::makeString(result.accessToken);
        json->asObject()["refreshToken"] = Core::Json::makeString(result.refreshToken);
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Unauthorized).json(*error);
    }
});

// ==================== EMAIL VERIFICATION ====================

// POST /api/auth/send-verification - Envia email de verificação
router.post("/api/auth/send-verification", [sendVerificationEmailUseCase](const Request& req) {
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("userId")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("userId is required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    Domains::Notification::UseCases::SendVerificationEmailDto dto;
    dto.userId = obj["userId"]->asString();
    
    try {
        sendVerificationEmailUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("Verification email sent successfully");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
});

// POST /api/auth/verify-email - Verifica email com token
router.post("/api/auth/verify-email", [verifyEmailUseCase](const Request& req) {
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("token")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("token is required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    Domains::IAM::UseCases::VerifyEmailDto dto;
    dto.token = obj["token"]->asString();
    
    try {
        verifyEmailUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("Email verified successfully!");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
});

// ==================== PASSWORD RESET ====================

// POST /api/auth/forgot-password - Solicita reset de senha
router.post("/api/auth/forgot-password", [requestPasswordResetUseCase](const Request& req) {
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("email")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("email is required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    Domains::Notification::UseCases::RequestPasswordResetDto dto;
    dto.email = obj["email"]->asString();
    dto.ipAddress = "127.0.0.1"; // TODO: Extract from request
    dto.userAgent = req.getHeader("User-Agent");
    
    try {
        requestPasswordResetUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("If the email exists, a password reset link has been sent");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
});

// POST /api/auth/reset-password - Reseta senha com token
router.post("/api/auth/reset-password", [resetPasswordUseCase](const Request& req) {
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("token") || !obj.count("newPassword")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("token and newPassword are required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    Domains::Notification::UseCases::ResetPasswordDto dto;
    dto.token = obj["token"]->asString();
    dto.newPassword = obj["newPassword"]->asString();
    
    try {
        resetPasswordUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("Password reset successfully");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
});

// ==================== ROTAS PROTEGIDAS ====================
// Criar AuthMiddleware
auto authMiddleware = createAuthMiddleware(jwtService);

// Helper para aplicar middleware a um handler
auto withAuth = [authMiddleware](std::function<Response(const Request&)> handler) {
    return [authMiddleware, handler](const Request& req) -> Response {
        Response res;
        bool authPassed = false;
        
        // Copiar request para poder modificar
        Request mutableReq = req;
        
        // Executar middleware
        authMiddleware(mutableReq, res, [&authPassed]() {
            authPassed = true;
        });
        
        // Se autenticação falhou, retornar resposta do middleware
        if (!authPassed) {
            return res;
        }
        
        // Autenticação OK, executar handler
        return handler(mutableReq);
    };
};

// Helper para aplicar role middleware a um handler
auto withRole = [](const std::vector<std::string>& allowedRoles, std::function<Response(const Request&)> handler) {
    auto roleMiddleware = createRoleMiddleware(allowedRoles);
    return [roleMiddleware, handler](const Request& req) -> Response {
        Response res;
        bool rolePassed = false;
        
        // Copiar request para poder modificar
        Request mutableReq = req;
        
        // Executar middleware de role
        roleMiddleware(mutableReq, res, [&rolePassed]() {
            rolePassed = true;
        });
        
        // Se verificação de role falhou, retornar resposta do middleware
        if (!rolePassed) {
            return res;
        }
        
        // Role OK, executar handler
        return handler(mutableReq);
    };
};

// Helper combinado: Auth + Role
auto withAuthAndRole = [&withAuth, &withRole](const std::vector<std::string>& allowedRoles, std::function<Response(const Request&)> handler) {
    return withAuth(withRole(allowedRoles, handler));
};

// GET /api/me - Retorna dados do usuário logado
router.get("/api/me", withAuth([usersController](const Request& req) {
    return usersController->me(req);
}));

// GET /api/users/:id - Busca usuário (protegida)
router.get("/api/users/:id", withAuth([usersController](const Request& req) {
    return usersController->getById(req);
}));

// PUT /api/users/:id - Atualizar usuário
router.put("/api/users/:id", withAuth([usersController](const Request& req) {
    return usersController->update(req);
}));

// PUT /api/users/:id/password - Trocar senha (formato snake_case)
router.put("/api/users/:id/password", withAuth([usersController](const Request& req) {
    return usersController->changePassword(req);
}));

// POST /api/users/:id/change-password - Trocar senha (formato camelCase - deprecated)
router.post("/api/users/:id/change-password", withAuth([usersController](const Request& req) {
    return usersController->changePasswordDeprecated(req);
}));

// DELETE /api/users/:id - Deletar usuário (soft delete)
router.del("/api/users/:id", withAuth([usersController](const Request& req) {
    return usersController->delete_(req);
}));

// GET /api/users - Listar usuários (apenas admins)
router.get("/api/users", withAuthAndRole({"admin", "owner"}, [usersController](const Request& req) {
    return usersController->list(req);
}));

// ==================== TENANT MANAGEMENT ====================

// GET /api/tenants/:id/members - Listar membros do tenant
router.get("/api/tenants/:tenantId/members", withAuthAndRole({"super_admin"}, [listTenantMembersUseCase](const Request& req) {
    std::string tenantId = req.getParam("tenantId");
    std::string loggedUserId = req.getCustomData("user_id");
    std::string loggedUserRoleStr = req.getCustomData("user_role");
    
    try {
        Domains::TenantManagement::ValueObjects::TenantUserRole loggedUserRole(loggedUserRoleStr);
        
        Domains::TenantManagement::UseCases::ListTenantMembersDto dto;
        dto.tenantId = tenantId;
        dto.requestingUserId = loggedUserId;
        dto.requestingUserRole = loggedUserRole;
        
        auto members = listTenantMembersUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["total"] = Core::Json::makeNumber(members.size());
        
        auto membersArray = Core::Json::makeArray();
        for (const auto& member : members) {
            auto memberObj = Core::Json::makeObject();
            memberObj->asObject()["userId"] = Core::Json::makeString(member.userId);
            memberObj->asObject()["email"] = Core::Json::makeString(member.email);
            memberObj->asObject()["name"] = Core::Json::makeString(member.name);
            memberObj->asObject()["role"] = Core::Json::makeString(member.role.toString());
            memberObj->asObject()["isActive"] = Core::Json::makeBool(member.isActive);
            membersArray->asArray().push_back(memberObj);
        }
        json->asObject()["members"] = membersArray;
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::invalid_argument& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid role: " + std::string(e.what()));
        return Response(StatusCode::BadRequest).json(*error);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Forbidden).json(*error);
    }
}));

// POST /api/tenants/:id/users - Adicionar usuário ao tenant
router.post("/api/tenants/:tenantId/users", withAuthAndRole({"super_admin"}, [addUserToTenantUseCase](const Request& req) {
    std::string tenantId = req.getParam("tenantId");
    std::string loggedUserId = req.getCustomData("user_id");
    std::string loggedUserRoleStr = req.getCustomData("user_role");
    
    // Parse body
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("userId") || !obj.count("role")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("userId and role are required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    try {
        Domains::TenantManagement::ValueObjects::TenantUserRole loggedUserRole(loggedUserRoleStr);
        Domains::TenantManagement::ValueObjects::TenantUserRole role(obj["role"]->asString());
        
        Domains::TenantManagement::UseCases::AddUserToTenantDto dto;
        dto.tenantId = tenantId;
        dto.userId = obj["userId"]->asString();
        dto.role = role;
        dto.requestingUserId = loggedUserId;
        dto.requestingUserRole = loggedUserRole;
        
        addUserToTenantUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("User added to tenant successfully");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::invalid_argument& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid role: " + std::string(e.what()));
        return Response(StatusCode::BadRequest).json(*error);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
}));

// PUT /api/tenants/:id/users/:userId/role - Atualizar role do usuário
router.put("/api/tenants/:tenantId/users/:userId/role", withAuthAndRole({"super_admin"}, [updateUserRoleUseCase](const Request& req) {
    std::string tenantId = req.getParam("tenantId");
    std::string userId = req.getParam("userId");
    std::string loggedUserId = req.getCustomData("user_id");
    std::string loggedUserRoleStr = req.getCustomData("user_role");
    
    // Parse body
    auto jsonValue = req.getJson();
    if (!jsonValue || !jsonValue->isObject()) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    auto obj = jsonValue->asObject();
    
    if (!obj.count("role")) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("role is required");
        return Response(StatusCode::BadRequest).json(*error);
    }
    
    try {
        Domains::TenantManagement::ValueObjects::TenantUserRole loggedUserRole(loggedUserRoleStr);
        Domains::TenantManagement::ValueObjects::TenantUserRole newRole(obj["role"]->asString());
        
        Domains::TenantManagement::UseCases::UpdateUserRoleDto dto;
        dto.tenantId = tenantId;
        dto.userId = userId;
        dto.newRole = newRole;
        dto.requestingUserId = loggedUserId;
        dto.requestingUserRole = loggedUserRole;
        
        updateUserRoleUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("User role updated successfully");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::invalid_argument& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString("Invalid role: " + std::string(e.what()));
        return Response(StatusCode::BadRequest).json(*error);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::BadRequest).json(*error);
    }
}));

// DELETE /api/tenants/:id/users/:userId - Remover usuário do tenant
router.del("/api/tenants/:tenantId/users/:userId", withAuthAndRole({"super_admin"}, [removeUserFromTenantUseCase](const Request& req) {
    std::string tenantId = req.getParam("tenantId");
    std::string userId = req.getParam("userId");
    std::string loggedUserId = req.getCustomData("user_id");
    std::string loggedUserRoleStr = req.getCustomData("user_role");
    
    try {
        Domains::TenantManagement::ValueObjects::TenantUserRole loggedUserRole(loggedUserRoleStr);
        
        Domains::TenantManagement::UseCases::RemoveUserFromTenantDto dto;
        dto.tenantId = tenantId;
        dto.userId = userId;
        dto.requestingUserId = loggedUserId;
        dto.requestingUserRole = loggedUserRole;
        
        removeUserFromTenantUseCase->execute(dto);
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["message"] = Core::Json::makeString("User removed from tenant successfully");
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::Forbidden).json(*error);
    }
}));

// ==================== AUDIT LOGS ====================

// GET /api/admin/audit-logs - Consultar logs de auditoria (admin only)
router.get("/api/admin/audit-logs", withAuthAndRole({"admin", "owner"}, [&](const Request& req) {
    std::string loggedUserId = req.getCustomData("user_id");
    std::string loggedUserRole = req.getCustomData("user_role");
    std::string tenantId = req.getCustomData("user_tenant_id");
    
    // TODO: Parse query params (user_id, tenant_id, action, limit, offset)
    // Por agora, buscar por tenant
    int limit = 50;
    int offset = 0;
    
    try {
        std::vector<Domains::Audit::Entities::AuditLog> logs;
        
        // Se tem tenant_id, buscar por tenant
        if (!tenantId.empty()) {
            logs = auditLogRepository->findByTenant(tenantId, limit, offset);
        } else {
            // Se não tem tenant, retornar vazio por segurança
            logs = {};
        }
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["total"] = Core::Json::makeNumber(logs.size());
        json->asObject()["limit"] = Core::Json::makeNumber(limit);
        json->asObject()["offset"] = Core::Json::makeNumber(offset);
        
        auto logsArray = Core::Json::makeArray();
        for (const auto& log : logs) {
            auto logObj = Core::Json::makeObject();
            logObj->asObject()["id"] = Core::Json::makeString(log.getId());
            
            if (log.getUserId().has_value()) {
                logObj->asObject()["user_id"] = Core::Json::makeString(log.getUserId().value());
            }
            
            if (log.getTenantId().has_value()) {
                logObj->asObject()["tenant_id"] = Core::Json::makeString(log.getTenantId().value());
            }
            
            logObj->asObject()["action"] = Core::Json::makeString(log.getAction());
            logObj->asObject()["resource"] = Core::Json::makeString(log.getResource());
            
            if (log.getResourceId().has_value()) {
                logObj->asObject()["resource_id"] = Core::Json::makeString(log.getResourceId().value());
            }
            
            logObj->asObject()["details"] = Core::Json::makeString(log.getDetails());
            logObj->asObject()["ip_address"] = Core::Json::makeString(log.getIpAddress());
            logObj->asObject()["user_agent"] = Core::Json::makeString(log.getUserAgent());
            
            logsArray->asArray().push_back(logObj);
        }
        json->asObject()["logs"] = logsArray;
        
        return Response(StatusCode::OK).json(*json);
    } catch (const std::exception& e) {
        auto error = Core::Json::makeObject();
        error->asObject()["status"] = Core::Json::makeString("error");
        error->asObject()["message"] = Core::Json::makeString(e.what());
        return Response(StatusCode::InternalServerError).json(*error);
    }
}));

    // ==================== WORKSPACE MANAGEMENT ====================
    
    // GET /api/workspaces - Listar workspaces (DEVE VIR ANTES DO /:id)
    router.get("/api/workspaces", withAuth(std::function<Response(const Request&)>([workspaceController](const Request& req) {
        return workspaceController->listWorkspaces(req);
    })));
    
    // POST /api/workspaces - Criar workspace
    router.post("/api/workspaces", withAuth(std::function<Response(const Request&)>([workspaceController](const Request& req) {
        return workspaceController->createWorkspace(req);
    })));
    
    // GET /api/workspaces/:id - Buscar workspace (DEPOIS DA ROTA SEM PARÂMETRO)
    router.get("/api/workspaces/:id", withAuth(std::function<Response(const Request&)>([workspaceController](const Request& req) {
        return workspaceController->getWorkspace(req);
    })));
    
    // PUT /api/workspaces/:id - Atualizar workspace
    router.put("/api/workspaces/:id", withAuth(std::function<Response(const Request&)>([workspaceController](const Request& req) {
        return workspaceController->updateWorkspace(req);
    })));
    
    // DELETE /api/workspaces/:id - Deletar workspace
    router.del("/api/workspaces/:id", withAuth(std::function<Response(const Request&)>([workspaceController](const Request& req) {
        return workspaceController->deleteWorkspace(req);
    })));
    
    // POST /api/workspaces/:id/members - Convidar membro
    router.post("/api/workspaces/:id/members", withAuth(std::function<Response(const Request&)>([workspaceController](const Request& req) {
        return workspaceController->inviteMember(req);
    })));
    
    // GET /api/workspaces/:id/members - Listar membros
    router.get("/api/workspaces/:id/members", withAuth(std::function<Response(const Request&)>([workspaceController](const Request& req) {
        return workspaceController->listMembers(req);
    })));
    
    // DELETE /api/workspaces/:id/members/:userId - Remover membro
    router.del("/api/workspaces/:id/members/:userId", withAuth(std::function<Response(const Request&)>([workspaceController](const Request& req) {
        return workspaceController->removeMember(req);
    })));
    
    // PUT /api/workspaces/:id/members/:userId/role - Atualizar role
    router.put("/api/workspaces/:id/members/:userId/role", withAuth(std::function<Response(const Request&)>([workspaceController](const Request& req) {
        return workspaceController->updateMemberRole(req);
    })));

    // ==================== PRODUCT MANAGEMENT ====================
    
    // GET /api/products/search?q=query - Buscar produtos (DEVE VIR ANTES DO /:id)
    router.get("/api/products/search", withAuth(std::function<Response(const Request&)>([productController](const Request& req) {
        return productController->searchProducts(req);
    })));
    
    // GET /api/products/low-stock - Listar produtos com estoque baixo (DEVE VIR ANTES DO /:id)
    router.get("/api/products/low-stock", withAuth(std::function<Response(const Request&)>([productController](const Request& req) {
        return productController->listLowStock(req);
    })));
    
    // GET /api/products - Listar produtos (DEVE VIR ANTES DO /:id)
    router.get("/api/products", withAuth(std::function<Response(const Request&)>([productController](const Request& req) {
        return productController->listProducts(req);
    })));
    
    // POST /api/products - Criar produto
    router.post("/api/products", withAuth(std::function<Response(const Request&)>([productController](const Request& req) {
        return productController->createProduct(req);
    })));
    
    // GET /api/products/:id - Buscar produto (DEPOIS DAS ROTAS SEM PARÂMETRO)
    router.get("/api/products/:id", withAuth(std::function<Response(const Request&)>([productController](const Request& req) {
        return productController->getProduct(req);
    })));
    
    // PUT /api/products/:id - Atualizar produto
    router.put("/api/products/:id", withAuth(std::function<Response(const Request&)>([productController](const Request& req) {
        return productController->updateProduct(req);
    })));
    
    // DELETE /api/products/:id - Deletar produto (soft delete por padrão)
    router.del("/api/products/:id", withAuth(std::function<Response(const Request&)>([productController](const Request& req) {
        return productController->deleteProduct(req);
    })));

    // ==================== INVENTORY MANAGEMENT ====================
    
    // POST /api/inventory/transactions - Registrar transação de inventário
    router.post("/api/inventory/transactions", withAuth(std::function<Response(const Request&)>([inventoryController](const Request& req) {
        return inventoryController->recordTransaction(req);
    })));
    
    // GET /api/inventory/history?product_id=xxx&location_id=xxx - Histórico de transações
    router.get("/api/inventory/history", withAuth(std::function<Response(const Request&)>([inventoryController](const Request& req) {
        return inventoryController->getHistory(req);
    })));
    
    // GET /api/inventory/stock/:productId - Estoque atual de um produto
    router.get("/api/inventory/stock/:productId", withAuth(std::function<Response(const Request&)>([inventoryController](const Request& req) {
        return inventoryController->getCurrentStock(req);
    })));

    // ==================== CUSTOMER MANAGEMENT ====================
    
    // GET /api/customers/search?q=query - Buscar clientes (DEVE VIR ANTES DO /:id)
    router.get("/api/customers/search", withAuth(std::function<Response(const Request&)>([customerController](const Request& req) {
        return customerController->searchCustomers(req);
    })));
    
    // GET /api/customers - Listar clientes
    router.get("/api/customers", withAuth(std::function<Response(const Request&)>([customerController](const Request& req) {
        return customerController->listCustomers(req);
    })));
    
    // POST /api/customers - Criar cliente
    router.post("/api/customers", withAuth(std::function<Response(const Request&)>([customerController](const Request& req) {
        return customerController->createCustomer(req);
    })));
    
    // GET /api/customers/:id - Buscar cliente (DEPOIS DAS ROTAS SEM PARÂMETRO)
    router.get("/api/customers/:id", withAuth(std::function<Response(const Request&)>([customerController](const Request& req) {
        return customerController->getCustomer(req);
    })));
    
    // PUT /api/customers/:id - Atualizar cliente
    router.put("/api/customers/:id", withAuth(std::function<Response(const Request&)>([customerController](const Request& req) {
        return customerController->updateCustomer(req);
    })));
    
    // DELETE /api/customers/:id - Deletar cliente (soft delete)
    router.del("/api/customers/:id", withAuth(std::function<Response(const Request&)>([customerController](const Request& req) {
        return customerController->deleteCustomer(req);
    })));

    // ==================== CATEGORY MANAGEMENT ====================
    
    // GET /api/categories/root - Categorias raiz (DEVE VIR ANTES DO /:id)
    router.get("/api/categories/root", withAuth(std::function<Response(const Request&)>([categoryController](const Request& req) {
        return categoryController->getRootCategories(req);
    })));
    
    // GET /api/categories - Listar categorias
    router.get("/api/categories", withAuth(std::function<Response(const Request&)>([categoryController](const Request& req) {
        return categoryController->listCategories(req);
    })));
    
    // POST /api/categories - Criar categoria
    router.post("/api/categories", withAuth(std::function<Response(const Request&)>([categoryController](const Request& req) {
        return categoryController->createCategory(req);
    })));
    
    // GET /api/categories/:id/children - Buscar filhos de uma categoria
    router.get("/api/categories/:id/children", withAuth(std::function<Response(const Request&)>([categoryController](const Request& req) {
        return categoryController->getCategoryChildren(req);
    })));
    
    // GET /api/categories/:id - Buscar categoria (DEPOIS DAS ROTAS SEM PARÂMETRO)
    router.get("/api/categories/:id", withAuth(std::function<Response(const Request&)>([categoryController](const Request& req) {
        return categoryController->getCategory(req);
    })));
    
    // PUT /api/categories/:id - Atualizar categoria
    router.put("/api/categories/:id", withAuth(std::function<Response(const Request&)>([categoryController](const Request& req) {
        return categoryController->updateCategory(req);
    })));
    
    // DELETE /api/categories/:id - Deletar categoria (soft delete)
    router.del("/api/categories/:id", withAuth(std::function<Response(const Request&)>([categoryController](const Request& req) {
        return categoryController->deleteCategory(req);
    })));

    // ==================== ORDER MANAGEMENT ====================
    
    // GET /api/orders - Listar pedidos
    router.get("/api/orders", withAuth(std::function<Response(const Request&)>([orderController](const Request& req) {
        return orderController->listOrders(req);
    })));
    
    // POST /api/orders - Criar pedido
    router.post("/api/orders", withAuth(std::function<Response(const Request&)>([orderController](const Request& req) {
        return orderController->createOrder(req);
    })));
    
    // GET /api/customers/:customerId/orders - Listar pedidos do cliente
    router.get("/api/customers/:customerId/orders", withAuth(std::function<Response(const Request&)>([orderController](const Request& req) {
        return orderController->listCustomerOrders(req);
    })));
    
    // GET /api/orders/:id - Buscar pedido
    router.get("/api/orders/:id", withAuth(std::function<Response(const Request&)>([orderController](const Request& req) {
        return orderController->getOrder(req);
    })));
    
    // PUT /api/orders/:id/status - Atualizar status do pedido
    router.put("/api/orders/:id/status", withAuth(std::function<Response(const Request&)>([orderController](const Request& req) {
        return orderController->updateStatus(req);
    })));
    
    // PUT /api/orders/:id/payment - Atualizar status de pagamento
    router.put("/api/orders/:id/payment", withAuth(std::function<Response(const Request&)>([orderController](const Request& req) {
        return orderController->updatePaymentStatus(req);
    })));
    
    // POST /api/orders/:id/cancel - Cancelar pedido
    router.post("/api/orders/:id/cancel", withAuth(std::function<Response(const Request&)>([orderController](const Request& req) {
        return orderController->cancelOrder(req);
    })));

    // ==================== PAYMENT ROUTES ====================
    
    // POST /api/payments - Criar payment
    router.post("/api/payments", withAuth(std::function<Response(const Request&)>([paymentController](const Request& req) {
        return paymentController->createPayment(req);
    })));
    
    // GET /api/payments - Listar payments (filtros: order_id, customer_id, tenant_id)
    router.get("/api/payments", withAuth(std::function<Response(const Request&)>([paymentController](const Request& req) {
        return paymentController->listPayments(req);
    })));
    
    // GET /api/payments/:id - Buscar payment
    router.get("/api/payments/:id", withAuth(std::function<Response(const Request&)>([paymentController](const Request& req) {
        return paymentController->getPayment(req);
    })));
    
    // POST /api/payments/:id/process - Processar/aprovar payment
    router.post("/api/payments/:id/process", withAuth(std::function<Response(const Request&)>([paymentController](const Request& req) {
        return paymentController->processPayment(req);
    })));
    
    // POST /api/payments/:id/refund - Reembolsar payment
    router.post("/api/payments/:id/refund", withAuth(std::function<Response(const Request&)>([paymentController](const Request& req) {
        return paymentController->refundPayment(req);
    })));
    
    // POST /api/payments/:id/cancel - Cancelar payment
    router.post("/api/payments/:id/cancel", withAuth(std::function<Response(const Request&)>([paymentController](const Request& req) {
        return paymentController->cancelPayment(req);
    })));
    
    // GET /api/payments/:id/transactions - Listar transações do payment
    router.get("/api/payments/:id/transactions", withAuth(std::function<Response(const Request&)>([paymentController](const Request& req) {
        return paymentController->listTransactions(req);
    })));

    // ==================== SUBSCRIPTION ROUTES ====================
    
    // ===== PLANS =====
    // POST /api/subscription/plans - Criar plano
    router.post("/api/subscription/plans", withAuth(std::function<Response(const Request&)>([subscriptionController](const Request& req) {
        return subscriptionController->createPlan(req);
    })));
    
    // GET /api/subscription/plans - Listar planos (filtros: active, public)
    router.get("/api/subscription/plans", withAuth(std::function<Response(const Request&)>([subscriptionController](const Request& req) {
        return subscriptionController->listPlans(req);
    })));
    
    // GET /api/subscription/plans/:id - Buscar plano
    router.get("/api/subscription/plans/:id", withAuth(std::function<Response(const Request&)>([subscriptionController](const Request& req) {
        return subscriptionController->getPlan(req);
    })));
    
    // ===== SUBSCRIPTIONS =====
    // POST /api/subscription/subscriptions - Criar assinatura
    router.post("/api/subscription/subscriptions", withAuth(std::function<Response(const Request&)>([subscriptionController](const Request& req) {
        return subscriptionController->createSubscription(req);
    })));
    
    // GET /api/subscription/subscriptions - Listar assinaturas (filtros: customer_id, status)
    router.get("/api/subscription/subscriptions", withAuth(std::function<Response(const Request&)>([subscriptionController](const Request& req) {
        return subscriptionController->listSubscriptions(req);
    })));
    
    // GET /api/subscription/subscriptions/:id - Buscar assinatura
    router.get("/api/subscription/subscriptions/:id", withAuth(std::function<Response(const Request&)>([subscriptionController](const Request& req) {
        return subscriptionController->getSubscription(req);
    })));
    
    // POST /api/subscription/subscriptions/:id/cancel - Cancelar assinatura
    router.post("/api/subscription/subscriptions/:id/cancel", withAuth(std::function<Response(const Request&)>([subscriptionController](const Request& req) {
        return subscriptionController->cancelSubscription(req);
    })));
    
    // POST /api/subscription/subscriptions/:id/renew - Processar renovação
    router.post("/api/subscription/subscriptions/:id/renew", withAuth(std::function<Response(const Request&)>([subscriptionController](const Request& req) {
        return subscriptionController->processRenewal(req);
    })));
    
    // GET /api/subscription/subscriptions/:id/billing - Histórico de cobranças
    router.get("/api/subscription/subscriptions/:id/billing", withAuth(std::function<Response(const Request&)>([subscriptionController](const Request& req) {
        return subscriptionController->getBillingHistory(req);
    })));
    
    // GET /api/subscription/stats - Estatísticas de assinaturas
    router.get("/api/subscription/stats", withAuth(std::function<Response(const Request&)>([subscriptionController](const Request& req) {
        return subscriptionController->getSubscriptionStats(req);
    })));

    // ==================== INVOICING ROUTES ====================
    
    // POST /api/invoices - Criar fatura
    router.post("/api/invoices", withAuth(std::function<Response(const Request&)>([invoiceController](const Request& req) {
        return invoiceController->createInvoice(req);
    })));
    
    // GET /api/invoices - Listar faturas (filtros: status)
    router.get("/api/invoices", withAuth(std::function<Response(const Request&)>([invoiceController](const Request& req) {
        return invoiceController->listInvoices(req);
    })));
    
    // GET /api/invoices/:id - Buscar fatura
    router.get("/api/invoices/:id", withAuth(std::function<Response(const Request&)>([invoiceController](const Request& req) {
        return invoiceController->getInvoice(req);
    })));
    
    // PUT /api/invoices/:id/status - Atualizar status da fatura
    router.put("/api/invoices/:id/status", withAuth(std::function<Response(const Request&)>([invoiceController](const Request& req) {
        return invoiceController->updateInvoiceStatus(req);
    })));
    
    // POST /api/invoices/:id/payment - Aplicar pagamento à fatura
    router.post("/api/invoices/:id/payment", withAuth(std::function<Response(const Request&)>([invoiceController](const Request& req) {
        return invoiceController->applyPayment(req);
    })));
    
    // GET /api/invoices/:id/items - Listar itens da fatura
    router.get("/api/invoices/:id/items", withAuth(std::function<Response(const Request&)>([invoiceController](const Request& req) {
        return invoiceController->getInvoiceItems(req);
    })));
    
    // GET /api/customers/:customerId/invoices - Listar faturas de um cliente
    router.get("/api/customers/:customerId/invoices", withAuth(std::function<Response(const Request&)>([invoiceController](const Request& req) {
        return invoiceController->listCustomerInvoices(req);
    })));

    // ==================== TASK MANAGEMENT ====================
    
    // POST /api/tasks - Criar task
    router.post("/api/tasks", withAuth(std::function<Response(const Request&)>([taskController](const Request& req) {
        return taskController->createTask(req);
    })));
    
    // GET /api/tasks - Listar tasks (filtros: workspace_id, project_id, assigned_to, status, created_by)
    router.get("/api/tasks", withAuth(std::function<Response(const Request&)>([taskController](const Request& req) {
        return taskController->listTasks(req);
    })));
    
    // GET /api/tasks/:id - Buscar task por ID
    router.get("/api/tasks/:id", withAuth(std::function<Response(const Request&)>([taskController](const Request& req) {
        return taskController->getTask(req);
    })));
    
    // PUT /api/tasks/:id - Atualizar task
    router.put("/api/tasks/:id", withAuth(std::function<Response(const Request&)>([taskController](const Request& req) {
        return taskController->updateTask(req);
    })));
    
    // PUT /api/tasks/:id/status - Atualizar status da task
    router.put("/api/tasks/:id/status", withAuth(std::function<Response(const Request&)>([taskController](const Request& req) {
        return taskController->updateStatus(req);
    })));
    
    // DELETE /api/tasks/:id - Deletar task
    router.del("/api/tasks/:id", withAuth(std::function<Response(const Request&)>([taskController](const Request& req) {
        return taskController->deleteTask(req);
    })));
    
    // POST /api/tasks/:id/comments - Adicionar comentário
    router.post("/api/tasks/:id/comments", withAuth(std::function<Response(const Request&)>([taskController](const Request& req) {
        return taskController->addComment(req);
    })));
    
    // GET /api/tasks/:id/comments - Listar comentários
    router.get("/api/tasks/:id/comments", withAuth(std::function<Response(const Request&)>([taskController](const Request& req) {
        return taskController->getComments(req);
    })));
    
    // POST /api/tasks/:id/checklists - Criar checklist
    router.post("/api/tasks/:id/checklists", withAuth(std::function<Response(const Request&)>([taskController](const Request& req) {
        return taskController->addChecklist(req);
    })));
    
    // GET /api/tasks/:id/checklists - Listar checklists
    router.get("/api/tasks/:id/checklists", withAuth(std::function<Response(const Request&)>([taskController](const Request& req) {
        return taskController->getChecklists(req);
    })));

    // ==================== CALENDAR / EVENTS ====================
    
    // GET /api/events - Listar eventos (DEVE VIR ANTES DO /:id)
    router.get("/api/events", withAuth(std::function<Response(const Request&)>([eventController](const Request& req) {
        return eventController->listEvents(req);
    })));
    
    // POST /api/events - Criar evento
    router.post("/api/events", withAuth(std::function<Response(const Request&)>([eventController](const Request& req) {
        return eventController->createEvent(req);
    })));
    
    // GET /api/events/:id - Buscar evento (DEPOIS DA ROTA SEM PARÂMETRO)
    router.get("/api/events/:id", withAuth(std::function<Response(const Request&)>([eventController](const Request& req) {
        return eventController->getEvent(req);
    })));
    
    // PUT /api/events/:id - Atualizar evento
    router.put("/api/events/:id", withAuth(std::function<Response(const Request&)>([eventController](const Request& req) {
        return eventController->updateEvent(req);
    })));
    
    // DELETE /api/events/:id - Deletar evento
    router.del("/api/events/:id", withAuth(std::function<Response(const Request&)>([eventController](const Request& req) {
        return eventController->deleteEvent(req);
    })));
    
    // POST /api/events/:id/participants - Adicionar participante
    router.post("/api/events/:id/participants", withAuth(std::function<Response(const Request&)>([eventController](const Request& req) {
        return eventController->addParticipant(req);
    })));
    
    // GET /api/events/:id/participants - Listar participantes
    router.get("/api/events/:id/participants", withAuth(std::function<Response(const Request&)>([eventController](const Request& req) {
        return eventController->getParticipants(req);
    })));
    
    // PUT /api/events/participants/:id/status - Atualizar status do participante
    router.put("/api/events/participants/:id/status", withAuth(std::function<Response(const Request&)>([eventController](const Request& req) {
        return eventController->updateParticipantStatus(req);
    })));
    
    // POST /api/events/:id/reminders - Adicionar lembrete
    router.post("/api/events/:id/reminders", withAuth(std::function<Response(const Request&)>([eventController](const Request& req) {
        return eventController->addReminder(req);
    })));
    
    // GET /api/events/:id/reminders - Listar lembretes
    router.get("/api/events/:id/reminders", withAuth(std::function<Response(const Request&)>([eventController](const Request& req) {
        return eventController->getReminders(req);
    })));

    // ==================== DATABASE/ADMIN ROUTES (5 endpoints) ====================
    LOG_INFO("Registering Database/Admin routes...");
    
    // POST /api/admin/database/query - Executar query SQL
    router.post("/api/admin/database/query", withAuth(std::function<Response(const Request&)>([databaseController](const Request& req) {
        return databaseController->executeQuery(req);
    })));
    
    // GET /api/admin/database/tables - Listar todas as tabelas
    router.get("/api/admin/database/tables", withAuth(std::function<Response(const Request&)>([databaseController](const Request& req) {
        return databaseController->listTables(req);
    })));
    
    // GET /api/admin/database/tables/:name/schema - Obter schema de uma tabela
    router.get("/api/admin/database/tables/:name/schema", withAuth(std::function<Response(const Request&)>([databaseController](const Request& req) {
        return databaseController->getTableSchema(req);
    })));
    
    // GET /api/admin/database/tables/:name/count - Contar registros de uma tabela
    router.get("/api/admin/database/tables/:name/count", withAuth(std::function<Response(const Request&)>([databaseController](const Request& req) {
        return databaseController->getTableCount(req);
    })));
    
    // GET /api/admin/database/stats - Obter estatísticas do banco de dados
    router.get("/api/admin/database/stats", withAuth(std::function<Response(const Request&)>([databaseController](const Request& req) {
        return databaseController->getDatabaseStats(req);
    })));
    
    LOG_INFO("✅ Database/Admin routes registered (5 endpoints)");

    std::cout << "\n========== CHECKPOINT ANTES DE FINANCE ==========" << std::endl;
    std::cout << std::flush;

    // ==================== FINANCE ROUTES (29 endpoints) ====================
    std::cout << "\n========== REGISTERING FINANCE ROUTES ==========" << std::endl;
    std::cout << "financeController pointer: " << (financeController ? "VALID" : "NULL") << std::endl;
    std::cout << std::flush;
    Utils::Logger::info("[MAIN] Registering Finance routes...");
    Utils::Logger::info("[MAIN] financeController pointer: " + std::string(financeController ? "valid" : "NULL"));
    
    // Account routes (6)
    Utils::Logger::info("[MAIN] About to register POST /api/finance/accounts...");
    
    // TEST: Rota SEM withAuth para debug
    router.post("/api/finance/test", std::function<Response(const Request&)>([financeController](const Request& req) {
        (void)req;
        std::cout << "[TEST ROUTE] /api/finance/test called!" << std::endl;
        auto json = Core::Json::makeObject();
        json->asObject()["message"] = Core::Json::makeString("Finance test route works!");
        json->asObject()["controller_valid"] = Core::Json::makeBool(financeController != nullptr);
        return Response(StatusCode::OK).json(*json);
    }));
    
    router.post("/api/finance/accounts", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        std::cout << "[ROUTE LAMBDA] POST /api/finance/accounts lambda ENTERED!" << std::endl;
        std::cout << "financeController pointer: " << (financeController ? "VALID" : "NULL") << std::endl;
        std::cout << std::flush;
        Utils::Logger::info("[ROUTE] POST /api/finance/accounts called!");
        Utils::Logger::info("[ROUTE] About to call financeController->createAccount...");
        auto response = financeController->createAccount(req);
        Utils::Logger::info("[ROUTE] financeController->createAccount returned!");
        return response;
    })));
    Utils::Logger::info("[MAIN] POST /api/finance/accounts registered!");
    router.get("/api/finance/accounts", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->listAccounts(req);
    })));
    router.get("/api/finance/accounts/:id/balance", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->getAccountBalance(req);
    })));
    router.get("/api/finance/accounts/:id", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->getAccount(req);
    })));
    router.put("/api/finance/accounts/:id", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->updateAccount(req);
    })));
    router.del("/api/finance/accounts/:id", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->deleteAccount(req);
    })));

    // Transaction routes (8)
    router.post("/api/finance/transactions/transfer", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->createTransfer(req);
    })));
    router.post("/api/finance/transactions/:id/complete", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->completeTransaction(req);
    })));
    router.post("/api/finance/transactions/:id/cancel", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->cancelTransaction(req);
    })));
    router.post("/api/finance/transactions", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->createTransaction(req);
    })));
    router.get("/api/finance/transactions", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->listTransactions(req);
    })));
    router.get("/api/finance/transactions/:id", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->getTransaction(req);
    })));
    router.put("/api/finance/transactions/:id", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->updateTransaction(req);
    })));
    router.del("/api/finance/transactions/:id", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->deleteTransaction(req);
    })));

    // Category routes (5)
    router.post("/api/finance/categories", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->createCategory(req);
    })));
    router.get("/api/finance/categories", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->listCategories(req);
    })));
    router.get("/api/finance/categories/:id", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->getCategory(req);
    })));
    router.put("/api/finance/categories/:id", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->updateCategory(req);
    })));
    router.del("/api/finance/categories/:id", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->deleteCategory(req);
    })));

    // Budget routes (5)
    router.post("/api/finance/budgets", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->createBudget(req);
    })));
    router.get("/api/finance/budgets", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->listBudgets(req);
    })));
    router.get("/api/finance/budgets/:id", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->getBudget(req);
    })));
    router.put("/api/finance/budgets/:id", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->updateBudget(req);
    })));
    router.del("/api/finance/budgets/:id", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->deleteBudget(req);
    })));

    // Report routes (5)
    router.get("/api/finance/reports/cashflow", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->getCashFlow(req);
    })));
    router.get("/api/finance/reports/income-vs-expense", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->getIncomeVsExpense(req);
    })));
    router.get("/api/finance/reports/by-category", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->getCategoryReport(req);
    })));
    router.get("/api/finance/reports/profit-loss", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->getProfitLoss(req);
    })));
    router.get("/api/finance/reports/budget-performance", withAuth(std::function<Response(const Request&)>([financeController](const Request& req) {
        return financeController->getBudgetPerformance(req);
    })));
    
    Utils::Logger::info("[MAIN] Finance module registered: 29 routes");

    // ==================== POOL STATS ====================
    router.get("/api/pool/stats", [&pool](const Request& req) {
        (void)req;
        
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["total_connections"] = Core::Json::makeNumber(pool.totalConnections());
        json->asObject()["available_connections"] = Core::Json::makeNumber(pool.availableConnections());
        json->asObject()["active_connections"] = Core::Json::makeNumber(pool.activeConnections());
        json->asObject()["pool_health"] = Core::Json::makeString(
            pool.availableConnections() > 0 ? "healthy" : "busy"
        );
        
        return Response(StatusCode::OK).json(*json);
    });
    
    // ==================== DB TEST (COM POOL) ====================
    router.get("/api/db/test", [&pool](const Request& req) {
        (void)req;
        
        auto json = Core::Json::makeObject();
        
        try {
            // Adquirir conexão do pool (RAII - devolve automaticamente)
            LOG_DEBUG("Adquirindo conexão do pool...");
            auto conn = pool.acquire();
            
            LOG_DEBUG("Executando query...");
            auto result = conn->execute("SELECT version(), current_database(), current_user");
            
            if (result.isSuccess() && result.rowCount() > 0) {
                json->asObject()["status"] = Core::Json::makeString("success");
                json->asObject()["connected"] = Core::Json::makeBool(true);
                json->asObject()["version"] = Core::Json::makeString(result.getValue(0, 0));
                json->asObject()["database"] = Core::Json::makeString(result.getValue(0, 1));
                json->asObject()["user"] = Core::Json::makeString(result.getValue(0, 2));
                json->asObject()["pool_stats"] = Core::Json::makeObject();
                json->asObject()["pool_stats"]->asObject()["total"] = Core::Json::makeNumber(pool.totalConnections());
                json->asObject()["pool_stats"]->asObject()["available"] = Core::Json::makeNumber(pool.availableConnections());
            } else {
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString(result.getError());
            }
            
            // Conexão devolvida automaticamente aqui!
            
        } catch (const std::exception& e) {
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString(e.what());
            return Response(StatusCode::InternalServerError).json(*json);
        }
        
        return Response(StatusCode::OK).json(*json);
    });
    
    // ==================== QUERY EXAMPLE ====================
    router.get("/api/db/query", [&pool](const Request& req) {
        (void)req;
        
        auto json = Core::Json::makeObject();
        
        try {
            auto conn = pool.acquire();
            
            // Exemplo: listar todas as tabelas do banco
            auto result = conn->execute(
                "SELECT table_name FROM information_schema.tables "
                "WHERE table_schema = 'public' "
                "ORDER BY table_name"
            );
            
            if (result.isSuccess()) {
                json->asObject()["status"] = Core::Json::makeString("success");
                json->asObject()["query"] = Core::Json::makeString("List public tables");
                json->asObject()["rows"] = Core::Json::makeNumber(result.rowCount());
                
                // Criar array de tabelas
                auto tablesArray = Core::Json::makeArray();
                for (int i = 0; i < result.rowCount() && i < 20; ++i) {
                    tablesArray->asArray().push_back(
                        Core::Json::makeString(result.getValue(i, 0))
                    );
                }
                json->asObject()["tables"] = tablesArray;
            } else {
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString(result.getError());
            }
            
        } catch (const std::exception& e) {
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString(e.what());
            return Response(StatusCode::InternalServerError).json(*json);
        }
        
        return Response(StatusCode::OK).json(*json);
    });
    
    // ==================== STRESS TEST ====================
    router.get("/api/stress/:connections", [&pool](const Request& req) {
        std::string connectionsStr = req.getParam("connections");
        int numConnections = std::stoi(connectionsStr);
        
        if (numConnections < 1 || numConnections > 20) {
            auto json = Core::Json::makeObject();
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString("Número de conexões deve estar entre 1 e 20");
            return Response(StatusCode::BadRequest).json(*json);
        }
        
        auto json = Core::Json::makeObject();
        
        try {
            LOG_INFO("🔥 Stress test: adquirindo " + std::to_string(numConnections) + " conexões...");
            
            // Adquirir múltiplas conexões
            std::vector<PooledConnection> connections;
            for (int i = 0; i < numConnections; ++i) {
                connections.push_back(pool.acquire());
                LOG_DEBUG("Conexão " + std::to_string(i+1) + " adquirida");
            }
            
            // Executar query em cada uma
            int successCount = 0;
            for (auto& conn : connections) {
                auto result = conn->execute("SELECT 1");
                if (result.isSuccess()) {
                    successCount++;
                }
            }
            
            json->asObject()["status"] = Core::Json::makeString("success");
            json->asObject()["connections_acquired"] = Core::Json::makeNumber(numConnections);
            json->asObject()["queries_executed"] = Core::Json::makeNumber(successCount);
            json->asObject()["pool_stats"] = Core::Json::makeObject();
            json->asObject()["pool_stats"]->asObject()["total"] = Core::Json::makeNumber(pool.totalConnections());
            json->asObject()["pool_stats"]->asObject()["available"] = Core::Json::makeNumber(pool.availableConnections());
            json->asObject()["pool_stats"]->asObject()["active"] = Core::Json::makeNumber(pool.activeConnections());
            
            // Conexões devolvidas automaticamente quando vector sai de escopo
            LOG_INFO("✅ Stress test completo!");
            
        } catch (const std::exception& e) {
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString(e.what());
            return Response(StatusCode::InternalServerError).json(*json);
        }
        
        return Response(StatusCode::OK).json(*json);
    });
    
    // ==================== REDIS CACHE TEST ====================
    router.get("/api/cache/test", [](const Request& req) {
        (void)req;
        auto json = Core::Json::makeObject();
        
        try {
            if (!globalCacheService) {
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString("Cache service not initialized");
                return Response(StatusCode::InternalServerError).json(*json);
            }
            
            // Test 1: SET
            std::string testKey = "test:cache:" + std::to_string(std::time(nullptr));
            std::string testValue = "Hello from Redis Cache!";
            
            bool setResult = globalCacheService->set(testKey, testValue, 60); // TTL 60s
            
            // Test 2: GET
            auto getValue = globalCacheService->get(testKey);
            
            // Test 3: TTL
            int ttl = globalCacheService->ttl(testKey);
            
            // Test 4: EXISTS
            bool exists = globalCacheService->exists(testKey);
            
            // Test 5: HSET/HGET
            bool hsetResult = globalCacheService->hset("test:user:cache", "name", "João");
            auto hgetValue = globalCacheService->hget("test:user:cache", "name");
            
            // Pool stats
            auto stats = std::dynamic_pointer_cast<Core::Cache::RedisCacheService>(globalCacheService)->getPoolStats();
            
            // Build response
            json->asObject()["status"] = Core::Json::makeString("success");
            json->asObject()["tests"] = Core::Json::makeObject();
            
            json->asObject()["tests"]->asObject()["set"] = Core::Json::makeBool(setResult);
            json->asObject()["tests"]->asObject()["get"] = Core::Json::makeString(getValue.value_or("(not found)"));
            json->asObject()["tests"]->asObject()["get_match"] = Core::Json::makeBool(getValue.value_or("") == testValue);
            json->asObject()["tests"]->asObject()["ttl"] = Core::Json::makeNumber(ttl);
            json->asObject()["tests"]->asObject()["exists"] = Core::Json::makeBool(exists);
            json->asObject()["tests"]->asObject()["hset"] = Core::Json::makeBool(hsetResult);
            json->asObject()["tests"]->asObject()["hget"] = Core::Json::makeString(hgetValue.value_or("(not found)"));
            
            json->asObject()["pool_stats"] = Core::Json::makeObject();
            json->asObject()["pool_stats"]->asObject()["total"] = Core::Json::makeNumber(stats.total);
            json->asObject()["pool_stats"]->asObject()["available"] = Core::Json::makeNumber(stats.available);
            json->asObject()["pool_stats"]->asObject()["in_use"] = Core::Json::makeNumber(stats.inUse);
            
            // Cleanup
            globalCacheService->del(testKey);
            globalCacheService->del("test:user:cache");
            
            return Response(StatusCode::OK).json(*json);
            
        } catch (const std::exception& e) {
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString(e.what());
            return Response(StatusCode::InternalServerError).json(*json);
        }
    });
    
    // ==================== MESSAGE QUEUE ENDPOINTS ====================
    
    // POST /api/queue/publish - Publica mensagem na fila
    router.post("/api/queue/publish", [](const Request& req) {
        auto json = Core::Json::makeObject();
        
        try {
            auto body = req.getJson();
            if (!body || !body->isObject()) {
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString("Invalid JSON body");
                return Response(StatusCode::BadRequest).json(*json);
            }
            
            auto obj = body->asObject();
            
            if (!obj.count("stream") || !obj["stream"]->isString()) {
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString("stream (string) is required");
                return Response(StatusCode::BadRequest).json(*json);
            }
            
            if (!obj.count("data") || !obj["data"]->isObject()) {
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString("data (object) is required");
                return Response(StatusCode::BadRequest).json(*json);
            }
            
            std::string stream = obj["stream"]->asString();
            
            // Converter JsonObject para map<string, string>
            std::map<std::string, std::string> data;
            for (const auto& [key, value] : obj["data"]->asObject()) {
                if (value->isString()) {
                    data[key] = value->asString();
                } else {
                    // Converter outros tipos para string
                    data[key] = value->toString();
                }
            }
            
            // Publicar mensagem
            std::string messageId = globalMessageQueue->publish(stream, data);
            
            if (messageId.empty()) {
                json->asObject()["status"] = Core::Json::makeString("error");
                json->asObject()["message"] = Core::Json::makeString("Failed to publish message");
                return Response(StatusCode::InternalServerError).json(*json);
            }
            
            json->asObject()["status"] = Core::Json::makeString("success");
            json->asObject()["message"] = Core::Json::makeString("Message published");
            json->asObject()["message_id"] = Core::Json::makeString(messageId);
            json->asObject()["stream"] = Core::Json::makeString(stream);
            
            return Response(StatusCode::OK).json(*json);
            
        } catch (const std::exception& e) {
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString(e.what());
            return Response(StatusCode::InternalServerError).json(*json);
        }
    });
    
    // GET /api/queue/status - Status das filas
    router.get("/api/queue/status", [](const Request& req) {
        (void)req;
        auto json = Core::Json::makeObject();
        
        try {
            json->asObject()["status"] = Core::Json::makeString("success");
            json->asObject()["connected"] = Core::Json::makeBool(globalMessageQueue->isConnected());
            
            // Estatísticas da fila de emails
            auto emailStats = globalMessageQueue->getStats("emails");
            json->asObject()["emails"] = Core::Json::makeObject();
            json->asObject()["emails"]->asObject()["length"] = Core::Json::makeNumber(emailStats["length"]);
            json->asObject()["emails"]->asObject()["dlq_length"] = Core::Json::makeNumber(emailStats["dlq_length"]);
            
            // Estatísticas do worker
            if (globalEmailWorker) {
                auto workerStats = globalEmailWorker->getStats();
                json->asObject()["email_worker"] = Core::Json::makeObject();
                json->asObject()["email_worker"]->asObject()["running"] = Core::Json::makeBool(globalEmailWorker->isRunning());
                json->asObject()["email_worker"]->asObject()["processed"] = Core::Json::makeNumber(workerStats.processed);
                json->asObject()["email_worker"]->asObject()["succeeded"] = Core::Json::makeNumber(workerStats.succeeded);
                json->asObject()["email_worker"]->asObject()["failed"] = Core::Json::makeNumber(workerStats.failed);
                json->asObject()["email_worker"]->asObject()["retried"] = Core::Json::makeNumber(workerStats.retried);
                json->asObject()["email_worker"]->asObject()["moved_to_dlq"] = Core::Json::makeNumber(workerStats.movedToDLQ);
            }
            
            // Configuração de retry
            auto retryConfig = globalMessageQueue->getRetryConfig();
            json->asObject()["retry_config"] = Core::Json::makeObject();
            json->asObject()["retry_config"]->asObject()["max_retries"] = Core::Json::makeNumber(retryConfig.maxRetries);
            json->asObject()["retry_config"]->asObject()["initial_delay_ms"] = Core::Json::makeNumber(retryConfig.initialDelayMs);
            json->asObject()["retry_config"]->asObject()["max_delay_ms"] = Core::Json::makeNumber(retryConfig.maxDelayMs);
            json->asObject()["retry_config"]->asObject()["backoff_multiplier"] = Core::Json::makeNumber(retryConfig.backoffMultiplier);
            
            return Response(StatusCode::OK).json(*json);
            
        } catch (const std::exception& e) {
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString(e.what());
            return Response(StatusCode::InternalServerError).json(*json);
        }
    });
    
    // ==================== WEBSOCKET INFO ====================
    router.get("/api/ws/info", [](const Request& req) {
        (void)req;
        auto json = Core::Json::makeObject();
        
        json->asObject()["status"] = Core::Json::makeString("success");
        json->asObject()["websocket"] = Core::Json::makeObject();
        json->asObject()["websocket"]->asObject()["enabled"] = Core::Json::makeBool(true);
        json->asObject()["websocket"]->asObject()["endpoint"] = Core::Json::makeString("ws://localhost:8080/ws");
        json->asObject()["websocket"]->asObject()["protocol"] = Core::Json::makeString("RFC 6455");
        json->asObject()["websocket"]->asObject()["active_connections"] = Core::Json::makeNumber(
            globalWsHandler ? globalWsHandler->getConnectionCount() : 0
        );
        
        json->asObject()["features"] = Core::Json::makeArray();
        json->asObject()["features"]->asArray().push_back(Core::Json::makeString("Text frames"));
        json->asObject()["features"]->asArray().push_back(Core::Json::makeString("Binary frames"));
        json->asObject()["features"]->asArray().push_back(Core::Json::makeString("Ping/Pong"));
        json->asObject()["features"]->asArray().push_back(Core::Json::makeString("Broadcasting"));
        json->asObject()["features"]->asArray().push_back(Core::Json::makeString("Message fragmentation"));
        
        json->asObject()["note"] = Core::Json::makeString("WebSocket core implementation complete. HTTP upgrade integration pending.");
        
        return Response(StatusCode::OK).json(*json);
    });
    
    // ==================== REDIS PUB/SUB TEST ENDPOINT ====================
    router.post("/api/pubsub/broadcast", [](const Request& req) {
        auto json = Core::Json::makeObject();
        
        try {
            std::string body = req.getBody();
            std::string message = "Test message from API";
            
            // Tentar extrair "message" do body (parsing simples)
            size_t msgPos = body.find("\"message\"");
            if (msgPos != std::string::npos) {
                size_t colonPos = body.find(":", msgPos);
                size_t quoteStart = body.find("\"", colonPos);
                size_t quoteEnd = body.find("\"", quoteStart + 1);
                if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                    message = body.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                }
            }
            
            if (!globalRedisPubSub) {
                json->asObject()["error"] = Core::Json::makeString("Redis Pub/Sub not initialized");
                return Response(StatusCode::InternalServerError).json(*json);
            }
            
            // Publicar no canal websocket:broadcast
            int receivers = globalRedisPubSub->publish("websocket:broadcast", message);
            
            LOG_INFO("📡 Published message via Pub/Sub - Receivers: " + std::to_string(receivers));
            
            json->asObject()["status"] = Core::Json::makeString("success");
            json->asObject()["message"] = Core::Json::makeString("Message published via Redis Pub/Sub");
            json->asObject()["receivers"] = Core::Json::makeNumber(receivers);
            json->asObject()["channel"] = Core::Json::makeString("websocket:broadcast");
            json->asObject()["sent_message"] = Core::Json::makeString(message);
            
            return Response(StatusCode::OK).json(*json);
            
        } catch (const std::exception& e) {
            json->asObject()["error"] = Core::Json::makeString(e.what());
            return Response(StatusCode::InternalServerError).json(*json);
        }
    });
    
    // ==================== PROMETHEUS METRICS ENDPOINT ====================
    router.get("/metrics", [](const Request& req) {
        (void)req; // Unused parameter
        
        if (!globalMetricsCollector) {
            return Response(StatusCode::InternalServerError)
                .setHeader("Content-Type", "text/plain")
                .setBody("# Metrics collector not initialized\n");
        }
        
        try {
            // Exportar métricas no formato Prometheus
            std::string prometheusFormat = globalMetricsCollector->exportPrometheus();
            
            // Log para debugging (opcional)
            LOG_DEBUG("[/metrics] Exporting " + std::to_string(prometheusFormat.length()) + " bytes of metrics");
            
            // Retornar com Content-Type correto para Prometheus
            return Response(StatusCode::OK)
                .setHeader("Content-Type", "text/plain; version=0.0.4; charset=utf-8")
                .setBody(prometheusFormat);
                
        } catch (const std::exception& e) {
            LOG_ERROR("[/metrics] Error exporting metrics: " + std::string(e.what()));
            return Response(StatusCode::InternalServerError)
                .setHeader("Content-Type", "text/plain")
                .setBody("# Error exporting metrics: " + std::string(e.what()) + "\n");
        }
    });
    LOG_INFO("✅ GET /metrics registered (Prometheus scraping endpoint)");
    
    // ==================== HEALTH CHECK ENDPOINTS ====================
    
    // ===== /health (Liveness Probe) =====
    router.get("/health", [](const Request& req) {
        (void)req; // Unused
        auto json = Core::Json::makeObject();
        json->asObject()["status"] = Core::Json::makeString("ok");
        json->asObject()["timestamp"] = Core::Json::makeNumber(std::time(nullptr));
        return Response(StatusCode::OK).json(*json);
    });
    LOG_INFO("✅ GET /health registered (liveness probe)");
    
    // ===== /ready (Readiness Probe) =====
    router.get("/ready", [](const Request& req) {
        (void)req; // Unused
        
        // Verificar se dependências críticas estão prontas
        bool dbReady = (globalPool != nullptr);
        bool redisReady = (globalRedisPool != nullptr);
        
        auto json = Core::Json::makeObject();
        json->asObject()["database"] = Core::Json::makeBool(dbReady);
        json->asObject()["redis"] = Core::Json::makeBool(redisReady);
        json->asObject()["status"] = Core::Json::makeString(
            (dbReady && redisReady) ? "ready" : "not_ready"
        );
        
        StatusCode statusCode = (dbReady && redisReady) ? StatusCode::OK : StatusCode::ServiceUnavailable;
        return Response(statusCode).json(*json);
    });
    LOG_INFO("✅ GET /ready registered (readiness probe)");
    
    // ===== /live (Deep Health Check) =====
    router.get("/live", [](const Request& req) {
        (void)req; // Unused
        auto start = std::chrono::high_resolution_clock::now();
        
        auto json = Core::Json::makeObject();
        json->asObject()["checks"] = Core::Json::makeObject();
        
        // Testar Database
        bool dbOk = false;
        try {
            if (globalPool) {
                auto conn = globalPool->acquire();
                auto result = conn->execute("SELECT 1");
                dbOk = result.isSuccess();
            }
        } catch (...) { 
            dbOk = false; 
        }
        json->asObject()["checks"]->asObject()["database"] = Core::Json::makeBool(dbOk);
        
        // Testar Redis
        bool redisOk = false;
        try {
            if (globalRedisPool) {
                auto redis = globalRedisPool->acquire();
                redisOk = redis->ping();
            }
        } catch (...) { 
            redisOk = false; 
        }
        json->asObject()["checks"]->asObject()["redis"] = Core::Json::makeBool(redisOk);
        
        // WebSocket Connections
        int wsConnections = (globalWsHandler ? globalWsHandler->getConnectionCount() : 0);
        json->asObject()["checks"]->asObject()["websocket_connections"] = 
            Core::Json::makeNumber(wsConnections);
        
        // Message Queue
        bool mqOk = (globalMessageQueue != nullptr);
        json->asObject()["checks"]->asObject()["message_queue"] = Core::Json::makeBool(mqOk);
        
        // Calcular tempo de resposta
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        json->asObject()["response_time_ms"] = Core::Json::makeNumber(duration.count());
        
        // Status geral
        bool healthy = dbOk && redisOk && mqOk;
        json->asObject()["status"] = Core::Json::makeString(healthy ? "healthy" : "unhealthy");
        
        return Response(healthy ? StatusCode::OK : StatusCode::ServiceUnavailable).json(*json);
    });
    LOG_INFO("✅ GET /live registered (deep health check)");
    
    // ==================== WEBSOCKET ENDPOINT ====================
    router.get("/ws", [](const Request& req) {
        // Verificar se é WebSocket upgrade request
        std::string upgrade = req.getHeader("Upgrade");
        std::string connection = req.getHeader("Connection");
        
        // Case insensitive
        std::transform(upgrade.begin(), upgrade.end(), upgrade.begin(), ::tolower);
        std::transform(connection.begin(), connection.end(), connection.begin(), ::tolower);
        
        if (upgrade == "websocket" && connection.find("upgrade") != std::string::npos) {
            // É um WebSocket upgrade request!
            LOG_INFO("🔌 WebSocket upgrade request detected!");
            
            // Por enquanto, retornar mensagem explicativa
            // TODO: Integrar com WebSocketHandler::handleUpgrade()
            
            auto json = Core::Json::makeObject();
            json->asObject()["status"] = Core::Json::makeString("error");
            json->asObject()["message"] = Core::Json::makeString("WebSocket HTTP Upgrade not yet integrated");
            json->asObject()["details"] = Core::Json::makeString(
                "WebSocket core is complete but HTTP upgrade needs Server.cpp modification. "
                "See WEBSOCKET-IMPLEMENTATION-COMPLETE.md for details."
            );
            json->asObject()["workaround"] = Core::Json::makeString(
                "Use websocket-test.html with a fully integrated server or wait for Server.cpp update"
            );
            
            return Response(StatusCode::NotImplemented).json(*json);
        }
        
        // Não é WebSocket, retornar página HTML informativa
        std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>WebSocket Endpoint</title>
    <style>
        body { font-family: sans-serif; max-width: 800px; margin: 50px auto; padding: 20px; }
        .box { background: #f0f0f0; padding: 20px; border-radius: 8px; margin: 20px 0; }
        .error { background: #fee; border-left: 4px solid #c33; }
        .info { background: #e3f2fd; border-left: 4px solid #2196f3; }
        code { background: #eee; padding: 2px 6px; border-radius: 3px; }
        h1 { color: #667eea; }
    </style>
</head>
<body>
    <h1>🔌 WebSocket Endpoint</h1>
    
    <div class="box error">
        <h2>⚠️ WebSocket Upgrade Not Available</h2>
        <p>This endpoint is reserved for WebSocket connections, but HTTP Upgrade is not yet fully integrated.</p>
    </div>
    
    <div class="box info">
        <h2>ℹ️ Current Status</h2>
        <ul>
            <li>✅ <strong>WebSocket Core:</strong> Fully implemented (RFC 6455)</li>
            <li>✅ <strong>Frame Parser/Builder:</strong> Complete</li>
            <li>✅ <strong>Connection Manager:</strong> Ready</li>
            <li>✅ <strong>Broadcasting:</strong> Implemented</li>
            <li>⏳ <strong>HTTP Upgrade:</strong> Pending Server.cpp integration</li>
        </ul>
    </div>
    
    <div class="box">
        <h2>🔧 What's Needed</h2>
        <p>To complete WebSocket support, <code>Server.cpp</code> needs modification to:</p>
        <ol>
            <li>Detect WebSocket Upgrade requests</li>
            <li>Pass socket to <code>WebSocketHandler::handleUpgrade()</code></li>
            <li>Not close socket after upgrade</li>
        </ol>
        <p>See <code>WEBSOCKET-IMPLEMENTATION-COMPLETE.md</code> for implementation details.</p>
    </div>
    
    <div class="box">
        <h2>📊 Try These Instead</h2>
        <ul>
            <li><a href="/api/ws/info">GET /api/ws/info</a> - WebSocket info endpoint</li>
            <li><a href="/api/cache/test">GET /api/cache/test</a> - Test Redis cache</li>
            <li><a href="/">GET /</a> - Go to home</li>
        </ul>
    </div>
</body>
</html>
        )";
        
        return Response(StatusCode::OK).html(html);
    });
    
    // ==================== FREELANCER APP ROUTES ====================
    LOG_INFO("[setupRoutes] Registering Freelancer App routes...");
    
    // Restaurantes Routes (rotas específicas ANTES de rotas com parâmetros)
    router.post("/api/restaurantes", [restauranteController](const Request& req) {
        return restauranteController->create(req);
    });
    
    router.get("/api/restaurantes", [restauranteController](const Request& req) {
        return restauranteController->list(req);
    });
    
    // Rotas específicas PRIMEIRO (antes de /:id)
    router.get("/api/restaurantes/nearby", [restauranteController](const Request& req) {
        return restauranteController->nearby(req);
    });
    
    router.get("/api/restaurantes/map", [restauranteController](const Request& req) {
        return restauranteController->map(req);
    });
    
    router.get("/api/restaurantes/user/:userId", [restauranteController](const Request& req) {
        return restauranteController->getByUserId(req);
    });
    
    // Rotas com :id por último
    router.get("/api/restaurantes/:id", [restauranteController](const Request& req) {
        return restauranteController->getById(req);
    });
    
    router.put("/api/restaurantes/:id", [restauranteController](const Request& req) {
        return restauranteController->update(req);
    });
    
    router.del("/api/restaurantes/:id", [restauranteController](const Request& req) {
        return restauranteController->deleteById(req);
    });
    
    LOG_INFO("[setupRoutes] ✓ Restaurantes routes registered!");
    
    // Freelancers Routes (rotas específicas ANTES de rotas com parâmetros)
    router.post("/api/freelancers", [freelancerController](const Request& req) {
        return freelancerController->create(req);
    });
    
    router.get("/api/freelancers", [freelancerController](const Request& req) {
        return freelancerController->list(req);
    });
    
    // Rotas específicas PRIMEIRO (antes de /:id)
    router.get("/api/freelancers/nearby", [freelancerController](const Request& req) {
        return freelancerController->nearby(req);
    });
    
    router.get("/api/freelancers/user/:userId", [freelancerController](const Request& req) {
        return freelancerController->getByUserId(req);
    });
    
    // Rotas com :id por último
    router.get("/api/freelancers/:id", [freelancerController](const Request& req) {
        return freelancerController->getById(req);
    });
    
    router.put("/api/freelancers/:id", [freelancerController](const Request& req) {
        return freelancerController->update(req);
    });
    
    router.del("/api/freelancers/:id", [freelancerController](const Request& req) {
        return freelancerController->deleteById(req);
    });
    
    LOG_INFO("[setupRoutes] ✓ Freelancers routes registered!");
    
    // Vagas Routes (rotas específicas ANTES de rotas com parâmetros)
    router.post("/api/vagas", [vagaController](const Request& req) {
        return vagaController->create(req);
    });
    
    router.get("/api/vagas", [vagaController](const Request& req) {
        return vagaController->list(req);
    });
    
    // Rotas com :id por último
    router.get("/api/vagas/:id", [vagaController](const Request& req) {
        return vagaController->getById(req);
    });
    
    router.put("/api/vagas/:id", [vagaController](const Request& req) {
        return vagaController->update(req);
    });
    
    router.del("/api/vagas/:id", [vagaController](const Request& req) {
        return vagaController->deleteById(req);
    });
    
    router.post("/api/vagas/:id/close", [vagaController](const Request& req) {
        return vagaController->close(req);
    });
    
    LOG_INFO("[setupRoutes] ✓ Vagas routes registered!");
    
    // ==================== CANDIDATURAS MODULE ====================
    LOG_INFO("[setupRoutes] Registering Candidaturas routes...");
    
    // POST /api/candidaturas - Criar candidatura
    router.post("/api/candidaturas", [candidaturaController](const Request& req) {
        return candidaturaController->create(req);
    });
    
    // GET /api/candidaturas/vaga/:vagaId - Listar candidaturas de uma vaga (ANTES DO /:id)
    router.get("/api/candidaturas/vaga/:vagaId", [candidaturaController](const Request& req) {
        return candidaturaController->getByVaga(req);
    });
    
    // GET /api/candidaturas/freelancer/:freelancerId - Listar candidaturas de um freelancer (ANTES DO /:id)
    router.get("/api/candidaturas/freelancer/:freelancerId", [candidaturaController](const Request& req) {
        return candidaturaController->getByFreelancer(req);
    });
    
    // PUT /api/candidaturas/:id/accept - Aceitar candidatura (ANTES DO /:id)
    router.put("/api/candidaturas/:id/accept", [candidaturaController](const Request& req) {
        return candidaturaController->accept(req);
    });
    
    // PUT /api/candidaturas/:id/reject - Rejeitar candidatura (ANTES DO /:id)
    router.put("/api/candidaturas/:id/reject", [candidaturaController](const Request& req) {
        return candidaturaController->reject(req);
    });
    
    // GET /api/candidaturas/:id - Buscar candidatura por ID (DEPOIS DAS ROTAS ESPECÍFICAS)
    router.get("/api/candidaturas/:id", [candidaturaController](const Request& req) {
        return candidaturaController->getById(req);
    });
    
    LOG_INFO("[setupRoutes] ✓ Candidaturas routes registered!");
    LOG_INFO("[setupRoutes] Freelancer App routes registered successfully!");
}

/**
 * Entry point
 */
int main(int argc, char* argv[]) {
    try {
        // ==================== INICIALIZAR LOGGER ====================
        LoggerNew::init(
            "server.log",                    // Arquivo de log
            LoggerNew::Level::DEBUG,         // Nível mínimo
            true,                            // Usar cores
            true                             // Debug mode (mostrar arquivo:linha)
        );
        
        LOG_INFO("═══════════════════════════════════════");
        LOG_INFO("  🚀 C++ BACKEND v2.0");
        LOG_INFO("  ✅ ConnectionPool");
        LOG_INFO("  ✅ Middleware System");
        LOG_INFO("  ✅ Logger melhorado");
        LOG_INFO("═══════════════════════════════════════");
        
        // ==================== INICIALIZAR GERADOR DE NÚMEROS ALEATÓRIOS ====================
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        LOG_INFO("Random number generator initialized");
        
        // ==================== CRIAR CONNECTION POOL ====================
        LOG_INFO("Criando MetricsCollector...");
        globalMetricsCollector = std::make_shared<Core::Utils::MetricsCollector>();
        
        LOG_INFO("Criando connection pool...");
        
        // Detectar se estamos em Docker (variável de ambiente ou hostname)
        const char* dockerEnv = std::getenv("DOCKER_CONTAINER");
        bool isDocker = (dockerEnv != nullptr) || (std::getenv("KUBERNETES_SERVICE_HOST") != nullptr);
        
        // No Docker, usar hostnames dos serviços; localmente, usar localhost
        std::string dbHost = isDocker ? "postgres" : "localhost";
        std::string dbPort = isDocker ? "5432" : "5433";  // Docker usa porta padrão internamente
        std::string redisHost = isDocker ? "redis" : "127.0.0.1";
        
        LOG_INFO("Environment detected: " + std::string(isDocker ? "Docker" : "Local"));
        LOG_INFO("PostgreSQL: " + dbHost + ":" + dbPort);
        LOG_INFO("Redis: " + redisHost + ":6379");
        
        std::string connectionString = 
            "host=" + dbHost + " port=" + dbPort + " dbname=moneymaker_dev user=moneymaker_user password=postgre123";
        
        globalPool = std::make_shared<ConnectionPool>(
            connectionString,
            2,    // Min: 2 conexões
            10,   // Max: 10 conexões
            5000  // Timeout: 5 segundos
        );
        
        LOG_INFO("✅ Connection pool criado!");
        
        // ==================== REDIS SETUP ====================
        LOG_INFO("🔴 Initializing Redis...");
        globalRedisPool = std::make_shared<Core::Cache::RedisPool>(
            redisHost,    // Host (Docker: "redis", Local: "127.0.0.1")
            6379,         // Port
            30,           // Pool size (aumentado de 10 para 30)
            15            // Timeout seconds (aumentado de 5 para 15)
        );
        globalCacheService = std::make_shared<Core::Cache::RedisCacheService>(globalRedisPool);
        LOG_INFO("✅ Redis Cache initialized (30 connections, 15s timeout)");
        
        // ==================== WEBSOCKET SETUP ====================
        LOG_INFO("🔌 Initializing WebSocket...");
        globalWsManager = std::make_shared<Core::WebSocket::WebSocketManager>();
        globalWsHandler = std::make_shared<Core::WebSocket::WebSocketHandler>(globalWsManager);
        
        // Configurar event handlers do WebSocket
        Core::WebSocket::WebSocketEventHandlers wsHandlers;
        
        wsHandlers.onConnect = [](const std::string& clientId) {
            LOG_INFO("WebSocket: Client connected - " + clientId);
            // Enviar mensagem de boas-vindas
            if (globalWsHandler) {
                globalWsHandler->sendTo(clientId, "{\"type\":\"welcome\",\"message\":\"Connected to C++ WebSocket Server!\",\"clientId\":\"" + clientId + "\"}");
            }
        };
        
        wsHandlers.onMessage = [](const std::string& clientId, const std::string& message) {
            LOG_INFO("WebSocket: Message from " + clientId + ": " + message);
            
            // Processar mensagem
            if (message == "PING") {
                // Responder com PONG
                globalWsHandler->sendTo(clientId, "PONG:" + std::to_string(std::time(nullptr)));
            } else {
                // Echo back
                globalWsHandler->sendTo(clientId, "{\"type\":\"echo\",\"message\":\"" + message + "\"}");
                
                // Broadcast para outros clientes
                globalWsHandler->broadcastExcept(clientId, "{\"type\":\"broadcast\",\"from\":\"" + clientId + "\",\"message\":\"" + message + "\"}");
            }
        };
        
        wsHandlers.onDisconnect = [](const std::string& clientId, uint16_t code, const std::string& reason) {
            LOG_INFO("WebSocket: Client disconnected - " + clientId + " (code: " + std::to_string(code) + ", reason: " + reason + ")");
        };
        
        wsHandlers.onError = [](const std::string& clientId, const std::string& error) {
            LOG_ERROR("WebSocket: Error for client " + clientId + ": " + error);
        };
        
        globalWsHandler->setEventHandlers(wsHandlers);
        LOG_INFO("✅ WebSocket initialized - Connections: " + std::to_string(globalWsHandler->getConnectionCount()));
        
        // ==================== MESSAGE QUEUE SETUP ====================
        LOG_INFO("📬 Initializing Message Queue...");
        
        // Configurar retry policy
        Core::Queue::RetryConfig retryConfig;
        retryConfig.maxRetries = 3;
        retryConfig.initialDelayMs = 1000;
        retryConfig.maxDelayMs = 30000;
        retryConfig.backoffMultiplier = 2.0;
        
        globalMessageQueue = std::make_shared<Core::Queue::RedisMessageQueue>(
            globalRedisPool,
            retryConfig
        );
        
        LOG_INFO("✅ Message Queue initialized (max retries: " + std::to_string(retryConfig.maxRetries) + ")");
        
        // Criar worker para processar emails em background
        Core::Queue::ConsumerGroupConfig emailConfig("emails", "email-workers", "worker-1");
        emailConfig.blockTimeMs = 10000;  // Bloquear 10s esperando mensagens (reduz polling)
        emailConfig.batchSize = 5;        // Processar até 5 mensagens por vez (reduz carga)
        emailConfig.autoCreateGroup = false;  // ✅ Já criado no QueueWorker constructor (evita logs repetidos)
        
        globalEmailWorker = std::make_shared<Core::Queue::QueueWorker>(
            globalMessageQueue,
            emailConfig,
            // Handler: processar mensagem
            [](const Core::Queue::Message& msg) -> bool {
                try {
                    auto to = msg.get("to");
                    auto subject = msg.get("subject");
                    auto body = msg.get("body");
                    
                    if (!to || !subject || !body) {
                        LOG_ERROR("Email message missing required fields");
                        return false;  // Retry
                    }
                    
                    LOG_INFO("📧 Processing email: " + to.value() + " - " + subject.value());
                    
                    // TODO: Integrar com EmailService real
                    // Por enquanto, apenas simula o envio
                    
                    LOG_INFO("✅ Email sent successfully to " + to.value());
                    return true;  // Sucesso
                    
                } catch (const std::exception& e) {
                    LOG_ERROR("Error processing email: " + std::string(e.what()));
                    return false;  // Retry
                }
            },
            // Error handler (opcional)
            [](const Core::Queue::Message& msg, const std::string& error) {
                LOG_ERROR("Email processing failed permanently: " + msg.id + " - " + error);
            }
        );
        
        // Iniciar worker em background
        globalEmailWorker->start();
        LOG_INFO("✅ Email worker started in background");
        
        // ==================== REDIS PUB/SUB SETUP ====================
        LOG_INFO("📡 Initializing Redis Pub/Sub for WebSocket broadcasting...");
        
        globalRedisPubSub = std::make_shared<Core::Cache::RedisPubSub>(redisHost, 6379, 5);
        
        // Subscribe to websocket broadcast channel
        bool subSuccess = globalRedisPubSub->subscribe("websocket:broadcast", 
            [](const std::string& /*channel*/, const std::string& message) {
                // Quando recebemos mensagem via Pub/Sub, fazer broadcast local
                if (globalWsManager) {
                    globalWsManager->broadcast(message);
                    LOG_DEBUG("[Pub/Sub] Broadcasting message to local WebSocket clients: " + 
                             message.substr(0, 50) + "...");
                }
            }
        );
        
        if (!subSuccess) {
            LOG_WARNING("Failed to subscribe to websocket:broadcast channel");
        }
        
        // Iniciar listener
        if (globalRedisPubSub->start()) {
            LOG_INFO("✅ Redis Pub/Sub initialized - Subscribed channels: " + 
                    std::to_string(globalRedisPubSub->getSubscribedChannels().size()));
        } else {
            LOG_ERROR("Failed to start Redis Pub/Sub");
        }
        
        // ==================== CRIAR REPOSITÓRIOS ====================
        auto tenantRepository = std::make_shared<Domains::TenantManagement::Repositories::TenantRepository>(globalPool);
        LOG_DEBUG("✓ TenantRepository criado");
        
        // ==================== CRIAR SERVIDOR ====================
        int port = (argc > 1) ? std::atoi(argv[1]) : 8080;
        globalServer = std::make_unique<Server>(port);
        
        // ==================== CONFIGURAR MIDDLEWARES ====================
        LOG_INFO("Configurando middlewares...");
        
        auto middlewares = std::make_shared<MiddlewareChain>();
        
        // 1. CORS (todas as rotas)
        middlewares->use(Middlewares::cors());
        LOG_DEBUG("✓ CORS middleware");
        
        // 2. Tenant Resolution (extrai tenant do subdomain)
        middlewares->use(createTenantMiddleware(tenantRepository));
        LOG_DEBUG("✓ Tenant middleware");
        
        // 2.5. Metrics Collection (coleta métricas de todas as rotas)
        middlewares->use(createMetricsMiddleware(globalMetricsCollector));
        LOG_DEBUG("✓ Metrics middleware");
        
        // 3. Request Logger (todas as rotas)
        middlewares->use(Middlewares::requestLogger());
        LOG_DEBUG("✓ Request logger middleware");
        
        // 3. Timing (todas as rotas)
        middlewares->use(Middlewares::timing());
        LOG_DEBUG("✓ Timing middleware");
        
        // 4. Error Handler (todas as rotas)
        middlewares->use(Middlewares::errorHandler());
        LOG_DEBUG("✓ Error handler middleware");
        
        LOG_INFO("✅ " + std::to_string(middlewares->size()) + " middlewares configurados!");
        
        // ==================== CONFIGURAR ROUTER ====================
        auto router = std::make_shared<Router>();
        LOG_INFO("[MAIN] Router criado no endereco: " + std::to_string(reinterpret_cast<uintptr_t>(router.get())));
        router->setMiddlewares(middlewares);  // Integrar middlewares
        
        // ==================== FINANCE MODULE ====================
        LOG_INFO("[MAIN] Creating Finance repositories...");
        auto financeAccountRepo = std::make_shared<Finance::AccountRepositoryImpl>(globalPool);
        auto financeTransactionRepo = std::make_shared<Finance::TransactionRepositoryImpl>(globalPool);
        auto financeCategoryRepo = std::make_shared<Finance::CategoryRepositoryImpl>(globalPool);
        auto financeBudgetRepo = std::make_shared<Finance::BudgetRepositoryImpl>(globalPool);
        auto financeReconciliationRepo = std::make_shared<Finance::ReconciliationRepositoryImpl>(globalPool);
        
        LOG_INFO("[MAIN] Creating Finance controller...");
        auto financeController = std::make_shared<Finance::FinanceController>(
            financeAccountRepo,
            financeTransactionRepo,
            financeCategoryRepo,
            financeBudgetRepo
        );
        LOG_INFO("[MAIN] Finance controller created successfully!");
        
        // ==================== FREELANCER APP CONTROLLERS ====================
        LOG_INFO("[MAIN] Creating Restaurantes controller...");
        auto restauranteRepository = std::make_shared<Domains::Restaurantes::Repositories::RestauranteRepository>(globalPool);
        auto createRestauranteUseCase = std::make_shared<Domains::Restaurantes::UseCases::CreateRestauranteUseCase>(restauranteRepository);
        auto getRestauranteUseCase = std::make_shared<Domains::Restaurantes::UseCases::GetRestauranteUseCase>(restauranteRepository);
        auto updateRestauranteUseCase = std::make_shared<Domains::Restaurantes::UseCases::UpdateRestauranteUseCase>(restauranteRepository);
        auto listRestaurantesUseCase = std::make_shared<Domains::Restaurantes::UseCases::ListRestaurantesUseCase>(restauranteRepository);
        auto searchNearbyRestaurantesUseCase = std::make_shared<Domains::Restaurantes::UseCases::SearchNearbyRestaurantesUseCase>(restauranteRepository);
        
        auto restauranteController = std::make_shared<Domains::Restaurantes::Controllers::RestauranteController>(
            createRestauranteUseCase,
            getRestauranteUseCase,
            updateRestauranteUseCase,
            listRestaurantesUseCase,
            searchNearbyRestaurantesUseCase
        );
        LOG_INFO("[MAIN] Restaurantes controller created!");
        
        LOG_INFO("[MAIN] Creating Freelancers controller...");
        auto freelancerRepository = std::make_shared<Domains::Freelances::Repositories::FreelancerRepository>(globalPool);
        auto createFreelancerUseCase = std::make_shared<Domains::Freelances::UseCases::CreateFreelancerUseCase>(freelancerRepository);
        auto getFreelancerUseCase = std::make_shared<Domains::Freelances::UseCases::GetFreelancerUseCase>(freelancerRepository);
        auto updateFreelancerUseCase = std::make_shared<Domains::Freelances::UseCases::UpdateFreelancerUseCase>(freelancerRepository);
        auto listFreelancersUseCase = std::make_shared<Domains::Freelances::UseCases::ListFreelancersUseCase>(freelancerRepository);
        auto searchNearbyFreelancersUseCase = std::make_shared<Domains::Freelances::UseCases::SearchNearbyFreelancersUseCase>(freelancerRepository);
        
        auto freelancerController = std::make_shared<Domains::Freelances::Controllers::FreelancerController>(
            createFreelancerUseCase,
            getFreelancerUseCase,
            updateFreelancerUseCase,
            listFreelancersUseCase,
            searchNearbyFreelancersUseCase
        );
        LOG_INFO("[MAIN] Freelancers controller created!");
        
        LOG_INFO("[MAIN] Creating Vagas controller...");
        auto vagaRepository = std::make_shared<Domains::Vagas::Repositories::VagaRepository>(globalPool);
        auto createVagaUseCase = std::make_shared<Domains::Vagas::UseCases::CreateVagaUseCase>(vagaRepository);
        auto getVagaUseCase = std::make_shared<Domains::Vagas::UseCases::GetVagaUseCase>(vagaRepository);
        auto updateVagaUseCase = std::make_shared<Domains::Vagas::UseCases::UpdateVagaUseCase>(vagaRepository);
        auto listVagasUseCase = std::make_shared<Domains::Vagas::UseCases::ListVagasUseCase>(vagaRepository);
        
        auto vagaController = std::make_shared<Domains::Vagas::Controllers::VagaController>(
            createVagaUseCase,
            getVagaUseCase,
            updateVagaUseCase,
            listVagasUseCase
        );
        LOG_INFO("[MAIN] Vagas controller created!");
        
        LOG_INFO("[MAIN] Creating Candidaturas controller...");
        auto candidaturaRepository = std::make_shared<Domains::Candidaturas::Repositories::CandidaturaRepository>(globalPool);
        
        auto candidaturaController = std::make_shared<Domains::Candidaturas::Controllers::CandidaturaController>(
            candidaturaRepository,
            vagaRepository
        );
        LOG_INFO("[MAIN] Candidaturas controller created!");
        
        LOG_INFO("[MAIN] About to call setupRoutes...");
        
        try {
            setupRoutes(*router, *globalPool, financeController, restauranteController, freelancerController, vagaController, candidaturaController);
            LOG_INFO("[MAIN] setupRoutes completed successfully!");
        } catch (const std::exception& e) {
            LOG_FATAL("[MAIN] setupRoutes EXCEPTION: " + std::string(e.what()));
            throw;
        } catch (...) {
            LOG_FATAL("[MAIN] setupRoutes UNKNOWN EXCEPTION!");
            throw;
        }
        
        // ==================== METRICS ENDPOINT ====================
        // Adicionar DEPOIS de setupRoutes para ter acesso a globalMetricsCollector
        router->get("/metrics", [](const Request& req) {
            (void)req;
            
            try {
                std::string prometheusOutput = globalMetricsCollector->exportPrometheus();
                
                Response response(StatusCode::OK);
                response.setHeader("Content-Type", "text/plain; version=0.0.4; charset=utf-8");
                response.setBody(prometheusOutput);
                return response;
                    
            } catch (const std::exception& e) {
                std::string error = "# ERROR: " + std::string(e.what()) + "\n";
                Response response(StatusCode::InternalServerError);
                response.setHeader("Content-Type", "text/plain");
                response.setBody(error);
                return response;
            }
        });
        
        globalServer->setRouter(router);
        
        // ==================== SIGNAL HANDLER ====================
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
        
        // ==================== INICIAR SERVIDOR ====================
        LOG_INFO("🚀 Servidor iniciando na porta " + std::to_string(port) + "...");
        globalServer->start();
        
    } catch (const std::exception& e) {
        LOG_FATAL(std::string("Erro fatal: ") + e.what());
        return 1;
    }
    
    return 0;
}

