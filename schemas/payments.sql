-- ================================================
-- SCHEMA: PAYMENTS
-- Descrição: Sistema de pagamentos multi-provider
-- ================================================

-- Tabela: payments
-- Armazena informações de pagamentos
CREATE TABLE IF NOT EXISTS payments (
    id TEXT PRIMARY KEY,
    order_id TEXT NOT NULL REFERENCES orders(id) ON DELETE CASCADE,
    customer_id TEXT NOT NULL REFERENCES customers(id) ON DELETE CASCADE,
    tenant_id TEXT NOT NULL,
    
    -- Informações do pagamento
    amount DECIMAL(15,2) NOT NULL CHECK (amount > 0),
    currency VARCHAR(3) NOT NULL DEFAULT 'BRL',
    provider VARCHAR(50) NOT NULL, -- 'stripe', 'paypal', 'pix', 'boleto', 'credit_card', 'manual'
    status VARCHAR(50) NOT NULL DEFAULT 'pending', -- 'pending', 'processing', 'completed', 'failed', 'refunded', 'cancelled'
    
    -- Metadados do provider
    provider_payment_id TEXT, -- ID do pagamento no provider externo
    provider_metadata JSONB, -- Dados adicionais do provider
    
    -- Informações do método de pagamento
    payment_method VARCHAR(50), -- 'credit_card', 'debit_card', 'pix', 'boleto', 'bank_transfer'
    payment_details JSONB, -- Detalhes do método (últimos 4 dígitos do cartão, chave PIX, etc)
    
    -- Datas importantes
    paid_at TIMESTAMP,
    expires_at TIMESTAMP, -- Para boletos/PIX
    
    -- Auditoria
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    created_by TEXT,
    updated_by TEXT,
    
    -- Índices para busca
    CONSTRAINT payments_status_check CHECK (status IN ('pending', 'processing', 'completed', 'failed', 'refunded', 'cancelled'))
);

-- Tabela: payment_transactions
-- Histórico de todas as transações (capturas, reembolsos, cancelamentos)
CREATE TABLE IF NOT EXISTS payment_transactions (
    id TEXT PRIMARY KEY,
    payment_id TEXT NOT NULL REFERENCES payments(id) ON DELETE CASCADE,
    tenant_id TEXT NOT NULL,
    
    -- Tipo e status da transação
    transaction_type VARCHAR(50) NOT NULL, -- 'capture', 'refund', 'cancel', 'chargeback'
    status VARCHAR(50) NOT NULL DEFAULT 'pending', -- 'pending', 'completed', 'failed'
    
    -- Valores
    amount DECIMAL(15,2) NOT NULL,
    currency VARCHAR(3) NOT NULL DEFAULT 'BRL',
    
    -- Metadados do provider
    provider_transaction_id TEXT, -- ID da transação no provider
    provider_response JSONB, -- Resposta completa do provider
    
    -- Motivo (para reembolsos/cancelamentos)
    reason TEXT,
    notes TEXT,
    
    -- Auditoria
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    created_by TEXT,
    
    CONSTRAINT payment_transactions_type_check CHECK (transaction_type IN ('capture', 'refund', 'cancel', 'chargeback'))
);

-- Tabela: payment_webhooks
-- Log de webhooks recebidos dos providers de pagamento
CREATE TABLE IF NOT EXISTS payment_webhooks (
    id TEXT PRIMARY KEY,
    tenant_id TEXT NOT NULL,
    
    -- Origem do webhook
    provider VARCHAR(50) NOT NULL,
    event_type VARCHAR(100) NOT NULL,
    
    -- Dados do evento
    payload JSONB NOT NULL,
    signature TEXT, -- Para validação de segurança
    
    -- Status do processamento
    processed BOOLEAN NOT NULL DEFAULT FALSE,
    processed_at TIMESTAMP,
    processing_error TEXT,
    
    -- Relacionamento (se identificado)
    payment_id TEXT REFERENCES payments(id) ON DELETE SET NULL,
    order_id TEXT REFERENCES orders(id) ON DELETE SET NULL,
    
    -- Auditoria
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    ip_address INET,
    user_agent TEXT
);

-- ================================================
-- ÍNDICES
-- ================================================

-- Payments
CREATE INDEX IF NOT EXISTS idx_payments_order_id ON payments(order_id);
CREATE INDEX IF NOT EXISTS idx_payments_customer_id ON payments(customer_id);
CREATE INDEX IF NOT EXISTS idx_payments_tenant_id ON payments(tenant_id);
CREATE INDEX IF NOT EXISTS idx_payments_status ON payments(status);
CREATE INDEX IF NOT EXISTS idx_payments_provider ON payments(provider);
CREATE INDEX IF NOT EXISTS idx_payments_provider_payment_id ON payments(provider_payment_id);
CREATE INDEX IF NOT EXISTS idx_payments_created_at ON payments(created_at DESC);

-- Transactions
CREATE INDEX IF NOT EXISTS idx_payment_transactions_payment_id ON payment_transactions(payment_id);
CREATE INDEX IF NOT EXISTS idx_payment_transactions_tenant_id ON payment_transactions(tenant_id);
CREATE INDEX IF NOT EXISTS idx_payment_transactions_type ON payment_transactions(transaction_type);
CREATE INDEX IF NOT EXISTS idx_payment_transactions_created_at ON payment_transactions(created_at DESC);

-- Webhooks
CREATE INDEX IF NOT EXISTS idx_payment_webhooks_tenant_id ON payment_webhooks(tenant_id);
CREATE INDEX IF NOT EXISTS idx_payment_webhooks_provider ON payment_webhooks(provider);
CREATE INDEX IF NOT EXISTS idx_payment_webhooks_payment_id ON payment_webhooks(payment_id);
CREATE INDEX IF NOT EXISTS idx_payment_webhooks_processed ON payment_webhooks(processed);
CREATE INDEX IF NOT EXISTS idx_payment_webhooks_created_at ON payment_webhooks(created_at DESC);

-- ================================================
-- COMENTÁRIOS
-- ================================================

COMMENT ON TABLE payments IS 'Pagamentos realizados pelos clientes';
COMMENT ON TABLE payment_transactions IS 'Histórico de transações de pagamento (capturas, reembolsos, etc)';
COMMENT ON TABLE payment_webhooks IS 'Log de webhooks recebidos dos provedores de pagamento';

COMMENT ON COLUMN payments.provider_payment_id IS 'ID do pagamento no sistema do provider (Stripe, PayPal, etc)';
COMMENT ON COLUMN payments.expires_at IS 'Data de expiração (usado para Boleto e PIX)';
COMMENT ON COLUMN payment_transactions.transaction_type IS 'Tipo: capture (captura inicial), refund (reembolso), cancel (cancelamento), chargeback (contestação)';
COMMENT ON COLUMN payment_webhooks.processed IS 'Se o webhook já foi processado pelo sistema';

