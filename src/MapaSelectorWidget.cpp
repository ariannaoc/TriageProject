#include "MapaSelectorWidget.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPalette>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QtMath>

#include <cmath>

namespace {
constexpr double kTamanoTile = 256.0;
constexpr double kLatMaximaWebMercator = 85.05112878;
constexpr double kPi = 3.14159265358979323846;

QString claveTile(int zoom, int x, int y) {
    return QStringLiteral("%1/%2/%3").arg(zoom).arg(x).arg(y);
}

bool enAreaMerida(double latitud, double longitud) {
    return latitud >= 7.4 && latitud <= 9.1 && longitud >= -72.3 && longitud <= -70.4;
}
}

MapaCanvas::MapaCanvas(QWidget* parent)
    : QWidget(parent), cacheTiles_(256) {
    setCursor(Qt::CrossCursor);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    connect(&red_, &QNetworkAccessManager::finished, this, &MapaCanvas::alTerminarTile);
}

QSize MapaCanvas::sizeHint() const {
    return QSize(640, 360);
}

void MapaCanvas::setUbicacion(double latitud, double longitud) {
    if (qFuzzyIsNull(latitud)) {
        latitud = 8.5897;
        longitud = -71.1561;
    }
    centroLat_ = qBound(-kLatMaximaWebMercator, latitud, kLatMaximaWebMercator);
    centroLng_ = qBound(-180.0, longitud, 180.0);
    marcadorLat_ = centroLat_;
    marcadorLng_ = centroLng_;
    update();
}

void MapaCanvas::setZoom(int zoom) {
    const int nuevoZoom = qBound(2, zoom, 19);
    if (nuevoZoom == zoom_) {
        return;
    }
    zoom_ = nuevoZoom;
    update();
}

void MapaCanvas::acercar() {
    setZoom(zoom_ + 1);
}

void MapaCanvas::alejar() {
    setZoom(zoom_ - 1);
}

QPointF MapaCanvas::mundoDesdeLatLng(double latitud, double longitud) const {
    const double cantidad = static_cast<double>(1 << zoom_) * kTamanoTile;
    const double latLimitada = qBound(-kLatMaximaWebMercator, latitud, kLatMaximaWebMercator);
    const double radianes = qDegreesToRadians(latLimitada);
    const double x = (longitud + 180.0) / 360.0 * cantidad;
    const double y = (1.0 - std::log(std::tan(radianes) + 1.0 / std::cos(radianes)) / kPi) /
                     2.0 * cantidad;
    return QPointF(x, y);
}

QPair<double, double> MapaCanvas::latLngDesdeMundo(const QPointF& mundo) const {
    const double cantidad = static_cast<double>(1 << zoom_) * kTamanoTile;
    const double longitud = mundo.x() / cantidad * 360.0 - 180.0;
    const double latitud = qRadiansToDegrees(
        std::atan(std::sinh(kPi * (1.0 - 2.0 * mundo.y() / cantidad))));
    return qMakePair(qBound(-kLatMaximaWebMercator, latitud, kLatMaximaWebMercator),
                     qBound(-180.0, longitud, 180.0));
}

QPointF MapaCanvas::origenVisible() const {
    const QPointF centro = mundoDesdeLatLng(centroLat_, centroLng_);
    return QPointF(centro.x() - width() / 2.0, centro.y() - height() / 2.0);
}

void MapaCanvas::publicarPunto(const QPoint& punto) {
    const QPointF origen = origenVisible();
    const QPair<double, double> coords =
        latLngDesdeMundo(QPointF(origen.x() + punto.x(), origen.y() + punto.y()));
    marcadorLat_ = coords.first;
    marcadorLng_ = coords.second;
    update();
    emit coordenadasCambiadas(marcadorLat_, marcadorLng_);
}

void MapaCanvas::solicitarTiles(const QPointF& origen) {
    const int cantidad = 1 << zoom_;
    const int x0 = static_cast<int>(std::floor(origen.x() / kTamanoTile));
    const int x1 = static_cast<int>(std::floor((origen.x() + width()) / kTamanoTile));
    const int y0 = static_cast<int>(std::floor(origen.y() / kTamanoTile));
    const int y1 = static_cast<int>(std::floor((origen.y() + height()) / kTamanoTile));

    for (int x = x0; x <= x1; ++x) {
        const int xEnvuelto = ((x % cantidad) + cantidad) % cantidad;
        for (int y = y0; y <= y1; ++y) {
            if (y < 0 || y >= cantidad) {
                continue;
            }
            const QString clave = claveTile(zoom_, xEnvuelto, y);
            if (cacheTiles_.contains(clave) || descargasPendientes_.contains(clave)) {
                continue;
            }

            descargasPendientes_.insert(clave);
            const QUrl url(QStringLiteral("https://tile.openstreetmap.org/%1/%2/%3.png")
                               .arg(zoom_)
                               .arg(xEnvuelto)
                               .arg(y));
            QNetworkRequest solicitud(url);
            solicitud.setRawHeader("User-Agent", "TriageProject/1.0");
            solicitud.setRawHeader("Accept", "image/png");
            QNetworkReply* reply = red_.get(solicitud);
            reply->setProperty("clave", clave);
        }
    }
}

