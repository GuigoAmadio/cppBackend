-- =========================================================
-- TASK MANAGEMENT MODULE - SQL SCHEMA
-- =========================================================
-- Created: 2025-10-22
-- Description: Complete task management system with comments,
--              attachments, checklists, dependencies, and tags
-- =========================================================

-- =========================================================
-- 1. TASKS TABLE
-- =========================================================
CREATE TABLE IF NOT EXISTS tasks (
    id TEXT PRIMARY KEY,
    tenant_id TEXT NOT NULL,
    workspace_id TEXT,
    project_id TEXT,
    
    -- Basic Info
    title TEXT NOT NULL,
    description TEXT,
    task_type TEXT NOT NULL DEFAULT 'task', -- task, bug, feature, story, epic
    
    -- Status & Priority
    status TEXT NOT NULL DEFAULT 'todo', -- todo, in_progress, review, blocked, done, cancelled
    priority TEXT NOT NULL DEFAULT 'medium', -- low, medium, high, urgent
    
    -- Assignment
    created_by TEXT NOT NULL,
    assigned_to TEXT,
    reporter TEXT,
    
    -- Planning
    estimated_hours NUMERIC(10,2),
    actual_hours NUMERIC(10,2) DEFAULT 0,
    story_points INTEGER,
    
    -- Dates
    start_date TIMESTAMP,
    due_date TIMESTAMP,
    completed_at TIMESTAMP,
    
    -- Relationships
    parent_task_id TEXT,
    depends_on TEXT[], -- Array of task IDs
    blocks TEXT[], -- Array of task IDs this task blocks
    
    -- Tags & Labels
    tags TEXT[],
    labels TEXT[],
    
    -- Metadata
    position INTEGER DEFAULT 0, -- For ordering/kanban
    is_archived BOOLEAN DEFAULT FALSE,
    
    -- Timestamps
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    -- Constraints
    CONSTRAINT valid_task_type CHECK (task_type IN ('task', 'bug', 'feature', 'story', 'epic', 'subtask')),
    CONSTRAINT valid_status CHECK (status IN ('todo', 'in_progress', 'review', 'blocked', 'done', 'cancelled')),
    CONSTRAINT valid_priority CHECK (priority IN ('low', 'medium', 'high', 'urgent'))
);

-- Indexes for tasks
CREATE INDEX IF NOT EXISTS idx_tasks_tenant ON tasks(tenant_id);
CREATE INDEX IF NOT EXISTS idx_tasks_workspace ON tasks(workspace_id);
CREATE INDEX IF NOT EXISTS idx_tasks_project ON tasks(project_id);
CREATE INDEX IF NOT EXISTS idx_tasks_assigned_to ON tasks(assigned_to);
CREATE INDEX IF NOT EXISTS idx_tasks_created_by ON tasks(created_by);
CREATE INDEX IF NOT EXISTS idx_tasks_status ON tasks(status);
CREATE INDEX IF NOT EXISTS idx_tasks_priority ON tasks(priority);
CREATE INDEX IF NOT EXISTS idx_tasks_due_date ON tasks(due_date);
CREATE INDEX IF NOT EXISTS idx_tasks_parent ON tasks(parent_task_id);
CREATE INDEX IF NOT EXISTS idx_tasks_archived ON tasks(is_archived);

