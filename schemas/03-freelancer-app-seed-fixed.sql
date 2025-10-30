-- ==========================================
-- FREELANCER APP - SEED DATA (FIXED)
-- Dados de teste para desenvolvimento
-- ==========================================

-- LIMPAR DADOS EXISTENTES (CUIDADO EM PRODUCAO!)
-- ==========================================
DELETE FROM avaliacoes;
DELETE FROM jobs_agreed;
DELETE FROM candidaturas;
DELETE FROM vagas;
DELETE FROM freelancer_profiles;
DELETE FROM restaurante_profiles;
DELETE FROM users WHERE email LIKE '%@test.com' OR email LIKE '%@exemplo.com' OR email LIKE '%@%.com.br';

-- ==========================================
-- 1. USERS + PROFILES (com CTEs para capturar IDs)
-- ==========================================

-- FREELANCERS
WITH new_users AS (
    INSERT INTO users (id, email, password_hash, name, is_active, email_verified, score, phone, avatar) VALUES
    (gen_random_uuid(), 'joao.silva@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Joao Silva', TRUE, TRUE, 4.8, '11987654321', 'https://i.pravatar.cc/150?img=1'),
    (gen_random_uuid(), 'maria.santos@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Maria Santos', TRUE, TRUE, 4.9, '11987654322', 'https://i.pravatar.cc/150?img=2'),
    (gen_random_uuid(), 'pedro.oliveira@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Pedro Oliveira', TRUE, TRUE, 4.5, '11987654323', 'https://i.pravatar.cc/150?img=3'),
    (gen_random_uuid(), 'ana.costa@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Ana Costa', TRUE, TRUE, 4.7, '11987654324', 'https://i.pravatar.cc/150?img=4'),
    (gen_random_uuid(), 'carlos.souza@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Carlos Souza', TRUE, TRUE, 4.6, '11987654325', 'https://i.pravatar.cc/150?img=5'),
    (gen_random_uuid(), 'juliana.lima@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Juliana Lima', TRUE, TRUE, 4.9, '11987654326', 'https://i.pravatar.cc/150?img=6'),
    (gen_random_uuid(), 'rafael.alves@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Rafael Alves', TRUE, TRUE, 4.3, '11987654327', 'https://i.pravatar.cc/150?img=7'),
    (gen_random_uuid(), 'fernanda.rocha@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Fernanda Rocha', TRUE, TRUE, 4.8, '11987654328', 'https://i.pravatar.cc/150?img=8'),
    (gen_random_uuid(), 'lucas.martins@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Lucas Martins', TRUE, TRUE, 4.4, '11987654329', 'https://i.pravatar.cc/150?img=9'),
    (gen_random_uuid(), 'camila.ferreira@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Camila Ferreira', TRUE, TRUE, 4.7, '11987654330', 'https://i.pravatar.cc/150?img=10'),
    (gen_random_uuid(), 'bruno.araujo@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Bruno Araujo', TRUE, TRUE, 4.5, '11987654331', 'https://i.pravatar.cc/150?img=11'),
    (gen_random_uuid(), 'patricia.barbosa@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Patricia Barbosa', TRUE, TRUE, 4.6, '11987654332', 'https://i.pravatar.cc/150?img=12'),
    (gen_random_uuid(), 'rodrigo.carvalho@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Rodrigo Carvalho', TRUE, TRUE, 4.9, '11987654333', 'https://i.pravatar.cc/150?img=13'),
    (gen_random_uuid(), 'aline.ribeiro@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Aline Ribeiro', TRUE, TRUE, 4.2, '11987654334', 'https://i.pravatar.cc/150?img=14'),
    (gen_random_uuid(), 'gustavo.castro@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Gustavo Castro', TRUE, TRUE, 4.8, '11987654335', 'https://i.pravatar.cc/150?img=15'),
    (gen_random_uuid(), 'mariana.dias@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Mariana Dias', TRUE, TRUE, 4.7, '11987654336', 'https://i.pravatar.cc/150?img=16'),
    (gen_random_uuid(), 'thiago.gomes@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Thiago Gomes', TRUE, TRUE, 4.4, '11987654337', 'https://i.pravatar.cc/150?img=17'),
    (gen_random_uuid(), 'leticia.moura@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Leticia Moura', TRUE, TRUE, 4.6, '11987654338', 'https://i.pravatar.cc/150?img=18'),
    (gen_random_uuid(), 'daniel.farias@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Daniel Farias', TRUE, TRUE, 4.5, '11987654339', 'https://i.pravatar.cc/150?img=19'),
    (gen_random_uuid(), 'isabela.monteiro@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Isabela Monteiro', TRUE, TRUE, 4.9, '11987654340', 'https://i.pravatar.cc/150?img=20')
    RETURNING id, email
)
INSERT INTO freelancer_profiles (id, user_id, especialidades, experiencia_anos, preco_minimo, cpf, latitude, longitude, raio_atuacao_km, disponibilidade, documentos_verificados, ativo)
SELECT 
    gen_random_uuid(), 
    id,
    CASE 
        WHEN email = 'joao.silva@test.com' THEN '["garcom", "barman"]'::jsonb
        WHEN email = 'maria.santos@test.com' THEN '["cozinheiro", "confeiteiro"]'::jsonb
        WHEN email = 'pedro.oliveira@test.com' THEN '["garcom"]'::jsonb
        WHEN email = 'ana.costa@test.com' THEN '["auxiliar_cozinha", "garcom"]'::jsonb
        WHEN email = 'carlos.souza@test.com' THEN '["barman", "sommelier"]'::jsonb
        WHEN email = 'juliana.lima@test.com' THEN '["cozinheiro", "chef"]'::jsonb
        WHEN email = 'rafael.alves@test.com' THEN '["garcom", "auxiliar_cozinha"]'::jsonb
        WHEN email = 'fernanda.rocha@test.com' THEN '["confeiteiro", "padeiro"]'::jsonb
        WHEN email = 'lucas.martins@test.com' THEN '["garcom"]'::jsonb
        WHEN email = 'camila.ferreira@test.com' THEN '["barman", "garcom"]'::jsonb
        WHEN email = 'bruno.araujo@test.com' THEN '["cozinheiro"]'::jsonb
        WHEN email = 'patricia.barbosa@test.com' THEN '["garcom", "maitre"]'::jsonb
        WHEN email = 'rodrigo.carvalho@test.com' THEN '["auxiliar_cozinha"]'::jsonb
        WHEN email = 'aline.ribeiro@test.com' THEN '["garcom"]'::jsonb
        WHEN email = 'gustavo.castro@test.com' THEN '["cozinheiro", "auxiliar_cozinha"]'::jsonb
        WHEN email = 'mariana.dias@test.com' THEN '["barman"]'::jsonb
        WHEN email = 'thiago.gomes@test.com' THEN '["garcom", "barman"]'::jsonb
        WHEN email = 'leticia.moura@test.com' THEN '["cozinheiro"]'::jsonb
        WHEN email = 'daniel.farias@test.com' THEN '["auxiliar_cozinha", "garcom"]'::jsonb
        ELSE '["confeiteiro"]'::jsonb
    END,
    CASE 
        WHEN email LIKE '%joao%' THEN 5
        WHEN email LIKE '%maria%' THEN 8
        WHEN email LIKE '%juliana%' THEN 12
        WHEN email LIKE '%carlos%' THEN 10
        ELSE 3
    END,
    CASE 
        WHEN email LIKE '%joao%' THEN 80.00
        WHEN email LIKE '%maria%' THEN 120.00
        WHEN email LIKE '%juliana%' THEN 200.00
        WHEN email LIKE '%carlos%' THEN 150.00
        ELSE 60.00
    END,
    '12345678' || LPAD((ROW_NUMBER() OVER())::text, 3, '0'),
    -23.5505 + (RANDOM() * 0.02 - 0.01),
    -46.6333 + (RANDOM() * 0.02 - 0.01),
    10 + (RANDOM() * 15)::int,
    '{"seg": ["08:00-18:00"], "ter": ["08:00-18:00"]}'::jsonb,
    email NOT LIKE '%rafael%' AND email NOT LIKE '%aline%',
    TRUE
