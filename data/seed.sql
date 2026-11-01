-- Limpiar registros previos
DELETE FROM centers;
DELETE FROM sqlite_sequence WHERE name='centers';

INSERT INTO centers (name, type, max_level, capacity, location, latitude, longitude)
VALUES 
    ('Instituto Autónomo Hospital Universitario de Los Andes (IAHULA)', 'Hospital Tipo IV', 1, 150, 'Av. 16 de Septiembre, Parroquia Domingo Peña, Mérida', 8.5839, -71.1578),
    ('Hospital Sor Juana Inés de la Cruz', 'Hospital Tipo II', 2, 50, 'Sector La Hechicera / Av. Alberto Carnevali, Parroquia Spinetti Dini, Mérida', 8.6215, -71.1520),
    ('Hospital Dr. Tulio Carnevalli Salvatierra (IVSS)', 'Hospital Tipo II', 2, 50, 'Av. Las Américas con Av. Ezio Valeri, Parroquia Mariano Picón Salas, Mérida', 8.5985, -71.1602),
    ('Ambulatorio Urbano Tipo III Venezuela', 'Ambulatorio Urbano Tipo III', 3, 8, 'Calle 16 entre Av. 3 y 4 / Urb. Humboldt, Parroquia Spinetti Dini, Mérida', 8.6112, -71.1465),
    ('Ambulatorio Urbano Tipo III Belén', 'Ambulatorio Urbano Tipo III', 3, 6, 'Pasaje Sánchez, Sector Belén, Parroquia Arias, Mérida', 8.6042, -71.1396),
    ('Ambulatorio Urbano Tipo III El Llano (Dr. Rafael Baez)', 'Ambulatorio Urbano Tipo III', 3, 6, 'Sector El Llano, Av. 3 entre calles 29 y 30, Parroquia Arias, Mérida', 8.5910, -71.1472),
    ('Ambulatorio Urbano Tipo III Los Sausales', 'Ambulatorio Urbano Tipo III', 3, 6, 'Urbanización Los Sausales, Av. Las Américas, Parroquia Mariano Picón Salas', 8.5855, -71.1685),
    ('Ambulatorio Urbano Tipo III Los Curos', 'Ambulatorio Urbano Tipo III', 3, 10, 'Urbanización Los Curos, Parroquia Osuna Rodríguez, Mérida', 8.5535, -71.1921),
    ('Ambulatorio Urbano Tipo I Humbolt', 'Ambulatorio Urbano Tipo I', 4, 5, 'Urbanización Humboldt, Av. Las Américas, Parroquia Caracciolo Parra Pérez, Mérida', 8.5815, -71.1730),
    ('Ambulatorio Urbano Tipo I La Milagrosa', 'Ambulatorio Urbano Tipo I', 4, 5, 'Calle Principal, Barrio La Milagrosa, Parroquia Milla, Mérida', 8.6185, -71.1350),
    ('Ambulatorio Urbano Tipo I Santa Elena', 'Ambulatorio Urbano Tipo I', 4, 5, 'Sector Santa Elena, Av. 16 de Septiembre, Parroquia Domingo Peña, Mérida', 8.5742, -71.1645),
    ('Ambulatorio Urbano Tipo I La Mara', 'Ambulatorio Urbano Tipo I', 4, 5, 'Calle 1 Guaicaipuro, Urb. La Mara, Parroquia Juan Rodríguez Suárez, Mérida', 8.5620, -71.1850),
    ('Ambulatorio Urbano Tipo I Arquidiócesano', 'Ambulatorio Urbano Tipo I', 4, 5, 'Sector Belén / Centro, Calle 17 entre Av. 2 y 3, Parroquia Arias, Mérida', 8.6015, -71.1410),
    ('Ambulatorio Urbano Tipo I La Carabobo', 'Ambulatorio Urbano Tipo I', 4, 5, 'Sector Cuenca del Chama, Barrio Carabobo, Parroquia Jacinto Plaza, Mérida', 8.5695, -71.1480);