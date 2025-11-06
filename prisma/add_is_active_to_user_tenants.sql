-- Migração: Adicionar coluna is_active e updated_at à tabela user_tenants
-- Data: 2024

-- Adicionar coluna is_active com default true para não afetar registros existentes
ALTER TABLE user_tenants
ADD COLUMN IF NOT EXISTS is_active BOOLEAN DEFAULT true;

-- Adicionar coluna updated_at
ALTER TABLE user_tenants
ADD COLUMN IF NOT EXISTS updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP;

-- Adicionar trigger para atualizar updated_at automaticamente
-- (Drop antes de criar para evitar erro se já existir)
DROP TRIGGER IF EXISTS update_user_tenants_updated_at ON user_tenants;
CREATE TRIGGER update_user_tenants_updated_at
BEFORE UPDATE ON user_tenants
FOR EACH ROW
EXECUTE FUNCTION update_updated_at_column();

-- Atualizar registros existentes para ter is_active = true e updated_at
UPDATE user_tenants
SET is_active = true, updated_at = CURRENT_TIMESTAMP
WHERE is_active IS NULL OR updated_at IS NULL;

-- Reindexar para otimizar queries com is_active
CREATE INDEX IF NOT EXISTS idx_user_tenants_user_active ON user_tenants(user_id) WHERE is_active = true;
CREATE INDEX IF NOT EXISTS idx_user_tenants_tenant_active ON user_tenants(tenant_id) WHERE is_active = true;
CREATE INDEX IF NOT EXISTS idx_user_tenants_role_active ON user_tenants(tenant_id, role) WHERE is_active = true;

-- Comentário da coluna
COMMENT ON COLUMN user_tenants.is_active IS 'Indica se o acesso do usuário ao tenant está ativo';
COMMENT ON COLUMN user_tenants.updated_at IS 'Última atualização do registro';

