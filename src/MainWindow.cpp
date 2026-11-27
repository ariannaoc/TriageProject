#include "MainWindow.hpp"
#include "ApiClient.hpp"
#include "VistaMain.hpp"
#include "BarraOcupacionWidget.hpp"
#include "PanelKpi3Widget.hpp"
#include "PanelKpi7Widget.hpp"

#include <QAbstractItemView>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDialog>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPalette>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcut>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

namespace {

const QString kEstiloDashboard = QStringLiteral(R"(
QWidget#vistaDashboard { background: #F4F6F9; }
QFrame#tarjeta { background: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 12px; }
QLabel#tituloCentro { font-size: 22px; font-weight: 700; color: #0F172A; }
QLabel#subtituloCentro { font-size: 13px; color: #64748B; }
QLabel#tituloTarjeta { font-size: 15px; font-weight: 700; color: #0F172A; }
QLabel#tituloKpi { font-size: 12px; font-weight: 700; color: #64748B; }
QLabel#valorKpi { font-size: 26px; font-weight: 700; color: #0F172A; }
QLabel#badgeNeutro, QLabel#badgeEspera, QLabel#badgeIngresado,
QLabel#badgeNivel1, QLabel#badgeNivel2, QLabel#badgeNivel3,
QLabel#badgeNivel4, QLabel#badgeNivel5 {
    padding: 4px 10px; border-radius: 10px; font-size: 12px; font-weight: 700;
}
QLabel#badgeNeutro { background: #F1F5F9; color: #475569; }
QLabel#badgeEspera { background: #FEF3C7; color: #92400E; }
QLabel#badgeIngresado { background: #DCFCE7; color: #166534; }
QLabel#badgeNivel1 { background: #FEE2E2; color: #B91C1C; }
QLabel#badgeNivel2 { background: #FFEDD5; color: #C2410C; }
QLabel#badgeNivel3 { background: #FEF9C3; color: #A16207; }
QLabel#badgeNivel4 { background: #DCFCE7; color: #15803D; }
QLabel#badgeNivel5 { background: #DBEAFE; color: #1D4ED8; }
QPushButton#btnActualizar { background: #0F766E; color: #FFFFFF; border: none; border-radius: 8px; padding: 8px 14px; font-weight: 700; }
QPushButton#btnActualizar:hover { background: #115E59; }
QPushButton#btnKpis { background: #FFFFFF; color: #0F766E; border: 1px solid #99F6E4; border-radius: 8px; padding: 8px 14px; font-weight: 700; }
QPushButton#btnKpis:hover { background: #F0FDFA; }
QPushButton#btnSalir { background: #FFFFFF; color: #DC2626; border: 1px solid #FECACA; border-radius: 8px; padding: 8px 14px; font-weight: 700; }
QPushButton#btnSalir:hover { background: #FEF2F2; }
QPushButton#btnIngresar { background: #10B981; color: #FFFFFF; border: none; border-radius: 8px; padding: 6px 12px; font-weight: 700; }
QPushButton#btnIngresar:hover { background: #059669; }
QPushButton#btnDarAlta { background: #FFFFFF; color: #2563EB; border: 1px solid #BFDBFE; border-radius: 8px; padding: 6px 12px; font-weight: 700; }
QPushButton#btnDarAlta:hover { background: #EFF6FF; }
QPushButton:disabled { background: #E2E8F0; color: #94A3B8; border-color: #E2E8F0; }
QComboBox#comboFiltro { background: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; padding: 6px 10px; color: #334155; }
QTableWidget#tablaPacientes { background: #FFFFFF; border: none; gridline-color: transparent; selection-background-color: #DBEAFE; selection-color: #1E3A8A; }
QTableWidget#tablaPacientes::item { border-bottom: 1px solid #EEF2F7; padding-left: 10px; color: #334155; }
QTableWidget#tablaPacientes::item:hover { background: #F8FAFC; }
QTableWidget#tablaPacientes::item:selected { background: #DBEAFE; color: #1E3A8A; }
QHeaderView::section { background: #F8FAFC; color: #475569; font-weight: 700; border: none; border-bottom: 1px solid #E2E8F0; padding: 10px 8px; }
QScrollArea#scrollDashboard { background: #F4F6F9; border: none; }
)");

QFrame* crearTarjeta(QWidget* parent) {
    auto* tarjeta = new QFrame(parent);
    tarjeta->setObjectName("tarjeta");
    tarjeta->setFrameShape(QFrame::NoFrame);

    auto* sombra = new QGraphicsDropShadowEffect(tarjeta);
    sombra->setBlurRadius(18);
    sombra->setOffset(0, 4);
    sombra->setColor(QColor(15, 23, 42, 26));
    tarjeta->setGraphicsEffect(sombra);
    return tarjeta;
}

QLabel* crearBadge(QWidget* parent, const QString& texto, const QString& objectName) {
    auto* badge = new QLabel(texto, parent);
    badge->setObjectName(objectName);
    badge->setAlignment(Qt::AlignCenter);
    return badge;
}

QFrame* crearTarjetaKpi(const QString& titulo, const QString& valor, QWidget* parent, QLabel*& etiquetaValor) {
    auto* tarjeta = crearTarjeta(parent);
    auto* layout = new QVBoxLayout(tarjeta);
    layout->setContentsMargins(18, 14, 18, 14);
    layout->setSpacing(4);

    auto* lblTitulo = new QLabel(titulo, tarjeta);
    lblTitulo->setObjectName("tituloKpi");
    etiquetaValor = new QLabel(valor, tarjeta);
    etiquetaValor->setObjectName("valorKpi");

    layout->addWidget(lblTitulo);
    layout->addWidget(etiquetaValor);
    return tarjeta;
}

QString nivelRomano(int nivel) {
    static const QStringList nombres{"I", "II", "III", "IV", "V"};
    if (nivel < 1 || nivel > 5) {
        return "-";
    }
    return nombres.at(nivel - 1);
}

QString resumirSintomas(const QString& sintomas) {
    if (sintomas.size() <= 70) {
        return sintomas;
    }
    return sintomas.left(67) + "...";
}

}  // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent),
    centerIdActual(-1), operacionDashboardEnCurso(false) {
    setWindowTitle("TriageMe - Sistema de Triage para Centros de Salud");
    resize(1180, 760);

    api = new ApiClient(this);
    precargarCentrosMerida();

    pilaVistas = new QStackedWidget(this);
    setCentralWidget(pilaVistas);

    configurarVistaMain();
    configurarVistaPacientePlaceholder();

    vistaLoginCentro = new LoginWidget(api, &arbolCentros, this);
    pilaVistas->addWidget(vistaLoginCentro);

    configurarVistaDashboardPlaceholder();

    connect(vistaLoginCentro, &LoginWidget::volverAlMain, this, [this]() {
        pilaVistas->setCurrentWidget(vistaMain);
    });

    connect(vistaLoginCentro, &LoginWidget::loginExitoso, this, &MainWindow::onLoginOk);

    connect(api, &ApiClient::centersLoaded, this, &MainWindow::onCentrosCargados);
    connect(api, &ApiClient::pendingLoaded, this, &MainWindow::onPendientesCargados);
    connect(api, &ApiClient::admittedLoaded, this, &MainWindow::onAdmitidosCargados);
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

    auto* atajoMovil = new QShortcut(QKeySequence("Ctrl+M"), this);
    atajoMovil->setContext(Qt::ApplicationShortcut);
    connect(atajoMovil, &QShortcut::activated, this, [this]() {
        const bool irAMovil = width() > 500;
        if (isMaximized()) {
            showNormal();
            QTimer::singleShot(50, this, [this, irAMovil]() {
                resize(irAMovil ? QSize(390, 844) : QSize(1180, 760));
            });
        } else {
            resize(irAMovil ? QSize(390, 844) : QSize(1180, 760));
        }
    });
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
    auto* scrollMain = new QScrollArea(this);
    scrollMain->setWidgetResizable(true);
    scrollMain->setFrameShape(QFrame::NoFrame);
    scrollMain->setWidget(v);

    vistaMain = scrollMain;
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
    vistaDashboardCentro->setObjectName("vistaDashboard");
    vistaDashboardCentro->setStyleSheet(kEstiloDashboard);
    vistaDashboardCentro->setMinimumWidth(760);

    auto* layout = new QVBoxLayout(vistaDashboardCentro);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(18);

    auto* cabecera = crearTarjeta(vistaDashboardCentro);
    auto* layCabecera = new QHBoxLayout(cabecera);
    layCabecera->setContentsMargins(22, 20, 22, 20);
    layCabecera->setSpacing(24);

    auto* bloqueTitulo = new QVBoxLayout();
    bloqueTitulo->setSpacing(8);
    lblTituloDashboard = new QLabel("Dashboard del Centro de Salud", cabecera);
    lblTituloDashboard->setObjectName("tituloCentro");
    lblDetalleCentroDashboard = new QLabel("Gestión de triaje y ocupación en tiempo real", cabecera);
    lblDetalleCentroDashboard->setObjectName("subtituloCentro");

    auto* badges = new QHBoxLayout();
    badges->setSpacing(8);
    badgeCategoria = crearBadge(cabecera, "Categoría: -", "badgeNeutro");
    badgeNivelMax = crearBadge(cabecera, "Nivel Máx MPPS: -", "badgeNeutro");
    badgeCamas = crearBadge(cabecera, "Camas disponibles: -", "badgeNeutro");
    badges->addWidget(badgeCategoria);
    badges->addWidget(badgeNivelMax);
    badges->addWidget(badgeCamas);
    badges->addStretch(1);

    bloqueTitulo->addWidget(lblTituloDashboard);
    bloqueTitulo->addWidget(lblDetalleCentroDashboard);
    bloqueTitulo->addLayout(badges);

    auto* bloqueCapacidad = new QVBoxLayout();
    bloqueCapacidad->setSpacing(10);
    barraOcupacion = new BarraOcupacionWidget(cabecera);
    barraOcupacion->setMinimumWidth(340);

    auto* accionesRapidas = new QHBoxLayout();
    accionesRapidas->setSpacing(8);
    btnActualizarPendientes = new QPushButton("Actualizar lista", cabecera);
    btnActualizarPendientes->setObjectName("btnActualizar");
    btnVerTodosKpis = new QPushButton("Ver todos los KPIs", cabecera);
    btnVerTodosKpis->setObjectName("btnKpis");
    auto* btnCerrarSesion = new QPushButton("Cerrar sesión", cabecera);
    btnCerrarSesion->setObjectName("btnSalir");
    accionesRapidas->addStretch(1);
    accionesRapidas->addWidget(btnActualizarPendientes);
    accionesRapidas->addWidget(btnVerTodosKpis);
    accionesRapidas->addWidget(btnCerrarSesion);

    bloqueCapacidad->addWidget(barraOcupacion);
    bloqueCapacidad->addLayout(accionesRapidas);

    layCabecera->addLayout(bloqueTitulo, 1);
    layCabecera->addLayout(bloqueCapacidad, 0);
    layout->addWidget(cabecera);

    auto* filaKpis = new QGridLayout();
    filaKpis->setSpacing(16);
    filaKpis->addWidget(crearTarjetaKpi("Pacientes en espera", "0", vistaDashboardCentro,
                                       lblKpiPendientes), 0, 0);
    filaKpis->addWidget(crearTarjetaKpi("Pacientes ingresados", "0", vistaDashboardCentro,
                                       lblKpiIngresados), 0, 1);
    filaKpis->addWidget(crearTarjetaKpi("Camas disponibles", "0", vistaDashboardCentro,
                                       lblKpiCamas), 0, 2);
    filaKpis->addWidget(crearTarjetaKpi("Nivel urgente predominante", "-", vistaDashboardCentro,
                                       lblKpiUrgencia), 0, 3);
    layout->addLayout(filaKpis);

    auto* tarjetaTabla = crearTarjeta(vistaDashboardCentro);
    auto* layTabla = new QVBoxLayout(tarjetaTabla);
    layTabla->setContentsMargins(20, 18, 20, 18);
    layTabla->setSpacing(12);

    auto* cabeceraTabla = new QHBoxLayout();
    cabeceraTabla->setSpacing(12);
    auto* tituloTabla = new QLabel("Pacientes del Centro", tarjetaTabla);
    tituloTabla->setObjectName("tituloTarjeta");
    lblEstadoDashboard = new QLabel("Sin pacientes cargados", tarjetaTabla);
    lblEstadoDashboard->setObjectName("subtituloCentro");

    comboFiltroPacientes = new QComboBox(tarjetaTabla);
    comboFiltroPacientes->setObjectName("comboFiltro");
    comboFiltroPacientes->addItem("Todos los pacientes");
    comboFiltroPacientes->addItem("En espera");
    comboFiltroPacientes->addItem("Ingresados");

    cabeceraTabla->addWidget(tituloTabla);
    cabeceraTabla->addWidget(lblEstadoDashboard);
    cabeceraTabla->addStretch(1);
    cabeceraTabla->addWidget(comboFiltroPacientes);
    layTabla->addLayout(cabeceraTabla);

    tablaPendientes = new QTableWidget(tarjetaTabla);
    tablaPendientes->setObjectName("tablaPacientes");
    tablaPendientes->setColumnCount(9);
    tablaPendientes->setHorizontalHeaderLabels({
        "ID", "Cédula", "Paciente", "Edad", "Nivel", "Puntaje",
        "Síntomas", "Estado", "Acciones"
    });
    tablaPendientes->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablaPendientes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablaPendientes->setAlternatingRowColors(true);
    tablaPendientes->setShowGrid(false);
    tablaPendientes->verticalHeader()->setVisible(false);
    tablaPendientes->verticalHeader()->setDefaultSectionSize(46);
    tablaPendientes->horizontalHeader()->setHighlightSections(false);
    tablaPendientes->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    tablaPendientes->setColumnWidth(0, 64);
    tablaPendientes->setColumnWidth(1, 110);
    tablaPendientes->setColumnWidth(2, 180);
    tablaPendientes->setColumnWidth(3, 60);
    tablaPendientes->setColumnWidth(4, 76);
    tablaPendientes->setColumnWidth(5, 76);
    tablaPendientes->setColumnWidth(7, 110);
    tablaPendientes->setColumnWidth(columnaAcciones, 210);
    QPalette paletaTabla = tablaPendientes->palette();
    paletaTabla.setColor(QPalette::AlternateBase, QColor("#F8FAFC"));
    tablaPendientes->setPalette(paletaTabla);
    layTabla->addWidget(tablaPendientes, 1);

    layout->addWidget(tarjetaTabla, 1);

    scrollDashboard = new QScrollArea(this);
    scrollDashboard->setObjectName("scrollDashboard");
    scrollDashboard->setStyleSheet(
        "QScrollArea#scrollDashboard { background: #F4F6F9; border: none; }");
    scrollDashboard->setFrameShape(QFrame::NoFrame);
    scrollDashboard->setWidgetResizable(true);
    scrollDashboard->setWidget(vistaDashboardCentro);

    connect(btnActualizarPendientes, &QPushButton::clicked, this, &MainWindow::refrescarPendientes);
    connect(btnVerTodosKpis, &QPushButton::clicked, this, &MainWindow::mostrarDialogoKpis);
    connect(btnCerrarSesion, &QPushButton::clicked, this, [this]() {
        pilaVistas->setCurrentWidget(vistaMain);
    });
    connect(comboFiltroPacientes, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::aplicarFiltroTabla);

    pilaVistas->addWidget(scrollDashboard);
}

void MainWindow::onLoginOk(const SesionCentro& sesion) {
    sesionActual = sesion;
    centerIdActual = sesion.centerId;
    capacidadDisponibleActual = sesion.capacity;
    pendientesActuales.clear();
    admitidosActuales.clear();
    reconstruirTablaPacientes();
    actualizarKpis();
    actualizarBarraOcupacion();

    lblTituloDashboard->setText(sesion.centerName);
    badgeCategoria->setText(QString("Categoría: %1").arg(sesion.centerType));
    badgeNivelMax->setText(QString("Nivel Máx MPPS: Nivel %1").arg(sesion.maxLevel));
    badgeCamas->setText(QString("Camas disponibles: %1").arg(capacidadDisponibleActual));
    lblKpiCamas->setText(QString::number(capacidadDisponibleActual));

    pilaVistas->setCurrentWidget(scrollDashboard);
    refrescarPendientes();
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
}

void MainWindow::onPendientesCargados(int centerId, const QVector<PacienteTemp>& pacientes) {
    if (centerId != centerIdActual) {
        return;
    }

    pendientesActuales = pacientes;
    pendientesRecibidos = true;
    reconstruirTablaPacientes();
    actualizarKpis();
    actualizarBarraOcupacion();
    if (admitidosRecibidos) {
        operacionDashboardEnCurso = false;
        setDashboardLoading(false);
    }
}

void MainWindow::onAdmitidosCargados(int centerId, const QVector<PacienteIngresado>& pacientes) {
    if (centerId != centerIdActual) {
        return;
    }

    admitidosActuales = pacientes;
    admitidosRecibidos = true;
    reconstruirTablaPacientes();
    actualizarKpis();
    actualizarBarraOcupacion();
    if (pendientesRecibidos) {
        operacionDashboardEnCurso = false;
        setDashboardLoading(false);
    }
}

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

