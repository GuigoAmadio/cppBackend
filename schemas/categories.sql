-- ==============================================================================
-- PRODUCT CATEGORIES MODULE
-- ==============================================================================

DROP TABLE IF EXISTS product_categories CASCADE;

CREATE TABLE product_categories (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    tenant_id TEXT NOT NULL,
    parent_id TEXT,  -- Categoria pai (para hierarquia)
    name VARCHAR(200) NOT NULL,
    slug VARCHAR(200) NOT NULL,
    description TEXT,
    image_url VARCHAR(500),
    display_order INTEGER DEFAULT 0,
    status VARCHAR(50) NOT NULL DEFAULT 'active',  -- active, inactive, archived
    created_by TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    -- Constraints
    UNIQUE(tenant_id, slug),
    FOREIGN KEY (parent_id) REFERENCES product_categories(id) ON DELETE SET NULL
);

-- Indexes
CREATE INDEX idx_categories_tenant ON product_categories(tenant_id);
CREATE INDEX idx_categories_parent ON product_categories(parent_id);
CREATE INDEX idx_categories_slug ON product_categories(slug);
CREATE INDEX idx_categories_status ON product_categories(status);
CREATE INDEX idx_categories_display_order ON product_categories(display_order);
CREATE INDEX idx_categories_created_at ON product_categories(created_at DESC);

-- Agora podemos adicionar a foreign key em products!
ALTER TABLE products 
ADD CONSTRAINT products_category_id_fkey 
FOREIGN KEY (category_id) REFERENCES product_categories(id) ON DELETE SET NULL;

-- ==============================================================================
-- FIM - CATEGORIES
-- ==============================================================================

