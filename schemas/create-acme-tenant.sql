-- Script para criar o tenant "acme" no banco de dados
-- Execute este script no PostgreSQL

-- 1. Inserir o tenant
INSERT INTO tenants (id, name, subdomain, plan, is_active, created_at, updated_at)
VALUES (
    gen_random_uuid(),
    'Acme Corporation',
    'acme',
    'premium',
    true,
    NOW(),
    NOW()
)
ON CONFLICT (subdomain) DO NOTHING;

-- 2. Verificar se foi criado
SELECT id, name, subdomain, plan, is_active, created_at 
FROM tenants 
WHERE subdomain = 'acme';

-- 3. (Opcional) Criar um usuário owner para o tenant acme
-- Primeiro, buscar o tenant_id
DO $$
DECLARE
    v_tenant_id UUID;
    v_user_id UUID;
BEGIN
    -- Pegar o ID do tenant acme
    SELECT id INTO v_tenant_id FROM tenants WHERE subdomain = 'acme';
    
    IF v_tenant_id IS NOT NULL THEN
        -- Criar usuário (senha: SecurePass123! -> hash bcrypt)
        INSERT INTO users (id, name, email, password_hash, email_verified, is_active, created_at, updated_at)
        VALUES (
            gen_random_uuid(),
            'Admin Acme',
            'admin@acme.com',
            '$2a$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyYIB.KPq7wS', -- SecurePass123!
            true,
            true,
            NOW(),
            NOW()
        )
        ON CONFLICT (email) DO NOTHING
        RETURNING id INTO v_user_id;
        
        -- Se o usuário foi criado, adicionar ao tenant como owner
        IF v_user_id IS NOT NULL THEN
            INSERT INTO user_tenants (user_id, tenant_id, role, joined_at)
            VALUES (v_user_id, v_tenant_id, 'owner', NOW())
            ON CONFLICT (user_id, tenant_id) DO NOTHING;
            
            RAISE NOTICE 'Tenant "acme" e usuário owner criados com sucesso!';
            RAISE NOTICE 'Login: admin@acme.com';
            RAISE NOTICE 'Senha: SecurePass123!';
        ELSE
            RAISE NOTICE 'Usuário admin@acme.com já existe';
        END IF;
    ELSE
        RAISE NOTICE 'Tenant "acme" não foi criado';
    END IF;
END $$;

