-- ==========================================
-- FREELANCER APP - SCHEMAS
-- ==========================================
-- Ordem de criação:
-- 1. Estender tabela users
-- 2. Perfis (restaurantes, freelancers)
-- 3. Vagas
-- 4. Candidaturas
-- 5. Jobs Agreed
-- 6. Avaliações
-- ==========================================

-- ==========================================
-- 1. ESTENDER TABELA USERS
-- ==========================================
ALTER TABLE users ADD COLUMN IF NOT EXISTS score DECIMAL(3,2) DEFAULT 0 CHECK (score >= 0 AND score <= 5);
ALTER TABLE users ADD COLUMN IF NOT EXISTS phone TEXT;
ALTER TABLE users ADD COLUMN IF NOT EXISTS avatar TEXT;

-- ==========================================
-- 2. MÓDULO: RESTAURANTES (Company Profiles)
-- ==========================================
CREATE TABLE IF NOT EXISTS restaurante_profiles (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id uuid UNIQUE NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    cnpj TEXT UNIQUE NOT NULL,
    razao_social TEXT,
    nome_fantasia TEXT,
    descricao TEXT,
    categoria TEXT,                              -- restaurante, bar, hotel, cafeteria
    logo TEXT,
    latitude DECIMAL(10,8) NOT NULL,
    longitude DECIMAL(11,8) NOT NULL,
    endereco_completo TEXT,
    cep TEXT,
    cidade TEXT,
    estado TEXT,
    horario_funcionamento JSONB,                 -- {"seg": "08:00-18:00", "ter": "08:00-18:00"}
    documentos_verificados BOOLEAN DEFAULT FALSE,
    ativo BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Índices para performance
CREATE INDEX IF NOT EXISTS idx_restaurante_user ON restaurante_profiles(user_id);
CREATE INDEX IF NOT EXISTS idx_restaurante_location ON restaurante_profiles USING GIST (point(latitude, longitude));
CREATE INDEX IF NOT EXISTS idx_restaurante_categoria ON restaurante_profiles(categoria);
CREATE INDEX IF NOT EXISTS idx_restaurante_cidade ON restaurante_profiles(cidade);
CREATE INDEX IF NOT EXISTS idx_restaurante_ativo ON restaurante_profiles(ativo);

-- Trigger para updated_at
CREATE OR REPLACE FUNCTION update_restaurante_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_restaurante_updated_at
BEFORE UPDATE ON restaurante_profiles
FOR EACH ROW
EXECUTE FUNCTION update_restaurante_updated_at();

-- ==========================================
-- 3. MÓDULO: FREELANCES (Freelancer Profiles)
-- ==========================================
CREATE TABLE IF NOT EXISTS freelancer_profiles (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id uuid UNIQUE NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    especialidades JSONB NOT NULL,               -- ["garcom", "cozinheiro", "barman"]
    experiencia_anos INTEGER,
    preco_minimo DECIMAL(10,2),
    cpf TEXT UNIQUE,
    latitude DECIMAL(10,8),
    longitude DECIMAL(11,8),
    raio_atuacao_km DECIMAL(5,2) DEFAULT 10,
    disponibilidade JSONB,                       -- {"seg": ["08:00-12:00", "14:00-18:00"]}
    documentos_verificados BOOLEAN DEFAULT FALSE,
    ativo BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Índices para performance
CREATE INDEX IF NOT EXISTS idx_freelancer_user ON freelancer_profiles(user_id);
CREATE INDEX IF NOT EXISTS idx_freelancer_location ON freelancer_profiles USING GIST (point(latitude, longitude));
CREATE INDEX IF NOT EXISTS idx_freelancer_especialidades ON freelancer_profiles USING GIN (especialidades);
CREATE INDEX IF NOT EXISTS idx_freelancer_ativo ON freelancer_profiles(ativo);

-- Trigger para updated_at
CREATE OR REPLACE FUNCTION update_freelancer_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_freelancer_updated_at
BEFORE UPDATE ON freelancer_profiles
FOR EACH ROW
EXECUTE FUNCTION update_freelancer_updated_at();

-- ==========================================
-- 4. MÓDULO: VAGAS (Job Postings)
-- ==========================================
CREATE TABLE IF NOT EXISTS vagas (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    restaurante_id uuid NOT NULL REFERENCES restaurante_profiles(id) ON DELETE CASCADE,
    titulo TEXT NOT NULL,
    descricao TEXT,
    categoria TEXT NOT NULL,                     -- garcom, cozinheiro, auxiliar, barman
    preco_oferecido DECIMAL(10,2) NOT NULL,
    data_trabalho TIMESTAMP NOT NULL,
    horario_inicio TIME,
    horario_fim TIME,
    duracao_horas DECIMAL(4,2),
    data_limite_candidatura TIMESTAMP,
    requisitos JSONB,                            -- {"experiencia_minima": 2, "habilidades": ["atendimento"]}
    vagas_disponiveis INTEGER DEFAULT 1 CHECK (vagas_disponiveis > 0),
    vagas_preenchidas INTEGER DEFAULT 0 CHECK (vagas_preenchidas >= 0),
    status TEXT CHECK (status IN ('aberta', 'fechada', 'cancelada')) DEFAULT 'aberta',
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW(),
    CHECK (vagas_preenchidas <= vagas_disponiveis)
);

-- Índices para performance
CREATE INDEX IF NOT EXISTS idx_vaga_restaurante ON vagas(restaurante_id);
CREATE INDEX IF NOT EXISTS idx_vaga_status ON vagas(status);
CREATE INDEX IF NOT EXISTS idx_vaga_data ON vagas(data_trabalho);
CREATE INDEX IF NOT EXISTS idx_vaga_categoria ON vagas(categoria);
CREATE INDEX IF NOT EXISTS idx_vaga_status_data ON vagas(status, data_trabalho);

-- Trigger para updated_at
CREATE OR REPLACE FUNCTION update_vaga_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_vaga_updated_at
BEFORE UPDATE ON vagas
FOR EACH ROW
EXECUTE FUNCTION update_vaga_updated_at();

-- ==========================================
-- 5. MÓDULO: CANDIDATURAS (Applications)
-- ==========================================
CREATE TABLE IF NOT EXISTS candidaturas (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    vaga_id uuid NOT NULL REFERENCES vagas(id) ON DELETE CASCADE,
    freelancer_id uuid NOT NULL REFERENCES freelancer_profiles(id) ON DELETE CASCADE,
    mensagem TEXT,
    preco_proposto DECIMAL(10,2),
    status TEXT CHECK (status IN ('pendente', 'aceita', 'recusada', 'cancelada')) DEFAULT 'pendente',
    submitted_at TIMESTAMP DEFAULT NOW(),
    responded_at TIMESTAMP,
    response_message TEXT,
    UNIQUE(vaga_id, freelancer_id)
);

-- Índices para performance
CREATE INDEX IF NOT EXISTS idx_candidatura_vaga ON candidaturas(vaga_id);
CREATE INDEX IF NOT EXISTS idx_candidatura_freelancer ON candidaturas(freelancer_id);
CREATE INDEX IF NOT EXISTS idx_candidatura_status ON candidaturas(status);
CREATE INDEX IF NOT EXISTS idx_candidatura_vaga_status ON candidaturas(vaga_id, status);

-- ==========================================
-- 6. MÓDULO: JOBS AGREED (Trabalhos Confirmados)
-- ==========================================
CREATE TABLE IF NOT EXISTS jobs_agreed (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    vaga_id uuid NOT NULL REFERENCES vagas(id),
    candidatura_id uuid NOT NULL REFERENCES candidaturas(id),
    freelancer_id uuid NOT NULL REFERENCES freelancer_profiles(id),
    restaurante_id uuid NOT NULL REFERENCES restaurante_profiles(id),
    status TEXT CHECK (status IN ('confirmado', 'em_andamento', 'concluido', 'cancelado')) DEFAULT 'confirmado',
    preco_acordado DECIMAL(10,2) NOT NULL,
    data_trabalho TIMESTAMP NOT NULL,
    horario_inicio TIME,
    horario_fim TIME,
    data_inicio_real TIMESTAMP,
    data_conclusao_real TIMESTAMP,
    observacoes TEXT,
    cancelado_por TEXT,                          -- 'restaurante' ou 'freelancer'
    motivo_cancelamento TEXT,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Índices para performance
CREATE INDEX IF NOT EXISTS idx_job_freelancer ON jobs_agreed(freelancer_id);
CREATE INDEX IF NOT EXISTS idx_job_restaurante ON jobs_agreed(restaurante_id);
CREATE INDEX IF NOT EXISTS idx_job_status ON jobs_agreed(status);
CREATE INDEX IF NOT EXISTS idx_job_data ON jobs_agreed(data_trabalho);
CREATE INDEX IF NOT EXISTS idx_job_candidatura ON jobs_agreed(candidatura_id);

-- Trigger para updated_at
CREATE OR REPLACE FUNCTION update_job_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_job_updated_at
BEFORE UPDATE ON jobs_agreed
FOR EACH ROW
EXECUTE FUNCTION update_job_updated_at();

-- ==========================================
-- 7. MÓDULO: AVALIAÇÕES (Ratings)
-- ==========================================
CREATE TABLE IF NOT EXISTS avaliacoes (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    job_id uuid NOT NULL REFERENCES jobs_agreed(id) ON DELETE CASCADE,
    rater_id uuid NOT NULL REFERENCES users(id),      -- Quem avaliou
    rated_id uuid NOT NULL REFERENCES users(id),      -- Quem foi avaliado
    score DECIMAL(2,1) CHECK (score BETWEEN 1 AND 5) NOT NULL,
    comentario TEXT,
    created_at TIMESTAMP DEFAULT NOW(),
    UNIQUE(job_id, rater_id)
);

-- Índices para performance
CREATE INDEX IF NOT EXISTS idx_avaliacao_rated ON avaliacoes(rated_id);
CREATE INDEX IF NOT EXISTS idx_avaliacao_job ON avaliacoes(job_id);
CREATE INDEX IF NOT EXISTS idx_avaliacao_rater ON avaliacoes(rater_id);

-- Trigger para atualizar score do usuário automaticamente
CREATE OR REPLACE FUNCTION update_user_score()
RETURNS TRIGGER AS $$
BEGIN
    UPDATE users
    SET score = (
        SELECT AVG(score)
        FROM avaliacoes
        WHERE rated_id = NEW.rated_id
    )
    WHERE id = NEW.rated_id;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_update_user_score
AFTER INSERT ON avaliacoes
FOR EACH ROW
EXECUTE FUNCTION update_user_score();

-- ==========================================
-- FIM DO SCHEMA FREELANCER APP
-- ==========================================

