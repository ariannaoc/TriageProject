#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>

#include "libs/httplib.h"
#include "libs/json.hpp"

using json = nlohmann::json;

const std::string DB_PATH = "data/triage.db";
const std::string TEMP_JSON_PATH = "data/temp_patients.json";

std::mutex json_mutex;
std::mutex db_mutex;

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

int main()
{
    httplib::Server svr;

    // Habilitar CORS para cualquier cliente (Escritorio / Android)
    svr.set_default_headers({{"Access-Control-Allow-Origin", "*"},
                             {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
                             {"Access-Control-Allow-Headers", "Content-Type, Authorization"}});

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

    // POST PACIENTES TEMPORALES (JSON)
    svr.Post("/api/temp-patients", [](const httplib::Request &req, httplib::Response &res)
             {
        try {
            auto new_p = json::parse(req.body);
            std::lock_guard<std::mutex> lock(json_mutex);

            json data = readTempPatients();
            auto& list = data["pending_patients"];

            int level = new_p.at("triage_level").get<int>();
            int count_same_level = 0;
            for (const auto& p : list) {
                if (p["triage_level"] == level) count_same_level++;
            }

            new_p["id"] = level * 100 + (count_same_level + 1);
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
            std::string cedula = payload.at("cedula").get<std::string>();

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
                res.set_content(R"({"error":"error"})", "application/json");
            }
                return;
        
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

    std::cout << "Servidor Triage API escuchando en http://0.0.0.0:8081\n";
    svr.listen("0.0.0.0", 8081);
    return 0;
}