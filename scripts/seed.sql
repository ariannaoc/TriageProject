-- Limpiar datos existentes (en orden por dependencias)
DELETE FROM triage_records;
DELETE FROM patients;
DELETE FROM users;
DELETE FROM centers;

-- Reiniciar contadores de AUTOINCREMENT para IDs deterministas
DELETE FROM sqlite_sequence WHERE name IN ('centers', 'users', 'triage_records');

-- Centros de salud (con coordenadas reales de Mérida, Venezuela)
-- IDs resultantes: IAHULA=1, Sor Juana=2, El Llano=3
INSERT INTO centers (name, type, max_level, capacity, location, latitude, longitude) VALUES
    ('IAHULA - Instituto Autónomo Hospital Universitario de Los Andes', 'Hospital Tipo IV', 1, 150, 'Av. 16 de Septiembre, Sector Campo de Oro, Parroquia Caracciolo Parra Pérez, Mérida', 8.5892, -71.1578),
    ('Hospital Sor Juana Inés de la Cruz (Maternidad)', 'Hospital Tipo III', 2, 60, 'Av. Urdaneta, esquina con Av. 5, Sector Glorias Patrias, Parroquia Domingo Peña, Mérida', 8.5925, -71.1460),
    ('Ambulatorio Urbano Tipo III El Llano', 'Ambulatorio Urbano Tipo III', 3, 25, 'Av. Los Próceres, Urb. Las Tapias, Sector El Llano, Parroquia Osuna Rodríguez, Mérida', 8.5880, -71.1610),
    ('Ambulatorio Urbano Tipo I San Jacinto (PUMM)', 'Ambulatorio Urbano Tipo I', 4, 5, 'Sector San Jacinto, Plaza Bolívar de San Jacinto, Parroquia Jacinto Plaza, Mérida', 8.5745, -71.1610),
    ('Ambulatorio Urbano Tipo I Arquidiócesano', 'Ambulatorio Urbano Tipo I', 4, 5, 'Sector Belén / Centro, Calle 17 entre Av. 2 y 3, Parroquia Arias, Mérida', 8.6015, -71.1410),
    ('Ambulatorio Urbano Tipo I La Carabobo', 'Ambulatorio Urbano Tipo I', 4, 5, 'Sector Cuenca del Chama, Barrio Carabobo, Parroquia Jacinto Plaza, Mérida', 8.5695, -71.1480);

-- Datos operativos para el motor de enrutamiento (max_level es inverso a complejidad)
UPDATE centers SET
    nivel_complejidad = 5 - max_level,
    capacidad_total = capacity,
    cola_actual = 0;

UPDATE centers SET cola_actual = 12 WHERE center_id = 1;
UPDATE centers SET cola_actual = 6 WHERE center_id = 2;

-- Limpiar usuarios previos de prueba
DELETE FROM users;
DELETE FROM sqlite_sequence WHERE name='users';

-- Usuarios de prueba (contraseñas hasheadas con SHA-256)
-- IAHULA: usuario 'iahula_admin', contraseña 'admin123'
INSERT INTO users (username, password_hash, role, center_id)
SELECT 'iahula_admin', '240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9', 'ADMIN', 1;

-- Sor Juana: usuario 'sorjuana_admin', contraseña '123456'
INSERT INTO users (username, password_hash, role, center_id)
SELECT 'sorjuana_admin', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', 'ADMIN', 2;
