-- =====================================================
-- CALENDAR MODULE SCHEMA
-- =====================================================

-- Events table
CREATE TABLE IF NOT EXISTS events (
    id TEXT PRIMARY KEY,
    tenant_id TEXT NOT NULL,
    workspace_id TEXT,
    
    -- Basic info
    title TEXT NOT NULL,
    description TEXT,
    location TEXT,
    
    -- Type and status
    event_type TEXT NOT NULL DEFAULT 'meeting',
    status TEXT NOT NULL DEFAULT 'confirmed',
    
    -- Date/time
    start_time TIMESTAMP NOT NULL,
    end_time TIMESTAMP NOT NULL,
    all_day BOOLEAN DEFAULT FALSE,
    timezone TEXT DEFAULT 'UTC',
    
    -- Ownership
    created_by TEXT NOT NULL,
    organizer_id TEXT,
    
    -- Related entities
    task_id TEXT,
    
    -- Recurring
    is_recurring BOOLEAN DEFAULT FALSE,
    recurring_rule_id TEXT,
    parent_event_id TEXT,
    
    -- Visibility
    is_private BOOLEAN DEFAULT FALSE,
    
    -- Metadata
    color TEXT,
    tags TEXT[],
    
    -- Timestamps
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    -- Constraints
    CONSTRAINT valid_event_type CHECK (event_type IN ('meeting', 'task', 'reminder', 'birthday', 'holiday', 'other')),
    CONSTRAINT valid_status CHECK (status IN ('confirmed', 'tentative', 'cancelled')),
    CONSTRAINT valid_times CHECK (end_time > start_time)
);

-- Event participants table
CREATE TABLE IF NOT EXISTS event_participants (
    id TEXT PRIMARY KEY,
    event_id TEXT NOT NULL,
    tenant_id TEXT NOT NULL,
    
    user_id TEXT NOT NULL,
    
    -- Participation
    role TEXT NOT NULL DEFAULT 'attendee',
    status TEXT NOT NULL DEFAULT 'pending',
    
    -- Metadata
    response_time TIMESTAMP,
    
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    -- Constraints
    CONSTRAINT valid_role CHECK (role IN ('organizer', 'required', 'optional', 'attendee')),
    CONSTRAINT valid_participant_status CHECK (status IN ('pending', 'accepted', 'declined', 'tentative')),
    
    -- Unique constraint
    UNIQUE(event_id, user_id)
);

-- Event reminders table
CREATE TABLE IF NOT EXISTS event_reminders (
    id TEXT PRIMARY KEY,
    event_id TEXT NOT NULL,
    tenant_id TEXT NOT NULL,
    
    user_id TEXT NOT NULL,
    
    -- Reminder config
    reminder_type TEXT NOT NULL DEFAULT 'notification',
    minutes_before INTEGER NOT NULL DEFAULT 15,
    
    -- Status
    is_sent BOOLEAN DEFAULT FALSE,
    sent_at TIMESTAMP,
    
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    -- Constraints
    CONSTRAINT valid_reminder_type CHECK (reminder_type IN ('notification', 'email', 'sms')),
    CONSTRAINT valid_minutes CHECK (minutes_before >= 0)
);

-- Recurring rules table
CREATE TABLE IF NOT EXISTS recurring_rules (
    id TEXT PRIMARY KEY,
    tenant_id TEXT NOT NULL,
    
    -- Recurrence pattern
    frequency TEXT NOT NULL,
    interval INTEGER NOT NULL DEFAULT 1,
    
    -- Week/month specific
    by_day TEXT[],
    by_month_day INTEGER[],
    by_month INTEGER[],
    
    -- End conditions
    count INTEGER,
    until_date TIMESTAMP,
    
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    -- Constraints
    CONSTRAINT valid_frequency CHECK (frequency IN ('daily', 'weekly', 'monthly', 'yearly')),
    CONSTRAINT valid_interval CHECK (interval > 0)
);

-- Indexes for performance
CREATE INDEX IF NOT EXISTS idx_events_created_by ON events(created_by);
CREATE INDEX IF NOT EXISTS idx_events_start_time ON events(start_time);

CREATE INDEX IF NOT EXISTS idx_participants_event ON event_participants(event_id);
CREATE INDEX IF NOT EXISTS idx_participants_user ON event_participants(user_id);

CREATE INDEX IF NOT EXISTS idx_reminders_event ON event_reminders(event_id);
CREATE INDEX IF NOT EXISTS idx_reminders_user ON event_reminders(user_id);

-- Trigger for updated_at
CREATE OR REPLACE FUNCTION update_events_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER update_events_updated_at
    BEFORE UPDATE ON events
    FOR EACH ROW
    EXECUTE FUNCTION update_events_updated_at();

CREATE TRIGGER update_participants_updated_at
    BEFORE UPDATE ON event_participants
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

