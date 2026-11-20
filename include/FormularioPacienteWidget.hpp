#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QJsonArray>
#include <QJsonObject>
#include "ModelosApi.hpp"

#if defined(HAVE_QT_POSITIONING)
class QGeoPositionInfo;
class QGeoPositionInfoSource;
#endif

#if defined(HAVE_QT_LOCATION)
class QQuickWidget;

class CoordenadasMapa : public QObject {
    Q_OBJECT
    Q_PROPERTY(double latitud READ latitud WRITE setLatitud NOTIFY coordenadasCambiadas)
    Q_PROPERTY(double longitud READ longitud WRITE setLongitud NOTIFY coordenadasCambiadas)

public:
    explicit CoordenadasMapa(QObject* parent = nullptr);
    double latitud() const { return mLatitud; }
    double longitud() const { return mLongitud; }

public slots:
    void setLatitud(double value);
    void setLongitud(double value);

signals:
    void coordenadasCambiadas();

private:
    double mLatitud;
    double mLongitud;
};
#endif

class FormularioPacienteWidget : public QWidget {
    Q_OBJECT

public:
    explicit FormularioPacienteWidget(QWidget* parent = nullptr);
    void setCentros(const QVector<CentroInfo>& centros);
    void setEnviando(bool enviando);
    void reiniciarFormulario();

signals:
    void volverAlMain();
    void pacienteListo(const QJsonObject& datos);

private:
    QComboBox* comboCentro;
    QComboBox* comboAfectado;
    QComboBox* comboSexo;
    QSpinBox* spinEdad;
    QLineEdit* txtNombre;
    QLineEdit* txtApellido;
    QLineEdit* txtUbicacion;
    QLabel* lblEstadoGps;
    QPushButton* btnGps;

#if defined(HAVE_QT_POSITIONING)
    QGeoPositionInfoSource* fuenteGps = nullptr;
#endif

#if defined(HAVE_QT_LOCATION)
    QQuickWidget* mapaWidget = nullptr;
    CoordenadasMapa* coordsMapa = nullptr;
#endif

    double latitud = 8.5897;
    double longitud = -71.1561;
    bool gpsObtenido = false;

    QCheckBox* chkCardio;
    QCheckBox* chkInmuno;
    QCheckBox* chkRenal;
    QCheckBox* chkEmbarazo;
    QCheckBox* chkRespiratorio;
    QCheckBox* chkMetabolico;
    QCheckBox* chkNeuro;

    QComboBox* comboSintoma;
    QLabel* lblEstadoFormulario;

    QPushButton* btnEvaluar;
    QPushButton* btnVolver;
    QPushButton* btnLimpiar;

    QJsonArray sintomasCatalogo;
    QJsonArray triageNiveles;
    QJsonObject factoresRiesgo;
    bool enviando;

    void configurarUI();
    void aplicarEstilos();
    void cargarCatalogos();
    void aplicarCoordenadas(double lat, double lon);
    void habilitarReferenciaManual(const QString& mensaje);
    void marcarUbicacionDetectada(const QString& referencia, const QString& estado);
#if defined(HAVE_QT_POSITIONING)
    void iniciarLecturaGps();
#endif
    int puntosFactor(const QString& id) const;
    int nivelParaPuntaje(int puntaje) const;

private slots:
    void verificarEmbarazo();
    void enviarFormulario();
    void solicitarUbicacion();
#if defined(HAVE_QT_POSITIONING)
    void onPosicionActualizada(const QGeoPositionInfo& info);
#endif
#if defined(HAVE_QT_LOCATION)
    void onCoordenadasMapaCambiadas();
#endif
};
