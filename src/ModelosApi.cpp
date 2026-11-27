#include "ModelosApi.hpp"

SesionCentro SesionCentro::fromJson(const QJsonObject& json) {
    SesionCentro sesion;
    sesion.userId = json.value("user_id").toInt();
    sesion.username = json.value("username").toString();
    sesion.centerId = json.value("center_id").toInt();
    sesion.centerName = json.value("center_name").toString();
    sesion.centerType = json.value("center_type").toString();
    sesion.maxLevel = json.value("max_level").toInt();
    sesion.capacity = json.value("capacity").toInt();
    sesion.location = json.value("location").toString();
    return sesion;
}

CentroInfo CentroInfo::fromJson(const QJsonObject& json) {
    CentroInfo centro;
    centro.centerId = json.value("center_id").toInt();
    centro.name = json.value("name").toString();
    centro.type = json.value("type").toString();
    centro.maxLevel = json.value("max_level").toInt();
    centro.capacity = json.value("capacity").toInt();
    centro.location = json.value("location").toString();
    centro.latitude = json.value("latitude").toDouble();
    centro.longitude = json.value("longitude").toDouble();
    return centro;
}

PacienteTemp PacienteTemp::fromJson(const QJsonObject& json) {
    PacienteTemp paciente;
    paciente.id = json.value("id").toInt();
    paciente.cedula = json.value("cedula").toString();
    paciente.centerId = json.value("center_id").toInt();
    paciente.firstName = json.value("first_name").toString();
    paciente.lastName = json.value("last_name").toString();
    paciente.age = json.value("age").toInt();
    paciente.sex = json.value("sex").toString();
    paciente.triageLevel = json.value("triage_level").toInt();
    paciente.triageScore = json.value("triage_score").toInt();
    paciente.symptoms = json.value("symptoms").toString();
    paciente.source = json.value("source").toString();
    paciente.createdAt = json.value("created_at").toString();
    paciente.symptomIds = json.value("symptom_ids").toArray();
    paciente.riskFactorIds = json.value("risk_factor_ids").toArray();
    return paciente;
}

PacienteIngresado PacienteIngresado::fromJson(const QJsonObject& json) {
    PacienteIngresado paciente;
    paciente.patientId = json.value("patient_id").toInt();
    paciente.cedula = json.value("cedula").toString();
    paciente.firstName = json.value("first_name").toString();
    paciente.lastName = json.value("last_name").toString();
    paciente.age = json.value("age").toInt();
    paciente.triageLevel = json.value("triage_level").toInt();
    paciente.triageScore = json.value("triage_score").toInt();
    paciente.symptoms = json.value("symptoms").toString();
    paciente.status = json.value("status").toString();
    paciente.admissionDate = json.value("admission_date").toString();
    return paciente;
}

DatoTendencia DatoTendencia::fromJson(const QJsonObject& json) {
    DatoTendencia dato;
    dato.fecha = QDate::fromString(json.value("date").toString(), Qt::ISODate);
    dato.nivelI = json.value("level_1").toInt();
    dato.nivelII = json.value("level_2").toInt();
    dato.nivelIII = json.value("level_3").toInt();
    dato.nivelIV = json.value("level_4").toInt();
    dato.nivelV = json.value("level_5").toInt();
    dato.total = json.value("total").toInt();
    return dato;
}
