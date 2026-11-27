#pragma once
#include <QJsonObject>
#include <QString>
#include <QVector>
#include <QDate>

struct SesionCentro {
    int centerId = -1;
    QString username;
    QString role;
    QString centerName;
    QString centerType;
    int maxLevel = 0;
    int capacity = 0;

    static SesionCentro fromJson(const QJsonObject& json);
};

struct CentroInfo {
    int centerId = -1;
    QString name;
    QString type;
    int maxLevel = 0;
    int capacity = 0;
    QString location;
    double latitude = 0.0;
    double longitude = 0.0;
    int nivelComplejidad = 1;
    int capacidadTotal = 0;
    int capacidadDisponible = 0;
    int colaActual = 0;

    static CentroInfo fromJson(const QJsonObject& json);
};

struct PacienteTemp {
    int id = -1;
    int centerId = -1;
    QString cedula;
    QString firstName;
    QString lastName;
    int age = 0;
    int triageLevel = 0;
    int triageScore = 0;
    QString symptoms;

    static PacienteTemp fromJson(const QJsonObject& json);
};

struct DatoTendencia {
    QDate fecha;
    int nivelI    = 0;
    int nivelII   = 0;
    int nivelIII  = 0;
    int nivelIV   = 0;
    int nivelV    = 0;
    int total     = 0;

    /// Acceso por índice 0..4 → I..V.
    int nivel(int idx) const {
        switch (idx) {
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

