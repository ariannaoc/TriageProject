#include "MainWindow.hpp"
#include "ApiClient.hpp"
#include "VistaMain.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QInputDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QLabel>
#include <QCryptographicHash>
#include "BarraOcupacionWidget.hpp"
#include <QDialog>
#include "PanelKpi3Widget.hpp"
#include <QTabWidget>
#include "PanelKpi7Widget.hpp"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent),
    centerIdActual(-1), operacionDashboardEnCurso(false) {
    setWindowTitle("TriageMe - Sistema de Triage para Centros de Salud");
    resize(900, 450);

    api = new ApiClient(this);
    precargarCentrosMerida();

    pilaVistas = new QStackedWidget(this);
    setCentralWidget(pilaVistas);

    configurarVistaMain();
    configurarVistaPacientePlaceholder();
    vistaModuloPaciente->setApiClient(api);

    vistaLoginCentro = new LoginWidget(api, &arbolCentros, this);
    pilaVistas->addWidget(vistaLoginCentro);

    configurarVistaDashboardPlaceholder();

    connect(vistaLoginCentro, &LoginWidget::volverAlMain, this, [this]() {
        pilaVistas->setCurrentWidget(vistaMain);
    });

    connect(vistaLoginCentro, &LoginWidget::loginExitoso, this, &MainWindow::onLoginOk);

    connect(api, &ApiClient::centersLoaded, this, &MainWindow::onCentrosCargados);
    connect(api, &ApiClient::pendingLoaded, this, &MainWindow::onPendientesCargados);
    connect(api, &ApiClient::trendsLoaded, this, &MainWindow::onTrendsLoaded);
    connect(api, &ApiClient::healthOk, this, [this]() {
        if (centerIdActual <= 0) {
            lblEstadoDashboard->setText("API disponible.");
        }
    });
    connect(api, &ApiClient::operationOk, this, &MainWindow::onOperationOk);
    connect(api, &ApiClient::failed, this, &MainWindow::onRequestFailed);

    connect(vistaModuloPaciente, &FormularioPacienteWidget::pacienteListo,
            this, &MainWindow::enviarPaciente);

    pilaVistas->setCurrentWidget(vistaMain);
    api->health();
    api->fetchCenters();
}

void MainWindow::precargarCentrosMerida() {
    QString hashDefault = QString(QCryptographicHash::hash("1234", QCryptographicHash::Sha256).toHex());

    arbolCentros.insert({"iahula", hashDefault.toStdString(),
                         "IAHULA - Hospital Universitario de Los Andes", "Hospital Tipo IV", 1, 150});
    arbolCentros.insert({"sorjuana", hashDefault.toStdString(),
                         "Hospital Sor Juana Inés de la Cruz", "Hospital Tipo III", 2, 60});
    arbolCentros.insert({"elllano", hashDefault.toStdString(),
                         "Ambulatorio Urbano Tipo III El Llano", "Ambulatorio Urbano III", 3, 25});
}

void MainWindow::configurarVistaMain() {
    auto* v = new VistaMain(this);
    vistaMain = v;

    connect(v, &VistaMain::pacienteClicked, this, [this]() {
        pilaVistas->setCurrentWidget(vistaModuloPaciente);
    });
    connect(v, &VistaMain::centroClicked, this, [this]() {
        pilaVistas->setCurrentWidget(vistaLoginCentro);
    });

    pilaVistas->addWidget(vistaMain);
}

void MainWindow::configurarVistaPacientePlaceholder() {
    vistaModuloPaciente = new FormularioPacienteWidget(this);

    connect(vistaModuloPaciente, &FormularioPacienteWidget::volverAlMain, this, [this]() {
        pilaVistas->setCurrentWidget(vistaMain);
    });

    pilaVistas->addWidget(vistaModuloPaciente);
}

