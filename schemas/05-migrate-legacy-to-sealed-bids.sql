-- ==========================================================
-- Migração de dados: vagas/candidaturas -> leiloes/offers
-- Pressupõe que 04-sealed-bids.sql já foi aplicado
-- Mantemos os mesmos IDs (uuid) quando possível para facilitar mapeamento
-- Porta/local do Postgres não é relevante aqui (rodar via psql -p 5433)
-- ==========================================================

-- 1) Migrar VAGAS -> LEILOES (preservar id = vagas.id)
--    status: aberta->aberto, fechada->fechado, cancelada->cancelado, outros->aberto
INSERT INTO leiloes (
    id, restaurante_id, titulo, descricao, categoria,
    valor_ideal, valor_max_quero,
    data_trabalho, duracao_horas, data_limite_offers,
    vagas_disponiveis, status, created_at, updated_at
)
SELECT
    v.id,
    v.restaurante_id,
    v.titulo,
    v.descricao,
    v.categoria,
    v.preco_oferecido AS valor_ideal,
    v.preco_oferecido AS valor_max_quero,
    v.data_trabalho,
    v.duracao_horas,
    COALESCE(v.data_limite_candidatura, v.data_trabalho - INTERVAL '1 day') AS data_limite_offers,
    v.vagas_disponiveis,
    CASE v.status
        WHEN 'aberta' THEN 'aberto'
        WHEN 'fechada' THEN 'fechado'
        WHEN 'cancelada' THEN 'cancelado'
        ELSE 'aberto'
    END AS status,
    COALESCE(v.created_at, NOW()),
    COALESCE(v.updated_at, NOW())
FROM vagas v
ON CONFLICT (id) DO NOTHING;

-- 2) Migrar CANDIDATURAS -> OFFERS (preservar id = candidaturas.id)
--    valor_oferecido: se candidato propôs preço, usa; senão, usa preco_oferecido da vaga
--    valor_minimo_aceito: se candidato propôs, usa; senão igual ao preço da vaga (comportamento neutro)
INSERT INTO offers (
    id, leilao_id, restaurante_id, freelancer_id,
    mensagem, shortlisted, valor_oferecido, valor_minimo_aceito,
    status, submitted_at, responded_at
)
SELECT
    c.id,
    c.vaga_id AS leilao_id,
    v.restaurante_id,
    c.freelancer_id,
    c.mensagem,
    FALSE AS shortlisted,
    COALESCE(c.preco_proposto, v.preco_oferecido) AS valor_oferecido,
    COALESCE(c.preco_proposto, v.preco_oferecido) AS valor_minimo_aceito,
    CASE c.status
        WHEN 'pendente'  THEN 'pendente'
        WHEN 'aceita'    THEN 'aceita'
        WHEN 'recusada'  THEN 'rejeitada'
        WHEN 'cancelada' THEN 'cancelada'
        ELSE 'pendente'
    END AS status,
    COALESCE(c.submitted_at, NOW()),
    c.responded_at
FROM candidaturas c
JOIN vagas v ON v.id = c.vaga_id
ON CONFLICT (id) DO NOTHING;

-- 3) Opcional: desativar criação nas tabelas antigas (via regra/permissionamento) após migração validada
--    Aqui somente documentamos; execução fica a cargo do rollout.


