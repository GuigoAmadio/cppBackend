-- ==============================================================================
-- SCHEMAS SQL - 13 NOVOS MÓDULOS (CORRIGIDO)
-- Data: 2025-10-21
-- Compatível com banco MoneyMaker existente (TEXT IDs)
-- ==============================================================================

-- ==============================================================================
-- 1. WORKSPACE (Corrigido para TEXT IDs)
-- ==============================================================================

-- Drop e recria tabela workspaces com estrutura correta
DROP TABLE IF EXISTS workspace_members CASCADE;
DROP TABLE IF EXISTS workspaces CASCADE;

CREATE TABLE workspaces (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    tenant_id TEXT NOT NULL,
    name VARCHAR(255) NOT NULL,
    slug VARCHAR(255) NOT NULL,
    description TEXT,
    type VARCHAR(50) NOT NULL DEFAULT 'team',
    settings JSONB DEFAULT '{}',
    is_active BOOLEAN DEFAULT true,
    created_by TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(tenant_id, slug)
);

CREATE TABLE workspace_members (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    workspace_id TEXT NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    user_id TEXT NOT NULL,
    role VARCHAR(50) NOT NULL DEFAULT 'member',
    invited_by TEXT,
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(workspace_id, user_id)
);

CREATE INDEX idx_workspaces_tenant ON workspaces(tenant_id);
CREATE INDEX idx_workspaces_slug ON workspaces(slug);
CREATE INDEX idx_workspace_members_workspace ON workspace_members(workspace_id);
CREATE INDEX idx_workspace_members_user ON workspace_members(user_id);

-- ==============================================================================
-- 2. TASK MANAGEMENT (Corrigido para TEXT IDs)
-- ==============================================================================

-- Drop e recria tabelas de tasks
DROP TABLE IF EXISTS task_attachments CASCADE;
DROP TABLE IF EXISTS task_comments CASCADE;
DROP TABLE IF EXISTS tasks CASCADE;
DROP TABLE IF EXISTS sprints CASCADE;
DROP TABLE IF EXISTS boards CASCADE;
DROP TABLE IF EXISTS projects CASCADE;

CREATE TABLE projects (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    workspace_id TEXT NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    key VARCHAR(10) NOT NULL,
    description TEXT,
    is_active BOOLEAN DEFAULT true,
    created_by TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(workspace_id, key)
);

CREATE TABLE boards (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    project_id TEXT NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    type VARCHAR(50) NOT NULL DEFAULT 'kanban',
    columns JSONB NOT NULL DEFAULT '[]',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE sprints (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    project_id TEXT NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    goal TEXT,
    start_date DATE NOT NULL,
    end_date DATE NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'planned',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE tasks (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    project_id TEXT NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
    board_id TEXT REFERENCES boards(id) ON DELETE SET NULL,
    sprint_id TEXT REFERENCES sprints(id) ON DELETE SET NULL,
    parent_task_id TEXT REFERENCES tasks(id) ON DELETE CASCADE,
    key VARCHAR(50) NOT NULL,
    title VARCHAR(500) NOT NULL,
    description TEXT,
    type VARCHAR(50) NOT NULL DEFAULT 'task',
    status VARCHAR(50) NOT NULL DEFAULT 'todo',
    priority VARCHAR(50) NOT NULL DEFAULT 'medium',
    story_points INT,
    assignee_id TEXT,
    reporter_id TEXT NOT NULL,
    due_date DATE,
    completed_at TIMESTAMP,
    position INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(project_id, key)
);

CREATE TABLE task_comments (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    task_id TEXT NOT NULL REFERENCES tasks(id) ON DELETE CASCADE,
    user_id TEXT NOT NULL,
    comment TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE task_attachments (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    task_id TEXT NOT NULL REFERENCES tasks(id) ON DELETE CASCADE,
    user_id TEXT NOT NULL,
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
-- 3. CALENDAR & SCHEDULING (Corrigido para TEXT IDs)
-- ==============================================================================

DROP TABLE IF EXISTS event_reminders CASCADE;
DROP TABLE IF EXISTS event_attendees CASCADE;
DROP TABLE IF EXISTS events CASCADE;
DROP TABLE IF EXISTS calendars CASCADE;

CREATE TABLE calendars (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    workspace_id TEXT NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    owner_id TEXT NOT NULL,
    name VARCHAR(255) NOT NULL,
    color VARCHAR(7) DEFAULT '#3788d8',
    is_default BOOLEAN DEFAULT false,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE events (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    calendar_id TEXT NOT NULL REFERENCES calendars(id) ON DELETE CASCADE,
    title VARCHAR(500) NOT NULL,
    description TEXT,
    type VARCHAR(50) NOT NULL DEFAULT 'event',
    location VARCHAR(500),
    start_time TIMESTAMP NOT NULL,
    end_time TIMESTAMP NOT NULL,
    is_all_day BOOLEAN DEFAULT false,
    recurrence_rule VARCHAR(500),
    created_by TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE event_attendees (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    event_id TEXT NOT NULL REFERENCES events(id) ON DELETE CASCADE,
    user_id TEXT NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'pending',
    is_organizer BOOLEAN DEFAULT false,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(event_id, user_id)
);

CREATE TABLE event_reminders (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    event_id TEXT NOT NULL REFERENCES events(id) ON DELETE CASCADE,
    user_id TEXT NOT NULL,
    minutes_before INT NOT NULL,
    type VARCHAR(50) NOT NULL DEFAULT 'email',
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
-- 4. TIME TRACKING (Corrigido para TEXT IDs)
-- ==============================================================================

DROP TABLE IF EXISTS time_entries CASCADE;
DROP TABLE IF EXISTS timesheets CASCADE;

CREATE TABLE timesheets (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    workspace_id TEXT NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    user_id TEXT NOT NULL,
    period_start DATE NOT NULL,
    period_end DATE NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'draft',
    total_minutes INT DEFAULT 0,
    submitted_at TIMESTAMP,
    approved_by TEXT,
    approved_at TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(workspace_id, user_id, period_start)
);

CREATE TABLE time_entries (
    id TEXT PRIMARY KEY DEFAULT gen_random_uuid()::text,
    timesheet_id TEXT REFERENCES timesheets(id) ON DELETE CASCADE,
    workspace_id TEXT NOT NULL REFERENCES workspaces(id) ON DELETE CASCADE,
    user_id TEXT NOT NULL,
    task_id TEXT REFERENCES tasks(id) ON DELETE SET NULL,
    description TEXT,
    start_time TIMESTAMP NOT NULL,
    end_time TIMESTAMP,
    duration_minutes INT NOT NULL DEFAULT 0,
    is_billable BOOLEAN DEFAULT true,
    is_running BOOLEAN DEFAULT false,
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
-- FIM - WORKSPACE, TASK, CALENDAR, TIME TRACKING
-- Demais módulos (Finance, Invoicing, etc) serão criados depois
-- ==============================================================================

