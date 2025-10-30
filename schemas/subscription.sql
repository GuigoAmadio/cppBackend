-- ================================================
-- SCHEMA: SUBSCRIPTION & BILLING
-- Descrição: Sistema de assinaturas recorrentes
-- ================================================

-- Tabela: subscription_plans
-- Planos de assinatura disponíveis
CREATE TABLE IF NOT EXISTS subscription_plans (
    id TEXT PRIMARY KEY,
    tenant_id TEXT NOT NULL,
    
    -- Informações do plano
    name VARCHAR(100) NOT NULL,
    description TEXT,
    plan_type VARCHAR(50) NOT NULL, -- 'basic', 'pro', 'enterprise', 'custom'
    
    -- Preços e cobrança
    price DECIMAL(15,2) NOT NULL CHECK (price >= 0),
    currency VARCHAR(3) NOT NULL DEFAULT 'BRL',
    billing_cycle VARCHAR(20) NOT NULL, -- 'monthly', 'quarterly', 'yearly', 'lifetime'
    trial_days INTEGER DEFAULT 0 CHECK (trial_days >= 0),
    
    -- Limites e features
    features JSONB, -- {"max_users": 10, "storage_gb": 100, "api_calls": 1000}
    limits JSONB,   -- {"projects": 5, "workspaces": 3}
    
    -- Status e visibilidade
    is_active BOOLEAN NOT NULL DEFAULT true,
    is_public BOOLEAN NOT NULL DEFAULT true, -- Visível no catálogo?
    
    -- Ordering
    display_order INTEGER DEFAULT 0,
    
    -- Auditoria
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    created_by TEXT,
    updated_by TEXT,
    
    CONSTRAINT subscription_plans_billing_cycle_check 
        CHECK (billing_cycle IN ('monthly', 'quarterly', 'yearly', 'lifetime'))
);

-- Tabela: subscriptions
-- Assinaturas ativas dos clientes
CREATE TABLE IF NOT EXISTS subscriptions (
    id TEXT PRIMARY KEY,
    tenant_id TEXT NOT NULL,
    customer_id TEXT NOT NULL,
    plan_id TEXT NOT NULL REFERENCES subscription_plans(id) ON DELETE RESTRICT,
    
    -- Status da assinatura
    status VARCHAR(50) NOT NULL DEFAULT 'active', -- 'trial', 'active', 'past_due', 'cancelled', 'expired', 'paused'
    
    -- Datas importantes
    started_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    trial_ends_at TIMESTAMP,
    current_period_start TIMESTAMP NOT NULL,
    current_period_end TIMESTAMP NOT NULL,
    cancelled_at TIMESTAMP,
    expires_at TIMESTAMP,
    paused_at TIMESTAMP,
    
    -- Informações de cobrança
    next_billing_date TIMESTAMP,
    last_billing_date TIMESTAMP,
    billing_amount DECIMAL(15,2) NOT NULL,
    currency VARCHAR(3) NOT NULL DEFAULT 'BRL',
    
    -- Contadores e métricas
    billing_cycles_completed INTEGER DEFAULT 0,
    failed_billing_attempts INTEGER DEFAULT 0,
    
    -- Metadata
    metadata JSONB, -- Dados adicionais customizáveis
    cancel_reason TEXT,
    
    -- Auditoria
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    created_by TEXT,
    updated_by TEXT,
    
    CONSTRAINT subscriptions_status_check 
        CHECK (status IN ('trial', 'active', 'past_due', 'cancelled', 'expired', 'paused'))
);

-- Tabela: subscription_billing_history
-- Histórico de todas as cobranças da assinatura
CREATE TABLE IF NOT EXISTS subscription_billing_history (
    id TEXT PRIMARY KEY,
    subscription_id TEXT NOT NULL REFERENCES subscriptions(id) ON DELETE CASCADE,
    tenant_id TEXT NOT NULL,
    
    -- Informações da cobrança
    billing_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    period_start TIMESTAMP NOT NULL,
    period_end TIMESTAMP NOT NULL,
    
    -- Valores
    amount DECIMAL(15,2) NOT NULL,
    currency VARCHAR(3) NOT NULL DEFAULT 'BRL',
    
    -- Status e pagamento
    status VARCHAR(50) NOT NULL DEFAULT 'pending', -- 'pending', 'paid', 'failed', 'refunded'
    payment_id TEXT, -- Referência ao payment (se pago)
    payment_method VARCHAR(50),
    
    -- Detalhes
    invoice_number VARCHAR(50),
    invoice_url TEXT,
    failure_reason TEXT,
    
    -- Metadata
    metadata JSONB,
    
    -- Auditoria
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    
    CONSTRAINT subscription_billing_history_status_check 
        CHECK (status IN ('pending', 'paid', 'failed', 'refunded'))
);

-- Tabela: subscription_usage
-- Métricas de uso das assinaturas (para billing baseado em uso)
CREATE TABLE IF NOT EXISTS subscription_usage (
    id TEXT PRIMARY KEY,
    subscription_id TEXT NOT NULL REFERENCES subscriptions(id) ON DELETE CASCADE,
    tenant_id TEXT NOT NULL,
    
    -- Tipo de uso
    metric_name VARCHAR(100) NOT NULL, -- 'api_calls', 'storage_gb', 'users', 'projects'
    metric_value DECIMAL(15,2) NOT NULL,
    metric_unit VARCHAR(50) NOT NULL, -- 'count', 'gb', 'hours', 'requests'
    
    -- Período
    period_start TIMESTAMP NOT NULL,
    period_end TIMESTAMP NOT NULL,
    
    -- Metadata
    metadata JSONB,
    
    -- Auditoria
    recorded_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Índices para performance
CREATE INDEX IF NOT EXISTS idx_subscription_plans_tenant ON subscription_plans(tenant_id);
CREATE INDEX IF NOT EXISTS idx_subscription_plans_active ON subscription_plans(is_active) WHERE is_active = true;

CREATE INDEX IF NOT EXISTS idx_subscriptions_tenant ON subscriptions(tenant_id);
CREATE INDEX IF NOT EXISTS idx_subscriptions_customer ON subscriptions(customer_id);
CREATE INDEX IF NOT EXISTS idx_subscriptions_plan ON subscriptions(plan_id);
CREATE INDEX IF NOT EXISTS idx_subscriptions_status ON subscriptions(status);
CREATE INDEX IF NOT EXISTS idx_subscriptions_next_billing ON subscriptions(next_billing_date) WHERE status IN ('active', 'trial');

CREATE INDEX IF NOT EXISTS idx_billing_history_subscription ON subscription_billing_history(subscription_id);
CREATE INDEX IF NOT EXISTS idx_billing_history_status ON subscription_billing_history(status);
CREATE INDEX IF NOT EXISTS idx_billing_history_date ON subscription_billing_history(billing_date);

CREATE INDEX IF NOT EXISTS idx_usage_subscription ON subscription_usage(subscription_id);
CREATE INDEX IF NOT EXISTS idx_usage_period ON subscription_usage(period_start, period_end);

-- Comentários
COMMENT ON TABLE subscription_plans IS 'Planos de assinatura disponíveis para os clientes';
COMMENT ON TABLE subscriptions IS 'Assinaturas ativas e históricas dos clientes';
COMMENT ON TABLE subscription_billing_history IS 'Histórico de cobranças das assinaturas';
COMMENT ON TABLE subscription_usage IS 'Métricas de uso das assinaturas para billing';

