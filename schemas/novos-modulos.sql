-- ==============================================================================
-- SCHEMAS SQL - 13 NOVOS MÓDULOS
-- Data: 2025-10-21
-- ==============================================================================

-- ==============================================================================
-- 1. WORKSPACE
-- ==============================================================================

CREATE TABLE IF NOT EXISTS workspaces (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    tenant_id UUID NOT NULL REFERENCES tenants(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    slug VARCHAR(255) NOT NULL,
    description TEXT,
    type VARCHAR(50) NOT NULL DEFAULT 'team', -- personal, team, enterprise
    settings JSONB DEFAULT '{}',
    is_active BOOLEAN DEFAULT true,
    created_by UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(tenant_id, slug)
);

CREATE TABLE IF NOT EXISTS workspace_members (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    role VARCHAR(50) NOT NULL DEFAULT 'member', -- owner, admin, member, guest
    invited_by UUID REFERENCES users(id) ON DELETE SET NULL,
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(workspace_id, user_id)
);

CREATE INDEX idx_workspaces_tenant ON workspaces(tenant_id);
CREATE INDEX idx_workspaces_slug ON workspaces(slug);
CREATE INDEX idx_workspace_members_workspace ON workspace_members(workspace_id);
CREATE INDEX idx_workspace_members_user ON workspace_members(user_id);

-- ==============================================================================
-- 2. TASK MANAGEMENT
-- ==============================================================================

CREATE TABLE IF NOT EXISTS projects (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    key VARCHAR(10) NOT NULL, -- Project key (ex: PROJ)
    description TEXT,
    is_active BOOLEAN DEFAULT true,
    created_by UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(workspace_id, key)
);

CREATE TABLE IF NOT EXISTS boards (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    project_id UUID NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    type VARCHAR(50) NOT NULL DEFAULT 'kanban', -- kanban, scrum
    columns JSONB NOT NULL DEFAULT '[]', -- [{id, name, order, wip_limit}]
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS sprints (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    project_id UUID NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    goal TEXT,
    start_date DATE NOT NULL,
    end_date DATE NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'planned', -- planned, active, completed
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS tasks (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    project_id UUID NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
    board_id UUID REFERENCES boards(id) ON DELETE SET NULL,
    sprint_id UUID REFERENCES sprints(id) ON DELETE SET NULL,
    parent_task_id UUID REFERENCES tasks(id) ON DELETE CASCADE,
    key VARCHAR(50) NOT NULL, -- Task key (ex: PROJ-123)
    title VARCHAR(500) NOT NULL,
    description TEXT,
    type VARCHAR(50) NOT NULL DEFAULT 'task', -- bug, feature, epic, story, task
    status VARCHAR(50) NOT NULL DEFAULT 'todo', -- todo, in_progress, review, done, blocked
    priority VARCHAR(50) NOT NULL DEFAULT 'medium', -- low, medium, high, urgent
    story_points INT,
    assignee_id UUID REFERENCES users(id) ON DELETE SET NULL,
    reporter_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    due_date DATE,
    completed_at TIMESTAMP,
    position INT NOT NULL DEFAULT 0, -- Order in board column
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(project_id, key)
);

CREATE TABLE IF NOT EXISTS task_comments (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    task_id UUID NOT NULL REFERENCES tasks(id) ON DELETE CASCADE,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    comment TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS task_attachments (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    task_id UUID NOT NULL REFERENCES tasks(id) ON DELETE CASCADE,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    filename VARCHAR(500) NOT NULL,
    file_path VARCHAR(1000) NOT NULL,
    file_size BIGINT NOT NULL,
    mime_type VARCHAR(100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_projects_workspace ON projects(workspace_id);
CREATE INDEX idx_boards_project ON boards(project_id);
CREATE INDEX idx_sprints_project ON sprints(project_id);
CREATE INDEX idx_tasks_project ON tasks(project_id);
CREATE INDEX idx_tasks_board ON tasks(board_id);
CREATE INDEX idx_tasks_sprint ON tasks(sprint_id);
CREATE INDEX idx_tasks_assignee ON tasks(assignee_id);
CREATE INDEX idx_tasks_status ON tasks(status);
CREATE INDEX idx_task_comments_task ON task_comments(task_id);
CREATE INDEX idx_task_attachments_task ON task_attachments(task_id);

-- ==============================================================================
-- 3. CALENDAR & SCHEDULING
-- ==============================================================================

CREATE TABLE IF NOT EXISTS calendars (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    owner_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    color VARCHAR(7) DEFAULT '#3788d8',
    is_default BOOLEAN DEFAULT false,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS events (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    calendar_id UUID NOT NULL REFERENCES calendars(id) ON DELETE CASCADE,
    title VARCHAR(500) NOT NULL,
    description TEXT,
    type VARCHAR(50) NOT NULL DEFAULT 'event', -- meeting, task, reminder, holiday
    location VARCHAR(500),
    start_time TIMESTAMP NOT NULL,
    end_time TIMESTAMP NOT NULL,
    is_all_day BOOLEAN DEFAULT false,
    recurrence_rule VARCHAR(500), -- RRULE format (RFC 5545)
    created_by UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS event_attendees (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    event_id UUID NOT NULL REFERENCES events(id) ON DELETE CASCADE,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    status VARCHAR(50) NOT NULL DEFAULT 'pending', -- pending, accepted, declined, tentative
    is_organizer BOOLEAN DEFAULT false,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(event_id, user_id)
);

CREATE TABLE IF NOT EXISTS event_reminders (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    event_id UUID NOT NULL REFERENCES events(id) ON DELETE CASCADE,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    minutes_before INT NOT NULL,
    type VARCHAR(50) NOT NULL DEFAULT 'email', -- email, push, popup
    sent BOOLEAN DEFAULT false,
    sent_at TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_calendars_workspace ON calendars(workspace_id);
CREATE INDEX idx_calendars_owner ON calendars(owner_id);
CREATE INDEX idx_events_calendar ON events(calendar_id);
CREATE INDEX idx_events_start_time ON events(start_time);
CREATE INDEX idx_event_attendees_event ON event_attendees(event_id);
CREATE INDEX idx_event_attendees_user ON event_attendees(user_id);
CREATE INDEX idx_event_reminders_event ON event_reminders(event_id);

-- ==============================================================================
-- 4. TIME TRACKING
-- ==============================================================================

CREATE TABLE IF NOT EXISTS timesheets (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    period_start DATE NOT NULL,
    period_end DATE NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'draft', -- draft, submitted, approved, rejected
    total_minutes INT DEFAULT 0,
    submitted_at TIMESTAMP,
    approved_by UUID REFERENCES users(id) ON DELETE SET NULL,
    approved_at TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(workspace_id, user_id, period_start)
);

CREATE TABLE IF NOT EXISTS time_entries (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    timesheet_id UUID REFERENCES timesheets(id) ON DELETE CASCADE,
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    task_id UUID REFERENCES tasks(id) ON DELETE SET NULL,
    description TEXT,
    start_time TIMESTAMP NOT NULL,
    end_time TIMESTAMP,
    duration_minutes INT NOT NULL DEFAULT 0,
    is_billable BOOLEAN DEFAULT true,
    is_running BOOLEAN DEFAULT false, -- Timer ativo
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_timesheets_workspace ON timesheets(workspace_id);
CREATE INDEX idx_timesheets_user ON timesheets(user_id);
CREATE INDEX idx_time_entries_timesheet ON time_entries(timesheet_id);
CREATE INDEX idx_time_entries_workspace ON time_entries(workspace_id);
CREATE INDEX idx_time_entries_user ON time_entries(user_id);
CREATE INDEX idx_time_entries_task ON time_entries(task_id);
CREATE INDEX idx_time_entries_running ON time_entries(is_running) WHERE is_running = true;

-- ==============================================================================
-- 5. FINANCE
-- ==============================================================================

CREATE TABLE IF NOT EXISTS financial_accounts (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    type VARCHAR(50) NOT NULL DEFAULT 'bank', -- bank, cash, credit_card
    currency VARCHAR(3) NOT NULL DEFAULT 'USD',
    initial_balance DECIMAL(15,2) DEFAULT 0,
    current_balance DECIMAL(15,2) DEFAULT 0,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS financial_categories (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    type VARCHAR(50) NOT NULL, -- income, expense
    color VARCHAR(7) DEFAULT '#3788d8',
    parent_id UUID REFERENCES financial_categories(id) ON DELETE CASCADE,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS financial_transactions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    account_id UUID NOT NULL REFERENCES financial_accounts(id) ON DELETE CASCADE,
    category_id UUID REFERENCES financial_categories(id) ON DELETE SET NULL,
    type VARCHAR(50) NOT NULL, -- income, expense, transfer
    amount DECIMAL(15,2) NOT NULL,
    description TEXT,
    transaction_date DATE NOT NULL,
    created_by UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS budgets (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    amount DECIMAL(15,2) NOT NULL,
    period_start DATE NOT NULL,
    period_end DATE NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'active', -- active, completed, exceeded
    spent DECIMAL(15,2) DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS budget_categories (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    budget_id UUID NOT NULL REFERENCES budgets(id) ON DELETE CASCADE,
    category_id UUID NOT NULL REFERENCES financial_categories(id) ON DELETE CASCADE,
    allocated_amount DECIMAL(15,2) NOT NULL,
    spent DECIMAL(15,2) DEFAULT 0,
    UNIQUE(budget_id, category_id)
);

CREATE INDEX idx_financial_accounts_workspace ON financial_accounts(workspace_id);
CREATE INDEX idx_financial_categories_workspace ON financial_categories(workspace_id);
CREATE INDEX idx_financial_transactions_workspace ON financial_transactions(workspace_id);
CREATE INDEX idx_financial_transactions_account ON financial_transactions(account_id);
CREATE INDEX idx_financial_transactions_date ON financial_transactions(transaction_date);
CREATE INDEX idx_budgets_workspace ON budgets(workspace_id);
CREATE INDEX idx_budget_categories_budget ON budget_categories(budget_id);

-- ==============================================================================
-- 6. INVOICING
-- ==============================================================================

CREATE TABLE IF NOT EXISTS quotes (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    customer_id UUID NOT NULL REFERENCES customers(id) ON DELETE CASCADE,
    quote_number VARCHAR(50) NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'draft', -- draft, sent, accepted, rejected, expired
    issue_date DATE NOT NULL,
    expiry_date DATE NOT NULL,
    subtotal DECIMAL(15,2) NOT NULL DEFAULT 0,
    tax_amount DECIMAL(15,2) DEFAULT 0,
    discount_amount DECIMAL(15,2) DEFAULT 0,
    total DECIMAL(15,2) NOT NULL DEFAULT 0,
    notes TEXT,
    terms TEXT,
    created_by UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(workspace_id, quote_number)
);

CREATE TABLE IF NOT EXISTS invoices (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    customer_id UUID NOT NULL REFERENCES customers(id) ON DELETE CASCADE,
    quote_id UUID REFERENCES quotes(id) ON DELETE SET NULL,
    invoice_number VARCHAR(50) NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'draft', -- draft, sent, paid, overdue, cancelled
    issue_date DATE NOT NULL,
    due_date DATE NOT NULL,
    paid_date DATE,
    subtotal DECIMAL(15,2) NOT NULL DEFAULT 0,
    tax_amount DECIMAL(15,2) DEFAULT 0,
    discount_amount DECIMAL(15,2) DEFAULT 0,
    total DECIMAL(15,2) NOT NULL DEFAULT 0,
    paid_amount DECIMAL(15,2) DEFAULT 0,
    notes TEXT,
    terms TEXT,
    created_by UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(workspace_id, invoice_number)
);

CREATE TABLE IF NOT EXISTS invoice_items (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    invoice_id UUID REFERENCES invoices(id) ON DELETE CASCADE,
    quote_id UUID REFERENCES quotes(id) ON DELETE CASCADE,
    description VARCHAR(500) NOT NULL,
    quantity DECIMAL(10,2) NOT NULL DEFAULT 1,
    unit_price DECIMAL(15,2) NOT NULL,
    tax_rate DECIMAL(5,2) DEFAULT 0,
    discount_rate DECIMAL(5,2) DEFAULT 0,
    total DECIMAL(15,2) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_quotes_workspace ON quotes(workspace_id);
CREATE INDEX idx_quotes_customer ON quotes(customer_id);
CREATE INDEX idx_quotes_number ON quotes(quote_number);
CREATE INDEX idx_invoices_workspace ON invoices(workspace_id);
CREATE INDEX idx_invoices_customer ON invoices(customer_id);
CREATE INDEX idx_invoices_number ON invoices(invoice_number);
CREATE INDEX idx_invoices_status ON invoices(status);
CREATE INDEX idx_invoice_items_invoice ON invoice_items(invoice_id);
CREATE INDEX idx_invoice_items_quote ON invoice_items(quote_id);

-- ==============================================================================
-- 7. SUBSCRIPTION & BILLING
-- ==============================================================================

CREATE TABLE IF NOT EXISTS subscription_plans (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name VARCHAR(255) NOT NULL UNIQUE,
    description TEXT,
    type VARCHAR(50) NOT NULL DEFAULT 'free', -- free, basic, pro, enterprise
    price DECIMAL(10,2) NOT NULL DEFAULT 0,
    billing_cycle VARCHAR(50) NOT NULL DEFAULT 'monthly', -- monthly, yearly
    features JSONB DEFAULT '[]', -- [feature_id, ...]
    limits JSONB DEFAULT '{}', -- {max_users: 10, max_storage_gb: 100, ...}
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS subscriptions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    tenant_id UUID NOT NULL REFERENCES tenants(id) ON DELETE CASCADE,
    plan_id UUID NOT NULL REFERENCES subscription_plans(id) ON DELETE CASCADE,
    status VARCHAR(50) NOT NULL DEFAULT 'active', -- active, cancelled, expired, past_due
    start_date DATE NOT NULL,
    end_date DATE,
    trial_ends_at DATE,
    next_billing_date DATE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS subscription_usage (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    subscription_id UUID NOT NULL REFERENCES subscriptions(id) ON DELETE CASCADE,
    metric VARCHAR(100) NOT NULL, -- users, storage_gb, api_calls, etc
    current_value INT NOT NULL DEFAULT 0,
    limit_value INT,
    period_start DATE NOT NULL,
    period_end DATE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_subscriptions_tenant ON subscriptions(tenant_id);
CREATE INDEX idx_subscriptions_plan ON subscriptions(plan_id);
CREATE INDEX idx_subscription_usage_subscription ON subscription_usage(subscription_id);

-- ==============================================================================
-- 8. PRODUCTS
-- ==============================================================================

CREATE TABLE IF NOT EXISTS product_categories (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    slug VARCHAR(255) NOT NULL,
    description TEXT,
    parent_id UUID REFERENCES product_categories(id) ON DELETE CASCADE,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(workspace_id, slug)
);

CREATE TABLE IF NOT EXISTS products (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    category_id UUID REFERENCES product_categories(id) ON DELETE SET NULL,
    name VARCHAR(500) NOT NULL,
    slug VARCHAR(500) NOT NULL,
    description TEXT,
    sku VARCHAR(100),
    price DECIMAL(15,2) NOT NULL,
    compare_at_price DECIMAL(15,2),
    cost_price DECIMAL(15,2),
    weight DECIMAL(10,2),
    dimensions JSONB, -- {length, width, height, unit}
    track_inventory BOOLEAN DEFAULT true,
    stock_quantity INT DEFAULT 0,
    low_stock_threshold INT DEFAULT 10,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(workspace_id, slug),
    UNIQUE(workspace_id, sku)
);

CREATE TABLE IF NOT EXISTS product_variants (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    product_id UUID NOT NULL REFERENCES products(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    sku VARCHAR(100),
    price DECIMAL(15,2) NOT NULL,
    compare_at_price DECIMAL(15,2),
    stock_quantity INT DEFAULT 0,
    options JSONB NOT NULL, -- {size: "M", color: "Red"}
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(product_id, sku)
);

CREATE TABLE IF NOT EXISTS product_images (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    product_id UUID NOT NULL REFERENCES products(id) ON DELETE CASCADE,
    variant_id UUID REFERENCES product_variants(id) ON DELETE CASCADE,
    url VARCHAR(1000) NOT NULL,
    alt_text VARCHAR(500),
    position INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_product_categories_workspace ON product_categories(workspace_id);
CREATE INDEX idx_products_workspace ON products(workspace_id);
CREATE INDEX idx_products_category ON products(category_id);
CREATE INDEX idx_products_sku ON products(sku);
CREATE INDEX idx_products_slug ON products(slug);
CREATE INDEX idx_product_variants_product ON product_variants(product_id);
CREATE INDEX idx_product_images_product ON product_images(product_id);

-- ==============================================================================
-- 9. ORDERS
-- ==============================================================================

CREATE TABLE IF NOT EXISTS carts (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    customer_id UUID REFERENCES customers(id) ON DELETE CASCADE,
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    session_id VARCHAR(255), -- For anonymous carts
    subtotal DECIMAL(15,2) DEFAULT 0,
    tax_amount DECIMAL(15,2) DEFAULT 0,
    discount_amount DECIMAL(15,2) DEFAULT 0,
    total DECIMAL(15,2) DEFAULT 0,
    expires_at TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS cart_items (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    cart_id UUID NOT NULL REFERENCES carts(id) ON DELETE CASCADE,
    product_id UUID NOT NULL REFERENCES products(id) ON DELETE CASCADE,
    variant_id UUID REFERENCES product_variants(id) ON DELETE CASCADE,
    quantity INT NOT NULL DEFAULT 1,
    unit_price DECIMAL(15,2) NOT NULL,
    total DECIMAL(15,2) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS orders (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    customer_id UUID NOT NULL REFERENCES customers(id) ON DELETE CASCADE,
    order_number VARCHAR(50) NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'pending', -- pending, processing, paid, shipped, delivered, cancelled
    subtotal DECIMAL(15,2) NOT NULL,
    tax_amount DECIMAL(15,2) DEFAULT 0,
    shipping_amount DECIMAL(15,2) DEFAULT 0,
    discount_amount DECIMAL(15,2) DEFAULT 0,
    total DECIMAL(15,2) NOT NULL,
    shipping_address JSONB,
    billing_address JSONB,
    shipping_method VARCHAR(100),
    notes TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(workspace_id, order_number)
);

CREATE TABLE IF NOT EXISTS order_items (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    order_id UUID NOT NULL REFERENCES orders(id) ON DELETE CASCADE,
    product_id UUID NOT NULL REFERENCES products(id) ON DELETE CASCADE,
    variant_id UUID REFERENCES product_variants(id) ON DELETE CASCADE,
    product_name VARCHAR(500) NOT NULL, -- Snapshot
    sku VARCHAR(100),
    quantity INT NOT NULL,
    unit_price DECIMAL(15,2) NOT NULL,
    total DECIMAL(15,2) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_carts_workspace ON carts(workspace_id);
CREATE INDEX idx_carts_customer ON carts(customer_id);
CREATE INDEX idx_carts_session ON carts(session_id);
CREATE INDEX idx_cart_items_cart ON cart_items(cart_id);
CREATE INDEX idx_orders_workspace ON orders(workspace_id);
CREATE INDEX idx_orders_customer ON orders(customer_id);
CREATE INDEX idx_orders_number ON orders(order_number);
CREATE INDEX idx_orders_status ON orders(status);
CREATE INDEX idx_order_items_order ON order_items(order_id);

-- ==============================================================================
-- 10. PAYMENTS
-- ==============================================================================

CREATE TABLE IF NOT EXISTS payments (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    order_id UUID REFERENCES orders(id) ON DELETE CASCADE,
    invoice_id UUID REFERENCES invoices(id) ON DELETE CASCADE,
    customer_id UUID NOT NULL REFERENCES customers(id) ON DELETE CASCADE,
    provider VARCHAR(50) NOT NULL, -- stripe, paypal, mercadopago
    provider_payment_id VARCHAR(255),
    amount DECIMAL(15,2) NOT NULL,
    currency VARCHAR(3) NOT NULL DEFAULT 'USD',
    status VARCHAR(50) NOT NULL DEFAULT 'pending', -- pending, processing, succeeded, failed, refunded
    payment_method VARCHAR(100), -- card, pix, boleto, paypal
    failure_reason TEXT,
    metadata JSONB,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS payment_transactions (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    payment_id UUID NOT NULL REFERENCES payments(id) ON DELETE CASCADE,
    type VARCHAR(50) NOT NULL, -- charge, refund, authorization, capture
    amount DECIMAL(15,2) NOT NULL,
    status VARCHAR(50) NOT NULL,
    provider_transaction_id VARCHAR(255),
    metadata JSONB,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS payment_refunds (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    payment_id UUID NOT NULL REFERENCES payments(id) ON DELETE CASCADE,
    amount DECIMAL(15,2) NOT NULL,
    reason TEXT,
    status VARCHAR(50) NOT NULL DEFAULT 'pending', -- pending, processing, succeeded, failed
    provider_refund_id VARCHAR(255),
    created_by UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_payments_workspace ON payments(workspace_id);
CREATE INDEX idx_payments_order ON payments(order_id);
CREATE INDEX idx_payments_invoice ON payments(invoice_id);
CREATE INDEX idx_payments_customer ON payments(customer_id);
CREATE INDEX idx_payments_provider_id ON payments(provider_payment_id);
CREATE INDEX idx_payment_transactions_payment ON payment_transactions(payment_id);
CREATE INDEX idx_payment_refunds_payment ON payment_refunds(payment_id);

-- ==============================================================================
-- 11. INVENTORY
-- ==============================================================================

CREATE TABLE IF NOT EXISTS warehouses (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    code VARCHAR(50) NOT NULL,
    address JSONB,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(workspace_id, code)
);

CREATE TABLE IF NOT EXISTS stock_levels (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    warehouse_id UUID NOT NULL REFERENCES warehouses(id) ON DELETE CASCADE,
    product_id UUID REFERENCES products(id) ON DELETE CASCADE,
    variant_id UUID REFERENCES product_variants(id) ON DELETE CASCADE,
    quantity INT NOT NULL DEFAULT 0,
    reserved_quantity INT DEFAULT 0,
    available_quantity INT GENERATED ALWAYS AS (quantity - reserved_quantity) STORED,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(warehouse_id, product_id, variant_id)
);

CREATE TABLE IF NOT EXISTS stock_movements (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    warehouse_id UUID NOT NULL REFERENCES warehouses(id) ON DELETE CASCADE,
    product_id UUID NOT NULL REFERENCES products(id) ON DELETE CASCADE,
    variant_id UUID REFERENCES product_variants(id) ON DELETE CASCADE,
    type VARCHAR(50) NOT NULL, -- in, out, transfer, adjustment
    quantity INT NOT NULL,
    reference_type VARCHAR(50), -- order, purchase, adjustment
    reference_id UUID,
    notes TEXT,
    created_by UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_warehouses_workspace ON warehouses(workspace_id);
CREATE INDEX idx_stock_levels_warehouse ON stock_levels(warehouse_id);
CREATE INDEX idx_stock_levels_product ON stock_levels(product_id);
CREATE INDEX idx_stock_movements_workspace ON stock_movements(workspace_id);
CREATE INDEX idx_stock_movements_warehouse ON stock_movements(warehouse_id);
CREATE INDEX idx_stock_movements_product ON stock_movements(product_id);

-- ==============================================================================
-- 12. CUSTOMER
-- ==============================================================================

CREATE TABLE IF NOT EXISTS customers (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    type VARCHAR(50) NOT NULL DEFAULT 'individual', -- individual, business
    name VARCHAR(255) NOT NULL,
    email VARCHAR(255),
    phone VARCHAR(50),
    document VARCHAR(100), -- CPF/CNPJ/Passport
    company_name VARCHAR(255),
    status VARCHAR(50) NOT NULL DEFAULT 'active', -- active, inactive, blocked
    notes TEXT,
    metadata JSONB,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS customer_addresses (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    customer_id UUID NOT NULL REFERENCES customers(id) ON DELETE CASCADE,
    type VARCHAR(50) NOT NULL DEFAULT 'billing', -- billing, shipping
    is_default BOOLEAN DEFAULT false,
    street VARCHAR(500) NOT NULL,
    number VARCHAR(50),
    complement VARCHAR(255),
    neighborhood VARCHAR(255),
    city VARCHAR(255) NOT NULL,
    state VARCHAR(100) NOT NULL,
    country VARCHAR(100) NOT NULL DEFAULT 'Brasil',
    zip_code VARCHAR(20) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS customer_notes (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    customer_id UUID NOT NULL REFERENCES customers(id) ON DELETE CASCADE,
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    note TEXT NOT NULL,
    is_internal BOOLEAN DEFAULT true, -- Visible only to team
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_customers_workspace ON customers(workspace_id);
CREATE INDEX idx_customers_email ON customers(email);
CREATE INDEX idx_customers_phone ON customers(phone);
CREATE INDEX idx_customer_addresses_customer ON customer_addresses(customer_id);
CREATE INDEX idx_customer_notes_customer ON customer_notes(customer_id);

-- ==============================================================================
-- 13. CRM
-- ==============================================================================

CREATE TABLE IF NOT EXISTS crm_leads (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    customer_id UUID REFERENCES customers(id) ON DELETE SET NULL,
    name VARCHAR(255) NOT NULL,
    email VARCHAR(255),
    phone VARCHAR(50),
    company VARCHAR(255),
    job_title VARCHAR(255),
    status VARCHAR(50) NOT NULL DEFAULT 'new', -- new, contacted, qualified, lost
    source VARCHAR(100), -- website, referral, cold_call, social_media
    score INT DEFAULT 0, -- Lead scoring
    assigned_to UUID REFERENCES users(id) ON DELETE SET NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS crm_opportunities (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    lead_id UUID REFERENCES crm_leads(id) ON DELETE SET NULL,
    customer_id UUID REFERENCES customers(id) ON DELETE CASCADE,
    name VARCHAR(500) NOT NULL,
    description TEXT,
    stage VARCHAR(100) NOT NULL DEFAULT 'prospecting', -- prospecting, proposal, negotiation, closed_won, closed_lost
    value DECIMAL(15,2),
    probability INT DEFAULT 0, -- % probability of closing
    expected_close_date DATE,
    actual_close_date DATE,
    assigned_to UUID REFERENCES users(id) ON DELETE SET NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS crm_contacts (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    customer_id UUID REFERENCES customers(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    email VARCHAR(255),
    phone VARCHAR(50),
    job_title VARCHAR(255),
    is_primary BOOLEAN DEFAULT false,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS crm_activities (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    workspace_id UUID NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    lead_id UUID REFERENCES crm_leads(id) ON DELETE CASCADE,
    opportunity_id UUID REFERENCES crm_opportunities(id) ON DELETE CASCADE,
    customer_id UUID REFERENCES customers(id) ON DELETE CASCADE,
    type VARCHAR(50) NOT NULL, -- call, email, meeting, note
    subject VARCHAR(500) NOT NULL,
    description TEXT,
    scheduled_at TIMESTAMP,
    completed_at TIMESTAMP,
    assigned_to UUID REFERENCES users(id) ON DELETE SET NULL,
    created_by UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_crm_leads_workspace ON crm_leads(workspace_id);
CREATE INDEX idx_crm_leads_assigned ON crm_leads(assigned_to);
CREATE INDEX idx_crm_leads_status ON crm_leads(status);
CREATE INDEX idx_crm_opportunities_workspace ON crm_opportunities(workspace_id);
CREATE INDEX idx_crm_opportunities_assigned ON crm_opportunities(assigned_to);
CREATE INDEX idx_crm_opportunities_stage ON crm_opportunities(stage);
CREATE INDEX idx_crm_contacts_workspace ON crm_contacts(workspace_id);
CREATE INDEX idx_crm_contacts_customer ON crm_contacts(customer_id);
CREATE INDEX idx_crm_activities_workspace ON crm_activities(workspace_id);
CREATE INDEX idx_crm_activities_lead ON crm_activities(lead_id);
CREATE INDEX idx_crm_activities_opportunity ON crm_activities(opportunity_id);

-- ==============================================================================
-- FIM DOS SCHEMAS
-- ==============================================================================