void MainWindow::configurarVistaDashboardPlaceholder() {
    vistaDashboardCentro = new QWidget(this);
    auto* layout = new QVBoxLayout(vistaDashboardCentro);
    layout->setContentsMargins(30, 20, 30, 20);

    lblTituloDashboard = new QLabel("Dashboard del Centro de Salud", this);
    lblTituloDashboard->setAlignment(Qt::AlignCenter);
    lblTituloDashboard->setStyleSheet("font-size: 22px; font-weight: bold; color: #1b5e20;");

    lblDetalleCentroDashboard = new QLabel("", this);
    lblDetalleCentroDashboard->setAlignment(Qt::AlignCenter);
    lblDetalleCentroDashboard->setStyleSheet("font-size: 14px; color: #333;");

    lblEstadoDashboard = new QLabel("", this);
    lblEstadoDashboard->setAlignment(Qt::AlignCenter);
    lblEstadoDashboard->setStyleSheet("font-size: 13px; color: #555;");

    tablaPendientes = new QTableWidget(this);
    tablaPendientes->setColumnCount(7);
    tablaPendientes->setHorizontalHeaderLabels({"ID", "Cédula", "Paciente", "Edad", "Nivel", "Puntaje", "Síntomas"});
    tablaPendientes->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaPendientes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaPendientes->setAlternatingRowColors(true);
    tablaPendientes->horizontalHeader()->setStretchLastSection(true);
    tablaPendientes->verticalHeader()->setVisible(false);

    btnActualizarPendientes = new QPushButton("Actualizar lista", this);
    btnAdmitir = new QPushButton("Admitir paciente seleccionado", this);
    btnDarAlta = new QPushButton("Dar de alta por cédula", this);
    auto* btnCerrarSesion = new QPushButton("🔒 Cerrar Sesión y Volver a Main", this);
    btnCerrarSesion->setMinimumHeight(40);

    auto* botones = new QHBoxLayout();
    botones->addWidget(btnActualizarPendientes);
    botones->addWidget(btnAdmitir);
    botones->addWidget(btnDarAlta);

    connect(btnActualizarPendientes, &QPushButton::clicked, this, &MainWindow::refrescarPendientes);
    connect(btnAdmitir, &QPushButton::clicked, this, &MainWindow::admitirSeleccionado);
    connect(btnDarAlta, &QPushButton::clicked, this, &MainWindow::darAltaSeleccionado);
    connect(btnCerrarSesion, &QPushButton::clicked, this, [this]() {
        pilaVistas->setCurrentWidget(vistaMain);
    });

    barraOcupacion = new BarraOcupacionWidget(this);
    auto* headerLayout = new QHBoxLayout();
    headerLayout->addWidget(lblTituloDashboard, 1);
    headerLayout->addWidget(barraOcupacion, 0);

    btnVerTodosKpis = new QPushButton("📊 Ver todos los KPIs", this);
    connect(btnVerTodosKpis, &QPushButton::clicked, this, &MainWindow::mostrarDialogoKpis);
    botones->addWidget(btnVerTodosKpis);

    layout->addLayout(headerLayout);
    layout->addWidget(lblDetalleCentroDashboard);
    layout->addWidget(lblEstadoDashboard);
    layout->addSpacing(10);
    layout->addWidget(tablaPendientes);
    layout->addLayout(botones);
    layout->addWidget(btnCerrarSesion);

    timerRefrescoKpis = new QTimer(this);
    timerRefrescoKpis->setInterval(30000);
    connect(timerRefrescoKpis, &QTimer::timeout, this, [this]() {
        refrescarPendientes();
        api->fetchCenters();
    });
    timerRefrescoKpis->start();

    pilaVistas->addWidget(vistaDashboardCentro);
}

void MainWindow::onLoginOk(const SesionCentro& sesion) {
    sesionActual = sesion;
    centerIdActual = sesion.centerId;

    lblTituloDashboard->setText(QString("🏥 Dashboard: %1").arg(sesion.centerName));
    actualizarCapacidad(sesion.capacity);
    pilaVistas->setCurrentWidget(vistaDashboardCentro);
    actualizarBarraOcupacion();
    refrescarPendientes();
    api->fetchCenters();
}

void MainWindow::onCentrosCargados(const QVector<CentroInfo>& centros) {
    centrosDisponibles = centros;
    vistaModuloPaciente->setCentros(centros);

    for (const CentroInfo& centro : centros) {
        if (centro.centerId == centerIdActual) {
            actualizarCapacidad(centro.capacity);
            break;
        }
    }

    actualizarBarraOcupacion();}

void MainWindow::onPendientesCargados(int centerId, const QVector<PacienteTemp>& pacientes) {
    pendientesActuales = pacientes;  // KPI 3: snapshot

    if (centerId != centerIdActual) {
        return;
    }

    operacionDashboardEnCurso = false;
    const int total = static_cast<int>(pacientes.size());
    tablaPendientes->setRowCount(total);

    for (int fila = 0; fila < total; ++fila) {
        const PacienteTemp& paciente = pacientes.at(fila);
        const QString cedula = paciente.cedula.isEmpty() ? "Sin cédula" : paciente.cedula;
        const QString nombre = (paciente.firstName + " " + paciente.lastName).trimmed();
        const QString celdas[] = {
            QString::number(paciente.id),
            cedula,
            nombre,
            QString::number(paciente.age),
            QString::number(paciente.triageLevel),
            QString::number(paciente.triageScore),
            paciente.symptoms
        };

        for (int columna = 0; columna < 7; ++columna) {
            tablaPendientes->setItem(fila, columna, new QTableWidgetItem(celdas[columna]));
        }
    }

    lblEstadoDashboard->setText(pacientes.isEmpty()
        ? "No hay pacientes pendientes en este centro."
        : QString("%1 paciente(s) pendiente(s).").arg(pacientes.size()));
    setDashboardLoading(false);

    actualizarBarraOcupacion();}

