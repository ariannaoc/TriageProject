#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

struct SesionCentro {
    int userId = 0;
    QString username;
    int centerId = 0;
    QString centerName;
    QString centerType;
    int maxLevel = 0;
    int capacity = 0;
    QString location;

    static SesionCentro fromJson(const QJsonObject& json);
};

struct CentroInfo {
    int centerId = 0;
    QString name;
    QString type;
    int maxLevel = 0;
    int capacity = 0;
    QString location;
    double latitude = 0.0;
    double longitude = 0.0;

    static CentroInfo fromJson(const QJsonObject& json);
};

struct PacienteTemp {
    int id = 0;
    QString cedula;
    int centerId = 0;
    QString firstName;
    QString lastName;
    int age = 0;
    QString sex;
    int triageLevel = 0;
    int triageScore = 0;
    QString symptoms;
    QString source;
    QString createdAt;
    QJsonArray symptomIds;
    QJsonArray riskFactorIds;

    static PacienteTemp fromJson(const QJsonObject& json);
};
