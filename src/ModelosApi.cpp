#include "ModelosApi.hpp"

SesionCentro SesionCentro::fromJson(const QJsonObject& json) {
    SesionCentro sesion;
    sesion.centerId = json.value("center_id").toInt(-1);
    sesion.username = json.value("username").toString();
    sesion.role = json.value("role").toString();
    sesion.centerName = json.value("center_name").toString();
    sesion.centerType = json.value("center_type").toString();
    sesion.maxLevel = json.value("max_level").toInt();
    sesion.capacity = json.value("capacity").toInt();
    return sesion;
}

CentroInfo CentroInfo::fromJson(const QJsonObject& json) {
    CentroInfo centro;
    centro.centerId = json.value("center_id").toInt(-1);
    centro.name = json.value("name").toString();
    centro.type = json.value("type").toString();
    centro.maxLevel = json.value("max_level").toInt();
    centro.capacity = json.value("capacity").toInt();
    centro.location = json.value("location").toString();
    centro.latitude = json.value("latitude").toDouble();
    centro.longitude = json.value("longitude").toDouble();
    centro.nivelComplejidad = json.value("nivel_complejidad").toInt(centro.maxLevel > 0 ? 5 - centro.maxLevel : 1);
    centro.capacidadTotal = json.value("capacidad_total").toInt(centro.capacity);
    centro.capacidadDisponible = json.value("capacidad_disponible").toInt(centro.capacity);
    centro.colaActual = json.value("cola_actual").toInt(0);
    return centro;
}

PacienteTemp PacienteTemp::fromJson(const QJsonObject& json) {
    PacienteTemp paciente;
    paciente.id = json.value("id").toInt(-1);
    paciente.centerId = json.value("center_id").toInt(-1);
    paciente.cedula = json.value("cedula").toString();
    paciente.firstName = json.value("first_name").toString();
    paciente.lastName = json.value("last_name").toString();
    paciente.age = json.value("age").toInt();
    paciente.triageLevel = json.value("triage_level").toInt();
    paciente.triageScore = json.value("triage_score").toInt();
    paciente.symptoms = json.value("symptoms").toString();
    return paciente;
}

DatoTendencia DatoTendencia::fromJson(const QJsonObject& json)
{
    DatoTendencia t;
    t.fecha    = QDate::fromString(json.value("date").toString(), Qt::ISODate);
    t.nivelI   = json.value("level_1").toInt();
    t.nivelII  = json.value("level_2").toInt();
    t.nivelIII = json.value("level_3").toInt();
    t.nivelIV  = json.value("level_4").toInt();
    t.nivelV   = json.value("level_5").toInt();
    t.total    = json.value("total").toInt();
    return t;
}
