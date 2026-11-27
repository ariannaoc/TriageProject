#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

#include "libs/httplib.h"
#include "libs/json.hpp"
#include <sqlite3.h>
#include <openssl/sha.h>

using json = nlohmann::json;
namespace fs = std::filesystem;

std::mutex json_mutex;

// Resuelve la raíz del repo y los scripts aunque el server se ejecute desde build/.
fs::path rutaBaseDatos() {
    if (const char* env = std::getenv("TRIAGE_DB_PATH")) {
        return fs::path(env);
    }
    if (const char* env = std::getenv("TRIAGE_DATA_DIR")) {
        return fs::path(env) / "triage.db";
    }
    return fs::current_path() / "data" / "triage.db";
}

fs::path rutaPacientesTemp() {
    return rutaBaseDatos().parent_path() / "temp_patients.json";
}

std::string hashPassword(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    std::string hex = "";
    char buf[3];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(buf, "%02x", hash[i]);
        hex += buf;
    }
    return hex;
}

fs::path resolverRaizProyecto() {
    fs::path cursor = fs::current_path();
    for (int i = 0; i < 6 && !cursor.empty(); ++i) {
        if (fs::exists(cursor / "scripts" / "database_setup.sql") || fs::exists(cursor / "CMakeLists.txt")) {
            return cursor;
        }
        cursor = cursor.parent_path();
    }
    return fs::current_path();
}

fs::path buscarScript(const fs::path& directorioDatos, const fs::path& raiz, const std::string& nombre) {
    const fs::path candidatos[] = {
        directorioDatos / "scripts" / nombre,
        directorioDatos.parent_path() / "scripts" / nombre,
        raiz / "scripts" / nombre,
        fs::current_path() / "scripts" / nombre
    };
    for (const auto& candidato : candidatos) {
        if (fs::exists(candidato)) {
            return candidato;
        }
    }
    return {};
}

bool tieneTabla(sqlite3* db, const char* tabla) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT 1 FROM sqlite_master WHERE type='table' AND name = ? LIMIT 1;", -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(stmt, 1, tabla, -1, SQLITE_TRANSIENT);
    const bool existe = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);
    return existe;
}

int contarCentros(sqlite3* db) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM centers;", -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }
    int total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return total;
}

bool tieneColumna(sqlite3* db, const char* tabla, const char* columna) {
    const std::string sql = std::string("PRAGMA table_info(") + tabla + ");";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    bool existe = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const auto* nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (nombre != nullptr && std::string(nombre) == columna) {
            existe = true;
            break;
        }
    }
    sqlite3_finalize(stmt);
    return existe;
}

void migrarColumnasCentros(sqlite3* db) {
    struct Columna { const char* nombre; const char* ddl; };
    const Columna columnas[] = {
        {"nivel_complejidad", "ALTER TABLE centers ADD COLUMN nivel_complejidad INTEGER CHECK(nivel_complejidad BETWEEN 1 AND 4);"},
        {"capacidad_total", "ALTER TABLE centers ADD COLUMN capacidad_total INTEGER CHECK(capacidad_total >= 0);"},
        {"cola_actual", "ALTER TABLE centers ADD COLUMN cola_actual INTEGER NOT NULL DEFAULT 0;"}
    };
    for (const auto& columna : columnas) {
        if (tieneColumna(db, "centers", columna.nombre)) {
            continue;
        }
        char* error = nullptr;
        if (sqlite3_exec(db, columna.ddl, nullptr, nullptr, &error) != SQLITE_OK) {
            std::cerr << "No se pudo migrar " << columna.nombre << ": "
                      << (error != nullptr ? error : sqlite3_errmsg(db)) << std::endl;
            sqlite3_free(error);
        } else {
            std::cout << "Columna migrada en centers: " << columna.nombre << std::endl;
        }
    }
    sqlite3_exec(db, "UPDATE centers SET nivel_complejidad = 5 - max_level WHERE nivel_complejidad IS NULL;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "UPDATE centers SET capacidad_total = capacity WHERE capacidad_total IS NULL;", nullptr, nullptr, nullptr);
}

bool ejecutarSql(sqlite3* db, const fs::path& archivo, std::string& mensajeError) {
    std::ifstream entrada(archivo);
    if (!entrada.is_open()) {
        mensajeError = "No se pudo abrir " + archivo.string();
        return false;
    }
    std::string sql((std::istreambuf_iterator<char>(entrada)), std::istreambuf_iterator<char>());
    char* error = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
        mensajeError = error != nullptr ? error : sqlite3_errmsg(db);
        sqlite3_free(error);
        return false;
    }
    return true;
}

