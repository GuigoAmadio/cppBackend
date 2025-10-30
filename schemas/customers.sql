-- ==============================================================================
-- CUSTOMERS MODULE
-- ==============================================================================

DROP TABLE IF EXISTS customers CASCADE;

CREATE TABLE customers (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    tenant_id TEXT NOT NULL,
    workspace_id TEXT,
    name VARCHAR(500) NOT NULL,
    email VARCHAR(255),
    phone VARCHAR(50),
    document VARCHAR(50),  -- CPF/CNPJ
    type VARCHAR(50) NOT NULL DEFAULT 'individual',  -- individual, business, foreign
    status VARCHAR(50) NOT NULL DEFAULT 'active',    -- active, inactive, blocked, prospect
    address JSONB,  -- {street, city, state, zip, country, ...}
    notes TEXT,
    tags TEXT[],
    created_by TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    UNIQUE(tenant_id, email),
    UNIQUE(tenant_id, document)
);

-- Indexes
CREATE INDEX idx_customers_tenant ON customers(tenant_id);
CREATE INDEX idx_customers_workspace ON customers(workspace_id);
CREATE INDEX idx_customers_email ON customers(email);
CREATE INDEX idx_customers_document ON customers(document);
CREATE INDEX idx_customers_type ON customers(type);
CREATE INDEX idx_customers_status ON customers(status);
CREATE INDEX idx_customers_created_at ON customers(created_at DESC);

-- ==============================================================================
-- FIM - CUSTOMERS
-- ==============================================================================