void MainWindow::onOperationOk(const QString& message) {
    operacionDashboardEnCurso = false;
    vistaModuloPaciente->setEnviando(false);
    QMessageBox::information(this, "Operación completada", message);
    api->fetchCenters();
    if (centerIdActual > 0) {
        refrescarPendientes();
    } else {
        setDashboardLoading(false);
    }
}

void MainWindow::onRequestFailed(const QString& operation, const QString& message) {
    if (operation == "health") {
        if (centerIdActual <= 0) {
            lblEstadoDashboard->setText("API no disponible.");
        }
        return;
    }

    if (operation == "centers") {
        if (centrosDisponibles.isEmpty()) {
            vistaModuloPaciente->setCentros({});
        }
        QMessageBox::critical(this, "Error de la API", message);
        return;
    }

    if (operation == "create") {
        vistaModuloPaciente->setEnviando(false);
        QMessageBox::critical(this, "Error de la API", message);
        return;
    }

    vistaModuloPaciente->setEnviando(false);
    if (!operacionDashboardEnCurso) {
        return;
    }
    operacionDashboardEnCurso = false;
    setDashboardLoading(false);
    lblEstadoDashboard->setText("No se pudo actualizar la información del centro.");
    QMessageBox::critical(this, "Error de la API", message);
}

void MainWindow::actualizarCapacidad(int capacidad) {
    lblDetalleCentroDashboard->setText(
        QString("Categoría: %1 | Nivel Máx MPPS: Nivel %2 | Camas disponibles: %3")
            .arg(sesionActual.centerType)
            .arg(sesionActual.maxLevel)
            .arg(capacidad));
}

void MainWindow::refrescarPendientes() {
    if (centerIdActual <= 0) {
        return;
    }
    operacionDashboardEnCurso = true;
    setDashboardLoading(true);
    api->fetchPendingPatients(centerIdActual);
}

void MainWindow::setDashboardLoading(bool cargando) {
    btnActualizarPendientes->setEnabled(!cargando);
    btnAdmitir->setEnabled(!cargando);
    btnDarAlta->setEnabled(!cargando);
    btnActualizarPendientes->setText(cargando ? "Actualizando…" : "Actualizar lista");
    if (cargando) {
        lblEstadoDashboard->setText("Cargando pacientes…");
    }
}

bool MainWindow::cedulaValida(const QString& cedula) const {
    static const QRegularExpression patron(QStringLiteral("^[A-Za-z0-9-]+$"));
    return patron.match(cedula).hasMatch();
}

void MainWindow::admitirSeleccionado() {
    const int fila = tablaPendientes->currentRow();
    if (fila < 0 || tablaPendientes->item(fila, 0) == nullptr) {
        QMessageBox::warning(this, "Sin selección", "Seleccione un paciente pendiente de la lista.");
        return;
    }

    const int tempId = tablaPendientes->item(fila, 0)->text().toInt();
    QString cedula = tablaPendientes->item(fila, 1)->text();
    if (cedula == "Sin cédula" || cedula.isEmpty()) {
        cedula = QInputDialog::getText(this, "Admitir paciente",
            "El paciente no tiene cédula. Ingrésela para admitirlo:");
    }
    cedula = cedula.trimmed();
    if (cedula.isEmpty()) {
        return;
    }
    if (!cedulaValida(cedula)) {
        QMessageBox::warning(this, "Cédula inválida",
            "Use solo letras, números y guiones (ej. V-12345678).");
        return;
    }

    operacionDashboardEnCurso = true;
    setDashboardLoading(true);
    api->admitPatient(centerIdActual, tempId, cedula);
}

void MainWindow::darAltaSeleccionado() {
    bool aceptado = false;
    const QString cedula = QInputDialog::getText(this, "Dar de alta",
        "Cédula del paciente admitido:", QLineEdit::Normal, "", &aceptado).trimmed();
    if (!aceptado || cedula.isEmpty()) {
        return;
    }
    if (!cedulaValida(cedula)) {
        QMessageBox::warning(this, "Cédula inválida",
            "Use solo letras, números y guiones (ej. V-12345678).");
        return;
    }

    operacionDashboardEnCurso = true;
    setDashboardLoading(true);
    api->dischargePatient(centerIdActual, cedula);
}

