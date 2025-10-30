-- ==========================================================
-- Sealed-Bids (Leilões & Offers) - Migração Inicial
-- Data: 2025-10-30
-- ==========================================================

-- Tabela: leiloes
CREATE TABLE IF NOT EXISTS leiloes (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    restaurante_id uuid NOT NULL REFERENCES restaurante_profiles(id) ON DELETE CASCADE,
    titulo TEXT NOT NULL,
    descricao TEXT,
    categoria TEXT NOT NULL,
    valor_ideal DECIMAL(10,2) NOT NULL,
    valor_max_quero DECIMAL(10,2) NOT NULL,
    preferencias_restricoes JSONB,
    data_trabalho TIMESTAMP NOT NULL,
    duracao_horas DECIMAL(4,2),
    data_limite_offers TIMESTAMP NOT NULL,
    vagas_disponiveis INTEGER DEFAULT 1 CHECK (vagas_disponiveis > 0),
    status TEXT CHECK (status IN ('aberto','fechado','cancelado','concluido')) DEFAULT 'aberto',
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW(),
    CHECK (valor_ideal <= valor_max_quero),
    CHECK (data_limite_offers < data_trabalho)
);

CREATE INDEX IF NOT EXISTS idx_leiloes_rest ON leiloes(restaurante_id);
CREATE INDEX IF NOT EXISTS idx_leiloes_status_data ON leiloes(status, data_trabalho);

-- Tabela: offers (nova, preservando candidaturas antigas durante migração)
CREATE TABLE IF NOT EXISTS offers (
    id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    leilao_id uuid REFERENCES leiloes(id) ON DELETE CASCADE,
    restaurante_id uuid NOT NULL REFERENCES restaurante_profiles(id) ON DELETE CASCADE,
    freelancer_id uuid NOT NULL REFERENCES freelancer_profiles(id) ON DELETE CASCADE,
    mensagem TEXT,
    shortlisted BOOLEAN DEFAULT FALSE,
    valor_oferecido DECIMAL(10,2) NOT NULL,
    valor_minimo_aceito DECIMAL(10,2) NOT NULL,
    status TEXT CHECK (status IN ('pendente','aceita','rejeitada','cancelada')) DEFAULT 'pendente',
    submitted_at TIMESTAMP DEFAULT NOW(),
    responded_at TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_offers_leilao ON offers(leilao_id);
CREATE INDEX IF NOT EXISTS idx_offers_rest ON offers(restaurante_id);
CREATE INDEX IF NOT EXISTS idx_offers_freelancer ON offers(freelancer_id);
CREATE INDEX IF NOT EXISTS idx_offers_status ON offers(status);

-- Índice único parcial para garantir 1 offer por freelancer por leilão
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM pg_indexes 
        WHERE schemaname = ANY (current_schemas(true))
          AND indexname = 'offers_unique_per_leilao_idx'
    ) THEN
        EXECUTE 'CREATE UNIQUE INDEX offers_unique_per_leilao_idx ON offers(leilao_id, freelancer_id) WHERE leilao_id IS NOT NULL';
    END IF;
END
$$;

-- Ajuste: flag para receber offers nao solicitadas
ALTER TABLE restaurante_profiles ADD COLUMN IF NOT EXISTS is_open BOOLEAN DEFAULT FALSE;
CREATE INDEX IF NOT EXISTS idx_restaurante_is_open ON restaurante_profiles(is_open) WHERE is_open = true;

-- Job agora pode referenciar leilao (opcional)
ALTER TABLE jobs_agreed ADD COLUMN IF NOT EXISTS leilao_id uuid REFERENCES leiloes(id);

-- Trigger de updated_at para leiloes
CREATE OR REPLACE FUNCTION update_leilao_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS trigger_leilao_updated_at ON leiloes;
CREATE TRIGGER trigger_leilao_updated_at
BEFORE UPDATE ON leiloes
FOR EACH ROW
EXECUTE FUNCTION update_leilao_updated_at();


