#pragma once
#include <QDate>
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

struct PacienteIngresado {
    int patientId = 0;
    QString cedula;
    QString firstName;
    QString lastName;
    int age = 0;
    int triageLevel = 0;
    int triageScore = 0;
    QString symptoms;
    QString status;
    QString admissionDate;

    static PacienteIngresado fromJson(const QJsonObject& json);
};

struct DatoTendencia {
    QDate fecha;
    int nivelI = 0;
    int nivelII = 0;
    int nivelIII = 0;
    int nivelIV = 0;
    int nivelV = 0;
    int total = 0;

    int nivel(int indice) const {
        switch (indice) {
            case 0: return nivelI;
            case 1: return nivelII;
            case 2: return nivelIII;
            case 3: return nivelIV;
            case 4: return nivelV;
            default: return 0;
        }
    }

    static DatoTendencia fromJson(const QJsonObject& json);
};
