#pragma once

#include <QCache>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QPixmap>
#include <QPushButton>
#include <QSet>
#include <QWidget>

class QMouseEvent;
class QNetworkReply;
class QPaintEvent;
class QWheelEvent;

class MapaCanvas : public QWidget {
    Q_OBJECT

public:
    explicit MapaCanvas(QWidget* parent = nullptr);

    void setUbicacion(double latitud, double longitud);
    void setZoom(int zoom);
    double latitud() const { return marcadorLat_; }
    double longitud() const { return marcadorLng_; }
    int zoom() const { return zoom_; }
    QSize sizeHint() const override;

signals:
    void coordenadasCambiadas(double latitud, double longitud);

public slots:
    void acercar();
    void alejar();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private slots:
    void alTerminarTile(QNetworkReply* reply);

private:
    QPointF mundoDesdeLatLng(double latitud, double longitud) const;
    QPair<double, double> latLngDesdeMundo(const QPointF& mundo) const;
    QPointF origenVisible() const;
    void publicarPunto(const QPoint& punto);
    void solicitarTiles(const QPointF& origen);

    double centroLat_ = 8.5897;
    double centroLng_ = -71.1561;
    double marcadorLat_ = 8.5897;
    double marcadorLng_ = -71.1561;
    int zoom_ = 14;
    bool arrastrando_ = false;

    QNetworkAccessManager red_;
    QCache<QString, QPixmap> cacheTiles_;
    QSet<QString> descargasPendientes_;
};

class MapaSelectorWidget : public QWidget {
    Q_OBJECT

public:
    explicit MapaSelectorWidget(QWidget* parent = nullptr);

    double latitud() const { return latitud_; }
    double longitud() const { return longitud_; }

public slots:
    void setUbicacion(double latitud, double longitud);

signals:
    void ubicacionSeleccionada(double latitud, double longitud);

private slots:
    void onCoordenadasCanvas(double latitud, double longitud);
    void buscarLugar();
    void alTerminarBusqueda(QNetworkReply* reply);

private:
    void publicarUbicacion(double latitud, double longitud);

    QLineEdit* buscador_ = nullptr;
    QPushButton* btnBuscar_ = nullptr;
    QLabel* lblEstado_ = nullptr;
    MapaCanvas* canvas_ = nullptr;
    QNetworkAccessManager redBusqueda_;

    double latitud_ = 8.5897;
    double longitud_ = -71.1561;
};
