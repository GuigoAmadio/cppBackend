-- ============================================
-- INVOICING MODULE - SQL SCHEMA
-- ============================================

-- Tabela principal de faturas (invoices)
CREATE TABLE IF NOT EXISTS invoices (
    id TEXT PRIMARY KEY,
    invoice_number TEXT NOT NULL UNIQUE,
    tenant_id TEXT NOT NULL,
    customer_id TEXT NOT NULL,
    
    -- Relacionamento opcional com order ou subscription
    order_id TEXT,
    subscription_id TEXT,
    
    -- Valores
    subtotal DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    tax DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    discount DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    total DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    amount_paid DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    amount_due DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    
    currency VARCHAR(3) NOT NULL DEFAULT 'BRL',
    
    -- Status e tipo
    status VARCHAR(20) NOT NULL DEFAULT 'draft',  -- draft, sent, paid, partial, overdue, cancelled, void
    invoice_type VARCHAR(20) NOT NULL DEFAULT 'standard',  -- standard, proforma, credit_note, debit_note
    
    -- Datas
    issue_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    due_date TIMESTAMP,
    paid_at TIMESTAMP,
    
    -- Notas e descrição
    description TEXT,
    notes TEXT,
    terms TEXT,
    
    -- Metadados
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    created_by TEXT,
    updated_by TEXT
);

-- Índices para invoices
CREATE INDEX IF NOT EXISTS idx_invoices_tenant ON invoices(tenant_id);
CREATE INDEX IF NOT EXISTS idx_invoices_customer ON invoices(customer_id);
CREATE INDEX IF NOT EXISTS idx_invoices_order ON invoices(order_id);
CREATE INDEX IF NOT EXISTS idx_invoices_subscription ON invoices(subscription_id);
CREATE INDEX IF NOT EXISTS idx_invoices_status ON invoices(status);
CREATE INDEX IF NOT EXISTS idx_invoices_due_date ON invoices(due_date);

-- Tabela de itens da fatura (invoice_items)
CREATE TABLE IF NOT EXISTS invoice_items (
    id TEXT PRIMARY KEY,
    invoice_id TEXT NOT NULL,
    tenant_id TEXT NOT NULL,
    
    -- Produto/Serviço
    product_id TEXT,
    description TEXT NOT NULL,
    
    -- Quantidades e valores
    quantity DECIMAL(10,2) NOT NULL DEFAULT 1.00,
    unit_price DECIMAL(10,2) NOT NULL,
    discount DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    tax_rate DECIMAL(5,2) NOT NULL DEFAULT 0.00,
    tax_amount DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    total DECIMAL(10,2) NOT NULL,
    
    -- Ordem de exibição
    sort_order INTEGER NOT NULL DEFAULT 0,
    
    -- Metadados
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    
    FOREIGN KEY (invoice_id) REFERENCES invoices(id) ON DELETE CASCADE
);

-- Índices para invoice_items
CREATE INDEX IF NOT EXISTS idx_invoice_items_invoice ON invoice_items(invoice_id);
CREATE INDEX IF NOT EXISTS idx_invoice_items_product ON invoice_items(product_id);

-- Tabela de relacionamento entre invoices e payments
CREATE TABLE IF NOT EXISTS invoice_payments (
    id TEXT PRIMARY KEY,
    invoice_id TEXT NOT NULL,
    payment_id TEXT NOT NULL,
    amount DECIMAL(10,2) NOT NULL,
    applied_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    
    FOREIGN KEY (invoice_id) REFERENCES invoices(id) ON DELETE CASCADE,
    FOREIGN KEY (payment_id) REFERENCES payments(id) ON DELETE RESTRICT
);

-- Índices para invoice_payments
CREATE INDEX IF NOT EXISTS idx_invoice_payments_invoice ON invoice_payments(invoice_id);
CREATE INDEX IF NOT EXISTS idx_invoice_payments_payment ON invoice_payments(payment_id);

-- Comentários
COMMENT ON TABLE invoices IS 'Faturas geradas para orders ou subscriptions';
COMMENT ON TABLE invoice_items IS 'Itens/linhas de uma fatura';
COMMENT ON TABLE invoice_payments IS 'Relacionamento entre faturas e pagamentos';

