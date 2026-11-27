#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QVector>
#include <QJsonObject>
#include "ModelosCentro.hpp"
#include "ModelosApi.hpp"
#include "LoginWidget.hpp"
#include "FormularioPacienteWidget.hpp"

class ApiClient;
class QTableWidget;
class QPushButton;
class QComboBox;
class QScrollArea;
class BarraOcupacionWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

    struct FilaPaciente {
        bool ingresado = false;
        int id = 0;
    };

private:
    ApiClient* api;
    QStackedWidget* pilaVistas;

    // Vistas del sistema
    QWidget* vistaMain;
    FormularioPacienteWidget* vistaModuloPaciente;
    LoginWidget* vistaLoginCentro;
    QWidget* vistaDashboardCentro;
    QScrollArea* scrollDashboard;

    // Cabecera del dashboard
    QLabel* lblTituloDashboard;
    QLabel* lblDetalleCentroDashboard;
    QLabel* lblEstadoDashboard;
    QLabel* badgeCategoria;
    QLabel* badgeNivelMax;
    QLabel* badgeCamas;
    BarraOcupacionWidget* barraOcupacion;
    QPushButton* btnActualizarPendientes;
    QPushButton* btnVerTodosKpis;

    // Tarjetas de resumen
    QLabel* lblKpiPendientes;
    QLabel* lblKpiIngresados;
    QLabel* lblKpiCamas;
    QLabel* lblKpiUrgencia;

    // Gestión de pacientes
    QTableWidget* tablaPendientes;
    QComboBox* comboFiltroPacientes;
    static constexpr int columnaAcciones = 8;
    QVector<FilaPaciente> filasPacientes;
    QVector<PacienteTemp> pendientesActuales;
    QVector<PacienteIngresado> admitidosActuales;
    int capacidadDisponibleActual = 0;
    bool pendientesRecibidos = false;
    bool admitidosRecibidos = false;

    // Árbol AVL de DeSiGNAR con las credenciales de los centros de salud de Mérida
    AVLTree<CredencialCentro> arbolCentros;
    QVector<CentroInfo> centrosDisponibles;
    SesionCentro sesionActual;
    int centerIdActual;
    bool operacionDashboardEnCurso;

    void precargarCentrosMerida();
    void configurarVistaMain();
    void configurarVistaPacientePlaceholder();
    void configurarVistaDashboardPlaceholder();
    void refrescarPendientes();
    void setDashboardLoading(bool cargando);
    void actualizarCapacidad(int capacidad);
    void reconstruirTablaPacientes();
    void aplicarFiltroTabla();
    void actualizarKpis();
    void actualizarBarraOcupacion();
    void mostrarDialogoKpis();
    void ingresarFila(int fila);
    void darAltaFila(int fila);

private slots:
    void onLoginOk(const SesionCentro& sesion);
    void onCentrosCargados(const QVector<CentroInfo>& centros);
    void onPendientesCargados(int centerId, const QVector<PacienteTemp>& pacientes);
    void onAdmitidosCargados(int centerId, const QVector<PacienteIngresado>& pacientes);
    void onOperationOk(const QString& message);
    void onRequestFailed(const QString& operation, const QString& message);
    void enviarPaciente(const QJsonObject& datos);

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;
};