void MapaCanvas::alTerminarTile(QNetworkReply* reply) {
    reply->deleteLater();
    const QString clave = reply->property("clave").toString();
    descargasPendientes_.remove(clave);

    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    QPixmap pixmap;
    if (pixmap.loadFromData(reply->readAll(), "PNG")) {
        cacheTiles_.insert(clave, new QPixmap(pixmap), 1);
        update();
    }
}

void MapaCanvas::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor("#E8F1F5"));

    const QPointF origen = origenVisible();
    solicitarTiles(origen);

    const int cantidad = 1 << zoom_;
    const int x0 = static_cast<int>(std::floor(origen.x() / kTamanoTile));
    const int x1 = static_cast<int>(std::floor((origen.x() + width()) / kTamanoTile));
    const int y0 = static_cast<int>(std::floor(origen.y() / kTamanoTile));
    const int y1 = static_cast<int>(std::floor((origen.y() + height()) / kTamanoTile));

    for (int x = x0; x <= x1; ++x) {
        const int xEnvuelto = ((x % cantidad) + cantidad) % cantidad;
        for (int y = y0; y <= y1; ++y) {
            if (y < 0 || y >= cantidad) {
                continue;
            }

            const QRectF destino(x * kTamanoTile - origen.x(),
                                 y * kTamanoTile - origen.y(),
                                 kTamanoTile, kTamanoTile);
            const QPixmap* tile = cacheTiles_.object(claveTile(zoom_, xEnvuelto, y));
            if (tile != nullptr && !tile->isNull()) {
                painter.drawPixmap(qRound(destino.left()), qRound(destino.top()), *tile);
            } else {
                painter.fillRect(destino, QColor("#DCE9EF"));
                painter.setPen(QPen(QColor("#B9CFD9"), 1));
                painter.drawRect(destino.adjusted(0, 0, -1, -1));
            }
        }
    }

    const QPointF pin = mundoDesdeLatLng(marcadorLat_, marcadorLng_) - origen;
    const QColor colorPin = enAreaMerida(marcadorLat_, marcadorLng_)
                                ? QColor("#16A34A")
                                : QColor("#DC2626");
    painter.setPen(Qt::NoPen);
    painter.setBrush(colorPin);
    painter.drawEllipse(QRectF(pin.x() - 10, pin.y() - 31, 20, 20));
    QPolygonF punta;
    punta << QPointF(pin.x(), pin.y())
          << QPointF(pin.x() - 7, pin.y() - 17)
          << QPointF(pin.x() + 7, pin.y() - 17);
    painter.drawPolygon(punta);
    painter.setBrush(Qt::white);
    painter.drawEllipse(QRectF(pin.x() - 4, pin.y() - 25, 8, 8));

    painter.fillRect(QRect(8, 8, 230, 24), QColor(255, 255, 255, 215));
    painter.setPen(QColor("#1F2937"));
    painter.drawText(QRect(8, 8, 230, 24), Qt::AlignCenter,
                     QStringLiteral("Zoom %1 · clic/arrastre para ajustar").arg(zoom_));
}

void MapaCanvas::mousePressEvent(QMouseEvent* event) {
    arrastrando_ = true;
    publicarPunto(event->position().toPoint());
}

void MapaCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (arrastrando_) {
        publicarPunto(event->position().toPoint());
    }
}

void MapaCanvas::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event)
    arrastrando_ = false;
}

void MapaCanvas::wheelEvent(QWheelEvent* event) {
    setZoom(zoom_ + (event->angleDelta().y() > 0 ? 1 : -1));
    event->accept();
}

