-- ============================================================
-- SCRIPT: Adicionar colunas faltantes na tabela tenants
-- ============================================================

-- 1. Criar ENUM para planos (se não existir)
DO $$ BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'tenant_plan') THEN
        CREATE TYPE tenant_plan AS ENUM ('FREE', 'PRO', 'ENTERPRISE');
    END IF;
END $$;

-- 2. Adicionar colunas faltantes
ALTER TABLE tenants ADD COLUMN IF NOT EXISTS plan tenant_plan DEFAULT 'FREE';
ALTER TABLE tenants ADD COLUMN IF NOT EXISTS max_users INTEGER DEFAULT 5;
ALTER TABLE tenants ADD COLUMN IF NOT EXISTS settings JSONB DEFAULT '{}';

-- 3. Atualizar tenant 'acme' para PRO com 100 usuários
UPDATE tenants SET plan = 'PRO', max_users = 100 WHERE subdomain = 'acme';

-- 4. Adicionar comentários
COMMENT ON COLUMN tenants.plan IS 'Subscription plan of the tenant';
COMMENT ON COLUMN tenants.max_users IS 'Maximum number of users allowed for the tenant';
COMMENT ON COLUMN tenants.settings IS 'JSONB column for tenant-specific settings';

-- 5. Verificar resultado
SELECT id, name, subdomain, plan, max_users, is_active, 
       settings, created_at, updated_at 
FROM tenants;

