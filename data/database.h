// database.h
#pragma once
#include <string>
#include <sqlite3.h>
#include <bcrypt/BCrypt.hpp>

class Database {
public:
    static sqlite3* connect(const std::string& dbPath);
    static bool authenticateCenter(sqlite3* db, const std::string& username, const std::string& password);
    static void registerCenter(sqlite3* db, int centerId, const std::string& username, const std::string& password);
};
