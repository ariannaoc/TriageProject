#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QResizeEvent>
#include <QSpinBox>
#include <QTimer>
#include <QWidget>

#ifdef HAVE_QT_POSITIONING
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#endif

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ApiClient.hpp"
#include "BosqueTriaje.hpp"
#include "GestorCentros.hpp"
#include "MapaSelectorWidget.hpp"
#include "ModelosApi.hpp"
#include "MotorEnrutamiento.hpp"
#include "MotorTriaje.hpp"

class QVBoxLayout;

class FormularioPacienteWidget : public QWidget {
    Q_OBJECT

public:
    explicit FormularioPacienteWidget(QWidget* parent = nullptr);

    void setApiClient(ApiClient* api);
    void setCentros(const QVector<CentroInfo>& centros);
    void setEnviando(bool enviando);

signals:
    void volverAlMain();
    void pacienteListo(const QJsonObject& datos);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void verificarEmbarazo();
    void onCategoriaCambiada(int indice);
    void detectarUbicacion();
#ifdef HAVE_QT_POSITIONING
    void onPosicionActualizada(const QGeoPositionInfo& info);
#endif
    void onUbicacionSeleccionada(double latitud, double longitud);
    void actualizarAsignacion();
    void enviarFormulario();

private:
    void cargarCatalogos();
    void configurarUI();
    void configurarGps();
    void aplicarEstilos();
    void reconstruirSintomas();
    void fallbackUbicacionPorIp();
    std::vector<CentroSalud> centrosLocales() const;
    bool calcularTriajeYCentro(ResultadoTriaje& triaje, ResultadoEnrutamiento& ruta) const;
    bool allowAmbulancia(NivelTriaje nivel) const;

    QLabel* lblFondo = nullptr;

    QComboBox* comboAfectado;
    QComboBox* comboSexo;
    QSpinBox* spinEdad;
    QLineEdit* txtNombre;
    QLineEdit* txtApellido;
    QLineEdit* txtUbicacion;
    QPushButton* btnUbicacion;
    QLabel* lblUbicacionEstado;
    QLabel* lblCoordenadas;
    MapaSelectorWidget* mapaSelector;

    QComboBox* comboCategoria;
    QWidget* widgetSintomas;
    QVBoxLayout* layoutSintomas;
    QLabel* lblGuiaSintomas;

    QLabel* lblAsignacion;
    QLabel* lblEstadoFormulario;

    QPushButton* btnEvaluar;
    QPushButton* btnVolver;

    ApiClient* api = nullptr;
    QNetworkAccessManager redUbicacion;

#ifdef HAVE_QT_POSITIONING
    QGeoPositionInfoSource* fuenteGps = nullptr;
    QTimer* temporizadorGps = nullptr;
#endif
    bool esperandoGps = false;

    std::vector<CategoriaTriaje> catalogoCategorias;
    std::vector<FactorRiesgo> factoresTriaje;
    QJsonArray triageNiveles;
    std::map<QString, QCheckBox*> checkFactores;
    std::map<int, QCheckBox*> checkSintomas;
    std::vector<OpcionSintoma> opcionesCategoriaActual;
    int categoriaActual = -1;

    BosqueTriaje bosqueTriaje_;
    std::unique_ptr<MotorTriaje> motorTriaje_;
    GestorCentros gestorCentros_;
    MotorEnrutamiento motorEnrutamiento_;

    double latPaciente = 8.5897;
    double lngPaciente = -71.1561;
    bool enviando = false;
    QString ultimoEstado;
};