bool inicializarBaseDatos(const fs::path& rutaDb) {
    std::error_code ec;
    fs::create_directories(rutaDb.parent_path(), ec);
    if (ec) {
        std::cerr << "No se pudo crear el directorio de datos: " << rutaDb.parent_path() << std::endl;
        return false;
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(rutaDb.string().c_str(), &db) != SQLITE_OK) {
        std::cerr << "No se pudo abrir la base de datos " << rutaDb << std::endl;
        return false;
    }

    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    const bool esNueva = !tieneTabla(db, "centers");
    const fs::path raiz = resolverRaizProyecto();
    const fs::path directorioDatos = rutaDb.parent_path();

    if (esNueva) {
        const fs::path script = buscarScript(directorioDatos, raiz, "database_setup.sql");
        if (script.empty()) {
            std::cerr << "No se encontró scripts/database_setup.sql para inicializar " << rutaDb << std::endl;
            sqlite3_close(db);
            return false;
        }
        std::string mensajeError;
        if (!ejecutarSql(db, script, mensajeError)) {
            std::cerr << "Error creando el esquema: " << mensajeError << std::endl;
            sqlite3_close(db);
            return false;
        }
        std::cout << "Esquema inicializado desde " << script.string() << std::endl;
    }

    migrarColumnasCentros(db);

    if (contarCentros(db) == 0) {
        const fs::path semilla = buscarScript(directorioDatos, raiz, "seed.sql");
        std::string mensajeError;
        if (!semilla.empty() && ejecutarSql(db, semilla, mensajeError)) {
            std::cout << "Datos semilla cargados desde " << semilla.string() << std::endl;
        } else if (semilla.empty()) {
            std::cerr << "No se encontró scripts/seed.sql para poblar la base de datos" << std::endl;
        } else {
            std::cerr << "No se pudieron cargar los datos semilla: " << mensajeError << std::endl;
        }
    }

    sqlite3_close(db);
    return true;
}

json readTempPatients() {
    const fs::path path = rutaPacientesTemp();
    std::ifstream f(path);
    if (!f.is_open())
        return json{{"pending_patients", json::array()}};
    try {
        return json::parse(f);
    }
    catch (...) {
        return json{{"pending_patients", json::array()}};
    }
}

void writeTempPatients(const json& data) {
    const fs::path path = rutaPacientesTemp();
    std::ofstream f(path);
    f << data.dump(4);
}