    if (operation == "trends") {
        return;
    }

    if (operation == "admitted") {
        admitidosRecibidos = true;
        lblEstadoDashboard->setText("No se pudo cargar la lista de pacientes ingresados.");
        if (pendientesRecibidos) {
            operacionDashboardEnCurso = false;
            setDashboardLoading(false);
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
    capacidadDisponibleActual = capacidad;
    badgeCamas->setText(QString("Camas disponibles: %1").arg(capacidad));
    lblKpiCamas->setText(QString::number(capacidad));
    actualizarBarraOcupacion();
}

void MainWindow::refrescarPendientes() {
    if (centerIdActual <= 0) {
        return;
    }
    pendientesRecibidos = false;
    admitidosRecibidos = false;
    operacionDashboardEnCurso = true;
    setDashboardLoading(true);
    api->fetchPendingPatients(centerIdActual);
    api->fetchAdmittedPatients(centerIdActual);
}

void MainWindow::setDashboardLoading(bool cargando) {
    btnActualizarPendientes->setEnabled(!cargando);
    btnActualizarPendientes->setText(cargando ? "Actualizando..." : "Actualizar lista");
    if (tablaPendientes == nullptr) {
        return;
    }
    for (int fila = 0; fila < tablaPendientes->rowCount(); ++fila) {
        if (auto* contenedor = tablaPendientes->cellWidget(fila, columnaAcciones)) {
            contenedor->setEnabled(!cargando);
        }
    }
    if (cargando) {
        lblEstadoDashboard->setText("Cargando pacientes...");
    }
}

void MainWindow::reconstruirTablaPacientes() {
    tablaPendientes->setRowCount(0);
    filasPacientes.clear();

    auto agregarFila = [this](int id, const QString& cedula, const QString& nombre, int edad,
                              int nivel, int puntaje, const QString& sintomas,
                              bool ingresado) {
        const int fila = tablaPendientes->rowCount();
        tablaPendientes->insertRow(fila);

        const QString celdas[] = {
            QString::number(id),
            cedula.isEmpty() ? QString("Sin cédula") : cedula,
            nombre,
            QString::number(edad),
            QString(),
            QString::number(puntaje),
            resumirSintomas(sintomas)
        };
        for (int columna = 0; columna < 7; ++columna) {
            auto* item = new QTableWidgetItem(celdas[columna]);
            if (columna == 0 || columna == 3 || columna == 5) {
                item->setTextAlignment(Qt::AlignCenter);
            }
            if (columna == 6) {
                item->setToolTip(sintomas);
            }
            tablaPendientes->setItem(fila, columna, item);
        }

        auto* badgeNivel = crearBadge(tablaPendientes, nivelRomano(nivel),
                                      QString("badgeNivel%1").arg(qBound(1, nivel, 5)));
        badgeNivel->setMinimumHeight(24);
        tablaPendientes->setCellWidget(fila, 4, badgeNivel);

        auto* badgeEstado = crearBadge(tablaPendientes,
                                       ingresado ? "Ingresado" : "En espera",
                                       ingresado ? "badgeIngresado" : "badgeEspera");
        badgeEstado->setMinimumHeight(24);
        tablaPendientes->setCellWidget(fila, 7, badgeEstado);

        auto* contenedor = new QWidget(tablaPendientes);
        auto* botonesFila = new QHBoxLayout(contenedor);
        botonesFila->setContentsMargins(6, 4, 6, 4);
        botonesFila->setSpacing(6);
        botonesFila->setAlignment(Qt::AlignCenter);

        auto* boton = new QPushButton(ingresado ? "Dar de alta" : "Ingresar", contenedor);
        boton->setObjectName(ingresado ? "btnDarAlta" : "btnIngresar");
        boton->setCursor(Qt::PointingHandCursor);
        boton->setMinimumHeight(30);
        if (ingresado) {
            connect(boton, &QPushButton::clicked, this, [this, fila]() { darAltaFila(fila); });
        } else {
            connect(boton, &QPushButton::clicked, this, [this, fila]() { ingresarFila(fila); });
        }
        botonesFila->addWidget(boton);
        tablaPendientes->setCellWidget(fila, columnaAcciones, contenedor);

        filasPacientes.append({ingresado, id});
    };

    for (const PacienteTemp& paciente : pendientesActuales) {
        agregarFila(paciente.id, paciente.cedula,
                    QString("%1 %2").arg(paciente.firstName, paciente.lastName).trimmed(),
                    paciente.age, paciente.triageLevel, paciente.triageScore, paciente.symptoms,
                    false);
    }
    for (const PacienteIngresado& paciente : admitidosActuales) {
        agregarFila(paciente.patientId, paciente.cedula,
                    QString("%1 %2").arg(paciente.firstName, paciente.lastName).trimmed(),
                    paciente.age, paciente.triageLevel, paciente.triageScore, paciente.symptoms,
                    true);
    }

    aplicarFiltroTabla();
}

void MainWindow::aplicarFiltroTabla() {
    const int filtro = comboFiltroPacientes != nullptr ? comboFiltroPacientes->currentIndex() : 0;
    for (int fila = 0; fila < filasPacientes.size(); ++fila) {
        const bool ingresado = filasPacientes.at(fila).ingresado;
        const bool visible = filtro == 0 ||
                             (filtro == 1 && !ingresado) ||
                             (filtro == 2 && ingresado);
        tablaPendientes->setRowHidden(fila, !visible);
    }
    lblEstadoDashboard->setText(
        QString("%1 en espera · %2 ingresado(s)")
            .arg(pendientesActuales.size())
            .arg(admitidosActuales.size()));
}

void MainWindow::actualizarKpis() {
    lblKpiPendientes->setText(QString::number(pendientesActuales.size()));
    lblKpiIngresados->setText(QString::number(admitidosActuales.size()));
    lblKpiCamas->setText(QString::number(capacidadDisponibleActual));

    int conteos[5] = {0, 0, 0, 0, 0};
    for (const PacienteTemp& paciente : pendientesActuales) {
        if (paciente.triageLevel >= 1 && paciente.triageLevel <= 5) {
            ++conteos[paciente.triageLevel - 1];
        }
    }
    for (const PacienteIngresado& paciente : admitidosActuales) {
        if (paciente.triageLevel >= 1 && paciente.triageLevel <= 5) {
            ++conteos[paciente.triageLevel - 1];
        }
    }

    int nivelPredominante = -1;
    for (int i = 0; i < 5; ++i) {
        if (conteos[i] > 0 && (nivelPredominante < 0 || conteos[i] > conteos[nivelPredominante])) {
            nivelPredominante = i;
        }
    }
    lblKpiUrgencia->setText(nivelPredominante < 0 ? "-" : nivelRomano(nivelPredominante + 1));
}

void MainWindow::actualizarBarraOcupacion() {
    const int ocupado = admitidosActuales.size();
    const int enCamino = pendientesActuales.size();
    const int capacidadTotal = capacidadDisponibleActual + ocupado;
    barraOcupacion->setDatos(ocupado, enCamino, capacidadTotal);
}

void MainWindow::mostrarDialogoKpis() {
    QDialog dialog(this);
    dialog.setWindowTitle("Panel de KPIs");
    dialog.resize(920, 640);

    auto* layout = new QVBoxLayout(&dialog);
    auto* pestañas = new QTabWidget(&dialog);
    layout->addWidget(pestañas);

    auto* resumen = new QWidget(&dialog);
    auto* layoutResumen = new QVBoxLayout(resumen);
    auto* textoResumen = new QLabel(
        QString("Centro: %1\n\nPacientes en espera: %2\nPacientes ingresados: %3\n"
                "Camas disponibles: %4\nNivel urgente predominante: %5")
            .arg(lblTituloDashboard->text())
            .arg(pendientesActuales.size())
            .arg(admitidosActuales.size())
            .arg(capacidadDisponibleActual)
            .arg(lblKpiUrgencia->text()), resumen);
    layoutResumen->addWidget(textoResumen);
    pestañas->addTab(resumen, "Resumen");

    auto* panelCriticos = new PanelKpi3Widget(&dialog);
    panelCriticos->setPendientes(pendientesActuales);
    pestañas->addTab(panelCriticos, "Nivel I/II en espera");

    auto* panelTendencias = new PanelKpi7Widget(centerIdActual, &dialog);
    connect(panelTendencias, &PanelKpi7Widget::solicitarTendencias, &dialog,
            [this](int days) { api->fetchTrends(centerIdActual, days); });
    connect(api, &ApiClient::trendsLoaded, &dialog,
            [panelTendencias](int centerId, const QVector<DatoTendencia>& tendencias) {
                if (centerId == panelTendencias->centerId()) {
                    panelTendencias->setTendencias(tendencias);
                }
            });
    pestañas->addTab(panelTendencias, "Tendencias (histórico)");
    api->fetchTrends(centerIdActual, 7);

    auto* botonCerrar = new QPushButton("Cerrar", &dialog);
    connect(botonCerrar, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(botonCerrar, 0, Qt::AlignRight);

    dialog.exec();
}

void MainWindow::ingresarFila(int fila) {
    if (fila < 0 || fila >= filasPacientes.size() || filasPacientes.at(fila).ingresado) {
        return;
    }

    QString cedula;
    if (auto* item = tablaPendientes->item(fila, 1)) {
        cedula = item->text();
    }
    if (cedula == "Sin cédula") {
        cedula.clear();
    }

    operacionDashboardEnCurso = true;
    setDashboardLoading(true);
    api->admitPatient(centerIdActual, filasPacientes.at(fila).id, cedula);
}

void MainWindow::darAltaFila(int fila) {
    if (fila < 0 || fila >= filasPacientes.size() || !filasPacientes.at(fila).ingresado) {
        return;
    }

    const int patientId = filasPacientes.at(fila).id;
    QString nombre;
    if (auto* item = tablaPendientes->item(fila, 2)) {
        nombre = item->text();
    }
    if (nombre.isEmpty()) {
        nombre = "Paciente sin nombre";
    }

    const auto respuesta = QMessageBox::question(
        this, "Dar de alta",
        QString("¿Dar de alta a %1 (ID %2)?").arg(nombre).arg(patientId),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (respuesta != QMessageBox::Yes) {
        return;
    }

    operacionDashboardEnCurso = true;
    setDashboardLoading(true);
    api->dischargePatientById(centerIdActual, patientId);
}

void MainWindow::enviarPaciente(const QJsonObject& datos) {
    api->createTempPatient(datos);
}
