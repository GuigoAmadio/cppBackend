-- ==============================================================================
-- PRODUCT & INVENTORY MODULES
-- Data: 2025-10-21
-- ==============================================================================

-- ==============================================================================
-- 1. PRODUCTS
-- ==============================================================================

DROP TABLE IF EXISTS product_variants CASCADE;
DROP TABLE IF EXISTS product_images CASCADE;
DROP TABLE IF EXISTS products CASCADE;
DROP TABLE IF EXISTS product_categories CASCADE;

CREATE TABLE product_categories (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    tenant_id TEXT NOT NULL,
    workspace_id TEXT REFERENCES workspaces(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    slug VARCHAR(255) NOT NULL,
    description TEXT,
    parent_id TEXT REFERENCES product_categories(id) ON DELETE CASCADE,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(tenant_id, slug)
);

CREATE TABLE products (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    tenant_id TEXT NOT NULL,
    workspace_id TEXT REFERENCES workspaces(id) ON DELETE CASCADE,
    category_id TEXT REFERENCES product_categories(id) ON DELETE SET NULL,
    code VARCHAR(100) NOT NULL,
    name VARCHAR(500) NOT NULL,
    description TEXT,
    type VARCHAR(50) NOT NULL DEFAULT 'physical', -- physical, digital, service
    status VARCHAR(50) NOT NULL DEFAULT 'active', -- active, inactive, archived
    
    -- Pricing
    price DECIMAL(12, 2) NOT NULL DEFAULT 0.00,
    cost DECIMAL(12, 2) DEFAULT 0.00,
    currency VARCHAR(3) DEFAULT 'BRL',
    
    -- Stock
    track_inventory BOOLEAN DEFAULT true,
    stock_quantity INT DEFAULT 0,
    low_stock_threshold INT DEFAULT 10,
    
    -- Attributes
    sku VARCHAR(100),
    barcode VARCHAR(100),
    weight DECIMAL(10, 2),
    weight_unit VARCHAR(10) DEFAULT 'kg',
    dimensions JSONB, -- { "length": 10, "width": 5, "height": 3, "unit": "cm" }
    
    -- Metadata
    metadata JSONB DEFAULT '{}',
    tags TEXT[],
    
    -- Timestamps
    created_by TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    UNIQUE(tenant_id, code),
    UNIQUE(tenant_id, sku)
);

CREATE TABLE product_images (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    product_id TEXT NOT NULL REFERENCES products(id) ON DELETE CASCADE,
    url VARCHAR(1000) NOT NULL,
    alt_text VARCHAR(500),
    position INT NOT NULL DEFAULT 0,
    is_primary BOOLEAN DEFAULT false,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE product_variants (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    product_id TEXT NOT NULL REFERENCES products(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL, -- "Tamanho P - Cor Azul"
    sku VARCHAR(100),
    barcode VARCHAR(100),
    price DECIMAL(12, 2) NOT NULL,
    cost DECIMAL(12, 2),
    stock_quantity INT DEFAULT 0,
    attributes JSONB NOT NULL, -- { "size": "P", "color": "blue" }
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(product_id, sku)
);

CREATE INDEX idx_products_tenant ON products(tenant_id);
CREATE INDEX idx_products_workspace ON products(workspace_id);
CREATE INDEX idx_products_category ON products(category_id);
CREATE INDEX idx_products_code ON products(code);
CREATE INDEX idx_products_sku ON products(sku);
CREATE INDEX idx_products_status ON products(status);
CREATE INDEX idx_product_categories_tenant ON product_categories(tenant_id);
CREATE INDEX idx_product_categories_workspace ON product_categories(workspace_id);
CREATE INDEX idx_product_images_product ON product_images(product_id);
CREATE INDEX idx_product_variants_product ON product_variants(product_id);

-- ==============================================================================
-- 2. INVENTORY
-- ==============================================================================

DROP TABLE IF EXISTS inventory_transactions CASCADE;
DROP TABLE IF EXISTS inventory_locations CASCADE;

CREATE TABLE inventory_locations (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    tenant_id TEXT NOT NULL,
    workspace_id TEXT REFERENCES workspaces(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    code VARCHAR(50) NOT NULL,
    type VARCHAR(50) NOT NULL DEFAULT 'warehouse', -- warehouse, store, dropship
    address TEXT,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(tenant_id, code)
);

CREATE TABLE inventory_transactions (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    tenant_id TEXT NOT NULL,
    product_id TEXT NOT NULL REFERENCES products(id) ON DELETE CASCADE,
    location_id TEXT REFERENCES inventory_locations(id) ON DELETE SET NULL,
    type VARCHAR(50) NOT NULL, -- in, out, adjustment, transfer
    quantity INT NOT NULL,
    quantity_before INT NOT NULL,
    quantity_after INT NOT NULL,
    reference_type VARCHAR(50), -- order, return, adjustment, transfer
    reference_id TEXT,
    notes TEXT,
    created_by TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_inventory_locations_tenant ON inventory_locations(tenant_id);
CREATE INDEX idx_inventory_locations_workspace ON inventory_locations(workspace_id);
CREATE INDEX idx_inventory_transactions_tenant ON inventory_transactions(tenant_id);
CREATE INDEX idx_inventory_transactions_product ON inventory_transactions(product_id);
CREATE INDEX idx_inventory_transactions_location ON inventory_transactions(location_id);
CREATE INDEX idx_inventory_transactions_type ON inventory_transactions(type);
CREATE INDEX idx_inventory_transactions_reference ON inventory_transactions(reference_type, reference_id);
CREATE INDEX idx_inventory_transactions_created_at ON inventory_transactions(created_at DESC);

-- ==============================================================================
-- FIM - PRODUCTS & INVENTORY
-- ==============================================================================