void MainWindow::enviarPaciente(const QJsonObject& datos) {
    api->createTempPatient(datos);
}

// =============================================================================
// KPI 2 — Ocupación en tiempo real comprometida
// =============================================================================

void MainWindow::actualizarBarraOcupacion()
{
    if (centerIdActual < 0 || !barraOcupacion) return;

    int capacidadTotal = sesionActual.capacity;
    int ocupado = 0;

    // Leer los datos reales y actualizados de la lista de centros
    for (const auto& c : centrosDisponibles) {
        if (c.centerId == centerIdActual) {
            ocupado = c.colaActual;
            if (c.capacity > 0) capacidadTotal = c.capacity;
            break;
        }
    }

    const int enCamino = tablaPendientes ? tablaPendientes->rowCount() : 0;

    // 1. Actualizar la barra gráfica (El 0/150)
    barraOcupacion->setDatos(ocupado, enCamino, capacidadTotal);

    // 2. Actualizar el texto "Camas disponibles"
    if (lblDetalleCentroDashboard) {
        int camasDisponibles = qMax(0, capacidadTotal - ocupado);
        lblDetalleCentroDashboard->setText(
            QString("Categoría: %1 | Nivel Máx MPPS: Nivel %2 | Camas disponibles: %3")
                .arg(sesionActual.centerType)
                .arg(sesionActual.maxLevel)
                .arg(camasDisponibles));
    }
}
void MainWindow::mostrarDialogoKpis()
{
    QDialog dlg(this);
    dlg.setWindowTitle("KPIs del Centro");
    dlg.resize(920, 640);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(12, 12, 12, 12);

    auto* titulo = new QLabel("\U0001F4CA Panel de KPIs", &dlg);
    titulo->setStyleSheet("font-size: 18px; font-weight: bold; color: #1b5e20;");
    titulo->setAlignment(Qt::AlignCenter);

    auto* tabs = new QTabWidget(&dlg);

    // -------- Pestaña 1: Resumen --------
    auto* tabResumen = new QWidget(tabs);
    auto* layResumen = new QVBoxLayout(tabResumen);
    auto* detalle = new QLabel(
        "KPIs disponibles:\n\n"
        "• KPI 3 — Nivel I/II en espera\n"
        "• KPI 7 — Tendencias por nivel de triaje\n\n"
        "KPIs descartados por falta de datos:\n\n"
        "• Door-to-Triage (no existe 'hora de llegada' al centro)\n"
        "• Pacientes en camino con ETA (no hay campo ETA ni motor de ruteo)",
        tabResumen);
    detalle->setWordWrap(true);
    detalle->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    layResumen->addWidget(detalle);
    layResumen->addStretch();
    tabs->addTab(tabResumen, "Resumen");

    // -------- Pestaña 2: KPI 3 --------
    auto* panelNivel = new PanelKpi3Widget(tabs);
    panelNivel->setPendientes(pendientesActuales);
    tabs->addTab(panelNivel, "Nivel I/II en espera");

    // -------- Pestaña 3: KPI 7 --------
    auto* panelTendencias = new PanelKpi7Widget(centerIdActual, tabs);
    connect(panelTendencias, &PanelKpi7Widget::solicitarTendencias,
            this, [this](int days) {
                if (centerIdActual >= 0)
                    api->fetchTrends(centerIdActual, days);
            });

    // Registrar el panel para que onTrendsLoaded() lo actualice
    m_panelKpi7 = panelTendencias;

    // Carga inicial (7 días por defecto)
    if (centerIdActual >= 0)
        api->fetchTrends(centerIdActual, 7);

    tabs->addTab(panelTendencias, "Tendencias (histórico)");

    layout->addWidget(titulo);
    layout->addWidget(tabs, 1);

    auto* cerrar = new QPushButton("Cerrar", &dlg);
    connect(cerrar, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(cerrar, 0, Qt::AlignRight);

    dlg.exec();

    // Al cerrar, el panel se destruye → evitar dangling
    m_panelKpi7 = nullptr;
}

// =============================================================================
// KPI 7 — Slot que recibe las tendencias desde ApiClient
// =============================================================================

void MainWindow::onTrendsLoaded(int centerId, const QVector<DatoTendencia>& tendencias)
{
    if (m_panelKpi7 && centerId == centerIdActual)
        m_panelKpi7->setTendencias(tendencias);
}


