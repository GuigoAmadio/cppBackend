-- ============================================
-- SCHEMA MULTITENANT - MoneyMaker Backend
-- ============================================
-- Estratégia: Shared Database, Shared Schema
-- Identificação: Subdomain + JWT
-- Múltiplos Tenants: SIM
-- ============================================

-- 1. TABELA DE TENANTS (Organizações/Empresas)
CREATE TABLE IF NOT EXISTS tenants (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name VARCHAR(255) NOT NULL,
    subdomain VARCHAR(100) UNIQUE NOT NULL,
    
    -- Configurações
    is_active BOOLEAN DEFAULT true,
    plan VARCHAR(50) DEFAULT 'free',  -- free, pro, enterprise
    max_users INTEGER DEFAULT 5,
    
    -- Metadados
    settings JSONB DEFAULT '{}',  -- Configurações customizadas do tenant
    
    -- Timestamps
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    -- Índices
    CONSTRAINT subdomain_format CHECK (subdomain ~ '^[a-z0-9-]+$')
);

-- Índice para busca rápida por subdomain
CREATE INDEX IF NOT EXISTS idx_tenants_subdomain ON tenants(subdomain) WHERE is_active = true;

-- 2. TABELA DE USERS (Global - sem tenant_id)
-- Um user pode pertencer a múltiplos tenants
CREATE TABLE IF NOT EXISTS users (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    
    -- Identificação global
    email VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    name VARCHAR(255) NOT NULL,
    
    -- Status
    is_active BOOLEAN DEFAULT true,
    email_verified BOOLEAN DEFAULT false,
    
    -- Timestamps
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_login_at TIMESTAMP,
    
    -- Índices
    CONSTRAINT email_format CHECK (email ~* '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$')
);

-- Índice para busca por email (se não existir)
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email) WHERE is_active = true;

-- 3. TABELA DE RELACIONAMENTO USER-TENANT (Many-to-Many)
-- Define quais users pertencem a quais tenants e com qual role
CREATE TABLE IF NOT EXISTS user_tenants (
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    tenant_id UUID NOT NULL REFERENCES tenants(id) ON DELETE CASCADE,
    
    -- Role específica NESTE tenant
    role VARCHAR(50) NOT NULL DEFAULT 'user',
    
    -- Status
    is_active BOOLEAN DEFAULT true,
    
    -- Timestamps
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    invited_by UUID REFERENCES users(id),
    
    -- Chave primária composta
    PRIMARY KEY (user_id, tenant_id),
    
    -- Constraint de roles válidas
    CONSTRAINT valid_role CHECK (role IN ('owner', 'admin', 'user', 'viewer'))
);

-- Índices para queries comuns
CREATE INDEX IF NOT EXISTS idx_user_tenants_user ON user_tenants(user_id) WHERE is_active = true;
CREATE INDEX IF NOT EXISTS idx_user_tenants_tenant ON user_tenants(tenant_id) WHERE is_active = true;
CREATE INDEX IF NOT EXISTS idx_user_tenants_role ON user_tenants(tenant_id, role) WHERE is_active = true;

-- 4. FUNÇÃO PARA ATUALIZAR updated_at automaticamente
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Triggers para updated_at (drop antes de criar para evitar erros)
DROP TRIGGER IF EXISTS update_tenants_updated_at ON tenants;
CREATE TRIGGER update_tenants_updated_at BEFORE UPDATE ON tenants
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

DROP TRIGGER IF EXISTS update_users_updated_at ON users;
CREATE TRIGGER update_users_updated_at BEFORE UPDATE ON users
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

DROP TRIGGER IF EXISTS update_user_tenants_updated_at ON user_tenants;
CREATE TRIGGER update_user_tenants_updated_at BEFORE UPDATE ON user_tenants
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- ============================================
-- DADOS DE EXEMPLO (APENAS PARA DESENVOLVIMENTO)
-- ============================================

-- Tenant de exemplo 1
INSERT INTO tenants (id, name, subdomain, plan, max_users) VALUES
('00000000-0000-0000-0000-000000000001', 'Demo Company', 'demo', 'pro', 10)
ON CONFLICT (subdomain) DO NOTHING;

-- Tenant de exemplo 2
INSERT INTO tenants (id, name, subdomain, plan, max_users) VALUES
('00000000-0000-0000-0000-000000000002', 'Test Corp', 'test', 'free', 5)
ON CONFLICT (subdomain) DO NOTHING;

-- User de exemplo (já existe do sistema anterior)
-- O user finaluser@test.com será associado aos tenants

-- Associar user aos tenants (precisa do ID do user finaluser@test.com)
-- Isso será feito depois que verificarmos o ID real

-- ============================================
-- VIEWS ÚTEIS
-- ============================================

-- View para ver users com seus tenants
CREATE OR REPLACE VIEW v_user_tenants AS
SELECT 
    u.id as user_id,
    u.email,
    u.name as user_name,
    t.id as tenant_id,
    t.name as tenant_name,
    t.subdomain,
    ut.role,
    ut.is_active as access_active,
    t.is_active as tenant_active
FROM users u
JOIN user_tenants ut ON u.id = ut.user_id
JOIN tenants t ON ut.tenant_id = t.id;

-- View para estatísticas de tenants
CREATE OR REPLACE VIEW v_tenant_stats AS
SELECT 
    t.id,
    t.name,
    t.subdomain,
    t.plan,
    COUNT(DISTINCT ut.user_id) as total_users,
    COUNT(DISTINCT CASE WHEN ut.role = 'admin' THEN ut.user_id END) as admin_users,
    COUNT(DISTINCT CASE WHEN ut.role = 'user' THEN ut.user_id END) as regular_users,
    t.max_users,
    t.is_active,
    t.created_at
FROM tenants t
LEFT JOIN user_tenants ut ON t.id = ut.tenant_id AND ut.is_active = true
GROUP BY t.id;

-- ============================================
-- COMENTÁRIOS DAS TABELAS
-- ============================================

COMMENT ON TABLE tenants IS 'Organizações/Empresas no sistema multitenant';
COMMENT ON TABLE users IS 'Usuários globais - podem pertencer a múltiplos tenants';
COMMENT ON TABLE user_tenants IS 'Relacionamento many-to-many entre users e tenants com role específica';

COMMENT ON COLUMN tenants.subdomain IS 'Subdomínio único para acesso (ex: demo.moneymaker.com)';
COMMENT ON COLUMN tenants.plan IS 'Plano de assinatura: free, pro, enterprise';
COMMENT ON COLUMN tenants.max_users IS 'Número máximo de usuários permitidos no tenant';
COMMENT ON COLUMN user_tenants.role IS 'Role do user NESTE tenant específico: owner, admin, user, viewer';
COMMENT ON COLUMN user_tenants.invited_by IS 'User que convidou este user para o tenant';

-- ============================================
-- FIM DO SCHEMA
-- ============================================

