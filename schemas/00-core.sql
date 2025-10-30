-- Core tables: users, tenants, user_tenants
-- Created for Docker deployment

-- Users table
CREATE TABLE IF NOT EXISTS users (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    email TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    name TEXT NOT NULL,
    is_active BOOLEAN DEFAULT TRUE,
    email_verified BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW(),
    last_login_at TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);

-- Tenants table
CREATE TABLE IF NOT EXISTS tenants (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    subdomain TEXT UNIQUE NOT NULL,
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_tenants_subdomain ON tenants(subdomain);

-- User-Tenant relationship
CREATE TABLE IF NOT EXISTS user_tenants (
    user_id uuid REFERENCES users(id) ON DELETE CASCADE,
    tenant_id TEXT REFERENCES tenants(id) ON DELETE CASCADE,
    role TEXT NOT NULL CHECK (role IN ('admin', 'member', 'viewer')),
    created_at TIMESTAMP DEFAULT NOW(),
    PRIMARY KEY (user_id, tenant_id)
);

CREATE INDEX IF NOT EXISTS idx_user_tenants_user ON user_tenants(user_id);
CREATE INDEX IF NOT EXISTS idx_user_tenants_tenant ON user_tenants(tenant_id);

-- Insert default tenant
INSERT INTO tenants (id, name, subdomain, is_active, created_at, updated_at)
VALUES ('acme-tenant-001', 'Acme Corporation', 'acme', TRUE, NOW(), NOW())
ON CONFLICT (subdomain) DO NOTHING;

-- Insert test user
INSERT INTO users (id, email, password_hash, name, is_active, email_verified, created_at, updated_at)
VALUES (
    '00000000-0000-0000-0000-000000000099'::uuid,
    'test@test.com',
    '$2a$10$nQ3WvzXQfp0YJ5yJ5H0xE.Xw0rP9Z9Z9Z9Z9Z9Z9Z9Z9Z9Z9Z9Z9Z',  -- Senha@123456
    'Test User',
    TRUE,
    TRUE,
    NOW(),
    NOW()
) ON CONFLICT (email) DO NOTHING;

-- Link test user to acme tenant
INSERT INTO user_tenants (user_id, tenant_id, role, created_at)
VALUES ('00000000-0000-0000-0000-000000000099'::uuid, 'acme-tenant-001', 'admin', NOW())
ON CONFLICT (user_id, tenant_id) DO NOTHING;