-- =========================================================
-- 2. TASK COMMENTS
-- =========================================================
CREATE TABLE IF NOT EXISTS task_comments (
    id TEXT PRIMARY KEY,
    task_id TEXT NOT NULL,
    tenant_id TEXT NOT NULL,
    
    -- Content
    content TEXT NOT NULL,
    
    -- Author
    created_by TEXT NOT NULL,
    
    -- Thread
    parent_comment_id TEXT, -- For replies/threads
    
    -- Metadata
    is_edited BOOLEAN DEFAULT FALSE,
    is_deleted BOOLEAN DEFAULT FALSE,
    
    -- Timestamps
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for comments
CREATE INDEX IF NOT EXISTS idx_comments_task ON task_comments(task_id);
CREATE INDEX IF NOT EXISTS idx_comments_tenant ON task_comments(tenant_id);
CREATE INDEX IF NOT EXISTS idx_comments_created_by ON task_comments(created_by);
CREATE INDEX IF NOT EXISTS idx_comments_parent ON task_comments(parent_comment_id);

-- =========================================================
-- 3. TASK ATTACHMENTS
-- =========================================================
CREATE TABLE IF NOT EXISTS task_attachments (
    id TEXT PRIMARY KEY,
    task_id TEXT NOT NULL,
    tenant_id TEXT NOT NULL,
    
    -- File Info
    filename TEXT NOT NULL,
    original_filename TEXT NOT NULL,
    file_size BIGINT NOT NULL,
    mime_type TEXT NOT NULL,
    file_path TEXT NOT NULL, -- Storage path/URL
    
    -- Metadata
    uploaded_by TEXT NOT NULL,
    description TEXT,
    
    -- Timestamps
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for attachments
CREATE INDEX IF NOT EXISTS idx_attachments_task ON task_attachments(task_id);
CREATE INDEX IF NOT EXISTS idx_attachments_tenant ON task_attachments(tenant_id);
CREATE INDEX IF NOT EXISTS idx_attachments_uploaded_by ON task_attachments(uploaded_by);

-- =========================================================
-- 4. TASK CHECKLISTS
-- =========================================================
CREATE TABLE IF NOT EXISTS task_checklists (
    id TEXT PRIMARY KEY,
    task_id TEXT NOT NULL,
    tenant_id TEXT NOT NULL,
    
    -- Info
    title TEXT NOT NULL,
    description TEXT,
    position INTEGER DEFAULT 0,
    
    -- Timestamps
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for checklists
CREATE INDEX IF NOT EXISTS idx_checklists_task ON task_checklists(task_id);
CREATE INDEX IF NOT EXISTS idx_checklists_tenant ON task_checklists(tenant_id);

-- =========================================================
-- 5. CHECKLIST ITEMS
-- =========================================================
CREATE TABLE IF NOT EXISTS checklist_items (
    id TEXT PRIMARY KEY,
    checklist_id TEXT NOT NULL,
    tenant_id TEXT NOT NULL,
    
    -- Content
    content TEXT NOT NULL,
    
    -- Status
    is_completed BOOLEAN DEFAULT FALSE,
    completed_by TEXT,
    completed_at TIMESTAMP,
    
    -- Ordering
    position INTEGER DEFAULT 0,
    
    -- Timestamps
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for checklist items
CREATE INDEX IF NOT EXISTS idx_checklist_items_checklist ON checklist_items(checklist_id);
CREATE INDEX IF NOT EXISTS idx_checklist_items_tenant ON checklist_items(tenant_id);
CREATE INDEX IF NOT EXISTS idx_checklist_items_completed ON checklist_items(is_completed);

-- =========================================================
-- 6. TASK ACTIVITY LOG (Audit/History)
-- =========================================================
CREATE TABLE IF NOT EXISTS task_activity (
    id TEXT PRIMARY KEY,
    task_id TEXT NOT NULL,
    tenant_id TEXT NOT NULL,
    
    -- Activity
    activity_type TEXT NOT NULL, -- created, updated, commented, status_changed, assigned, etc.
    field_name TEXT, -- Which field changed
    old_value TEXT,
    new_value TEXT,
    
    -- Actor
    user_id TEXT NOT NULL,
    
    -- Timestamp
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for activity
CREATE INDEX IF NOT EXISTS idx_activity_task ON task_activity(task_id);
CREATE INDEX IF NOT EXISTS idx_activity_tenant ON task_activity(tenant_id);
CREATE INDEX IF NOT EXISTS idx_activity_user ON task_activity(user_id);
CREATE INDEX IF NOT EXISTS idx_activity_type ON task_activity(activity_type);

-- =========================================================
-- TRIGGERS FOR updated_at
-- =========================================================
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Apply triggers
DROP TRIGGER IF EXISTS update_tasks_updated_at ON tasks;
CREATE TRIGGER update_tasks_updated_at 
    BEFORE UPDATE ON tasks 
    FOR EACH ROW 
    EXECUTE FUNCTION update_updated_at_column();

DROP TRIGGER IF EXISTS update_task_comments_updated_at ON task_comments;
CREATE TRIGGER update_task_comments_updated_at 
    BEFORE UPDATE ON task_comments 
    FOR EACH ROW 
    EXECUTE FUNCTION update_updated_at_column();

DROP TRIGGER IF EXISTS update_task_checklists_updated_at ON task_checklists;
CREATE TRIGGER update_task_checklists_updated_at 
    BEFORE UPDATE ON task_checklists 
    FOR EACH ROW 
    EXECUTE FUNCTION update_updated_at_column();

DROP TRIGGER IF EXISTS update_checklist_items_updated_at ON checklist_items;
CREATE TRIGGER update_checklist_items_updated_at 
    BEFORE UPDATE ON checklist_items 
    FOR EACH ROW 
    EXECUTE FUNCTION update_updated_at_column();

-- =========================================================
-- END OF TASK MANAGEMENT SCHEMA
-- =========================================================

