-- Atualizar tabela tenants para incluir campos faltantes
-- Plan, max_users, settings

-- 1. Criar tipo ENUM para plano
DO $$ BEGIN
    CREATE TYPE tenant_plan AS ENUM ('FREE', 'PRO', 'ENTERPRISE');
EXCEPTION
    WHEN duplicate_object THEN null;
END $$;

-- 2. Adicionar colunas faltantes
ALTER TABLE tenants 
ADD COLUMN IF NOT EXISTS plan tenant_plan DEFAULT 'FREE',
ADD COLUMN IF NOT EXISTS max_users INTEGER DEFAULT 5,
ADD COLUMN IF NOT EXISTS settings JSONB DEFAULT '{}'::jsonb;

-- 3. Atualizar tenant existente (acme) para usar plan PRO
UPDATE tenants 
SET plan = 'PRO', max_users = 100, settings = '{"features": ["all"]}'::jsonb
WHERE subdomain = 'acme';

-- 4. Comentários
COMMENT ON COLUMN tenants.plan IS 'Plano de assinatura: FREE (5 usuários), PRO (100 usuários), ENTERPRISE (ilimitado)';
COMMENT ON COLUMN tenants.max_users IS 'Número máximo de usuários permitidos no tenant';
COMMENT ON COLUMN tenants.settings IS 'Configurações customizadas do tenant em formato JSON';