FROM new_users;

-- RESTAURANTES
WITH new_rest_users AS (
    INSERT INTO users (id, email, password_hash, name, is_active, email_verified, score, phone, avatar) VALUES
    (gen_random_uuid(), 'contato@pizzariadonna.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Pizzaria Donna Bella', TRUE, TRUE, 4.7, '1132221111', 'https://i.pravatar.cc/150?img=51'),
    (gen_random_uuid(), 'contato@cantinaitalia.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Cantina Italia', TRUE, TRUE, 4.8, '1132221112', 'https://i.pravatar.cc/150?img=52'),
    (gen_random_uuid(), 'contato@sushihouse.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Sushi House', TRUE, TRUE, 4.9, '1132221113', 'https://i.pravatar.cc/150?img=53'),
    (gen_random_uuid(), 'contato@churrascariagarucho.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Churrascaria Gaucho', TRUE, TRUE, 4.6, '1132221114', 'https://i.pravatar.cc/150?img=54'),
    (gen_random_uuid(), 'contato@bistrofrances.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Bistro Francais', TRUE, TRUE, 4.8, '1132221115', 'https://i.pravatar.cc/150?img=55'),
    (gen_random_uuid(), 'contato@bardajapa.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Bar da Japa', TRUE, TRUE, 4.5, '1132221116', 'https://i.pravatar.cc/150?img=56'),
    (gen_random_uuid(), 'contato@restaurantevegetariano.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Restaurante Vegetariano Verde Vida', TRUE, TRUE, 4.7, '1132221117', 'https://i.pravatar.cc/150?img=57'),
    (gen_random_uuid(), 'contato@hamburgueriaartesanal.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Hamburgueria Artesanal Burger Lab', TRUE, TRUE, 4.8, '1132221118', 'https://i.pravatar.cc/150?img=58'),
    (gen_random_uuid(), 'contato@cafeteriaroma.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Cafeteria Aroma', TRUE, TRUE, 4.6, '1132221119', 'https://i.pravatar.cc/150?img=59'),
    (gen_random_uuid(), 'contato@taqueriaole.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Taqueria Ole', TRUE, TRUE, 4.7, '1132221120', 'https://i.pravatar.cc/150?img=60'),
    (gen_random_uuid(), 'contato@massasdelicia.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Massas Delicia', TRUE, TRUE, 4.5, '1132221121', 'https://i.pravatar.cc/150?img=61'),
    (gen_random_uuid(), 'contato@chopariacentral.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Choparia Central', TRUE, TRUE, 4.8, '1132221122', 'https://i.pravatar.cc/150?img=62'),
    (gen_random_uuid(), 'contato@doceriadavovo.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Doceria da Vovo', TRUE, TRUE, 4.9, '1132221123', 'https://i.pravatar.cc/150?img=63'),
    (gen_random_uuid(), 'contato@restaurantepeixaria.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Restaurante Peixaria', TRUE, TRUE, 4.6, '1132221124', 'https://i.pravatar.cc/150?img=64'),
    (gen_random_uuid(), 'contato@lanchonetedobairro.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Lanchonete do Bairro', TRUE, TRUE, 4.4, '1132221125', 'https://i.pravatar.cc/150?img=65')
    RETURNING id, email, name
)
INSERT INTO restaurante_profiles (id, user_id, cnpj, razao_social, nome_fantasia, descricao, categoria, logo, latitude, longitude, endereco_completo, cep, cidade, estado, horario_funcionamento, documentos_verificados, ativo)
SELECT 
    gen_random_uuid(),
    id,
    '12345678000' || LPAD((ROW_NUMBER() OVER())::text, 3, '0'),
    name || ' Ltda',
    name,
    CASE 
        WHEN email LIKE '%pizza%' THEN 'Pizzas artesanais no forno a lenha'
        WHEN email LIKE '%cantina%' THEN 'Comida italiana tradicional'
        WHEN email LIKE '%sushi%' THEN 'Culinaria japonesa contemporanea'
        WHEN email LIKE '%churrascaria%' THEN 'Rodizio de carnes nobres'
        WHEN email LIKE '%bistro%' THEN 'Alta gastronomia francesa'
        WHEN email LIKE '%bar%' THEN 'Bar com petiscos e drinks'
        WHEN email LIKE '%vegetariano%' THEN 'Culinaria vegetariana e vegana'
        WHEN email LIKE '%hamburgueria%' THEN 'Hambur gueres artesanais gourmet'
        WHEN email LIKE '%cafeteria%' THEN 'Cafe especial e brunch'
        WHEN email LIKE '%taqueria%' THEN 'Comida mexicana autentica'
        WHEN email LIKE '%massas%' THEN 'Massas frescas e molhos artesanais'
        WHEN email LIKE '%choparia%' THEN 'Choperia com petiscos variados'
        WHEN email LIKE '%doceria%' THEN 'Doces e bolos caseiros'
        WHEN email LIKE '%peixaria%' THEN 'Frutos do mar frescos'
        ELSE 'Lanches e porcoes tradicionais'
    END,
    CASE 
        WHEN email LIKE '%pizza%' THEN 'pizzaria'
        WHEN email LIKE '%sushi%' THEN 'japones'
        WHEN email LIKE '%churrascaria%' THEN 'churrascaria'
        WHEN email LIKE '%bistro%' THEN 'bistro'
        WHEN email LIKE '%bar%' THEN 'bar'
        WHEN email LIKE '%vegetariano%' THEN 'vegetariano'
        WHEN email LIKE '%hamburgueria%' THEN 'hamburgueria'
        WHEN email LIKE '%cafeteria%' THEN 'cafeteria'
        WHEN email LIKE '%taqueria%' THEN 'mexicano'
        WHEN email LIKE '%choparia%' THEN 'choperia'
        WHEN email LIKE '%doceria%' THEN 'doceria'
        WHEN email LIKE '%peixaria%' THEN 'frutos_do_mar'
        WHEN email LIKE '%lanchonete%' THEN 'lanchonete'
        ELSE 'restaurante'
    END,
    'https://via.placeholder.com/200x200?text=' || SUBSTRING(name FROM 1 FOR 10),
    -23.5505 + (RANDOM() * 0.02 - 0.01),
    -46.6333 + (RANDOM() * 0.02 - 0.01),
    'Rua Augusta, ' || (1000 + (ROW_NUMBER() OVER()) * 100)::text || ' - Consolacao',
    '01305-' || LPAD((ROW_NUMBER() OVER())::text, 3, '0'),
    'Sao Paulo',
    'SP',
    '{"seg": "11:00-22:00", "ter": "11:00-22:00", "qua": "11:00-22:00", "qui": "11:00-23:00", "sex": "11:00-00:00", "sab": "12:00-00:00"}'::jsonb,
    TRUE,
    TRUE
