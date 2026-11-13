#pragma once

// =============================================================================
// MotorTriaje.hpp
// Motor de triaje: integra las puntuaciones por categoría (BosqueTriaje) con
// los factores de riesgo del paciente y asigna el nivel de triaje final.
//
// Solo depende de modelos_triaje.hpp y BosqueTriaje.hpp. No debe incluir
// ModelosCentro.hpp (solo contiene CredencialCentro para el login; el vínculo
// con CentroSalud es el campo username).
// =============================================================================

#include <map>
#include <string>
#include <vector>

#include "BosqueTriaje.hpp"
#include "modelos_triaje.hpp"

class MotorTriaje
{
public:
    /// `bosque` se guarda por referencia (BosqueTriaje no es copiable porque
    /// posee sus árboles con unique_ptr): el llamador debe garantizar que
    /// sobreviva al motor.
    /// `factoresDisponibles` se guarda POR VALOR (copia): son pocos elementos y
    /// así el motor no depende de la vida útil del contenedor cargado del JSON.
    explicit MotorTriaje(const BosqueTriaje& bosque,
                         const std::vector<FactorRiesgo>& factoresDisponibles);

    /// Evalúa el triaje de un paciente.
    ///
    /// - `seleccionPorCategoria`: id de categoría -> ids de síntomas elegidos.
    ///   Una categoría con vector vacío equivale a responder "No" al filtro.
    /// - `edadPaciente` / `sexoPaciente` ('F' o 'M'): para reglas de edad y
    ///   para la restricción de RF_PREGNANCY.
    /// - `idsFactoresActivos`: ids de factores clínicos marcados por el
    ///   paciente (p. ej. "RF_CARDIO"). Las reglas de edad se aplican solas.
    ///
    /// CONVENCIÓN DEL CENTINELA: si no hay ningún síntoma seleccionado se
    /// devuelve `sin_sintomas = true` con `nivel_final = NivelTriaje::V` como
    /// valor centinela (no es un triaje real). El consumidor DEBE consultar
    /// `sin_sintomas` antes de usar `nivel_final`.
    ResultadoTriaje evaluar(
        const std::map<int, std::vector<int>>& seleccionPorCategoria,
        int edadPaciente,
        char sexoPaciente,
        const std::vector<std::string>& idsFactoresActivos) const;

    /// Mapea una puntuación final a nivel (triage_levels.json), inclusivo en
    /// ambos extremos: I 80-100, II 60-79, III 40-59, IV 20-39, V 0-19.
    /// Saturación defensiva: > 100 -> I; < 0 -> V.
    NivelTriaje mapearPuntuacionANivel(int puntuacion) const;

    const BosqueTriaje& bosque() const { return bosque_; }
    const std::vector<FactorRiesgo>& factoresDisponibles() const { return factores_; }

private:
    const BosqueTriaje&       bosque_;
    std::vector<FactorRiesgo> factores_;
};
