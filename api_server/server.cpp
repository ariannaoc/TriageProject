#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <sqlite3.h>
#include <openssl/sha.h>

#include "libs/httplib.h"
#include "libs/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

std::string DB_PATH;
std::string TEMP_JSON_PATH;

std::mutex json_mutex;
std::mutex db_mutex;

namespace {

std::string variableEntorno(const char* nombre) {
    const char* valor = std::getenv(nombre);
    return valor != nullptr && *valor != '\0' ? std::string(valor) : std::string();
}

fs::path rutaAbsoluta(const fs::path& ruta) {
    std::error_code error;
    const fs::path resultado = fs::absolute(ruta, error);
    return error ? ruta : resultado;
}

fs::path directorioEjecutable(const char* argv0) {
    std::error_code error;
#if defined(__linux__)
    const fs::path desdeProc = fs::canonical("/proc/self/exe", error);
    if (!error && !desdeProc.empty()) {
        return desdeProc.parent_path();
    }
    error.clear();
#endif
    const fs::path desdeArgv = fs::canonical(argv0, error);
    if (!error && !desdeArgv.empty()) {
        return desdeArgv.parent_path();
    }
    error.clear();
    const fs::path absoluta = fs::absolute(argv0, error);
    if (!error && !absoluta.empty()) {
        return absoluta.parent_path();
    }
    return fs::current_path();
}

fs::path raizProyecto(const fs::path& directorio) {
    fs::path actual = directorio;
    for (int nivel = 0; nivel < 3; ++nivel) {
        if (fs::is_directory(actual / "data")) {
            return actual;
        }
        const fs::path padre = actual.parent_path();
        if (padre.empty() || padre == actual) {
            break;
        }
        actual = padre;
    }
    return directorio;
}

fs::path buscarScript(const fs::path& directorioDatos, const fs::path& raiz, const char* nombre) {
    const fs::path candidatos[] = {
        directorioDatos / nombre,
        directorioDatos.parent_path() / "scripts" / nombre,
        raiz / "scripts" / nombre,
        raiz / "data" / nombre
    };
    for (const fs::path& candidato : candidatos) {
        if (fs::is_regular_file(candidato)) {
            return candidato;
        }
    }
    return {};
}

bool hayTablas(sqlite3* db) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name IN ('centers', 'users', 'patients');";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    int encontradas = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ++encontradas;
    }
    sqlite3_finalize(stmt);
    return encontradas == 3;
}

int contarCentros(sqlite3* db) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM centers;", -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }
    int total = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return total;
}

bool ejecutarSql(sqlite3* db, const fs::path& archivo, std::string& mensajeError) {
    std::ifstream entrada(archivo);
    if (!entrada.is_open()) {
        mensajeError = "No se pudo leer " + archivo.string();
        return false;
    }
    std::ostringstream contenido;
    contenido << entrada.rdbuf();
    char* error = nullptr;
    const int codigo = sqlite3_exec(db, contenido.str().c_str(), nullptr, nullptr, &error);
    if (codigo != SQLITE_OK) {
        mensajeError = error != nullptr ? error : sqlite3_errmsg(db);
        sqlite3_free(error);
        return false;
    }
    return true;
}

bool inicializarBase(const fs::path& rutaDb, const fs::path& directorioDatos, const fs::path& raiz) {
    std::cout << "Base de datos Triage: " << rutaAbsoluta(rutaDb).string() << std::endl;

    std::error_code error;
    fs::create_directories(rutaDb.parent_path(), error);
    if (error) {
        std::cerr << "No se pudo crear el directorio de datos: " << error.message() << std::endl;
        return false;
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(rutaDb.c_str(), &db) != SQLITE_OK) {
        std::cerr << "No se pudo abrir la base de datos: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    if (!hayTablas(db)) {
        const fs::path script = buscarScript(directorioDatos, raiz, "database_setup.sql");
        if (script.empty()) {
            std::cerr << "No se encontró database_setup.sql para inicializar el esquema." << std::endl;
            sqlite3_close(db);
            return false;
        }
        std::string mensajeError;
        if (!ejecutarSql(db, script, mensajeError)) {
            std::cerr << "Error al aplicar " << script.string() << ": " << mensajeError << std::endl;
            sqlite3_close(db);
            return false;
        }
        std::cout << "Esquema inicializado desde " << script.string() << std::endl;
    }

    if (contarCentros(db) == 0) {
        const fs::path semilla = buscarScript(directorioDatos, raiz, "seed.sql");
        std::string mensajeError;
        if (!semilla.empty() && ejecutarSql(db, semilla, mensajeError)) {
            std::cout << "Datos de prueba cargados desde " << semilla.string() << std::endl;
        } else if (!semilla.empty()) {
            std::cerr << "Error al cargar " << semilla.string() << ": " << mensajeError << std::endl;
        }
    }

    sqlite3_close(db);
    return true;
}

}

