#pragma once
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVector>
#include "ModelosApi.hpp"

class ApiClient : public QObject {
    Q_OBJECT

public:
    explicit ApiClient(QObject* parent = nullptr);

    void health();
    void login(const QString& username, const QString& password);
    void fetchCenters();
    void fetchPendingPatients(int centerId);
    void createTempPatient(const QJsonObject& patient);
    void admitPatient(int centerId, int tempId, const QString& cedula);
    void dischargePatient(int centerId, const QString& cedula);
    void geocodeAddress(const QString& direccion);

signals:
    void requestStarted(const QString& operation);
    void requestFinished();
    void healthOk();
    void loginOk(const SesionCentro& sesion);
    void centersLoaded(const QVector<CentroInfo>& centros);
    void pendingLoaded(int centerId, const QVector<PacienteTemp>& pacientes);
    void operationOk(const QString& message);
    void geocodificado(double latitud, double longitud);
    void failed(const QString& operation, const QString& message);

private slots:
    void onFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager manager;
    QString baseUrl;

    QUrl endpoint(const QString& path) const;
    void get(const QString& path, const QString& operation);
    void postJson(const QString& path, const QJsonObject& body, const QString& operation);
    void prepareReply(QNetworkReply* reply, const QString& operation);
    void fail(QNetworkReply* reply, const QString& message);
    QString errorMessage(const QJsonObject& body, int status, const QString& fallback) const;
};
