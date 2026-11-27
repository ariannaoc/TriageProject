#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QVector>
#include <QJsonObject>
#include <QTimer>
#include "ModelosCentro.hpp"
#include "ModelosApi.hpp"
#include "LoginWidget.hpp"
#include "FormularioPacienteWidget.hpp"

class ApiClient;
class BarraOcupacionWidget;
class PanelKpi3Widget;
class PanelKpi7Widget;
class QTableWidget;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    ApiClient* api;
    QStackedWidget* pilaVistas;

    // Vistas del sistema
    QWidget* vistaMain;
    FormularioPacienteWidget* vistaModuloPaciente;
    LoginWidget* vistaLoginCentro;
    QWidget* vistaDashboardCentro;

    // Etiquetas dinámicas del Dashboard para mostrar el centro autenticado
    QLabel* lblTituloDashboard;
    QLabel* lblDetalleCentroDashboard;
    QLabel* lblEstadoDashboard;
    QTableWidget* tablaPendientes;
    QPushButton* btnActualizarPendientes;
    QPushButton* btnAdmitir;
    QPushButton* btnDarAlta;

    // KPI 2 — Ocupación en tiempo real comprometida
    BarraOcupacionWidget* barraOcupacion;

    // KPI 3 — Snapshot de pendientes para el panel Nivel I/II
    QVector<PacienteTemp> pendientesActuales;

    // KPI 7 — Panel de tendencias históricas (activo solo con el diálogo abierto)
    PanelKpi7Widget* m_panelKpi7 = nullptr;
    QPushButton* btnVerTodosKpis;
    QTimer* timerRefrescoKpis;

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
    void actualizarBarraOcupacion();
    void onTrendsLoaded(int centerId, const QVector<DatoTendencia>& tendencias);
    void mostrarDialogoKpis();
    void setDashboardLoading(bool cargando);
    void actualizarCapacidad(int capacidad);
    bool cedulaValida(const QString& cedula) const;

private slots:
    void onLoginOk(const SesionCentro& sesion);
    void onCentrosCargados(const QVector<CentroInfo>& centros);
    void onPendientesCargados(int centerId, const QVector<PacienteTemp>& pacientes);
    void onOperationOk(const QString& message);
    void onRequestFailed(const QString& operation, const QString& message);
    void admitirSeleccionado();
    void darAltaSeleccionado();
    void enviarPaciente(const QJsonObject& datos);

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;
};
