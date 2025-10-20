-- ========================================
-- EMAIL VERIFICATION & PASSWORD RESET
-- ========================================

-- Tabela para tokens de verificação de email
CREATE TABLE IF NOT EXISTS email_verification_tokens (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL,
    token VARCHAR(255) NOT NULL UNIQUE,
    expires_at TIMESTAMP NOT NULL,
    used_at TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_email_verify_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_email_verification_token ON email_verification_tokens(token);
CREATE INDEX IF NOT EXISTS idx_email_verification_user ON email_verification_tokens(user_id);
CREATE INDEX IF NOT EXISTS idx_email_verification_expires ON email_verification_tokens(expires_at);

-- Tabela para tokens de reset de senha
CREATE TABLE IF NOT EXISTS password_reset_tokens (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL,
    token VARCHAR(255) NOT NULL UNIQUE,
    expires_at TIMESTAMP NOT NULL,
    used_at TIMESTAMP,
    ip_address VARCHAR(45),
    user_agent TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_password_reset_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_password_reset_token ON password_reset_tokens(token);
CREATE INDEX IF NOT EXISTS idx_password_reset_user ON password_reset_tokens(user_id);
CREATE INDEX IF NOT EXISTS idx_password_reset_expires ON password_reset_tokens(expires_at);

-- Comentários
COMMENT ON TABLE email_verification_tokens IS 'Tokens para verificação de email (expira em 24h)';
COMMENT ON TABLE password_reset_tokens IS 'Tokens para reset de senha (expira em 1h)';

COMMENT ON COLUMN email_verification_tokens.token IS 'Token único UUID v4';
COMMENT ON COLUMN email_verification_tokens.used_at IS 'Timestamp de quando o token foi usado (null = ainda não usado)';

COMMENT ON COLUMN password_reset_tokens.token IS 'Token único UUID v4';
COMMENT ON COLUMN password_reset_tokens.ip_address IS 'IP de onde o reset foi solicitado (segurança)';
COMMENT ON COLUMN password_reset_tokens.used_at IS 'Timestamp de quando o token foi usado (null = ainda não usado)';

