#include "ApiClient.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrlQuery>

namespace {
constexpr int kTimeoutMs = 8000;
}

ApiClient::ApiClient(QObject* parent)
    : QObject(parent),
      baseUrl(qEnvironmentVariable("TRIAGE_API_URL", QString("http://127.0.0.1:8081"))) {
    if (baseUrl.endsWith('/')) {
        baseUrl.chop(1);
    }
    connect(&manager, &QNetworkAccessManager::finished, this, &ApiClient::onFinished);
}

void ApiClient::health() {
    get("/api/health", "health");
}

void ApiClient::login(const QString& username, const QString& password) {
    postJson("/api/login", QJsonObject{{"username", username}, {"password", password}}, "login");
}

void ApiClient::fetchCenters() {
    get("/api/centers", "centers");
}

void ApiClient::fetchPendingPatients(int centerId) {
    QNetworkRequest request(endpoint(QString("/api/centers/%1/pending").arg(centerId)));
    QNetworkReply* reply = manager.get(request);
    reply->setProperty("centerId", centerId);
    prepareReply(reply, "pending");
}

void ApiClient::createTempPatient(const QJsonObject& patient) {
    postJson("/api/temp-patients", patient, "create");
}

void ApiClient::admitPatient(int centerId, int tempId, const QString& cedula) {
    postJson(QString("/api/centers/%1/admit/%2").arg(centerId).arg(tempId),
             QJsonObject{{"cedula", cedula}}, "admit");
}

void ApiClient::dischargePatient(int centerId, const QString& cedula) {
    postJson(QString("/api/centers/%1/discharge/%2").arg(centerId).arg(cedula),
             QJsonObject{}, "discharge");
}

void ApiClient::geocodeAddress(const QString& direccion) {
    QUrl url(QStringLiteral("https://nominatim.openstreetmap.org/search"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), direccion.trimmed() + QStringLiteral(", Mérida, Venezuela"));
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("jsonv2"));
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("bounded"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("viewbox"), QStringLiteral("-71.30,8.70,-70.95,8.45"));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "TriageProject/1.0 (Qt desktop client)");
    request.setRawHeader("Accept", "application/json");
    prepareReply(manager.get(request), "geocode");
}

QUrl ApiClient::endpoint(const QString& path) const {
    return QUrl(baseUrl + path);
}

void ApiClient::get(const QString& path, const QString& operation) {
    QNetworkRequest request(endpoint(path));
    request.setRawHeader("Accept", "application/json");
    prepareReply(manager.get(request), operation);
}

void ApiClient::postJson(const QString& path, const QJsonObject& body, const QString& operation) {
    QNetworkRequest request(endpoint(path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    prepareReply(manager.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact)), operation);
}

void ApiClient::prepareReply(QNetworkReply* reply, const QString& operation) {
    reply->setProperty("op", operation);
    auto* timer = new QTimer(reply);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [reply]() {
        reply->setProperty("timedOut", true);
        reply->abort();
    });
    timer->start(kTimeoutMs);
    emit requestStarted(operation);
}

void ApiClient::fail(QNetworkReply* reply, const QString& message) {
    emit failed(reply->property("op").toString(), message);
    emit requestFinished();
    reply->deleteLater();
}

QString ApiClient::errorMessage(const QJsonObject& body, int status, const QString& fallback) const {
    QString detail = body.value("message").toString();
    if (detail.isEmpty()) {
        detail = body.value("error").toString();
    }
    if (detail.isEmpty()) {
        detail = fallback;
    }
    return QString("HTTP %1: %2").arg(status).arg(detail);
}

void ApiClient::onFinished(QNetworkReply* reply) {
    const QString operation = reply->property("op").toString();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray bytes = reply->readAll();

    if (reply->error() != QNetworkReply::NoError) {
        if (reply->property("timedOut").toBool()) {
            fail(reply, "Tiempo de espera agotado al conectar con la API.");
        } else {
            fail(reply, "Error de conexión con la API: " + reply->errorString());
        }
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        fail(reply, "Respuesta inválida de la API: " + parseError.errorString());
        return;
    }

    if (operation == "geocode") {
        if (doc.isArray() && !doc.array().isEmpty()) {
            const QJsonObject primero = doc.array().first().toObject();
            bool okLat = false;
            bool okLng = false;
            const double lat = primero.value(QStringLiteral("lat")).toString().toDouble(&okLat);
            const double lng = primero.value(QStringLiteral("lon")).toString().toDouble(&okLng);
            if (okLat && okLng) {
                emit geocodificado(lat, lng);
            } else {
                emit failed("geocode", "No se pudo interpretar la ubicación devuelta.");
            }
        } else {
            emit failed("geocode", "No se encontró la dirección ingresada.");
        }
        emit requestFinished();
        reply->deleteLater();
        return;
    }

    const QJsonObject body = doc.object();
    if (status < 200 || status >= 300) {
        fail(reply, errorMessage(body, status, "Error inesperado del servidor"));
        return;
    }

    if (operation == "health") {
        emit healthOk();
    } else if (operation == "login") {
        const QJsonObject user = body.value("user").toObject();
        if (user.isEmpty()) {
            fail(reply, "La API no devolvió datos de usuario.");
            return;
        }
        emit loginOk(SesionCentro::fromJson(user));
    } else if (operation == "centers") {
        QVector<CentroInfo> centros;
        for (const QJsonValue& item : body.value("centers").toArray()) {
            centros.append(CentroInfo::fromJson(item.toObject()));
        }
        emit centersLoaded(centros);
    } else if (operation == "pending") {
        QVector<PacienteTemp> pacientes;
        for (const QJsonValue& item : body.value("patients").toArray()) {
            pacientes.append(PacienteTemp::fromJson(item.toObject()));
        }
        emit pendingLoaded(reply->property("centerId").toInt(), pacientes);
    } else if (operation == "create") {
        emit operationOk("Paciente registrado en la cola del centro.");
    } else if (operation == "admit") {
        emit operationOk("Paciente admitido correctamente.");
    } else if (operation == "discharge") {
        emit operationOk("Paciente dado de alta correctamente.");
    }

    emit requestFinished();
    reply->deleteLater();
}
