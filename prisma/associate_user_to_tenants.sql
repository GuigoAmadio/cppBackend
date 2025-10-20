-- Associar user finaluser@test.com aos tenants de exemplo
-- Este script é executado DEPOIS do schema_multitenant.sql

DO $$
DECLARE
    v_user_id UUID;
BEGIN
    -- Pegar o ID do user finaluser@test.com (se existir)
    SELECT id INTO v_user_id FROM users WHERE email = 'finaluser@test.com' LIMIT 1;
    
    IF v_user_id IS NOT NULL THEN
        -- Associar ao tenant 'demo' como admin
        INSERT INTO user_tenants (user_id, tenant_id, role)
        VALUES (v_user_id, '00000000-0000-0000-0000-000000000001', 'admin')
        ON CONFLICT (user_id, tenant_id) DO UPDATE
        SET role = EXCLUDED.role, is_active = true;
        
        -- Associar ao tenant 'test' como user
        INSERT INTO user_tenants (user_id, tenant_id, role)
        VALUES (v_user_id, '00000000-0000-0000-0000-000000000002', 'user')
        ON CONFLICT (user_id, tenant_id) DO UPDATE
        SET role = EXCLUDED.role, is_active = true;
        
        RAISE NOTICE 'User % associado aos tenants!', v_user_id;
    ELSE
        RAISE NOTICE 'User finaluser@test.com não encontrado';
    END IF;
END
$$;