FROM new_rest_users;

-- ==========================================
-- 2. VAGAS (40+ vagas abertas)
-- ==========================================

-- Criar vagas para os restaurantes
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status)
SELECT 
    gen_random_uuid(),
    rp.id,
    CASE 
        WHEN random() < 0.3 THEN 'Garcom para Final de Semana'
        WHEN random() < 0.5 THEN 'Cozinheiro Experiente'
        WHEN random() < 0.7 THEN 'Auxiliar de Cozinha - Urgente'
        WHEN random() < 0.9 THEN 'Barman para Noite'
        ELSE 'Maitre para Evento Especial'
    END,
    CASE 
        WHEN random() < 0.5 THEN 'Precisamos de profissional experiente para turno movimentado'
        ELSE 'Vaga para trabalho em equipe dinamica'
    END,
    CASE 
        WHEN random() < 0.4 THEN 'garcom'
        WHEN random() < 0.7 THEN 'cozinheiro'
        WHEN random() < 0.85 THEN 'auxiliar_cozinha'
        ELSE 'barman'
    END,
    60 + (RANDOM() * 140)::numeric(10,2),
    CURRENT_DATE + ((RANDOM() * 20)::int + 1) * INTERVAL '1 day',
    ((10 + (RANDOM() * 8)::int)::text || ':00:00')::time,
    ((18 + (RANDOM() * 6)::int)::text || ':00:00')::time,
    4 + (RANDOM() * 6)::int,
    jsonb_build_object(
        'experiencia_minima', (RANDOM() * 5)::int,
        'habilidades', jsonb_build_array('atendimento', 'agilidade')
    ),
    1 + (RANDOM() * 3)::int,
    0,
    'aberta'
