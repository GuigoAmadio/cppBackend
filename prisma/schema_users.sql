-- ===================================================================
-- Schema SQL - Users Table
-- ===================================================================
-- Tabela de usuários com suporte a autenticação JWT
-- ===================================================================

-- Extensão para UUID (se ainda não existe)
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Tabela de usuários
CREATE TABLE IF NOT EXISTS users (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    email VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    name VARCHAR(255) NOT NULL,
    role VARCHAR(50) DEFAULT 'user' NOT NULL,
    is_active BOOLEAN DEFAULT true NOT NULL,
    email_verified BOOLEAN DEFAULT false NOT NULL,
    created_at TIMESTAMP DEFAULT NOW() NOT NULL,
    updated_at TIMESTAMP DEFAULT NOW() NOT NULL,
    last_login_at TIMESTAMP,
    
    -- Constraints
    CONSTRAINT email_format CHECK (email ~* '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$'),
    CONSTRAINT valid_role CHECK (role IN ('user', 'admin', 'moderator'))
);

-- Índices para performance
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
CREATE INDEX IF NOT EXISTS idx_users_role ON users(role);
CREATE INDEX IF NOT EXISTS idx_users_created_at ON users(created_at DESC);
CREATE INDEX IF NOT EXISTS idx_users_active ON users(is_active) WHERE is_active = true;

-- Trigger para atualizar updated_at automaticamente
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ language 'plpgsql';

CREATE TRIGGER update_users_updated_at BEFORE UPDATE ON users
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Dados de exemplo (opcional - para testes)
-- Password: "admin123" (bcrypt hash - você precisará gerar isso)
-- INSERT INTO users (email, password_hash, name, role) VALUES
-- ('admin@example.com', '$2a$12$exemplo_hash_aqui', 'Admin User', 'admin'),
-- ('user@example.com', '$2a$12$exemplo_hash_aqui', 'Regular User', 'user');

-- Comentários
COMMENT ON TABLE users IS 'Tabela de usuários do sistema';
COMMENT ON COLUMN users.id IS 'UUID único do usuário';
COMMENT ON COLUMN users.email IS 'Email único do usuário (usado para login)';
COMMENT ON COLUMN users.password_hash IS 'Hash bcrypt da senha';
COMMENT ON COLUMN users.role IS 'Papel do usuário (user, admin, moderator)';
COMMENT ON COLUMN users.is_active IS 'Usuário ativo ou bloqueado';
COMMENT ON COLUMN users.email_verified IS 'Email foi verificado';