int main() {
    httplib::Server svr;

    const std::string db_path = rutaBaseDatos().string();
    if (!inicializarBaseDatos(db_path)) {
        std::cerr << "No se pudo preparar la base de datos en " << db_path << std::endl;
        return 1;
    }
    std::cout << "Base de datos: " << db_path << std::endl;

    auto get_db = [&]() -> sqlite3*
    {
        sqlite3* db;
        if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK)
        {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return nullptr;
        }
        sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
        sqlite3_busy_timeout(db, 5000);
        return db;
    };

    svr.Get("/api/health",
            [](const httplib::Request&, httplib::Response& res)
            { res.set_content(json{{"status", "ok"}}.dump(), "application/json"); });

    svr.Get("/api/centers",
            [&](const httplib::Request&, httplib::Response& res)
            {
        sqlite3* db = get_db();
        if (!db) { res.status = 500; return; }

        const char* sql = "SELECT center_id, name, type, max_level, capacity, location, latitude, longitude, "
                          "nivel_complejidad, capacidad_total, cola_actual FROM centers;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

        json result = json::array();
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            result.push_back({
                {"center_id", sqlite3_column_int(stmt, 0)},
                {"name", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))},
                {"type", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))},
                {"max_level", sqlite3_column_int(stmt, 3)},
                {"capacity", sqlite3_column_int(stmt, 4)},
                {"location", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))},
                {"latitude", sqlite3_column_double(stmt, 6)},
                {"longitude", sqlite3_column_double(stmt, 7)},
                {"nivel_complejidad", sqlite3_column_int(stmt, 8)},
                {"capacidad_total", sqlite3_column_int(stmt, 9)},
                {"capacidad_disponible", sqlite3_column_int(stmt, 4)},
                {"cola_actual", sqlite3_column_int(stmt, 10)}
            });
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        res.set_content(json{{"centers", result}}.dump(), "application/json"); });

    svr.Get("/api/centers/:id/pending",
            [&](const httplib::Request& req, httplib::Response& res)
            {
        int id = std::stoi(req.path_params.at("id"));
        std::lock_guard<std::mutex> lock(json_mutex);
        json data = readTempPatients();
        json result = json::array();
        for (auto& p : data["pending_patients"]) {
            if (p["center_id"] == id) result.push_back(p);
        }
        res.set_content(json{{"patients", result}}.dump(), "application/json"); });

    svr.Post("/api/temp-patients",
             [&](const httplib::Request& req, httplib::Response& res)
             {
        try {
            auto new_p = json::parse(req.body);
            std::lock_guard<std::mutex> lock(json_mutex);

            json data = readTempPatients();
            int max_id = 0;
            for (const auto& p : data["pending_patients"])
                max_id = std::max(max_id, p["id"].get<int>());
            new_p["id"] = max_id + 1;

            data["pending_patients"].push_back(new_p);
            writeTempPatients(data);
            res.set_content(json{{"status", "success"}, {"id", new_p["id"]}}.dump(), "application/json");
        } catch (...) { res.status = 400; } });

    svr.Post("/api/centers/:centerId/admit/:tempId",
             [&](const httplib::Request& req, httplib::Response& res)
             {
        int centerId = std::stoi(req.path_params.at("centerId"));
        int tempId = std::stoi(req.path_params.at("tempId"));
        auto body = json::parse(req.body);
        std::string cedula = body["cedula"];

        sqlite3* db = get_db();
        if (!db) { res.status = 500; return; }

        sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION;", nullptr, nullptr, nullptr);
        bool success = true;

        std::lock_guard<std::mutex> lock(json_mutex);
        json data = readTempPatients();
        json target;
        bool found = false;
        json remaining = json::array();

        for (auto& p : data["pending_patients"]) {
            if (p["id"] == tempId && p["center_id"] == centerId) {
                target = p;
                found = true;
            } else {
                remaining.push_back(p);
            }
        }

        if (!found) {
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            res.status = 404;
            sqlite3_close(db);
            return;
        }

        // 1. Insertar en Pacientes
        const char* sqlPat = "INSERT INTO patients (cedula, first_name, last_name, age, sex) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt* stmtPat;
        sqlite3_prepare_v2(db, sqlPat, -1, &stmtPat, nullptr);
        sqlite3_bind_text(stmtPat, 1, cedula.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmtPat, 2, target["first_name"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmtPat, 3, target["last_name"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmtPat, 4, target["age"].get<int>());
        std::string sexo = target.contains("sex") && target["sex"].is_string() ? target["sex"].get<std::string>() : "M";
        sqlite3_bind_text(stmtPat, 5, sexo.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmtPat) != SQLITE_DONE) success = false;
        sqlite3_finalize(stmtPat);

        // 2. Insertar en Triage
        if (success) {
            const char* sqlTri = "INSERT INTO triage_records (cedula, center_id, triage_level, triage_score, symptoms, ambulance_requested, status) VALUES (?, ?, ?, ?, ?, ?, 'ADMITTED');";
            sqlite3_stmt* stmtTri;
            sqlite3_prepare_v2(db, sqlTri, -1, &stmtTri, nullptr);
            sqlite3_bind_text(stmtTri, 1, cedula.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmtTri, 2, centerId);
            sqlite3_bind_int(stmtTri, 3, target["triage_level"].get<int>());
            sqlite3_bind_int(stmtTri, 4, target["triage_score"].get<int>());
            sqlite3_bind_text(stmtTri, 5, target["symptoms"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmtTri, 6, target["ambulance_requested"].get<bool>() ? 1 : 0);
            if (sqlite3_step(stmtTri) != SQLITE_DONE) success = false;
            sqlite3_finalize(stmtTri);
        }

        // 3. Reducir capacidad
        if (success) {
            const char* sqlCap = "UPDATE centers SET cola_actual = cola_actual + 1 WHERE center_id = ?;";
            sqlite3_stmt* stmtCap;
            sqlite3_prepare_v2(db, sqlCap, -1, &stmtCap, nullptr);
            sqlite3_bind_int(stmtCap, 1, centerId);
            sqlite3_step(stmtCap);
            sqlite3_finalize(stmtCap);
        }

        if (success) {
            sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
            data["pending_patients"] = remaining;
            writeTempPatients(data);
            res.set_content(json{{"status", "success"}}.dump(), "application/json");
        } else {
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            res.status = 500;
        }
        sqlite3_close(db); });

    svr.Post("/api/login",
             [&](const httplib::Request& req, httplib::Response& res)
             {
        try {
            auto body = json::parse(req.body);
            std::string u = body["username"];
            std::string p = hashPassword(body["password"]);

            sqlite3* db = get_db();
            if (!db) { res.status = 500; return; }

            const char* sql = "SELECT u.username, u.role, c.center_id, c.name, c.type, c.max_level, c.capacity "
                              "FROM users u JOIN centers c ON u.center_id = c.center_id "
                              "WHERE u.username = ? AND u.password_hash = ?;";
            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
            sqlite3_bind_text(stmt, 1, u.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, p.c_str(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                json user = {
                    {"username", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
                    {"role", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))},
                    {"center_id", sqlite3_column_int(stmt, 2)},
                    {"center_name", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))},
                    {"center_type", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))},
                    {"max_level", sqlite3_column_int(stmt, 5)},
                    {"capacity", sqlite3_column_int(stmt, 6)}
                };
                res.set_content(json{{"status", "success"}, {"user", user}}.dump(), "application/json");
            } else {
                res.status = 401;
                res.set_content(json{{"status", "error"}, {"message", "Credenciales inválidas"}}.dump(), "application/json");
            }
            sqlite3_finalize(stmt);
            sqlite3_close(db);
        } catch (...) { res.status = 400; } });

    std::cout << "Iniciando servidor Triage en http://0.0.0.0:8081..." << std::endl;
    // -------------------------------------------------------------------------
    // KPI 7 — Tendencias históricas por día y nivel de triaje
    // GET /api/centers/:id/trends?days=N
    // -------------------------------------------------------------------------
    svr.Get("/api/centers/:id/trends", [](const httplib::Request& req, httplib::Response& res) {
        sqlite3* db = nullptr;
        if (sqlite3_open(rutaBaseDatos().string().c_str(), &db) != SQLITE_OK) {
            res.status = 500;
            res.set_content(json{{"error", "No se pudo abrir la base de datos"}}.dump(), "application/json");
            if (db) sqlite3_close(db);
            return;
        }

        int centerId = 0;
        try {
            centerId = std::stoi(req.path_params.at("id"));
        } catch (...) {
            res.status = 400;
            res.set_content(json{{"error", "center_id inválido"}}.dump(), "application/json");
            sqlite3_close(db);
            return;
        }

        int days = 7;
        if (req.has_param("days")) {
            try { days = std::stoi(req.get_param_value("days")); } catch (...) {}
            if (days < 1)  days = 1;
            if (days > 90) days = 90;
        }

        const char* sql =
            "SELECT date(created_at) AS dia, "
            "  SUM(CASE WHEN triage_level=1 THEN 1 ELSE 0 END) AS lvl1, "
            "  SUM(CASE WHEN triage_level=2 THEN 1 ELSE 0 END) AS lvl2, "
            "  SUM(CASE WHEN triage_level=3 THEN 1 ELSE 0 END) AS lvl3, "
            "  SUM(CASE WHEN triage_level=4 THEN 1 ELSE 0 END) AS lvl4, "
            "  SUM(CASE WHEN triage_level=5 THEN 1 ELSE 0 END) AS lvl5, "
            "  COUNT(*) AS total "
            "FROM triage_records "
            "WHERE center_id = ? "
            "  AND date(created_at) >= date('now', ?) "
            "GROUP BY date(created_at) "
            "ORDER BY dia ASC;";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            res.status = 500;
            res.set_content(json{{"error", "Error de consulta"}}.dump(), "application/json");
            sqlite3_close(db);
            return;
        }

        sqlite3_bind_int(stmt, 1, centerId);
        const std::string offset = "-" + std::to_string(days - 1) + " days";
        sqlite3_bind_text(stmt, 2, offset.c_str(), -1, SQLITE_TRANSIENT);

        json trends = json::array();
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* dia = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            trends.push_back({
                {"date",    dia ? dia : ""},
                {"level_1", sqlite3_column_int(stmt, 1)},
                {"level_2", sqlite3_column_int(stmt, 2)},
                {"level_3", sqlite3_column_int(stmt, 3)},
                {"level_4", sqlite3_column_int(stmt, 4)},
                {"level_5", sqlite3_column_int(stmt, 5)},
                {"total",   sqlite3_column_int(stmt, 6)}
            });
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        res.set_content(json{
            {"center_id", centerId},
            {"days", days},
            {"trends", trends}
        }.dump(), "application/json");
    });


    svr.Post("/api/centers/:centerId/discharge/:cedula",
             [&](const httplib::Request& req, httplib::Response& res)
             {
        int centerId = std::stoi(req.path_params.at("centerId"));
        std::string cedula = req.path_params.at("cedula");

        sqlite3* db = get_db();
        if (!db) { res.status = 500; return; }

        sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION;", nullptr, nullptr, nullptr);
        bool success = true;

        // 1. Cambiar estado a DISCHARGED
        const char* sqlDis = "UPDATE triage_records SET status = 'DISCHARGED' WHERE cedula = ? AND center_id = ? AND status = 'ADMITTED';";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sqlDis, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, cedula.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, centerId);
        if (sqlite3_step(stmt) != SQLITE_DONE) success = false;
        sqlite3_finalize(stmt);

        // 2. Restar a cola_actual
        if (success) {
            const char* sqlCap = "UPDATE centers SET cola_actual = CASE WHEN cola_actual > 0 THEN cola_actual - 1 ELSE 0 END WHERE center_id = ?;";
            sqlite3_prepare_v2(db, sqlCap, -1, &stmt, nullptr);
            sqlite3_bind_int(stmt, 1, centerId);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }

        if (success) {
            sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
            res.set_content(json{{"status", "success"}}.dump(), "application/json");
        } else {
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            res.status = 500;
        }
        sqlite3_close(db); });

    svr.listen("0.0.0.0", 8081);

    return 0;
}