// Generar hash SHA-256 de una cadena usando OpenSSL
std::string sha256(const std::string &input)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(input.c_str()), input.size(), hash);

    std::ostringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

json readTempPatients()
{
    std::ifstream file(TEMP_JSON_PATH);
    if (!file.is_open())
    {
        return json{{"pending_patients", json::array()}};
    }
    json j;
    file >> j;
    return j;
}

void writeTempPatients(const json &j)
{
    std::ofstream file(TEMP_JSON_PATH);
    file << j.dump(2);
}

int main(int argc, char* argv[])
{
    const fs::path raiz = raizProyecto(directorioEjecutable(argc > 0 ? argv[0] : ""));
    fs::path directorioDatos = raiz / "data";
    const std::string dbEntorno = variableEntorno("TRIAGE_DB_PATH");
    const std::string datosEntorno = variableEntorno("TRIAGE_DATA_DIR");

    if (!dbEntorno.empty()) {
        const fs::path rutaDb = rutaAbsoluta(dbEntorno);
        DB_PATH = rutaDb.string();
        directorioDatos = rutaDb.parent_path();
    } else {
        if (!datosEntorno.empty()) {
            directorioDatos = rutaAbsoluta(datosEntorno);
        }
        DB_PATH = (directorioDatos / "triage.db").string();
    }
    TEMP_JSON_PATH = (directorioDatos / "temp_patients.json").string();

    if (!inicializarBase(DB_PATH, directorioDatos, raiz)) {
        return 1;
    }
    std::cout << "Pacientes temporales: " << TEMP_JSON_PATH << std::endl;

    httplib::Server svr;

    // Habilitar CORS para cualquier cliente (Escritorio / Android)
    svr.set_default_headers({{"Access-Control-Allow-Origin", "*"},
                             {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
                             {"Access-Control-Allow-Headers", "Content-Type"}});

    svr.Options(".*", [](const httplib::Request &, httplib::Response &res)
                { res.status = 204; });

    // GET STATUS SERVER
    svr.Get("/api/health", [](const httplib::Request &, httplib::Response &res)
            { res.set_content(R"({"status":"online","service":"TriageProject API"})", "application/json"); });

    // POST LOGIN
    svr.Post("/api/login", [](const httplib::Request &req, httplib::Response &res)
             {
        try {
            auto body = json::parse(req.body);
            std::string username = body.at("username").get<std::string>();
            std::string password = body.at("password").get<std::string>();
            std::string pass_hash = sha256(password);

            std::lock_guard<std::mutex> lock(db_mutex);
            sqlite3* db;
            if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) {
                res.status = 500;
                res.set_content(R"({"status":"error","message":"No se pudo abrir la base de datos"})", "application/json");
                return;
            }

            const char* sql = R"(
                SELECT u.user_id, u.username, u.center_id, c.name, c.type, c.max_level, c.capacity, c.location
                FROM users u
                JOIN centers c ON u.center_id = c.center_id
                WHERE u.username = ? AND u.password_hash = ?;
            )";

            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, pass_hash.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                json response = {
                    {"status", "ok"},
                    {"user", {
                        {"user_id", sqlite3_column_int(stmt, 0)},
                        {"username", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))},
                        {"center_id", sqlite3_column_int(stmt, 2)},
                        {"center_name", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))},
                        {"center_type", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))},
                        {"max_level", sqlite3_column_int(stmt, 5)},
                        {"capacity", sqlite3_column_int(stmt, 6)},
                        {"location", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7))}
                    }}
                };
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                res.set_content(response.dump(), "application/json");
            } else {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                res.status = 401;
                res.set_content(R"({"status":"error","message":"Usuario o contraseña incorrectos"})", "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"status", "error"}, {"message", e.what()}}.dump(), "application/json");
        } });

    // GET CENTROS DE SALUD DISPONIBLES
    svr.Get("/api/centers", [](const httplib::Request &, httplib::Response &res)
            {
        std::lock_guard<std::mutex> lock(db_mutex);
        sqlite3* db;
        if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) {
            res.status = 500;
            res.set_content(R"({"error":"No se pudo abrir la base de datos"})", "application/json");
            return;
        }

        const char* sql = "SELECT center_id, name, type, max_level, capacity, location, latitude, longitude FROM centers;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

        json centers = json::array();
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            centers.push_back({
                {"center_id", sqlite3_column_int(stmt, 0)},
                {"name", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))},
                {"type", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))},
                {"max_level", sqlite3_column_int(stmt, 3)},
                {"capacity", sqlite3_column_int(stmt, 4)},
                {"location", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))},
                {"latitude", sqlite3_column_double(stmt, 6)},
                {"longitude", sqlite3_column_double(stmt, 7)}
            });
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        res.set_content(json{{"centers", centers}}.dump(), "application/json"); });

    // GET PACIENTES TEMPORALES PENDIENTES
    svr.Get(R"(/api/centers/(\d+)/pending)", [](const httplib::Request &req, httplib::Response &res)
            {
        int center_id = std::stoi(req.matches[1]);
        std::lock_guard<std::mutex> lock(json_mutex);

        json data = readTempPatients();
        json filtered = json::array();

        for (const auto& p : data["pending_patients"]) {
            if (p["center_id"] == center_id) {
                filtered.push_back(p);
            }
        }

        res.set_content(json{{"center_id", center_id}, {"patients", filtered}}.dump(), "application/json"); });

    // GET PACIENTES INGRESADOS (para acciones por fila en el dashboard)
    svr.Get(R"(/api/centers/(\d+)/admitted)", [](const httplib::Request &req, httplib::Response &res)
            {
        const int center_id = std::stoi(req.matches[1]);
        std::lock_guard<std::mutex> lock(db_mutex);
        sqlite3* db = nullptr;
        if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) {
            res.status = 500;
            res.set_content(R"({"error":"No se pudo abrir la base de datos"})", "application/json");
            return;
        }

        const char* sql = R"(
            SELECT rowid, cedula, first_name, last_name, age, triage_level,
                   triage_score, symptoms, status, admission_date
            FROM patients
            WHERE center_id = ? AND status = 'ADMITTED'
            ORDER BY admission_date DESC;
        )";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, center_id);

        json patients = json::array();
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            auto texto = [stmt](int columna) -> std::string {
                const auto* valor = sqlite3_column_text(stmt, columna);
                return valor != nullptr ? reinterpret_cast<const char*>(valor) : "";
            };
            patients.push_back({
                {"patient_id", sqlite3_column_int(stmt, 0)},
                {"cedula", texto(1)},
                {"first_name", texto(2)},
                {"last_name", texto(3)},
                {"age", sqlite3_column_int(stmt, 4)},
                {"triage_level", sqlite3_column_int(stmt, 5)},
                {"triage_score", sqlite3_column_int(stmt, 6)},
                {"symptoms", texto(7)},
                {"status", texto(8)},
                {"admission_date", texto(9)}
            });
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        res.set_content(json{{"center_id", center_id}, {"patients", patients}}.dump(), "application/json"); });

    // GET TENDENCIAS HISTÓRICAS POR NIVEL (para el panel de KPIs)
    svr.Get(R"(/api/centers/(\d+)/trends)", [](const httplib::Request &req, httplib::Response &res)
            {
        const int center_id = std::stoi(req.matches[1]);
        int days = 7;
        if (req.has_param("days")) {
            try { days = std::stoi(req.get_param_value("days")); } catch (...) { days = 7; }
        }
        days = std::max(1, std::min(90, days));

        std::lock_guard<std::mutex> lock(db_mutex);
        sqlite3* db = nullptr;
        if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) {
            res.status = 500;
            res.set_content(R"({"error":"No se pudo abrir la base de datos"})", "application/json");
            return;
        }

        const char* sql = R"(
            SELECT date(admission_date) AS dia,
                   SUM(CASE WHEN triage_level = 1 THEN 1 ELSE 0 END),
                   SUM(CASE WHEN triage_level = 2 THEN 1 ELSE 0 END),
                   SUM(CASE WHEN triage_level = 3 THEN 1 ELSE 0 END),
                   SUM(CASE WHEN triage_level = 4 THEN 1 ELSE 0 END),
                   SUM(CASE WHEN triage_level = 5 THEN 1 ELSE 0 END),
                   COUNT(*)
            FROM patients
            WHERE center_id = ? AND triage_level IS NOT NULL
              AND date(admission_date) >= date('now', ?)
            GROUP BY date(admission_date)
            ORDER BY dia ASC;
        )";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, center_id);
        const std::string offset = "-" + std::to_string(days - 1) + " days";
        sqlite3_bind_text(stmt, 2, offset.c_str(), -1, SQLITE_TRANSIENT);

        json trends = json::array();
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            trends.push_back({
                {"date", sqlite3_column_text(stmt, 0) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)) : ""},
                {"level_1", sqlite3_column_int(stmt, 1)},
                {"level_2", sqlite3_column_int(stmt, 2)},
                {"level_3", sqlite3_column_int(stmt, 3)},
                {"level_4", sqlite3_column_int(stmt, 4)},
                {"level_5", sqlite3_column_int(stmt, 5)},
                {"total", sqlite3_column_int(stmt, 6)}
            });
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        res.set_content(json{{"center_id", center_id}, {"days", days}, {"trends", trends}}.dump(), "application/json"); });

    // POST PACIENTES TEMPORALES (JSON)
    svr.Post("/api/temp-patients", [](const httplib::Request &req, httplib::Response &res)
             {
        try {
            auto new_p = json::parse(req.body);
            if (!new_p.contains("latitud") || !new_p["latitud"].is_number()) {
                new_p["latitud"] = 8.5897;
            }
            if (!new_p.contains("longitud") || !new_p["longitud"].is_number()) {
                new_p["longitud"] = -71.1561;
            }
            std::lock_guard<std::mutex> lock(json_mutex);

            json data = readTempPatients();
            auto& list = data["pending_patients"];

            int level = new_p.at("triage_level").get<int>();
            int count_same_level = 0;
            for (const auto& p : list) {
                if (p["triage_level"] == level) count_same_level++;
            }

            int nuevo_id = level * 100 + (count_same_level + 1);
            bool id_ocupado = true;
            while (id_ocupado) {
                id_ocupado = false;
                for (const auto& p : list) {
                    if (p["id"] == nuevo_id) {
                        id_ocupado = true;
                        ++nuevo_id;
                        break;
                    }
                }
            }
            new_p["id"] = nuevo_id;
            new_p["source"] = "SYSTEM";

            list.push_back(new_p);
            writeTempPatients(data);

            res.status = 201;
            res.set_content(json{{"status", "created"}, {"patient", new_p}}.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        } });

    // 5. ADMITIR PACIENTE (PASA DE JSON A SQLITE): POST /api/centers/:cid/admit/:tid
    svr.Post(R"(/api/centers/(\d+)/admit/(\d+))", [](const httplib::Request &req, httplib::Response &res)
             {
        int center_id = std::stoi(req.matches[1]);
        int temp_id = std::stoi(req.matches[2]);
    
        try
        {
            auto payload = json::parse(req.body);
            std::string cedula = payload.value("cedula", "");

            std::lock_guard<std::mutex> lock_j(json_mutex);
            json data = readTempPatients();
            auto &list = data["pending_patients"];

            int found_idx = -1;
            json target;
            for (size_t i = 0; i < list.size(); ++i)
            {
                if (list[i]["id"] == temp_id && list[i]["center_id"] == center_id)
                {
                    found_idx = static_cast<int>(i);
                    target = list[i];
                    break;
                }
            }

            if (found_idx == -1)
            {
                res.status = 404;
                res.set_content(R"({"error":"Paciente temporal no encontrado en este centro"})", "application/json");
                return;
            }

            if (cedula.empty() || cedula == "Sin cédula") {
                cedula = "SIN-" + std::to_string(temp_id);
            }

            // Insertar en SQLite
            std::lock_guard<std::mutex> lock_db(db_mutex);
            sqlite3 *db;
            sqlite3_open(DB_PATH.c_str(), &db);

            const char *sql = R"(
                INSERT INTO patients (cedula, center_id, first_name, last_name, age, triage_level, triage_score, source, status, symptoms)
                VALUES (?, ?, ?, ?, ?, ?, ?, 'SYSTEM', 'ADMITTED', ?);
            )";

            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

            std::string fname = payload.value("first_name", target["first_name"].get<std::string>());
            std::string lname = payload.value("last_name", target["last_name"].get<std::string>());
            int age = payload.value("age", target["age"].get<int>());
            int t_level = target["triage_level"].get<int>();
            int t_score = target["triage_score"].get<int>();
            std::string symptoms = target["symptoms"].get<std::string>();

            sqlite3_bind_text(stmt, 1, cedula.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, center_id);
            sqlite3_bind_text(stmt, 3, fname.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, lname.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 5, age);
            sqlite3_bind_int(stmt, 6, t_level);
            sqlite3_bind_int(stmt, 7, t_score);
            sqlite3_bind_text(stmt, 8, symptoms.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                res.status = 400;
                res.set_content(R"({"error":"No se pudo admitir el paciente"})", "application/json");
                return;
            }

        sqlite3_finalize(stmt);

        // Descontar 1 cama disponible del centro
        sqlite3_stmt *upd;
        sqlite3_prepare_v2(db, "UPDATE centers SET capacity = MAX(0, capacity - 1) WHERE center_id = ?;", -1, &upd, nullptr);
        sqlite3_bind_int(upd, 1, center_id);
        sqlite3_step(upd);
        sqlite3_finalize(upd);
        sqlite3_close(db);

        // Eliminar del JSON temporal
        list.erase(list.begin() + found_idx);
        writeTempPatients(data);

        res.set_content(json{{"status", "admitted"}, {"cedula", cedula}}.dump(), "application/json");
        } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        } });

    // POST DAR DE ALTA PACIENTE (CAMBIA ESTADO A DISCHARGED)
    svr.Post(R"(/api/centers/(\d+)/discharge/([A-Za-z0-9\-]+))", [](const httplib::Request &req, httplib::Response &res)
             {
        int center_id = std::stoi(req.matches[1]);
        std::string cedula = req.matches[2];

        std::lock_guard<std::mutex> lock_db(db_mutex);
        sqlite3* db;
        sqlite3_open(DB_PATH.c_str(), &db);

        const char* sql = R"(
            UPDATE patients
            SET status = 'DISCHARGED', discharge_date = CURRENT_TIMESTAMP
            WHERE cedula = ? AND center_id = ? AND status = 'ADMITTED';
        )";

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, cedula.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, center_id);
        sqlite3_step(stmt);
        int changed = sqlite3_changes(db);
        sqlite3_finalize(stmt);

        if (changed == 0) {
            sqlite3_close(db);
            res.status = 404;
            res.set_content(R"({"error":"Paciente admitido no encontrado en este centro"})", "application/json");
            return;
        }

        // Restaurar 1 cama disponible
        sqlite3_stmt* upd;
        sqlite3_prepare_v2(db, "UPDATE centers SET capacity = capacity + 1 WHERE center_id = ?;", -1, &upd, nullptr);
        sqlite3_bind_int(upd, 1, center_id);
        sqlite3_step(upd);
        sqlite3_finalize(upd);
        sqlite3_close(db);

        res.set_content(json{{"status", "discharged"}, {"cedula", cedula}}.dump(), "application/json"); });

    // ALTA POR ID INTERNO: funciona también para pacientes "Sin cédula"
    svr.Post(R"(/api/centers/(\d+)/discharge-id/(\d+))", [](const httplib::Request &req, httplib::Response &res)
             {
        const int center_id = std::stoi(req.matches[1]);
        const int patient_id = std::stoi(req.matches[2]);

        std::lock_guard<std::mutex> lock_db(db_mutex);
        sqlite3* db = nullptr;
        sqlite3_open(DB_PATH.c_str(), &db);

        const char* sql = R"(
            UPDATE patients
            SET status = 'DISCHARGED', discharge_date = CURRENT_TIMESTAMP
            WHERE rowid = ? AND center_id = ? AND status = 'ADMITTED';
        )";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, patient_id);
        sqlite3_bind_int(stmt, 2, center_id);
        sqlite3_step(stmt);
        const int changed = sqlite3_changes(db);
        sqlite3_finalize(stmt);

        if (changed == 0) {
            sqlite3_close(db);
            res.status = 404;
            res.set_content(R"({"error":"Paciente admitido no encontrado en este centro"})", "application/json");
            return;
        }

        sqlite3_stmt* upd = nullptr;
        sqlite3_prepare_v2(db, "UPDATE centers SET capacity = capacity + 1 WHERE center_id = ?;", -1, &upd, nullptr);
        sqlite3_bind_int(upd, 1, center_id);
        sqlite3_step(upd);
        sqlite3_finalize(upd);
        sqlite3_close(db);

        res.set_content(json{{"status", "discharged"}, {"patient_id", patient_id}}.dump(), "application/json"); });

    std::cout << "Servidor Triage API escuchando en http://0.0.0.0:8081\n";
    svr.listen("0.0.0.0", 8081);
    return 0;
}