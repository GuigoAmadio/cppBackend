-- ========================================
-- AUDIT LOGS
-- ========================================

CREATE TABLE IF NOT EXISTS audit_logs (
    id BIGSERIAL PRIMARY KEY,
    user_id UUID,
    tenant_id UUID,
    action VARCHAR(100) NOT NULL,
    resource VARCHAR(100) NOT NULL,
    resource_id VARCHAR(100),
    details JSONB,
    ip_address VARCHAR(45),
    user_agent TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_audit_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE SET NULL,
    CONSTRAINT fk_audit_tenant FOREIGN KEY (tenant_id) REFERENCES tenants(id) ON DELETE SET NULL
);

CREATE INDEX IF NOT EXISTS idx_audit_user ON audit_logs(user_id);
CREATE INDEX IF NOT EXISTS idx_audit_tenant ON audit_logs(tenant_id);
CREATE INDEX IF NOT EXISTS idx_audit_action ON audit_logs(action);
CREATE INDEX IF NOT EXISTS idx_audit_timestamp ON audit_logs(created_at DESC);
CREATE INDEX IF NOT EXISTS idx_audit_resource ON audit_logs(resource, resource_id);

COMMENT ON TABLE audit_logs IS 'Log de auditoria para rastreabilidade e compliance (LGPD/GDPR)';
COMMENT ON COLUMN audit_logs.action IS 'Ação executada (ex: USER_LOGIN, USER_DELETED, ROLE_CHANGED)';
COMMENT ON COLUMN audit_logs.resource IS 'Tipo de recurso afetado (ex: user, tenant, role)';
COMMENT ON COLUMN audit_logs.details IS 'Detalhes adicionais em JSON (ex: old_value, new_value)';

