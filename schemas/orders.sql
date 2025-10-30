-- ==============================================================================
-- ORDERS MODULE - SQL SCHEMA
-- ==============================================================================

-- Orders table
CREATE TABLE IF NOT EXISTS orders (
    id TEXT PRIMARY KEY,
    tenant_id TEXT NOT NULL,
    customer_id TEXT NOT NULL,
    user_id TEXT NOT NULL,  -- Usuario que criou o pedido
    order_number TEXT NOT NULL,
    
    -- Status
    status TEXT NOT NULL DEFAULT 'pending',  -- pending, confirmed, processing, shipped, delivered, cancelled
    
    -- Valores
    subtotal NUMERIC(15,2) NOT NULL DEFAULT 0,
    discount NUMERIC(15,2) NOT NULL DEFAULT 0,
    tax NUMERIC(15,2) NOT NULL DEFAULT 0,
    shipping_cost NUMERIC(15,2) NOT NULL DEFAULT 0,
    total NUMERIC(15,2) NOT NULL DEFAULT 0,
    
    -- Payment
    payment_method TEXT,  -- credit_card, debit_card, pix, boleto, cash
    payment_status TEXT NOT NULL DEFAULT 'pending',  -- pending, paid, failed, refunded
    
    -- Shipping
    shipping_address TEXT,
    shipping_city TEXT,
    shipping_state TEXT,
    shipping_zip_code TEXT,
    shipping_country TEXT DEFAULT 'BR',
    
    -- Notes
    customer_notes TEXT,
    internal_notes TEXT,
    
    -- Timestamps
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    confirmed_at TIMESTAMP,
    shipped_at TIMESTAMP,
    delivered_at TIMESTAMP,
    cancelled_at TIMESTAMP,
    
    -- Foreign keys
    CONSTRAINT fk_orders_tenant FOREIGN KEY (tenant_id) REFERENCES tenants(id) ON DELETE CASCADE,
    CONSTRAINT fk_orders_customer FOREIGN KEY (customer_id) REFERENCES customers(id) ON DELETE RESTRICT,
    CONSTRAINT fk_orders_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE RESTRICT
);

-- Order Items table
CREATE TABLE IF NOT EXISTS order_items (
    id TEXT PRIMARY KEY,
    order_id TEXT NOT NULL,
    product_id TEXT NOT NULL,
    
    -- Product snapshot (para manter histórico)
    product_name TEXT NOT NULL,
    product_code TEXT,
    
    -- Pricing
    quantity NUMERIC(10,2) NOT NULL,
    unit_price NUMERIC(15,2) NOT NULL,
    discount NUMERIC(15,2) NOT NULL DEFAULT 0,
    tax NUMERIC(15,2) NOT NULL DEFAULT 0,
    subtotal NUMERIC(15,2) NOT NULL,
    total NUMERIC(15,2) NOT NULL,
    
    -- Metadata
    notes TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    -- Foreign keys
    CONSTRAINT fk_order_items_order FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE,
    CONSTRAINT fk_order_items_product FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE RESTRICT
);

-- Indexes for orders
CREATE INDEX IF NOT EXISTS idx_orders_tenant ON orders(tenant_id);
CREATE INDEX IF NOT EXISTS idx_orders_customer ON orders(customer_id);
CREATE INDEX IF NOT EXISTS idx_orders_user ON orders(user_id);
CREATE INDEX IF NOT EXISTS idx_orders_status ON orders(status);
CREATE INDEX IF NOT EXISTS idx_orders_payment_status ON orders(payment_status);
CREATE INDEX IF NOT EXISTS idx_orders_order_number ON orders(order_number);
CREATE INDEX IF NOT EXISTS idx_orders_created_at ON orders(created_at DESC);

-- Indexes for order_items
CREATE INDEX IF NOT EXISTS idx_order_items_order ON order_items(order_id);
CREATE INDEX IF NOT EXISTS idx_order_items_product ON order_items(product_id);

-- Unique constraint for order_number per tenant
CREATE UNIQUE INDEX IF NOT EXISTS idx_orders_tenant_order_number ON orders(tenant_id, order_number);

-- Comments
COMMENT ON TABLE orders IS 'Pedidos de vendas';
COMMENT ON TABLE order_items IS 'Itens dos pedidos';
COMMENT ON COLUMN orders.status IS 'Status do pedido: pending, confirmed, processing, shipped, delivered, cancelled';
COMMENT ON COLUMN orders.payment_status IS 'Status do pagamento: pending, paid, failed, refunded';
COMMENT ON COLUMN orders.payment_method IS 'Método de pagamento: credit_card, debit_card, pix, boleto, cash';

