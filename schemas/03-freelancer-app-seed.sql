-- ==========================================
-- FREELANCER APP - SEED DATA
-- Dados de teste para desenvolvimento
-- ==========================================

-- LIMPAR DADOS EXISTENTES (CUIDADO EM PRODUÇÃO!)
-- ==========================================
DELETE FROM avaliacoes;
DELETE FROM jobs_agreed;
DELETE FROM candidaturas;
DELETE FROM vagas;
DELETE FROM freelancer_profiles;
DELETE FROM restaurante_profiles;
DELETE FROM users WHERE email LIKE '%@test.com' OR email LIKE '%@exemplo.com';

-- ==========================================
-- 1. USERS (Base para Freelancers e Restaurantes)
-- ==========================================

-- 20 Freelancers
INSERT INTO users (id, email, password_hash, name, is_active, email_verified, score, phone, avatar) VALUES
('f1-user-001', 'joao.silva@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'João Silva', TRUE, TRUE, 4.8, '11987654321', 'https://i.pravatar.cc/150?img=1'),
('f2-user-002', 'maria.santos@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Maria Santos', TRUE, TRUE, 4.9, '11987654322', 'https://i.pravatar.cc/150?img=2'),
('f3-user-003', 'pedro.oliveira@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Pedro Oliveira', TRUE, TRUE, 4.5, '11987654323', 'https://i.pravatar.cc/150?img=3'),
('f4-user-004', 'ana.costa@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Ana Costa', TRUE, TRUE, 4.7, '11987654324', 'https://i.pravatar.cc/150?img=4'),
('f5-user-005', 'carlos.souza@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Carlos Souza', TRUE, TRUE, 4.6, '11987654325', 'https://i.pravatar.cc/150?img=5'),
('f6-user-006', 'juliana.lima@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Juliana Lima', TRUE, TRUE, 4.9, '11987654326', 'https://i.pravatar.cc/150?img=6'),
('f7-user-007', 'rafael.alves@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Rafael Alves', TRUE, TRUE, 4.3, '11987654327', 'https://i.pravatar.cc/150?img=7'),
('f8-user-008', 'fernanda.rocha@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Fernanda Rocha', TRUE, TRUE, 4.8, '11987654328', 'https://i.pravatar.cc/150?img=8'),
('f9-user-009', 'lucas.martins@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Lucas Martins', TRUE, TRUE, 4.4, '11987654329', 'https://i.pravatar.cc/150?img=9'),
('f10-user-010', 'camila.ferreira@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Camila Ferreira', TRUE, TRUE, 4.7, '11987654330', 'https://i.pravatar.cc/150?img=10'),
('f11-user-011', 'bruno.araujo@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Bruno Araújo', TRUE, TRUE, 4.5, '11987654331', 'https://i.pravatar.cc/150?img=11'),
('f12-user-012', 'patricia.barbosa@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Patricia Barbosa', TRUE, TRUE, 4.6, '11987654332', 'https://i.pravatar.cc/150?img=12'),
('f13-user-013', 'rodrigo.carvalho@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Rodrigo Carvalho', TRUE, TRUE, 4.9, '11987654333', 'https://i.pravatar.cc/150?img=13'),
('f14-user-014', 'aline.ribeiro@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Aline Ribeiro', TRUE, TRUE, 4.2, '11987654334', 'https://i.pravatar.cc/150?img=14'),
('f15-user-015', 'gustavo.castro@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Gustavo Castro', TRUE, TRUE, 4.8, '11987654335', 'https://i.pravatar.cc/150?img=15'),
('f16-user-016', 'mariana.dias@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Mariana Dias', TRUE, TRUE, 4.7, '11987654336', 'https://i.pravatar.cc/150?img=16'),
('f17-user-017', 'thiago.gomes@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Thiago Gomes', TRUE, TRUE, 4.4, '11987654337', 'https://i.pravatar.cc/150?img=17'),
('f18-user-018', 'leticia.moura@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Letícia Moura', TRUE, TRUE, 4.6, '11987654338', 'https://i.pravatar.cc/150?img=18'),
('f19-user-019', 'daniel.farias@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Daniel Farias', TRUE, TRUE, 4.5, '11987654339', 'https://i.pravatar.cc/150?img=19'),
('f20-user-020', 'isabela.monteiro@test.com', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Isabela Monteiro', TRUE, TRUE, 4.9, '11987654340', 'https://i.pravatar.cc/150?img=20');

-- 15 Restaurantes
INSERT INTO users (id, email, password_hash, name, is_active, email_verified, score, phone, avatar) VALUES
('r1-user-101', 'contato@pizzariadonna.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Pizzaria Donna Bella', TRUE, TRUE, 4.7, '1132221111', 'https://i.pravatar.cc/150?img=51'),
('r2-user-102', 'contato@cantinaitalia.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Cantina Itália', TRUE, TRUE, 4.8, '1132221112', 'https://i.pravatar.cc/150?img=52'),
('r3-user-103', 'contato@sushihouse.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Sushi House', TRUE, TRUE, 4.9, '1132221113', 'https://i.pravatar.cc/150?img=53'),
('r4-user-104', 'contato@churrascariagarucho.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Churrascaria Gaúcho', TRUE, TRUE, 4.6, '1132221114', 'https://i.pravatar.cc/150?img=54'),
('r5-user-105', 'contato@bistrofrances.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Bistro Français', TRUE, TRUE, 4.8, '1132221115', 'https://i.pravatar.cc/150?img=55'),
('r6-user-106', 'contato@bardajapa.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Bar da Japa', TRUE, TRUE, 4.5, '1132221116', 'https://i.pravatar.cc/150?img=56'),
('r7-user-107', 'contato@restaurantevegetariano.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Restaurante Vegetariano Verde Vida', TRUE, TRUE, 4.7, '1132221117', 'https://i.pravatar.cc/150?img=57'),
('r8-user-108', 'contato@hamburgueriaartesanal.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Hamburgueria Artesanal Burger Lab', TRUE, TRUE, 4.8, '1132221118', 'https://i.pravatar.cc/150?img=58'),
('r9-user-109', 'contato@cafeteriaroma.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Cafeteria Aroma', TRUE, TRUE, 4.6, '1132221119', 'https://i.pravatar.cc/150?img=59'),
('r10-user-110', 'contato@taqueriaolé.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Taqueria Olé', TRUE, TRUE, 4.7, '1132221120', 'https://i.pravatar.cc/150?img=60'),
('r11-user-111', 'contato@massasdelicia.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Massas Delícia', TRUE, TRUE, 4.5, '1132221121', 'https://i.pravatar.cc/150?img=61'),
('r12-user-112', 'contato@chopariacentral.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Choparia Central', TRUE, TRUE, 4.8, '1132221122', 'https://i.pravatar.cc/150?img=62'),
('r13-user-113', 'contato@doceriadavovo.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Doceria da Vovó', TRUE, TRUE, 4.9, '1132221123', 'https://i.pravatar.cc/150?img=63'),
('r14-user-114', 'contato@restaurantepeixaria.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Restaurante Peixaria', TRUE, TRUE, 4.6, '1132221124', 'https://i.pravatar.cc/150?img=64'),
('r15-user-115', 'contato@lanchonetedobairro.com.br', '$2a$10$abcdefghijklmnopqrstuvwxyz123456', 'Lanchonete do Bairro', TRUE, TRUE, 4.4, '1132221125', 'https://i.pravatar.cc/150?img=65');

-- ==========================================
-- 2. FREELANCER PROFILES
-- ==========================================
INSERT INTO freelancer_profiles (id, user_id, especialidades, experiencia_anos, preco_minimo, cpf, latitude, longitude, raio_atuacao_km, disponibilidade, documentos_verificados, ativo) VALUES
(gen_random_uuid(), 'f1-user-001', '["garcom", "barman"]', 5, 80.00, '12345678901', -23.5505, -46.6333, 15, '{"seg": ["08:00-18:00"], "ter": ["08:00-18:00"], "qua": ["08:00-18:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f2-user-002', '["cozinheiro", "confeiteiro"]', 8, 120.00, '12345678902', -23.5489, -46.6388, 20, '{"seg": ["06:00-14:00"], "ter": ["06:00-14:00"], "qua": ["06:00-14:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f3-user-003', '["garcom"]', 3, 60.00, '12345678903', -23.5617, -46.6563, 10, '{"seg": ["18:00-23:00"], "ter": ["18:00-23:00"], "sex": ["18:00-02:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f4-user-004', '["auxiliar_cozinha", "garcom"]', 2, 50.00, '12345678904', -23.5558, -46.6396, 12, '{"seg": ["08:00-17:00"], "ter": ["08:00-17:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f5-user-005', '["barman", "sommelier"]', 10, 150.00, '12345678905', -23.5475, -46.6361, 25, '{"qui": ["19:00-02:00"], "sex": ["19:00-03:00"], "sab": ["19:00-03:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f6-user-006', '["cozinheiro", "chef"]', 12, 200.00, '12345678906', -23.5629, -46.6544, 30, '{"seg": ["10:00-22:00"], "ter": ["10:00-22:00"], "qua": ["10:00-22:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f7-user-007', '["garcom", "auxiliar_cozinha"]', 1, 45.00, '12345678907', -23.5502, -46.6420, 8, '{"seg": ["11:00-20:00"], "ter": ["11:00-20:00"]}', FALSE, TRUE),
(gen_random_uuid(), 'f8-user-008', '["confeiteiro", "padeiro"]', 6, 100.00, '12345678908', -23.5580, -46.6310, 15, '{"seg": ["05:00-13:00"], "ter": ["05:00-13:00"], "qua": ["05:00-13:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f9-user-009', '["garcom"]', 4, 70.00, '12345678909', -23.5440, -46.6450, 12, '{"qua": ["18:00-23:00"], "qui": ["18:00-23:00"], "sex": ["18:00-01:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f10-user-010', '["barman", "garcom"]', 7, 90.00, '12345678910', -23.5590, -46.6280, 18, '{"qui": ["17:00-01:00"], "sex": ["17:00-02:00"], "sab": ["17:00-02:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f11-user-011', '["cozinheiro"]', 9, 130.00, '12345678911', -23.5520, -46.6490, 20, '{"seg": ["09:00-18:00"], "ter": ["09:00-18:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f12-user-012', '["garcom", "maitre"]', 11, 140.00, '12345678912', -23.5465, -46.6300, 22, '{"ter": ["18:00-23:00"], "qua": ["18:00-23:00"], "qui": ["18:00-23:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f13-user-013', '["auxiliar_cozinha"]', 2, 55.00, '12345678913', -23.5600, -46.6500, 10, '{"seg": ["07:00-16:00"], "ter": ["07:00-16:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f14-user-014', '["garcom"]', 3, 65.00, '12345678914', -23.5530, -46.6370, 12, '{"sex": ["19:00-00:00"], "sab": ["19:00-01:00"]}', FALSE, TRUE),
(gen_random_uuid(), 'f15-user-015', '["cozinheiro", "auxiliar_cozinha"]', 5, 95.00, '12345678915', -23.5450, -46.6400, 15, '{"seg": ["11:00-20:00"], "ter": ["11:00-20:00"], "qua": ["11:00-20:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f16-user-016', '["barman"]', 4, 85.00, '12345678916', -23.5570, -46.6330, 14, '{"qui": ["18:00-01:00"], "sex": ["18:00-02:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f17-user-017', '["garcom", "barman"]', 6, 95.00, '12345678917', -23.5490, -46.6470, 16, '{"seg": ["17:00-23:00"], "ter": ["17:00-23:00"], "qua": ["17:00-23:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f18-user-018', '["cozinheiro"]', 7, 110.00, '12345678918', -23.5610, -46.6290, 18, '{"seg": ["08:00-17:00"], "ter": ["08:00-17:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f19-user-019', '["auxiliar_cozinha", "garcom"]', 2, 52.00, '12345678919', -23.5545, -46.6410, 10, '{"seg": ["10:00-19:00"], "ter": ["10:00-19:00"]}', TRUE, TRUE),
(gen_random_uuid(), 'f20-user-020', '["confeiteiro"]', 8, 115.00, '12345678920', -23.5485, -46.6355, 17, '{"ter": ["06:00-14:00"], "qua": ["06:00-14:00"], "qui": ["06:00-14:00"]}', TRUE, TRUE);

-- ==========================================
-- 3. RESTAURANTE PROFILES
-- ==========================================
INSERT INTO restaurante_profiles (id, user_id, cnpj, razao_social, nome_fantasia, descricao, categoria, logo, latitude, longitude, endereco_completo, cep, cidade, estado, horario_funcionamento, documentos_verificados, ativo) VALUES
(gen_random_uuid(), 'r1-user-101', '12345678000101', 'Pizzaria Donna Bella Ltda', 'Pizzaria Donna Bella', 'Pizzas artesanais no forno a lenha', 'pizzaria', 'https://via.placeholder.com/200x200?text=Pizza', -23.5505, -46.6333, 'Rua Augusta, 1000 - Consolação', '01305-100', 'São Paulo', 'SP', '{"seg": "18:00-23:00", "ter": "18:00-23:00", "qua": "18:00-23:00", "qui": "18:00-00:00", "sex": "18:00-01:00", "sab": "18:00-01:00", "dom": "18:00-23:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r2-user-102', '12345678000102', 'Cantina Itália Ltda', 'Cantina Itália', 'Comida italiana tradicional', 'restaurante', 'https://via.placeholder.com/200x200?text=Cantina', -23.5560, -46.6610, 'Av. Paulista, 1500 - Bela Vista', '01310-200', 'São Paulo', 'SP', '{"seg": "11:30-15:00,18:00-22:00", "ter": "11:30-15:00,18:00-22:00", "qua": "11:30-15:00,18:00-22:00", "qui": "11:30-15:00,18:00-22:00", "sex": "11:30-15:00,18:00-23:00", "sab": "12:00-23:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r3-user-103', '12345678000103', 'Sushi House Ltda', 'Sushi House', 'Culinária japonesa contemporânea', 'japones', 'https://via.placeholder.com/200x200?text=Sushi', -23.5489, -46.6388, 'Rua Oscar Freire, 500 - Jardins', '01426-000', 'São Paulo', 'SP', '{"ter": "18:00-23:00", "qua": "18:00-23:00", "qui": "18:00-23:00", "sex": "18:00-00:00", "sab": "12:00-00:00", "dom": "12:00-22:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r4-user-104', '12345678000104', 'Churrascaria Gaúcho Ltda', 'Churrascaria Gaúcho', 'Rodízio de carnes nobres', 'churrascaria', 'https://via.placeholder.com/200x200?text=Churrasco', -23.5617, -46.6563, 'Rua da Consolação, 2000 - Consolação', '01301-100', 'São Paulo', 'SP', '{"seg": "11:00-15:00,18:00-23:00", "ter": "11:00-15:00,18:00-23:00", "qua": "11:00-15:00,18:00-23:00", "qui": "11:00-15:00,18:00-23:00", "sex": "11:00-15:00,18:00-00:00", "sab": "11:00-00:00", "dom": "11:00-22:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r5-user-105', '12345678000105', 'Bistro Français Ltda', 'Bistro Français', 'Alta gastronomia francesa', 'bistro', 'https://via.placeholder.com/200x200?text=Bistro', -23.5558, -46.6396, 'Alameda Lorena, 1200 - Jardins', '01424-001', 'São Paulo', 'SP', '{"ter": "19:00-23:00", "qua": "19:00-23:00", "qui": "19:00-23:00", "sex": "19:00-00:00", "sab": "19:00-00:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r6-user-106', '12345678000106', 'Bar da Japa Ltda', 'Bar da Japa', 'Bar com petiscos e drinks', 'bar', 'https://via.placeholder.com/200x200?text=Bar', -23.5475, -46.6361, 'Rua Haddock Lobo, 800 - Cerqueira César', '01414-001', 'São Paulo', 'SP', '{"ter": "18:00-01:00", "qua": "18:00-01:00", "qui": "18:00-02:00", "sex": "18:00-03:00", "sab": "16:00-03:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r7-user-107', '12345678000107', 'Verde Vida Restaurante Ltda', 'Verde Vida', 'Culinária vegetariana e vegana', 'vegetariano', 'https://via.placeholder.com/200x200?text=Vegano', -23.5629, -46.6544, 'Rua Teodoro Sampaio, 1500 - Pinheiros', '05405-100', 'São Paulo', 'SP', '{"seg": "11:00-15:00", "ter": "11:00-15:00", "qua": "11:00-15:00", "qui": "11:00-15:00", "sex": "11:00-15:00,18:00-21:00", "sab": "11:00-16:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r8-user-108', '12345678000108', 'Burger Lab Ltda', 'Burger Lab', 'Hambúrgueres artesanais gourmet', 'hamburgueria', 'https://via.placeholder.com/200x200?text=Burger', -23.5502, -46.6420, 'Rua Augusta, 2500 - Consolação', '01413-000', 'São Paulo', 'SP', '{"seg": "18:00-00:00", "ter": "18:00-00:00", "qua": "18:00-00:00", "qui": "18:00-01:00", "sex": "18:00-02:00", "sab": "18:00-02:00", "dom": "18:00-23:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r9-user-109', '12345678000109', 'Cafeteria Aroma Ltda', 'Cafeteria Aroma', 'Café especial e brunch', 'cafeteria', 'https://via.placeholder.com/200x200?text=Cafe', -23.5580, -46.6310, 'Rua dos Pinheiros, 300 - Pinheiros', '05422-010', 'São Paulo', 'SP', '{"seg": "07:00-19:00", "ter": "07:00-19:00", "qua": "07:00-19:00", "qui": "07:00-19:00", "sex": "07:00-20:00", "sab": "08:00-20:00", "dom": "08:00-18:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r10-user-110', '12345678000110', 'Taqueria Olé Ltda', 'Taqueria Olé', 'Comida mexicana autêntica', 'mexicano', 'https://via.placeholder.com/200x200?text=Tacos', -23.5440, -46.6450, 'Rua Frei Caneca, 1000 - Consolação', '01307-002', 'São Paulo', 'SP', '{"ter": "18:00-23:00", "qua": "18:00-23:00", "qui": "18:00-00:00", "sex": "18:00-01:00", "sab": "12:00-01:00", "dom": "12:00-22:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r11-user-111', '12345678000111', 'Massas Delícia Ltda', 'Massas Delícia', 'Massas frescas e molhos artesanais', 'restaurante', 'https://via.placeholder.com/200x200?text=Massa', -23.5590, -46.6280, 'Av. Rebouças, 2000 - Pinheiros', '05401-400', 'São Paulo', 'SP', '{"seg": "11:30-15:00,18:00-22:00", "ter": "11:30-15:00,18:00-22:00", "qua": "11:30-15:00,18:00-22:00", "qui": "11:30-15:00,18:00-22:00", "sex": "11:30-15:00,18:00-23:00", "sab": "12:00-23:00", "dom": "12:00-21:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r12-user-112', '12345678000112', 'Choparia Central Ltda', 'Choparia Central', 'Choperia com petiscos variados', 'choperia', 'https://via.placeholder.com/200x200?text=Chopp', -23.5520, -46.6490, 'Rua Maria Antônia, 500 - Vila Buarque', '01222-010', 'São Paulo', 'SP', '{"ter": "17:00-00:00", "qua": "17:00-00:00", "qui": "17:00-01:00", "sex": "17:00-02:00", "sab": "14:00-02:00", "dom": "14:00-22:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r13-user-113', '12345678000113', 'Doceria da Vovó Ltda', 'Doceria da Vovó', 'Doces e bolos caseiros', 'doceria', 'https://via.placeholder.com/200x200?text=Doces', -23.5465, -46.6300, 'Rua Henrique Schaumann, 600 - Pinheiros', '05413-010', 'São Paulo', 'SP', '{"seg": "09:00-19:00", "ter": "09:00-19:00", "qua": "09:00-19:00", "qui": "09:00-19:00", "sex": "09:00-20:00", "sab": "09:00-20:00", "dom": "10:00-18:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r14-user-114', '12345678000114', 'Peixaria Restaurante Ltda', 'Restaurante Peixaria', 'Frutos do mar frescos', 'frutos_do_mar', 'https://via.placeholder.com/200x200?text=Peixe', -23.5600, -46.6500, 'Rua Cardeal Arcoverde, 1000 - Pinheiros', '05407-002', 'São Paulo', 'SP', '{"ter": "12:00-15:00,18:00-22:00", "qua": "12:00-15:00,18:00-22:00", "qui": "12:00-15:00,18:00-22:00", "sex": "12:00-15:00,18:00-23:00", "sab": "12:00-23:00", "dom": "12:00-21:00"}', TRUE, TRUE),
(gen_random_uuid(), 'r15-user-115', '12345678000115', 'Lanchonete do Bairro Ltda', 'Lanchonete do Bairro', 'Lanches e porções tradicionais', 'lanchonete', 'https://via.placeholder.com/200x200?text=Lanche', -23.5530, -46.6370, 'Rua Bela Cintra, 1500 - Cerqueira César', '01415-002', 'São Paulo', 'SP', '{"seg": "10:00-22:00", "ter": "10:00-22:00", "qua": "10:00-22:00", "qui": "10:00-22:00", "sex": "10:00-00:00", "sab": "10:00-00:00", "dom": "10:00-20:00"}', TRUE, TRUE);

-- ==========================================
-- 4. VAGAS (Job Postings)
-- ==========================================
-- Nota: Vamos criar vagas para os próximos 30 dias

-- Pizzaria Donna Bella (5 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Garçom para Final de Semana', 
    'Precisamos de garçom experiente para atendimento no salão durante finais de semana movimentados', 
    'garcom', 
    85.00, 
    CURRENT_DATE + INTERVAL '2 days', 
    '18:00', 
    '23:00', 
    5, 
    '{"experiencia_minima": 2, "habilidades": ["atendimento", "agilidade"]}', 
    2, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r1-user-101';

INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Auxiliar de Cozinha - Urgente', 
    'Auxiliar para preparação de ingredientes e limpeza da cozinha', 
    'auxiliar_cozinha', 
    60.00, 
    CURRENT_DATE + INTERVAL '1 day', 
    '16:00', 
    '22:00', 
    6, 
    '{"experiencia_minima": 0}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r1-user-101';

INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Barman para Sexta à Noite', 
    'Barman para preparar drinks e atender no balcão', 
    'barman', 
    100.00, 
    CURRENT_DATE + INTERVAL '5 days', 
    '19:00', 
    '01:00', 
    6, 
    '{"experiencia_minima": 3, "habilidades": ["preparo_drinks", "atendimento"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r1-user-101';

-- Cantina Itália (4 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Cozinheiro Italiano - Almoço', 
    'Cozinheiro experiente em culinária italiana para turno de almoço', 
    'cozinheiro', 
    150.00, 
    CURRENT_DATE + INTERVAL '3 days', 
    '10:00', 
    '16:00', 
    6, 
    '{"experiencia_minima": 5, "habilidades": ["cozinha_italiana", "massas"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r2-user-102';

INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Garçom - Turno Noite', 
    'Garçom para atendimento no jantar, experiência com vinhos é um diferencial', 
    'garcom', 
    90.00, 
    CURRENT_DATE + INTERVAL '4 days', 
    '18:00', 
    '23:00', 
    5, 
    '{"experiencia_minima": 3, "habilidades": ["atendimento", "conhecimento_vinhos"]}', 
    2, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r2-user-102';

-- Sushi House (6 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Auxiliar de Sushiman', 
    'Auxiliar para preparação de ingredientes e montagem de pratos', 
    'auxiliar_cozinha', 
    70.00, 
    CURRENT_DATE + INTERVAL '2 days', 
    '17:00', 
    '23:00', 
    6, 
    '{"experiencia_minima": 1, "habilidades": ["agilidade", "higiene"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r3-user-103';

INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Garçom com Inglês - Sábado', 
    'Garçom para atendimento, inglês fluente necessário', 
    'garcom', 
    110.00, 
    CURRENT_DATE + INTERVAL '6 days', 
    '18:00', 
    '00:00', 
    6, 
    '{"experiencia_minima": 2, "habilidades": ["ingles_fluente", "atendimento"]}', 
    2, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r3-user-103';

-- Churrascaria Gaúcho (3 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Garçom - Rodízio Domingo', 
    'Garçom para servir rodízio de carnes no domingo', 
    'garcom', 
    95.00, 
    CURRENT_DATE + INTERVAL '7 days', 
    '11:00', 
    '17:00', 
    6, 
    '{"experiencia_minima": 2, "habilidades": ["atendimento", "forca_fisica"]}', 
    3, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r4-user-104';

INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Auxiliar de Cozinha - Churrasqueiro', 
    'Auxiliar para ajudar no preparo das carnes', 
    'auxiliar_cozinha', 
    75.00, 
    CURRENT_DATE + INTERVAL '5 days', 
    '10:00', 
    '18:00', 
    8, 
    '{"experiencia_minima": 1}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r4-user-104';

-- Bistro Français (4 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Garçom Experiente - Alta Gastronomia', 
    'Garçom com experiência em alta gastronomia, francês é diferencial', 
    'garcom', 
    130.00, 
    CURRENT_DATE + INTERVAL '8 days', 
    '19:00', 
    '23:00', 
    4, 
    '{"experiencia_minima": 5, "habilidades": ["alta_gastronomia", "vinhos", "frances"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r5-user-105';

INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Sommelier para Evento Especial', 
    'Sommelier para evento de degustação de vinhos', 
    'sommelier', 
    200.00, 
    CURRENT_DATE + INTERVAL '10 days', 
    '19:00', 
    '23:00', 
    4, 
    '{"experiencia_minima": 7, "habilidades": ["vinhos", "frances", "harmonizacao"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r5-user-105';

-- Bar da Japa (5 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Barman - Sexta Noite', 
    'Barman para preparar drinks autorais', 
    'barman', 
    120.00, 
    CURRENT_DATE + INTERVAL '5 days', 
    '20:00', 
    '02:00', 
    6, 
    '{"experiencia_minima": 4, "habilidades": ["drinks_autorais", "flair"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r6-user-106';

INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Garçom - Sábado à Noite', 
    'Garçom para atendimento no sábado, movimento intenso', 
    'garcom', 
    95.00, 
    CURRENT_DATE + INTERVAL '6 days', 
    '18:00', 
    '02:00', 
    8, 
    '{"experiencia_minima": 2, "habilidades": ["atendimento", "agilidade"]}', 
    2, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r6-user-106';

-- Verde Vida (3 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Cozinheiro Vegano', 
    'Cozinheiro com experiência em culinária vegana e vegetariana', 
    'cozinheiro', 
    110.00, 
    CURRENT_DATE + INTERVAL '4 days', 
    '09:00', 
    '15:00', 
    6, 
    '{"experiencia_minima": 3, "habilidades": ["cozinha_vegana", "criatividade"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r7-user-107';

-- Burger Lab (4 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Chapeiro - Finais de Semana', 
    'Chapeiro para preparo de hambúrgueres artesanais', 
    'cozinheiro', 
    90.00, 
    CURRENT_DATE + INTERVAL '6 days', 
    '18:00', 
    '00:00', 
    6, 
    '{"experiencia_minima": 2, "habilidades": ["chapa", "agilidade"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r8-user-108';

INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Atendente de Caixa', 
    'Atendente para caixa e pedidos no balcão', 
    'garcom', 
    70.00, 
    CURRENT_DATE + INTERVAL '3 days', 
    '18:00', 
    '23:00', 
    5, 
    '{"experiencia_minima": 0, "habilidades": ["atendimento", "simpatia"]}', 
    2, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r8-user-108';

-- Cafeteria Aroma (3 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Barista - Manhã', 
    'Barista para preparo de cafés especiais', 
    'barman', 
    85.00, 
    CURRENT_DATE + INTERVAL '2 days', 
    '07:00', 
    '13:00', 
    6, 
    '{"experiencia_minima": 2, "habilidades": ["cafe_especial", "latte_art"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r9-user-109';

INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Confeiteiro - Finais de Semana', 
    'Confeiteiro para produção de bolos e doces', 
    'confeiteiro', 
    100.00, 
    CURRENT_DATE + INTERVAL '6 days', 
    '06:00', 
    '12:00', 
    6, 
    '{"experiencia_minima": 3, "habilidades": ["confeitaria", "decoracao"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r9-user-109';

-- Taqueria Olé (4 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Cozinheiro Mexicano', 
    'Cozinheiro com experiência em comida mexicana', 
    'cozinheiro', 
    105.00, 
    CURRENT_DATE + INTERVAL '5 days', 
    '17:00', 
    '23:00', 
    6, 
    '{"experiencia_minima": 3, "habilidades": ["cozinha_mexicana", "tacos"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r10-user-110';

INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Garçom - Espanhol Fluente', 
    'Garçom com espanhol fluente para atendimento', 
    'garcom', 
    95.00, 
    CURRENT_DATE + INTERVAL '7 days', 
    '18:00', 
    '23:00', 
    5, 
    '{"experiencia_minima": 2, "habilidades": ["espanhol", "atendimento"]}', 
    2, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r10-user-110';

-- Massas Delícia (3 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Auxiliar de Cozinha - Massas Frescas', 
    'Auxiliar para produção de massas frescas', 
    'auxiliar_cozinha', 
    65.00, 
    CURRENT_DATE + INTERVAL '3 days', 
    '08:00', 
    '14:00', 
    6, 
    '{"experiencia_minima": 1, "habilidades": ["massas", "agilidade"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r11-user-111';

-- Choparia Central (4 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Garçom - Chopeira', 
    'Garçom com experiência em tirar chopp', 
    'garcom', 
    80.00, 
    CURRENT_DATE + INTERVAL '4 days', 
    '17:00', 
    '00:00', 
    7, 
    '{"experiencia_minima": 2, "habilidades": ["chopeira", "atendimento"]}', 
    2, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r12-user-112';

INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Cozinheiro - Petiscos', 
    'Cozinheiro para preparar petiscos variados', 
    'cozinheiro', 
    95.00, 
    CURRENT_DATE + INTERVAL '6 days', 
    '17:00', 
    '00:00', 
    7, 
    '{"experiencia_minima": 3, "habilidades": ["fritura", "montagem"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r12-user-112';

-- Doceria da Vovó (2 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Confeiteiro - Sábado', 
    'Confeiteiro para produção de bolos e doces finos', 
    'confeiteiro', 
    110.00, 
    CURRENT_DATE + INTERVAL '6 days', 
    '06:00', 
    '14:00', 
    8, 
    '{"experiencia_minima": 4, "habilidades": ["confeitaria_fina", "decoracao"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r13-user-113';

-- Restaurante Peixaria (3 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Cozinheiro - Frutos do Mar', 
    'Cozinheiro especializado em frutos do mar', 
    'cozinheiro', 
    140.00, 
    CURRENT_DATE + INTERVAL '8 days', 
    '11:00', 
    '17:00', 
    6, 
    '{"experiencia_minima": 5, "habilidades": ["frutos_do_mar", "peixe"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r14-user-114';

INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Garçom - Domingo Almoço', 
    'Garçom para turno de almoço no domingo', 
    'garcom', 
    90.00, 
    CURRENT_DATE + INTERVAL '7 days', 
    '11:00', 
    '17:00', 
    6, 
    '{"experiencia_minima": 2}', 
    2, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r14-user-114';

-- Lanchonete do Bairro (2 vagas)
INSERT INTO vagas (id, restaurante_id, titulo, descricao, categoria, preco_oferecido, data_trabalho, horario_inicio, horario_fim, duracao_horas, requisitos, vagas_disponiveis, vagas_preenchidas, status) 
SELECT 
    gen_random_uuid(), 
    rp.id, 
    'Chapeiro - Noturno', 
    'Chapeiro para período noturno', 
    'cozinheiro', 
    75.00, 
    CURRENT_DATE + INTERVAL '3 days', 
    '18:00', 
    '23:00', 
    5, 
    '{"experiencia_minima": 1, "habilidades": ["chapa", "fritura"]}', 
    1, 
    0, 
    'aberta'
FROM restaurante_profiles rp WHERE rp.user_id = 'r15-user-115';

-- Total: ~50 vagas criadas

-- ==========================================
-- 5. CANDIDATURAS (Sample Applications)
-- ==========================================
-- Vamos criar algumas candidaturas de exemplo

-- João Silva (f1) se candidata para vaga de garçom na Pizzaria
INSERT INTO candidaturas (id, vaga_id, freelancer_id, mensagem, preco_proposto, status, submitted_at)
SELECT 
    gen_random_uuid(),
    v.id,
    fp.id,
    'Olá! Tenho 5 anos de experiência como garçom e barman. Estou disponível para trabalhar no horário solicitado.',
    85.00,
    'pendente',
    NOW() - INTERVAL '2 hours'
FROM vagas v
JOIN restaurante_profiles rp ON v.restaurante_id = rp.id
JOIN freelancer_profiles fp ON fp.user_id = 'f1-user-001'
WHERE rp.user_id = 'r1-user-101' AND v.titulo = 'Garçom para Final de Semana'
LIMIT 1;

-- Maria Santos (f2) se candidata para vaga de cozinheiro na Cantina Itália
INSERT INTO candidaturas (id, vaga_id, freelancer_id, mensagem, preco_proposto, status, submitted_at)
SELECT 
    gen_random_uuid(),
    v.id,
    fp.id,
    'Sou cozinheira com 8 anos de experiência, especializada em culinária italiana. Adoraria fazer parte da equipe!',
    150.00,
    'pendente',
    NOW() - INTERVAL '5 hours'
FROM vagas v
JOIN restaurante_profiles rp ON v.restaurante_id = rp.id
JOIN freelancer_profiles fp ON fp.user_id = 'f2-user-002'
WHERE rp.user_id = 'r2-user-102' AND v.titulo LIKE '%Cozinheiro Italiano%'
LIMIT 1;

-- Pedro Oliveira (f3) se candidata para pizzaria
INSERT INTO candidaturas (id, vaga_id, freelancer_id, mensagem, preco_proposto, status, submitted_at)
SELECT 
    gen_random_uuid(),
    v.id,
    fp.id,
    'Disponível para trabalhar, tenho experiência como garçom.',
    82.00,
    'pendente',
    NOW() - INTERVAL '1 hour'
FROM vagas v
JOIN restaurante_profiles rp ON v.restaurante_id = rp.id
JOIN freelancer_profiles fp ON fp.user_id = 'f3-user-003'
WHERE rp.user_id = 'r1-user-101' AND v.titulo = 'Garçom para Final de Semana'
LIMIT 1;

-- Carlos Souza (f5 - barman experiente) para Bar da Japa
INSERT INTO candidaturas (id, vaga_id, freelancer_id, mensagem, preco_proposto, status, submitted_at)
SELECT 
    gen_random_uuid(),
    v.id,
    fp.id,
    'Barman com 10 anos de experiência, especializado em drinks autorais e flair bartending.',
    125.00,
    'pendente',
    NOW() - INTERVAL '30 minutes'
FROM vagas v
JOIN restaurante_profiles rp ON v.restaurante_id = rp.id
JOIN freelancer_profiles fp ON fp.user_id = 'f5-user-005'
WHERE rp.user_id = 'r6-user-106' AND v.titulo LIKE '%Barman%'
LIMIT 1;

-- Juliana Lima (f6 - chef) para Bistro Français
INSERT INTO candidaturas (id, vaga_id, freelancer_id, mensagem, preco_proposto, status, submitted_at)
SELECT 
    gen_random_uuid(),
    v.id,
    fp.id,
    'Chef com 12 anos de experiência, trabalhei em restaurantes franceses na Europa. Falo francês fluentemente.',
    200.00,
    'aceita',
    NOW() - INTERVAL '1 day',
    NOW() - INTERVAL '6 hours',
    'Perfeito! Aguardamos você no evento.'
FROM vagas v
JOIN restaurante_profiles rp ON v.restaurante_id = rp.id
JOIN freelancer_profiles fp ON fp.user_id = 'f6-user-006'
WHERE rp.user_id = 'r5-user-105' AND v.titulo LIKE '%Sommelier%'
LIMIT 1;

-- Ana Costa (f4) para auxiliar na pizzaria
INSERT INTO candidaturas (id, vaga_id, freelancer_id, mensagem, preco_proposto, status, submitted_at)
SELECT 
    gen_random_uuid(),
    v.id,
    fp.id,
    'Tenho experiência como auxiliar de cozinha e garçom, estou disponível para o horário.',
    60.00,
    'aceita',
    NOW() - INTERVAL '18 hours',
    NOW() - INTERVAL '12 hours',
    'Contratado! Compareça 15 minutos antes do horário.'
FROM vagas v
JOIN restaurante_profiles rp ON v.restaurante_id = rp.id
JOIN freelancer_profiles fp ON fp.user_id = 'f4-user-004'
WHERE rp.user_id = 'r1-user-101' AND v.titulo LIKE '%Auxiliar%'
LIMIT 1;

-- Rafael Alves (f7) recusado
INSERT INTO candidaturas (id, vaga_id, freelancer_id, mensagem, preco_proposto, status, submitted_at, responded_at, response_message)
SELECT 
    gen_random_uuid(),
    v.id,
    fp.id,
    'Gostaria de trabalhar como garçom.',
    70.00,
    'recusada',
    NOW() - INTERVAL '2 days',
    NOW() - INTERVAL '1 day',
    'Obrigado pelo interesse, mas já preenchemos a vaga.'
FROM vagas v
JOIN restaurante_profiles rp ON v.restaurante_id = rp.id
JOIN freelancer_profiles fp ON fp.user_id = 'f7-user-007'
WHERE rp.user_id = 'r3-user-103' AND v.titulo LIKE '%Garçom%'
LIMIT 1;

-- Mais 10 candidaturas variadas
INSERT INTO candidaturas (id, vaga_id, freelancer_id, mensagem, preco_proposto, status, submitted_at)
SELECT 
    gen_random_uuid(),
    v.id,
    fp.id,
    'Tenho disponibilidade e experiência para a vaga.',
    v.preco_oferecido,
    'pendente',
    NOW() - (RANDOM() * INTERVAL '12 hours')
FROM vagas v
CROSS JOIN freelancer_profiles fp
WHERE v.status = 'aberta' 
AND NOT EXISTS (
    SELECT 1 FROM candidaturas c 
    WHERE c.vaga_id = v.id AND c.freelancer_id = fp.id
)
ORDER BY RANDOM()
LIMIT 10;

-- ==========================================
-- 6. JOBS AGREED (Trabalhos Confirmados)
-- ==========================================
-- Criar alguns trabalhos já aceitos e confirmados

-- Trabalho 1: Ana Costa na Pizzaria (já concluído)
INSERT INTO jobs_agreed (id, vaga_id, candidatura_id, freelancer_id, restaurante_id, status, preco_acordado, data_trabalho, horario_inicio, horario_fim, data_inicio_real, data_conclusao_real)
SELECT 
    gen_random_uuid(),
    c.vaga_id,
    c.id,
    c.freelancer_id,
    v.restaurante_id,
    'concluido',
    c.preco_proposto,
    v.data_trabalho,
    v.horario_inicio,
    v.horario_fim,
    (v.data_trabalho || ' ' || v.horario_inicio)::timestamp - INTERVAL '10 minutes',
    (v.data_trabalho || ' ' || v.horario_fim)::timestamp + INTERVAL '5 minutes'
FROM candidaturas c
JOIN vagas v ON c.vaga_id = v.id
JOIN freelancer_profiles fp ON c.freelancer_id = fp.id
WHERE fp.user_id = 'f4-user-004' 
AND c.status = 'aceita'
AND v.data_trabalho < CURRENT_DATE
LIMIT 1;

-- Trabalho 2: Juliana Lima no Bistro (confirmado, ainda não iniciado)
INSERT INTO jobs_agreed (id, vaga_id, candidatura_id, freelancer_id, restaurante_id, status, preco_acordado, data_trabalho, horario_inicio, horario_fim)
SELECT 
    gen_random_uuid(),
    c.vaga_id,
    c.id,
    c.freelancer_id,
    v.restaurante_id,
    'confirmado',
    c.preco_proposto,
    v.data_trabalho,
    v.horario_inicio,
    v.horario_fim
FROM candidaturas c
JOIN vagas v ON c.vaga_id = v.id
JOIN freelancer_profiles fp ON c.freelancer_id = fp.id
WHERE fp.user_id = 'f6-user-006' 
AND c.status = 'aceita'
LIMIT 1;

-- ==========================================
-- 7. AVALIAÇÕES (Ratings)
-- ==========================================
-- Criar avaliações mútuas para o trabalho concluído

-- Restaurante avalia Freelancer (Ana Costa)
INSERT INTO avaliacoes (id, job_id, rater_id, rated_id, score, comentario)
SELECT 
    gen_random_uuid(),
    ja.id,
    rp.user_id,
    fp.user_id,
    4.5,
    'Excelente profissional! Pontual, educada e muito eficiente. Recomendo!'
FROM jobs_agreed ja
JOIN freelancer_profiles fp ON ja.freelancer_id = fp.id
JOIN restaurante_profiles rp ON ja.restaurante_id = rp.id
WHERE fp.user_id = 'f4-user-004' 
AND ja.status = 'concluido'
LIMIT 1;

-- Freelancer avalia Restaurante
INSERT INTO avaliacoes (id, job_id, rater_id, rated_id, score, comentario)
SELECT 
    gen_random_uuid(),
    ja.id,
    fp.user_id,
    rp.user_id,
    4.8,
    'Ótimo ambiente de trabalho, equipe acolhedora e pagamento em dia!'
FROM jobs_agreed ja
JOIN freelancer_profiles fp ON ja.freelancer_id = fp.id
JOIN restaurante_profiles rp ON ja.restaurante_id = rp.id
WHERE fp.user_id = 'f4-user-004' 
AND ja.status = 'concluido'
LIMIT 1;

-- Criar mais algumas avaliações históricas para freelancers
-- (simulando trabalhos anteriores)

-- Maria Santos (já tem score 4.9)
INSERT INTO avaliacoes (id, job_id, rater_id, rated_id, score, comentario, created_at)
SELECT 
    gen_random_uuid(),
    NULL, -- trabalho antigo, não existe mais no sistema
    'r2-user-102',
    'f2-user-002',
    5.0,
    'Chef excepcional! Voltarei a contratar com certeza.',
    NOW() - INTERVAL '30 days';

INSERT INTO avaliacoes (id, job_id, rater_id, rated_id, score, comentario, created_at)
SELECT 
    gen_random_uuid(),
    NULL,
    'r5-user-105',
    'f2-user-002',
    4.8,
    'Muito profissional e competente.',
    NOW() - INTERVAL '45 days';

-- João Silva (score 4.8)
INSERT INTO avaliacoes (id, job_id, rater_id, rated_id, score, comentario, created_at)
SELECT 
    gen_random_uuid(),
    NULL,
    'r1-user-101',
    'f1-user-001',
    4.7,
    'Bom garçom, atencioso com os clientes.',
    NOW() - INTERVAL '20 days';

INSERT INTO avaliacoes (id, job_id, rater_id, rated_id, score, comentario, created_at)
SELECT 
    gen_random_uuid(),
    NULL,
    'r6-user-106',
    'f1-user-001',
    4.9,
    'Excelente barman! Drinks perfeitos.',
    NOW() - INTERVAL '15 days';

-- ==========================================
-- FIM DO SEED
-- ==========================================

-- RESUMO:
-- ✅ 35 Users (20 freelancers + 15 restaurantes)
-- ✅ 20 Freelancer Profiles
-- ✅ 15 Restaurante Profiles
-- ✅ ~50 Vagas abertas
-- ✅ ~17 Candidaturas (pendentes, aceitas, recusadas)
-- ✅ 2 Jobs Agreed (1 concluído, 1 confirmado)
-- ✅ 6 Avaliações (incluindo históricas)

SELECT 'SEED CONCLUÍDO COM SUCESSO!' AS status;

