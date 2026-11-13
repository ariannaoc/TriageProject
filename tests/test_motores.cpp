// =============================================================================
// tests/test_motores.cpp
// Suite de pruebas formales para MotorTriaje y MotorEnrutamiento.
// No depende de Qt: solo de DeSiGNAR y de los headers del motor.
// =============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "BosqueTriaje.hpp"
#include "MotorTriaje.hpp"
#include "GestorCentros.hpp"
#include "MotorEnrutamiento.hpp"

static int fallos = 0;
static int total  = 0;

static void check(const std::string& nombre, bool ok)
{
    ++total;
    std::cout << (ok ? "[OK]  " : "[FALLO] ") << nombre << "\n";
    if (!ok) ++fallos;
}

// -----------------------------------------------------------------------------
// Fixtures
// -----------------------------------------------------------------------------

// Las 7 categorías de symptoms.json, con sus puntos reales.
std::vector<CategoriaTriaje> fixtureCategorias()
{
    return {
        {1, "Alergias y Piel", {
            {101, "Lesion cutanea leve",  10, NivelTriaje::V},
            {102, "Afeccion en piel",     25, NivelTriaje::IV},
            {103, "Reaccion moderada",    45, NivelTriaje::III},
            {104, "Anafilaxia",           80, NivelTriaje::I}
        }},
        {2, "Infecciones y Piel Avanzada", {
            {201, "Pustula",             10, NivelTriaje::V},
            {202, "Supuracion",          25, NivelTriaje::IV},
            {203, "Celulitis",           45, NivelTriaje::III}
        }},
        {3, "Fiebre y Dolor Abdominal", {
            {301, "Fiebre moderada",      15, NivelTriaje::V},
            {302, "Fiebre alta",          25, NivelTriaje::IV},
            {303, "Colico severo",        45, NivelTriaje::III}
        }},
        {4, "Dolor Toracico y Respiratorio", {
            {401, "Molestia atipica",     20, NivelTriaje::IV},
            {402, "Dolor moderado",       45, NivelTriaje::III},
            {403, "Dolor opresivo agudo", 65, NivelTriaje::II}
        }},
        {5, "Traumatismos y Extremidades", {
            {501, "Contusion",            10, NivelTriaje::V},
            {502, "Esguince severo",      25, NivelTriaje::IV},
            {503, "Fractura cerrada",     45, NivelTriaje::III},
            {504, "Fractura expuesta",    65, NivelTriaje::II},
            {505, "Amputacion",           85, NivelTriaje::I}
        }},
        {6, "Sangrado y Lesiones", {
            {601, "Cortada superficial",   5, NivelTriaje::V},
            {602, "Laceracion moderada",  25, NivelTriaje::IV},
            {603, "Hemorragia activa",    45, NivelTriaje::III},
            {604, "Hemorragia masiva",    70, NivelTriaje::I}
        }},
        {7, "Malestar General", {
            {701, "Gripales leves",        5, NivelTriaje::V},
            {702, "Febricula",            10, NivelTriaje::V},
            {703, "Cefalea tensional",    10, NivelTriaje::V},
            {704, "Nauseas leves",        10, NivelTriaje::V},
            {705, "Mareo leve",           15, NivelTriaje::V},
            {706, "Migrana intensa",      25, NivelTriaje::IV}
        }}
    };
}

// Los 9 factores de risk_factors.json.
std::vector<FactorRiesgo> fixtureFactores()
{
    return {
        {"AGE_INFANT",   "Infantil < 5",            15, true,  "age < 5"},
        {"AGE_ELDERLY",  "Adulto mayor > 65",       15, true,  "age > 65"},
        {"RF_CARDIO",    "Cardiovascular severa",   10, false, ""},
        {"RF_INMUNO",    "Inmunosupresion",         10, false, ""},
        {"RF_RENAL",     "Renal cronica",           10, false, ""},
        {"RF_PREGNANCY", "Embarazo/postparto",      10, false, ""},
        {"RF_RESP",      "Respiratorio cronico",     5, false, ""},
        {"RF_METABOLIC", "Metabolico/vascular",      5, false, ""},
        {"RF_NEURO",     "Neurologico previo",       5, false, ""}
    };
}

// 5 centros alrededor de Mérida (8.6, -71.1) con complejidades 1..4.
std::vector<CentroSalud> fixtureCentros()
{
    std::vector<CentroSalud> v;
    // id, username, nombre, nivel_complejidad, cap_total, cap_disp, cola, lat, lng
    v.push_back({1, "hosp4", "Hospital Universitario", 4, 100, 60, 10, 8.600, -71.100});
    v.push_back({2, "hosp3", "Hospital Tipo III",      3,  80, 40, 20, 8.640, -71.140});
    v.push_back({3, "amb2",  "Ambulatorio II",         2,  30, 15,  5, 8.660, -71.160});
    // Dos ambulatorios en la misma ubicación, distinto estado:
    v.push_back({4, "amb1a", "Ambulatorio I - A",      1,  20, 18,  2, 8.670, -71.170});
    v.push_back({5, "amb1b", "Ambulatorio I - B",      1,  20,  2, 18, 8.670, -71.170});
    return v;
}