FROM restaurante_profiles rp
CROSS JOIN generate_series(1, 3) gs
LIMIT 45;

-- ==========================================
-- 3. CANDIDATURAS (20 candidaturas de exemplo)
-- ==========================================

INSERT INTO candidaturas (id, vaga_id, freelancer_id, mensagem, preco_proposto, status, submitted_at)
SELECT 
    gen_random_uuid(),
    v.id,
    fp.id,
    'Tenho experiencia e disponibilidade para a vaga. Aguardo retorno!',
    v.preco_oferecido + (RANDOM() * 20 - 10)::numeric(10,2),
    CASE 
        WHEN RANDOM() < 0.7 THEN 'pendente'
        WHEN RANDOM() < 0.9 THEN 'aceita'
        ELSE 'recusada'
    END,
    NOW() - (RANDOM() * INTERVAL '48 hours')
FROM vagas v
CROSS JOIN freelancer_profiles fp
WHERE v.status = 'aberta' 
AND NOT EXISTS (
    SELECT 1 FROM candidaturas c 
    WHERE c.vaga_id = v.id AND c.freelancer_id = fp.id
)
ORDER BY RANDOM()
LIMIT 25;

-- ==========================================
-- 4. JOBS AGREED (3 trabalhos confirmados)
-- ==========================================

