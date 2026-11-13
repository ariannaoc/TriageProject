#pragma once

// =============================================================================
// modelos_triaje.hpp
// Estructuras de datos compartidas del motor de triaje y enrutamiento.
// Solo definiciones de datos: no contiene lógica de negocio.
//
// NOTA: CentroSalud se define AQUÍ (no en ModelosCentro.hpp, que solo
// contiene CredencialCentro para el login). El vínculo entre ambos es el
// campo username. Cuando se integre la API de Google Maps, se rellenarán
// latitud/longitud reales.
// =============================================================================

#include <string>
#include <utility>
#include <vector>

/// Nivel de triaje (I = más crítico, V = menos urgente).
enum class NivelTriaje
{
    I   = 1,
    II  = 2,
    III = 3,
    IV  = 4,
    V   = 5
};

/// Representación textual del nivel ("I".."V").
inline std::string to_string(NivelTriaje nivel)
{
    switch (nivel)
    {
        case NivelTriaje::I:   return "I";
        case NivelTriaje::II:  return "II";
        case NivelTriaje::III: return "III";
        case NivelTriaje::IV:  return "IV";
        case NivelTriaje::V:   return "V";
    }
    return "";
}

/// Factor de riesgo (reglas de edad + condiciones crónicas de risk_factors.json).
struct FactorRiesgo
{
    std::string id;
    std::string nombre;
    int         puntos;
    bool        esReglaEdad;
    std::string condicionEdad;
};

/// Opción de síntoma dentro de una categoría.
struct OpcionSintoma
{
    int         id;
    std::string etiqueta;
    int         base_points;
    NivelTriaje nivel_estimado;
};

/// Categoría de síntomas con sus opciones.
struct CategoriaTriaje
{
    int                        id;
    std::string                nombre;
    std::vector<OpcionSintoma> opciones;
};

/// Resultado de evaluar el triaje de un paciente.
struct ResultadoTriaje
{
    NivelTriaje nivel_final;                   ///< Centinela V si sin_sintomas == true
    int         puntuacion_categoria_ganadora;
    int         puntuacion_factores_riesgo;
    std::string categoria_ganadora;
    bool        sin_sintomas;
};

/// Centro de salud con datos operativos para el enrutamiento.
/// El campo username vincula con CredencialCentro (login).
/// nivel_complejidad: 1 = menor complejidad (ambulatorio), 4 = mayor (hospital tipo IV).
/// Se mapea desde NivelTriaje en el MotorEnrutamiento.
struct CentroSalud
{
    int         id                   = -1;
    std::string username;                        ///< Vínculo con CredencialCentro
    std::string nombre;
    int         nivel_complejidad    = 1;        ///< 1 (menor) a 4 (mayor)
    int         capacidad_total      = 0;
    int         capacidad_disponible = 0;
    int         cola_actual          = 0;
    double      latitud              = 0.0;
    double      longitud             = 0.0;
};

/// Resultado del enrutamiento hacia un centro de salud.
struct ResultadoEnrutamiento
{
    CentroSalud centro_recomendado;                        ///< Centro con mejor score (id = -1 si no hay)
    double      score                                     = 0.0;
    std::vector<std::pair<CentroSalud, double>> ranking;   ///< Centros candidatos ordenados
    bool        sin_centro_disponible                     = true;
};