// -----------------------------------------------------------------------------
// Tests del motor de triaje
// -----------------------------------------------------------------------------

void testTriajeNivelI(const MotorTriaje& motor)
{
    // 70 años (AGE_ELDERLY +15) + dolor opresivo agudo (403=65) = 80 → Nivel I
    auto r = motor.evaluar({{4, {403}}}, 70, 'M', {});
    check("Triaje Nivel I: 70 anios + dolor opresivo (65+15=80)",
          r.nivel_final == NivelTriaje::I &&
          r.puntuacion_categoria_ganadora == 65 &&
          r.puntuacion_factores_riesgo == 15 &&
          r.categoria_ganadora == "Dolor Toracico y Respiratorio" &&
          !r.sin_sintomas);
}

void testTriajeEmpate(const MotorTriaje& motor)
{
    // Cat 3 (302=25) y Cat 7 (706=25) empatan. Gana Cat 3 por menor id.
    auto r = motor.evaluar({{3, {302}}, {7, {706}}}, 30, 'M', {});
    check("Triaje empate 25/25: gana Cat 3 por menor id, nivel IV",
          r.nivel_final == NivelTriaje::IV &&
          r.puntuacion_categoria_ganadora == 25 &&
          r.categoria_ganadora == "Fiebre y Dolor Abdominal");
}

void testTriajeSinSintomas(const MotorTriaje& motor)
{
    auto r = motor.evaluar({}, 30, 'M', {});
    check("Triaje sin sintomas: sin_sintomas=true, centinela V",
          r.sin_sintomas &&
          r.nivel_final == NivelTriaje::V &&
          r.puntuacion_categoria_ganadora == 0 &&
          r.puntuacion_factores_riesgo == 0);
}

void testTriajeMultiplesFactores(const MotorTriaje& motor)
{
    // 70 anios (+15) + RF_CARDIO (+10) + RF_RENAL (+10) sobre Cat 4 (65) = 100
    auto r = motor.evaluar({{4, {403}}}, 70, 'M', {"RF_CARDIO", "RF_RENAL"});
    check("Triaje multi-factor: 65+15+10+10=100, Nivel I",
          r.nivel_final == NivelTriaje::I &&
          r.puntuacion_factores_riesgo == 35);
}

void testTriajePregnancyHombre(const MotorTriaje& motor)
{
    // Hombre de 30 años con RF_PREGNANCY: no aplica
    auto r = motor.evaluar({{4, {403}}}, 30, 'M', {"RF_PREGNANCY"});
    check("Triaje RF_PREGNANCY en hombre: no suma (65, nivel II)",
          r.puntuacion_factores_riesgo == 0 &&
          r.nivel_final == NivelTriaje::II);
}

void testTriajePregnancyMujer(const MotorTriaje& motor)
{
    // Mujer de 30 años con RF_PREGNANCY: aplica +10 → 75, nivel II
    auto r = motor.evaluar({{4, {403}}}, 30, 'F', {"RF_PREGNANCY"});
    check("Triaje RF_PREGNANCY en mujer 30: suma 10 (75, nivel II)",
          r.puntuacion_factores_riesgo == 10 &&
          r.nivel_final == NivelTriaje::II);
}

void testTriajeFronteraInfantil(const MotorTriaje& motor)
{
    // 4 años → AGE_INFANT aplica (+15); 5 años → no aplica
    auto r4 = motor.evaluar({{4, {403}}}, 4, 'M', {});
    auto r5 = motor.evaluar({{4, {403}}}, 5, 'M', {});
    check("Triaje AGE_INFANT: 4 anios +15 (80, I); 5 anios +0 (65, II)",
          r4.puntuacion_factores_riesgo == 15 && r4.nivel_final == NivelTriaje::I &&
          r5.puntuacion_factores_riesgo == 0  && r5.nivel_final == NivelTriaje::II);
}

// -----------------------------------------------------------------------------
// Tests del motor de enrutamiento
// -----------------------------------------------------------------------------

void testEnrutamientoNivelI(const MotorTriaje& motor, const GestorCentros& gestor)
{
    // Nivel I → complejidad mínima 4 → solo el Hospital Universitario (id=1)
    auto triaje = motor.evaluar({{4, {403}}}, 70, 'M', {});
    MotorEnrutamiento enrut;
    auto r = enrut.recomendar(triaje, 8.600, -71.100, gestor);
    check("Enrutamiento Nivel I: solo complejidad 4 (Hospital Universitario)",
          !r.sin_centro_disponible &&
          r.centro_recomendado.id == 1 &&
          r.centro_recomendado.nivel_complejidad == 4 &&
          !r.ranking.empty());
}

