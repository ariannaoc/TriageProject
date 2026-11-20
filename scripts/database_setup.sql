-- CENTERS
CREATE TABLE IF NOT EXISTS centers (
    center_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    type TEXT NOT NULL,
    max_level INTEGER NOT NULL CHECK(max_level BETWEEN 1 AND 5),
    capacity INTEGER NOT NULL CHECK(capacity >= 0),
    location TEXT NOT NULL,
    latitude REAL NOT NULL,
    longitude REAL NOT NULL
);

-- USERS
CREATE TABLE IF NOT EXISTS users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    center_id INTEGER NOT NULL,
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    FOREIGN KEY (center_id) REFERENCES centers (center_id) ON DELETE CASCADE
);

-- PATIENTS
CREATE TABLE IF NOT EXISTS patients (
    cedula TEXT PRIMARY KEY,
    center_id INTEGER NOT NULL,
    first_name TEXT NOT NULL,
    last_name TEXT NOT NULL,
    age INTEGER NOT NULL,
    triage_level INTEGER NULL,
    triage_score INTEGER NULL,
    source TEXT NOT NULL CHECK(source IN ('SYSTEM', 'MANUAL')),
    status TEXT NOT NULL CHECK(status IN ('ADMITTED', 'DISCHARGED')),
    admission_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    discharge_date DATETIME NULL,
    symptoms TEXT NULL,
    FOREIGN KEY (center_id) REFERENCES centers (center_id)
);

-- Indexes
CREATE INDEX IF NOT EXISTS idx_patients_center_status ON patients(center_id, status);
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);