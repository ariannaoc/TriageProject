#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ApiClient.hpp"
#include "BosqueTriaje.hpp"
#include "GestorCentros.hpp"
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

private slots:
    void verificarEmbarazo();
    void onCategoriaCambiada(int indice);
    void detectarUbicacion();
    void onPosicionActualizada(const QGeoPositionInfo& info);
    void onGeocodificado(double latitud, double longitud);
    void onErrorGeocodificacion(const QString& operacion, const QString& mensaje);
    void enviarFormulario();

private:
    QComboBox* comboAfectado;
    QComboBox* comboSexo;
    QSpinBox* spinEdad;
    QLineEdit* txtNombre;
    QLineEdit* txtApellido;
    QLineEdit* txtUbicacion;
    QPushButton* btnUbicacion;
    QLabel* lblUbicacionEstado;

    QComboBox* comboCategoria;
    QWidget* widgetSintomas;
    QVBoxLayout* layoutSintomas;
    QLabel* lblGuiaSintomas;

    QLabel* lblEstadoFormulario;

    // TODO(DEBUG): Eliminar sección temporal de selección manual de centro
    QCheckBox* chkForzarCentroDebug;
    QComboBox* comboCentroDebug;

    QPushButton* btnEvaluar;
    QPushButton* btnVolver;

    ApiClient* api = nullptr;
    QGeoPositionInfoSource* fuenteGps = nullptr;

    std::vector<CategoriaTriaje> catalogoCategorias;
    std::vector<FactorRiesgo> factoresTriaje;
    QJsonArray triageNiveles;
    std::map<QString, QCheckBox*> checkFactores;
    std::map<int, QCheckBox*> checkSintomas;
    std::vector<OpcionSintoma> opcionesCategoriaActual;
    int categoriaActual = -1;
    std::map<int, std::vector<int>> seleccionPorCategoria;

    BosqueTriaje bosqueTriaje_;
    std::unique_ptr<MotorTriaje> motorTriaje_;
    GestorCentros gestorCentros_;
    MotorEnrutamiento motorEnrutamiento_;

    double latPaciente = 8.5839;
    double lngPaciente = -71.1578;
    bool ubicacionResuelta = false;
    bool esperandoGeocodificacion = false;
    bool enviando = false;
    QString ultimoEstado;

    void cargarCatalogos();
    void configurarUI();
    void configurarGps();
    void aplicarEstilos();
    void reconstruirSintomas();
    bool resolveUbicacion();
    bool allowAmbulancia(NivelTriaje nivel) const;
};
