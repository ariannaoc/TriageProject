-- CENTERS
CREATE TABLE centers (
    center_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    type TEXT NOT NULL,
    max_level INTEGER NOT NULL CHECK(max_level BETWEEN 1 AND 4),
    capacity INTEGER NOT NULL CHECK(capacity >= 0),
    location TEXT NOT NULL,
    latitude REAL NOT NULL,
    longitude REAL NOT NULL,
    nivel_complejidad INTEGER CHECK(nivel_complejidad BETWEEN 1 AND 4),
    capacidad_total INTEGER CHECK(capacidad_total >= 0),
    cola_actual INTEGER NOT NULL DEFAULT 0
);

-- USERS
CREATE TABLE users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    role TEXT NOT NULL DEFAULT 'ADMIN',
    center_id INTEGER,
    FOREIGN KEY (center_id) REFERENCES centers(center_id) ON DELETE SET NULL
);

-- PATIENTS
CREATE TABLE patients (
    cedula TEXT PRIMARY KEY,
    first_name TEXT NOT NULL,
    last_name TEXT NOT NULL,
    age INTEGER NOT NULL CHECK(age > 0),
    sex TEXT NOT NULL CHECK(sex IN ('M','F'))
);

-- TRIAGE RECORDS
CREATE TABLE triage_records (
    triage_id INTEGER PRIMARY KEY AUTOINCREMENT,
    cedula TEXT NOT NULL,
    center_id INTEGER NOT NULL,
    triage_level INTEGER NOT NULL CHECK(triage_level BETWEEN 1 AND 5),
    triage_score INTEGER NOT NULL CHECK(triage_score BETWEEN 0 AND 100),
    symptoms TEXT NOT NULL,
    ambulance_requested BOOLEAN NOT NULL DEFAULT 0,
    status TEXT NOT NULL CHECK(status IN ('ADMITTED','DISCHARGED','REFERRED')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (cedula) REFERENCES patients(cedula) ON DELETE CASCADE,
    FOREIGN KEY (center_id) REFERENCES centers(center_id) ON DELETE CASCADE
);

-- INDEXES
CREATE INDEX idx_patients_cedula ON patients(cedula);
CREATE INDEX idx_triage_cedula ON triage_records(cedula);
CREATE INDEX idx_triage_center ON triage_records(center_id);
CREATE INDEX idx_triage_level ON triage_records(triage_level);
