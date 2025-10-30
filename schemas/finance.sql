-- ==============================================================================
-- FINANCE MODULE - COMPLETE SCHEMA
-- Data: 2025-10-23
-- Suporte: Multi-currency, Reconciliation, Budgets, Reports
-- ==============================================================================

-- Drop old MoneyMaker tables if they exist
DROP TABLE IF EXISTS financial_goals CASCADE;
DROP TABLE IF EXISTS budget_categories CASCADE;
DROP TABLE IF EXISTS budgets CASCADE;

-- ==============================================================================
-- ACCOUNTS (Contas Bancárias/Caixa)
-- ==============================================================================

CREATE TABLE IF NOT EXISTS financial_accounts (
    id TEXT PRIMARY KEY,
    tenant_id TEXT NOT NULL,
    workspace_id TEXT,
    name VARCHAR(255) NOT NULL,
    type VARCHAR(50) NOT NULL, -- checking, savings, cash, credit_card, investment
    currency VARCHAR(3) NOT NULL DEFAULT 'BRL',
    balance NUMERIC(15,2) NOT NULL DEFAULT 0,
    initial_balance NUMERIC(15,2) NOT NULL DEFAULT 0,
    bank_name VARCHAR(255),
    account_number VARCHAR(100),
    is_active BOOLEAN DEFAULT TRUE,
    created_by TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- ==============================================================================
-- CATEGORIES (Categorias de Receita/Despesa)
-- ==============================================================================

CREATE TABLE IF NOT EXISTS transaction_categories (
    id TEXT PRIMARY KEY,
    tenant_id TEXT NOT NULL,
    workspace_id TEXT,
    name VARCHAR(255) NOT NULL,
    type VARCHAR(50) NOT NULL, -- income, expense
    parent_id TEXT REFERENCES transaction_categories(id) ON DELETE CASCADE,
    color VARCHAR(7) DEFAULT '#3788d8',
    icon VARCHAR(50),
    is_system BOOLEAN DEFAULT FALSE,
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- ==============================================================================
-- TRANSACTIONS (Transações Financeiras)
-- ==============================================================================

CREATE TABLE IF NOT EXISTS financial_transactions (
    id TEXT PRIMARY KEY,
    tenant_id TEXT NOT NULL,
    workspace_id TEXT,
    account_id TEXT NOT NULL REFERENCES financial_accounts(id) ON DELETE CASCADE,
    category_id TEXT REFERENCES transaction_categories(id) ON DELETE SET NULL,
    type VARCHAR(50) NOT NULL, -- income, expense, transfer
    amount NUMERIC(15,2) NOT NULL,
    currency VARCHAR(3) NOT NULL DEFAULT 'BRL',
    status VARCHAR(50) NOT NULL DEFAULT 'pending', -- pending, completed, cancelled, reconciled
    description TEXT,
    transaction_date DATE NOT NULL,
    reference_id TEXT, -- Link para Payment, Invoice, Order, Subscription
    reference_type VARCHAR(50), -- payment, invoice, order, subscription
    from_account_id TEXT REFERENCES financial_accounts(id) ON DELETE SET NULL,
    to_account_id TEXT REFERENCES financial_accounts(id) ON DELETE SET NULL,
    reconciliation_id TEXT,
    reconciled_at TIMESTAMP,
    created_by TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- ==============================================================================
-- BUDGETS (Orçamentos)
-- ==============================================================================

CREATE TABLE IF NOT EXISTS budgets (
    id TEXT PRIMARY KEY,
    tenant_id TEXT NOT NULL,
    workspace_id TEXT,
    name VARCHAR(255) NOT NULL,
    category_id TEXT REFERENCES transaction_categories(id) ON DELETE SET NULL,
    amount NUMERIC(15,2) NOT NULL,
    spent NUMERIC(15,2) DEFAULT 0,
    period VARCHAR(50) NOT NULL, -- daily, weekly, monthly, quarterly, yearly, custom
    start_date DATE NOT NULL,
    end_date DATE NOT NULL,
    is_recurring BOOLEAN DEFAULT FALSE,
    alert_percentage INT DEFAULT 80, -- Alert when 80% spent
    status VARCHAR(50) NOT NULL DEFAULT 'active', -- active, completed, exceeded, cancelled
    created_by TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- ==============================================================================
-- RECONCILIATIONS (Reconciliações Bancárias)
-- ==============================================================================

CREATE TABLE IF NOT EXISTS reconciliations (
    id TEXT PRIMARY KEY,
    tenant_id TEXT NOT NULL,
    workspace_id TEXT,
    account_id TEXT NOT NULL REFERENCES financial_accounts(id) ON DELETE CASCADE,
    statement_date DATE NOT NULL,
    statement_balance NUMERIC(15,2) NOT NULL,
    system_balance NUMERIC(15,2) NOT NULL,
    difference NUMERIC(15,2) NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'pending', -- pending, matched, unmatched, completed
    matched_transactions INT DEFAULT 0,
    unmatched_transactions INT DEFAULT 0,
    notes TEXT,
    reconciled_by TEXT,
    reconciled_at TIMESTAMP,
    created_by TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- ==============================================================================
-- INDEXES
-- ==============================================================================

-- Accounts
CREATE INDEX idx_accounts_tenant ON financial_accounts(tenant_id);
CREATE INDEX idx_accounts_workspace ON financial_accounts(workspace_id);
CREATE INDEX idx_accounts_type ON financial_accounts(type);
CREATE INDEX idx_accounts_currency ON financial_accounts(currency);

-- Categories
CREATE INDEX idx_categories_tenant ON transaction_categories(tenant_id);
CREATE INDEX idx_categories_workspace ON transaction_categories(workspace_id);
CREATE INDEX idx_categories_type ON transaction_categories(type);
CREATE INDEX idx_categories_parent ON transaction_categories(parent_id);

-- Transactions
CREATE INDEX idx_transactions_tenant ON financial_transactions(tenant_id);
CREATE INDEX idx_transactions_workspace ON financial_transactions(workspace_id);
CREATE INDEX idx_transactions_account ON financial_transactions(account_id);
CREATE INDEX idx_transactions_category ON financial_transactions(category_id);
CREATE INDEX idx_transactions_date ON financial_transactions(transaction_date);
CREATE INDEX idx_transactions_type ON financial_transactions(type);
CREATE INDEX idx_transactions_status ON financial_transactions(status);
CREATE INDEX idx_transactions_reference ON financial_transactions(reference_id, reference_type);

-- Budgets
CREATE INDEX idx_budgets_tenant ON budgets(tenant_id);
CREATE INDEX idx_budgets_workspace ON budgets(workspace_id);
CREATE INDEX idx_budgets_category ON budgets(category_id);
CREATE INDEX idx_budgets_period ON budgets(period);
CREATE INDEX idx_budgets_dates ON budgets(start_date, end_date);

-- Reconciliations
CREATE INDEX idx_reconciliations_tenant ON reconciliations(tenant_id);
CREATE INDEX idx_reconciliations_workspace ON reconciliations(workspace_id);
CREATE INDEX idx_reconciliations_account ON reconciliations(account_id);
CREATE INDEX idx_reconciliations_date ON reconciliations(statement_date);
CREATE INDEX idx_reconciliations_status ON reconciliations(status);

-- ==============================================================================
-- SAMPLE DATA (Default Categories)
-- ==============================================================================

-- Income Categories
INSERT INTO transaction_categories (id, tenant_id, name, type, color, icon, is_system) VALUES
('cat-income-salary', 'system', 'Salary', 'income', '#22c55e', 'salary', true),
('cat-income-freelance', 'system', 'Freelance', 'income', '#10b981', 'freelance', true),
('cat-income-investment', 'system', 'Investment', 'income', '#14b8a6', 'investment', true),
('cat-income-other', 'system', 'Other Income', 'income', '#06b6d4', 'other', true);

-- Expense Categories
INSERT INTO transaction_categories (id, tenant_id, name, type, color, icon, is_system) VALUES
('cat-expense-food', 'system', 'Food & Dining', 'expense', '#ef4444', 'food', true),
('cat-expense-transport', 'system', 'Transportation', 'expense', '#f59e0b', 'transport', true),
('cat-expense-housing', 'system', 'Housing', 'expense', '#8b5cf6', 'housing', true),
('cat-expense-utilities', 'system', 'Utilities', 'expense', '#ec4899', 'utilities', true),
('cat-expense-entertainment', 'system', 'Entertainment', 'expense', '#f97316', 'entertainment', true),
('cat-expense-health', 'system', 'Health', 'expense', '#14b8a6', 'health', true),
('cat-expense-education', 'system', 'Education', 'expense', '#06b6d4', 'education', true),
('cat-expense-shopping', 'system', 'Shopping', 'expense', '#a855f7', 'shopping', true),
('cat-expense-other', 'system', 'Other Expense', 'expense', '#6b7280', 'other', true);

-- ==============================================================================
-- FIM DO SCHEMA
-- ==============================================================================