MapaSelectorWidget::MapaSelectorWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    auto* filaBusqueda = new QHBoxLayout();
    filaBusqueda->setSpacing(6);
    buscador_ = new QLineEdit(this);
    buscador_->setPlaceholderText("Buscar sector, calle o centro (Nominatim)…");
    btnBuscar_ = new QPushButton("Buscar", this);
    filaBusqueda->addWidget(buscador_, 1);
    filaBusqueda->addWidget(btnBuscar_);
    layout->addLayout(filaBusqueda);

    auto* marcoMapa = new QFrame(this);
    auto* layoutMapa = new QHBoxLayout(marcoMapa);
    layoutMapa->setContentsMargins(0, 0, 0, 0);
    layoutMapa->setSpacing(4);

    canvas_ = new MapaCanvas(marcoMapa);
    canvas_->setMinimumHeight(280);
    layoutMapa->addWidget(canvas_, 1);

    auto* controlesZoom = new QVBoxLayout();
    controlesZoom->setSpacing(4);
    auto* btnAcercar = new QPushButton("+", marcoMapa);
    auto* btnAlejar = new QPushButton("−", marcoMapa);
    btnAcercar->setFixedSize(32, 32);
    btnAlejar->setFixedSize(32, 32);
    btnAcercar->setToolTip("Acercar");
    btnAlejar->setToolTip("Alejar");
    controlesZoom->addWidget(btnAcercar);
    controlesZoom->addWidget(btnAlejar);
    controlesZoom->addStretch(1);
    layoutMapa->addLayout(controlesZoom);
    layout->addWidget(marcoMapa, 1);

    lblEstado_ = new QLabel("Mapa centrado en Mérida. Haga clic o arrastre para ajustar el pin.", this);
    lblEstado_->setWordWrap(true);
    layout->addWidget(lblEstado_);

    connect(canvas_, &MapaCanvas::coordenadasCambiadas,
            this, &MapaSelectorWidget::onCoordenadasCanvas);
    connect(buscador_, &QLineEdit::returnPressed, this, &MapaSelectorWidget::buscarLugar);
    connect(btnBuscar_, &QPushButton::clicked, this, &MapaSelectorWidget::buscarLugar);
    connect(btnAcercar, &QPushButton::clicked, canvas_, &MapaCanvas::acercar);
    connect(btnAlejar, &QPushButton::clicked, canvas_, &MapaCanvas::alejar);
    connect(&redBusqueda_, &QNetworkAccessManager::finished,
            this, &MapaSelectorWidget::alTerminarBusqueda);

    QTimer::singleShot(0, this, [this]() {
        emit ubicacionSeleccionada(latitud_, longitud_);
    });
}

void MapaSelectorWidget::setUbicacion(double latitud, double longitud) {
    publicarUbicacion(latitud, longitud);
}

void MapaSelectorWidget::publicarUbicacion(double latitud, double longitud) {
    if (qFuzzyIsNull(latitud)) {
        latitud = 8.5897;
        longitud = -71.1561;
    }

    latitud_ = qBound(-89.0, latitud, 89.0);
    longitud_ = qBound(-180.0, longitud, 180.0);
    if (canvas_ != nullptr) {
        canvas_->setUbicacion(latitud_, longitud_);
    }
    emit ubicacionSeleccionada(latitud_, longitud_);
}

void MapaSelectorWidget::onCoordenadasCanvas(double latitud, double longitud) {
    latitud_ = latitud;
    longitud_ = longitud;
    lblEstado_->setText(QStringLiteral("Ubicación fijada: %1, %2")
                            .arg(latitud_, 0, 'f', 6)
                            .arg(longitud_, 0, 'f', 6));
    emit ubicacionSeleccionada(latitud_, longitud_);
}

void MapaSelectorWidget::buscarLugar() {
    const QString texto = buscador_->text().trimmed();
    if (texto.isEmpty()) {
        lblEstado_->setText("Escriba un sector, calle o centro para buscar.");
        return;
    }

    QUrl url(QStringLiteral("https://nominatim.openstreetmap.org/search"));
    QUrlQuery consulta;
    consulta.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    consulta.addQueryItem(QStringLiteral("q"), texto);
    consulta.addQueryItem(QStringLiteral("countrycodes"), QStringLiteral("ve"));
    consulta.addQueryItem(QStringLiteral("limit"), QStringLiteral("1"));
    url.setQuery(consulta);

    QNetworkRequest solicitud(url);
    solicitud.setRawHeader("User-Agent", "TriageProject/1.0");
    solicitud.setRawHeader("Accept", "application/json");
    lblEstado_->setText("Buscando en Nominatim…");
    redBusqueda_.get(solicitud);
}

void MapaSelectorWidget::alTerminarBusqueda(QNetworkReply* reply) {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        lblEstado_->setText("No se pudo buscar el lugar; ajuste el pin en el mapa.");
        return;
    }

    const QJsonArray resultados = QJsonDocument::fromJson(reply->readAll()).array();
    if (resultados.isEmpty()) {
        lblEstado_->setText("Sin resultados en Venezuela; ajuste el pin en el mapa.");
        return;
    }

    const QJsonObject primero = resultados.first().toObject();
    bool latValida = false;
    bool lngValida = false;
    const double latitud = primero.value(QStringLiteral("lat")).toString().toDouble(&latValida);
    const double longitud = primero.value(QStringLiteral("lon")).toString().toDouble(&lngValida);
    if (!latValida || !lngValida) {
        lblEstado_->setText("Resultado de búsqueda inválido; ajuste el pin en el mapa.");
        return;
    }

    setUbicacion(latitud, longitud);
    canvas_->setZoom(15);
    lblEstado_->setText(primero.value(QStringLiteral("display_name")).toString());
}
