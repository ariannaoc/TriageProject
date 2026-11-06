// database.cpp
#include "database.h"
#include <iostream>

sqlite3* Database::connect(const std::string& dbPath) {
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error al abrir BD: " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }
    return db;
}

bool Database::authenticateCenter(sqlite3* db, const std::string& username, const std::string& password) {
    const char* sql = "SELECT password_hash FROM users WHERE username = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    // Consulta preparada: previene inyección SQL
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* hashText = sqlite3_column_text(stmt, 0);
        std::string storedHash(reinterpret_cast<const char*>(hashText));

        // Verificar contraseña con bcrypt
        if (BCrypt::validatePassword(password, storedHash)) {
            success = true;
        }
    }
    sqlite3_finalize(stmt);
    return success;
}

void Database::registerCenter(sqlite3* db, int centerId, const std::string& username, const std::string& password) {
    // Generar hash bcrypt (costo 12 por defecto)
    std::string hash = BCrypt::generateHash(password);

    const char* sql = "INSERT INTO users (center_id, username, password_hash) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, centerId);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, hash.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Error al registrar: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
}
