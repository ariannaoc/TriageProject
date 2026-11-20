#pragma once
#include <string>
#include <avltree.hpp>

using namespace Designar;

struct CredencialCentro {
    std::string username;       // Clave de búsqueda única
    std::string password_hash;
    std::string nombre_centro;
    std::string tipo_centro;    // Ej: "Hospital Tipo IV", "Ambulatorio Urbano III"
    int_t nivel_max_mpps;       // 1 (Nivel I) a 5 (Nivel V)
    int_t capacidad_camas;

    // Operadores requeridos por Designar::AVLTree para ordenar y buscar por username
    bool operator<(const CredencialCentro& otro) const {
        return username < otro.username;
    }
    bool operator==(const CredencialCentro& otro) const {
        return username == otro.username;
    }
};