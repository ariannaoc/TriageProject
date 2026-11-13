#pragma once

// =============================================================================
// MotorEnrutamiento.hpp
// Dado un ResultadoTriaje y la ubicación del paciente, recomienda el centro
// de salud más adecuado combinando cercanía, capacidad disponible y cola.
// =============================================================================

#include "GestorCentros.hpp"
#include "modelos_triaje.hpp"

class MotorEnrutamiento
{
public:
    /// Mapea el nivel de triaje a la complejidad mínima del centro
    /// (adaptación propia basada en la normativa MPPS). Ver .cpp.
    static int complejidadMinimaParaNivel(NivelTriaje nivel);

    /// Recomienda un centro y devuelve el ranking completo.
    ///
    /// - Si `triaje.sin_sintomas` es true, cortocircuita sin consultar el
    ///   GestorCentros: `sin_centro_disponible = true`, `id = -1`.
    /// - `score = w1 * (1 / tiempo_h) + w2 * capacidad_normalizada
    ///          - w3 * cola_normalizada`
    ///   donde `capacidad_normalizada = disponible / total` y
    ///         `cola_normalizada     = cola_actual / total` (total==0 -> 1).
    /// - Pesos por defecto: w1=0.5, w2=0.3, w3=0.2 (configurables).
    ResultadoEnrutamiento recomendar(
        const ResultadoTriaje& triaje,
        double latPaciente,
        double lngPaciente,
        const GestorCentros& gestor,
        double w1 = 0.5,
        double w2 = 0.3,
        double w3 = 0.2) const;

private:
    static constexpr int    kCantidadCercanos = 5;
    static constexpr double kVelocidadKmH     = 40.0;  // simulación hasta Google Maps
};