INSERT INTO jobs_agreed (id, vaga_id, candidatura_id, freelancer_id, restaurante_id, status, preco_acordado, data_trabalho, horario_inicio, horario_fim)
SELECT 
    gen_random_uuid(),
    c.vaga_id,
    c.id,
    c.freelancer_id,
    v.restaurante_id,
    CASE 
        WHEN RANDOM() < 0.5 THEN 'confirmado'
        ELSE 'concluido'
    END,
    c.preco_proposto,
    v.data_trabalho,
    v.horario_inicio,
    v.horario_fim
FROM candidaturas c
JOIN vagas v ON c.vaga_id = v.id
WHERE c.status = 'aceita'
LIMIT 3;

-- ==========================================
-- 5. AVALIACOES (5 avaliacoes)
-- ==========================================

INSERT INTO avaliacoes (id, job_id, rater_id, rated_id, score, comentario)
SELECT 
    gen_random_uuid(),
    ja.id,
    rp.user_id,
    fp.user_id,
    4.0 + (RANDOM() * 1.0)::numeric(2,1),
    CASE 
        WHEN RANDOM() < 0.3 THEN 'Excelente profissional! Recomendo!'
        WHEN RANDOM() < 0.6 THEN 'Muito bom, pontual e eficiente.'
        ELSE 'Bom trabalho, atendeu as expectativas.'
    END
FROM jobs_agreed ja
JOIN freelancer_profiles fp ON ja.freelancer_id = fp.id
JOIN restaurante_profiles rp ON ja.restaurante_id = rp.id
WHERE ja.status = 'concluido'
LIMIT 5;

-- ==========================================
-- RESUMO FINAL
-- ==========================================

SELECT 
    'SEED CONCLUIDO COM SUCESSO!' AS status,
    (SELECT COUNT(*) FROM users WHERE email LIKE '%@test.com%' OR email LIKE '%@exemplo.com%' OR email LIKE '%@.com.br%') AS total_users,
    (SELECT COUNT(*) FROM freelancer_profiles) AS total_freelancers,
    (SELECT COUNT(*) FROM restaurante_profiles) AS total_restaurantes,
    (SELECT COUNT(*) FROM vagas) AS total_vagas,
    (SELECT COUNT(*) FROM candidaturas) AS total_candidaturas,
    (SELECT COUNT(*) FROM jobs_agreed) AS total_jobs,
    (SELECT COUNT(*) FROM avaliacoes) AS total_avaliacoes;