void testEnrutamientoNivelIII(const MotorTriaje& motor, const GestorCentros& gestor)
{
    // Nivel III (45 pts, dolor moderado 402) → complejidad >= 2
    auto triaje = motor.evaluar({{4, {402}}}, 30, 'M', {});
    MotorEnrutamiento enrut;
    auto r = enrut.recomendar(triaje, 8.600, -71.100, gestor);
    check("Enrutamiento Nivel III: complejidad >= 2",
          !r.sin_centro_disponible &&
          r.centro_recomendado.nivel_complejidad >= 2);
}

void testEnrutamientoNivelIV(const MotorTriaje& motor, const GestorCentros& gestor)
{
    // Nivel IV (25 pts, migraña 706) → complejidad >= 1 (todos los centros)
    auto triaje = motor.evaluar({{7, {706}}}, 30, 'M', {});
    MotorEnrutamiento enrut;
    auto r = enrut.recomendar(triaje, 8.600, -71.100, gestor);
    check("Enrutamiento Nivel IV: encuentra centro valido (cualquier complejidad)",
          !r.sin_centro_disponible &&
          r.centro_recomendado.id >= 1);
}

void testEnrutamientoSinSintomas(const MotorTriaje& motor, const GestorCentros& gestor)
{
    auto triaje = motor.evaluar({}, 30, 'M', {});
    MotorEnrutamiento enrut;
    auto r = enrut.recomendar(triaje, 8.600, -71.100, gestor);
    check("Enrutamiento sin sintomas: cortocircuito (id=-1, sin_centro=true)",
          r.sin_centro_disponible &&
          r.centro_recomendado.id == -1);
}

void testEnrutamientoPesoCapacidad(const GestorCentros& gestor)
{
    // Paciente Nivel V justo en la ubicación de los dos ambulatorios (id 4 y 5).
    // Ambos están a distancia 0; el score debe preferir id=4 (18/20 disponibles,
    // cola 2) sobre id=5 (2/20 disponibles, cola 18).
    ResultadoTriaje triaje{};
    triaje.nivel_final   = NivelTriaje::V;
    triaje.sin_sintomas  = false;
    MotorEnrutamiento enrut;
    auto r = enrut.recomendar(triaje, 8.670, -71.170, gestor);
    check("Enrutamiento peso: capacidad alta + cola baja gana (id=4)",
          !r.sin_centro_disponible &&
          r.centro_recomendado.id == 4);
}

void testEnrutamientoComplejidadInsuficiente(const GestorCentros& gestor)
{
    // Nivel I con un gestor que solo tiene centros de complejidad < 4
    std::vector<CentroSalud> pocos = {
        {10, "amb", "Ambulatorio", 1, 20, 10, 2, 8.600, -71.100}
    };
    GestorCentros gestorPocos;
    gestorPocos.cargarCentros(pocos);

    ResultadoTriaje triaje{};
    triaje.nivel_final  = NivelTriaje::I;
    triaje.sin_sintomas = false;
    MotorEnrutamiento enrut;
    auto r = enrut.recomendar(triaje, 8.600, -71.100, gestorPocos);
    check("Enrutamiento sin centro apto: sin_centro=true, id=-1",
          r.sin_centro_disponible && r.centro_recomendado.id == -1);
}

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------
int main()
{
    BosqueTriaje bosque;
    bosque.construirDesdeCategorias(fixtureCategorias());

    MotorTriaje motor(bosque, fixtureFactores());

    GestorCentros gestor;
    gestor.cargarCentros(fixtureCentros());

    std::cout << "=== TESTS DEL MOTOR DE TRIAJE ===\n";
    testTriajeNivelI(motor);
    testTriajeEmpate(motor);
    testTriajeSinSintomas(motor);
    testTriajeMultiplesFactores(motor);
    testTriajePregnancyHombre(motor);
    testTriajePregnancyMujer(motor);
    testTriajeFronteraInfantil(motor);

    std::cout << "\n=== TESTS DEL MOTOR DE ENRUTAMIENTO ===\n";
    testEnrutamientoNivelI(motor, gestor);
    testEnrutamientoNivelIII(motor, gestor);
    testEnrutamientoNivelIV(motor, gestor);
    testEnrutamientoSinSintomas(motor, gestor);
    testEnrutamientoPesoCapacidad(gestor);
    testEnrutamientoComplejidadInsuficiente(gestor);

    std::cout << "\n=== RESUMEN ===\n";
    std::cout << "Total: " << total << " checks, " << fallos << " fallos\n";
    std::cout << (fallos == 0 ? "TODOS LOS TESTS PASARON\n"
                              : "FALLOS: " + std::to_string(fallos) + "\n");
    return fallos;
}
