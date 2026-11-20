#include "MotorEnrutamiento.hpp"

#include <algorithm>
#include <cmath>

namespace
{
constexpr double kPi         = 3.14159265358979323846;
constexpr double kKmPorGrado = 111.0;

/// Distancia euclidiana local (km); misma aproximación que GestorCentros.
double distanciaKm(double lat1, double lng1, double lat2, double lng2)
{
    const double rad  = lat1 * kPi / 180.0;
    const double dlat = (lat2 - lat1) * kKmPorGrado;
    const double dlng = (lng2 - lng1) * kKmPorGrado * std::cos(rad);
    return std::sqrt(dlat * dlat + dlng * dlng);
}
}  // namespace

int MotorEnrutamiento::complejidadMinimaParaNivel(NivelTriaje nivel)
{
    // Tabla de complejidad mínima (adaptación propia basada en MPPS):
    //   I   -> 4 (máxima complejidad, hospital tipo IV)
    //   II  -> 3
    //   III -> 2
    //   IV  -> 1 (ambulatorio)
    //   V   -> 1
    switch (nivel)
    {
        case NivelTriaje::I:   return 4;
        case NivelTriaje::II:  return 3;
        case NivelTriaje::III: return 2;
        case NivelTriaje::IV:  return 1;
        case NivelTriaje::V:   return 1;
    }
    return 1;  // inalcanzable con enum válido
}

ResultadoEnrutamiento MotorEnrutamiento::recomendar(
    const ResultadoTriaje& triaje,
    double latPaciente,
    double lngPaciente,
    const GestorCentros& gestor,
    double w1, double w2, double w3) const
{
    ResultadoEnrutamiento resultado{};
    resultado.score                 = 0.0;
    resultado.sin_centro_disponible = true;

    // Cortocircuito: sin síntomas no hay derivación.
    if (triaje.sin_sintomas)
        return resultado;

    const int complejidad = complejidadMinimaParaNivel(triaje.nivel_final);
    const auto candidatos = gestor.obtenerCercanos(latPaciente, lngPaciente,
                                                   kCantidadCercanos, complejidad);
    if (candidatos.empty())
        return resultado;

    std::vector<std::pair<CentroSalud, double>> ranking;
    ranking.reserve(candidatos.size());

    for (const auto& c : candidatos)
    {
        const double d        = distanciaKm(latPaciente, lngPaciente,
                                            c.latitud, c.longitud);
        const double tiempo_h = d / kVelocidadKmH;
        // Guardia contra división por cero si el centro coincide con el paciente.
        const double inv_tiempo = (tiempo_h > 1e-3) ? (1.0 / tiempo_h) : 1000.0;

        // Normalización automática: total==0 se protege con 1.
        const double denom    = (c.capacidad_total > 0)
                                    ? static_cast<double>(c.capacidad_total) : 1.0;
        const double cap_norm = static_cast<double>(c.capacidad_disponible) / denom;
        const double cola_norm = static_cast<double>(c.cola_actual) / denom;

        const double score = w1 * inv_tiempo + w2 * cap_norm - w3 * cola_norm;
        ranking.emplace_back(c, score);
    }

    std::sort(ranking.begin(), ranking.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    resultado.centro_recomendado    = ranking.front().first;
    resultado.score                 = ranking.front().second;
    resultado.ranking               = std::move(ranking);
    resultado.sin_centro_disponible = false;
    return resultado;
}
