#include "MotorTriaje.hpp"

#include <set>
#include <sstream>

namespace
{
// Restricción de RF_PREGNANCY (risk_factors.json, "constraints"): solo aplica
// a mujeres con 10 <= edad <= 55. Si el paciente la marca sin cumplirla, no
// suma puntos. FactorRiesgo no guarda estas restricciones, por eso se fijan aquí.
constexpr char kSexoEmbarazo     = 'F';
constexpr int  kEdadMinEmbarazo  = 10;
constexpr int  kEdadMaxEmbarazo  = 55;
const char*    kIdEmbarazo       = "RF_PREGNANCY";

/// Evalúa una condición de edad con formato "age <op> N" (op: <, >, <=, >=).
/// Devuelve false si la condición no se puede interpretar.
bool cumpleCondicionEdad(const std::string& condicion, int edad)
{
    std::istringstream in(condicion);
    std::string var, op;
    int umbral = 0;
    if (!(in >> var >> op >> umbral) || var != "age")
        return false;

    if (op == "<")  return edad <  umbral;
    if (op == "<=") return edad <= umbral;
    if (op == ">")  return edad >  umbral;
    if (op == ">=") return edad >= umbral;
    return false;
}
}  // namespace

MotorTriaje::MotorTriaje(const BosqueTriaje& bosque,
                         const std::vector<FactorRiesgo>& factoresDisponibles)
    : bosque_(bosque), factores_(factoresDisponibles)
{
}

NivelTriaje MotorTriaje::mapearPuntuacionANivel(int puntuacion) const
{
    // Umbrales mínimos de triage_levels.json. Al comparar solo contra el mínimo
    // se obtiene la saturación defensiva: > 100 cae en I y < 0 cae en V.
    if (puntuacion >= 80) return NivelTriaje::I;
    if (puntuacion >= 60) return NivelTriaje::II;
    if (puntuacion >= 40) return NivelTriaje::III;
    if (puntuacion >= 20) return NivelTriaje::IV;
    return NivelTriaje::V;
}

ResultadoTriaje MotorTriaje::evaluar(
    const std::map<int, std::vector<int>>& seleccionPorCategoria,
    int edadPaciente,
    char sexoPaciente,
    const std::vector<std::string>& idsFactoresActivos) const
{
    // (a) Caso "sin síntomas": el paciente respondió "No" a todos los filtros.
    // Se devuelve nivel V como valor CENTINELA (no es un triaje real); el
    // consumidor debe mirar `sin_sintomas` antes de usar `nivel_final`.
    if (!bosque_.tieneSintomasSeleccionados(seleccionPorCategoria))
        return ResultadoTriaje{NivelTriaje::V, 0, 0, "", true};

    // (b)+(c) Puntuar cada categoría seleccionada y elegir la ganadora.
    // std::map itera por id ascendente y solo se reemplaza al ganador con una
    // puntuación ESTRICTAMENTE mayor: en caso de empate gana la categoría de
    // menor id (orden A->E del documento).
    bool hayGanadora          = false;
    int  idGanadora           = 0;
    int  puntuacionGanadora   = 0;
    for (const auto& [idCategoria, opciones] : seleccionPorCategoria)
    {
        if (opciones.empty())
            continue;  // Respondió "No" al filtro de esta categoría

        const int puntuacion = bosque_.evaluarCategoria(idCategoria, opciones);
        if (!hayGanadora || puntuacion > puntuacionGanadora)
        {
            hayGanadora        = true;
            idGanadora         = idCategoria;
            puntuacionGanadora = puntuacion;
        }
    }

    // (d) Puntos de factores de riesgo. Se suman UNA SOLA VEZ y únicamente a la
    // categoría ganadora (decisión de diseño acordada): sumarlos a cada
    // categoría inflaría la puntuación según cuántas categorías marque el
    // paciente y no según su vulnerabilidad real.
    const std::set<std::string> activos(idsFactoresActivos.begin(), idsFactoresActivos.end());
    int puntosRiesgo = 0;
    for (const auto& factor : factores_)
    {
        if (factor.esReglaEdad)
        {
            // Reglas de edad (AGE_INFANT, AGE_ELDERLY): automáticas, no
            // dependen de lo que marque el paciente.
            if (cumpleCondicionEdad(factor.condicionEdad, edadPaciente))
                puntosRiesgo += factor.puntos;
            continue;
        }

        if (activos.count(factor.id) == 0)
            continue;  // El paciente no marcó este factor

        // RF_PREGNANCY solo aplica a sexo 'F' y 10 <= edad <= 55; si no se
        // cumple, no suma aunque el paciente lo haya marcado.
        if (factor.id == kIdEmbarazo)
        {
            const bool aplica = sexoPaciente == kSexoEmbarazo
                             && edadPaciente >= kEdadMinEmbarazo
                             && edadPaciente <= kEdadMaxEmbarazo;
            if (!aplica)
                continue;
        }
        puntosRiesgo += factor.puntos;
    }

    // (e) Puntuación final.
    const int puntuacionFinal = puntuacionGanadora + puntosRiesgo;

    // (f)+(g) Nivel y resultado. categoria_ganadora lleva el nombre legible
    // de la categoría (BosqueTriaje::nombreCategoria).
    ResultadoTriaje resultado{};
    resultado.nivel_final                    = mapearPuntuacionANivel(puntuacionFinal);
    resultado.puntuacion_categoria_ganadora  = puntuacionGanadora;
    resultado.puntuacion_factores_riesgo     = puntosRiesgo;
    resultado.categoria_ganadora             = bosque_.nombreCategoria(idGanadora);
    resultado.sin_sintomas                   = false;
    return resultado;
}
