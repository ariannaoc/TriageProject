#include "MainWindow.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QCryptographicHash>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Sistema de Triaje Automático y Derivación Distribuida - Mérida");
    resize(850, 580);

    precargarCentrosMerida();

    pilaVistas = new QStackedWidget(this);
    setCentralWidget(pilaVistas);

    // 1. Configurar Pantalla Principal (Main)
    configurarVistaMain();

    // 2. Configurar Vista 1 de Paciente (Placeholder para el equipo)
    configurarVistaPacientePlaceholder();

    // 3. Configurar Vista 1 de Centro de Salud (Login / Registro)
    vistaLoginCentro = new LoginWidget(&arbolCentros, this);
    pilaVistas->addWidget(vistaLoginCentro);

    // 4. Configurar Vista 2 de Centro de Salud (Dashboard tras Login exitoso)
    configurarVistaDashboardPlaceholder();

    // Conectar señales del LoginWidget
    connect(vistaLoginCentro, &LoginWidget::volverAlMain, this, [this]() {
        pilaVistas->setCurrentWidget(vistaMain);
    });

    connect(vistaLoginCentro, &LoginWidget::loginExitoso, this, [this](const CredencialCentro& centro) {
        lblTituloDashboard->setText(QString("🏥 Dashboard: %1").arg(QString::fromStdString(centro.nombre_centro)));
        lblDetalleCentroDashboard->setText(
            QString("Categoría: %1 | Nivel Máx MPPS: Nivel %2 | Camas disponibles: %3")
                .arg(QString::fromStdString(centro.tipo_centro))
                .arg(centro.nivel_max_mpps)
                .arg(centro.capacidad_camas)
        );
        pilaVistas->setCurrentWidget(vistaDashboardCentro);
    });

    pilaVistas->setCurrentWidget(vistaMain);
}

void MainWindow::precargarCentrosMerida() {
    QString hashDefault = QString(QCryptographicHash::hash("1234", QCryptographicHash::Sha256).toHex());

    // Centros de referencia de Mérida según la propuesta del proyecto
    arbolCentros.insert({"iahula", hashDefault.toStdString(),
                         "IAHULA - Hospital Universitario de Los Andes", "Hospital Tipo IV", 1, 150});
    arbolCentros.insert({"sorjuana", hashDefault.toStdString(),
                         "Hospital Sor Juana Inés de la Cruz", "Hospital Tipo III", 2, 60});
    arbolCentros.insert({"elllano", hashDefault.toStdString(),
                         "Ambulatorio Urbano Tipo III El Llano", "Ambulatorio Urbano III", 3, 25});
}

void MainWindow::configurarVistaMain() {
    vistaMain = new QWidget(this);
    auto* layout = new QVBoxLayout(vistaMain);
    layout->setContentsMargins(70, 50, 70, 50);
    layout->setSpacing(20);

    auto* lblTitulo = new QLabel("Sistema de Triaje Automático\ny Derivación Distribuida", this);
    lblTitulo->setAlignment(Qt::AlignCenter);
    lblTitulo->setStyleSheet("font-size: 26px; font-weight: bold; color: #1b263b;");

    auto* lblSub = new QLabel("Red Asistencial de la Ciudad de Mérida (Normativa MPPS)\nSeleccione su rol para continuar:", this);
    lblSub->setAlignment(Qt::AlignCenter);
    lblSub->setStyleSheet("font-size: 14px; color: #555;");

    // Botón Rol 1: Reportar una emergencia (Paciente) - Público por defecto
    auto* btnPaciente = new QPushButton("🚨  Reportar una emergencia (Paciente)", this);
    btnPaciente->setMinimumHeight(70);
    btnPaciente->setCursor(Qt::PointingHandCursor);
    btnPaciente->setStyleSheet(
        "QPushButton { font-size: 18px; font-weight: bold; background-color: #d32f2f; color: white; border-radius: 10px; }"
        "QPushButton:hover { background-color: #b71c1c; }"
    );

    // Botón Rol 2: Centro de Salud - Privado (Requiere Login)
    auto* btnCentro = new QPushButton("🏥  Centro de Salud (Acceso Privado / Login)", this);
    btnCentro->setMinimumHeight(60);
    btnCentro->setCursor(Qt::PointingHandCursor);
    btnCentro->setStyleSheet(
        "QPushButton { font-size: 16px; font-weight: bold; background-color: #1976d2; color: white; border-radius: 10px; }"
        "QPushButton:hover { background-color: #0d47a1; }"
    );

    layout->addStretch();
    layout->addWidget(lblTitulo);
    layout->addWidget(lblSub);
    layout->addSpacing(15);
    layout->addWidget(btnPaciente);
    layout->addWidget(btnCentro);
    layout->addStretch();

    connect(btnPaciente, &QPushButton::clicked, this, [this]() {
        pilaVistas->setCurrentWidget(vistaModuloPaciente);
    });

    connect(btnCentro, &QPushButton::clicked, this, [this]() {
        pilaVistas->setCurrentWidget(vistaLoginCentro);
    });

    pilaVistas->addWidget(vistaMain);
}

void MainWindow::configurarVistaPacientePlaceholder() {
    vistaModuloPaciente = new QWidget(this);
    auto* layout = new QVBoxLayout(vistaModuloPaciente);
    layout->setContentsMargins(50, 40, 50, 40);

    auto* lbl = new QLabel("Módulo Paciente (Público)\nVista 1: Formulario Dinámico - Árbol de Decisiones", this);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet("font-size: 20px; font-weight: bold;");

    auto* btnVolver = new QPushButton("⬅ Volver a Main", this);
    btnVolver->setMinimumHeight(40);
    connect(btnVolver, &QPushButton::clicked, this, [this]() {
        pilaVistas->setCurrentWidget(vistaMain);
    });

    layout->addStretch();
    layout->addWidget(lbl);
    layout->addSpacing(20);
    layout->addWidget(btnVolver);
    layout->addStretch();

    pilaVistas->addWidget(vistaModuloPaciente);
}

void MainWindow::configurarVistaDashboardPlaceholder() {
    vistaDashboardCentro = new QWidget(this);
    auto* layout = new QVBoxLayout(vistaDashboardCentro);
    layout->setContentsMargins(50, 40, 50, 40);

    lblTituloDashboard = new QLabel("Dashboard del Centro de Salud", this);
    lblTituloDashboard->setAlignment(Qt::AlignCenter);
    lblTituloDashboard->setStyleSheet("font-size: 22px; font-weight: bold; color: #1b5e20;");

    lblDetalleCentroDashboard = new QLabel("", this);
    lblDetalleCentroDashboard->setAlignment(Qt::AlignCenter);
    lblDetalleCentroDashboard->setStyleSheet("font-size: 14px; color: #333;");

    auto* infoCola = new QLabel("Vista 2: Dashboard de Control y Gestión de Cola de Pacientes en Camino\n(Sesión iniciada correctamente)", this);
    infoCola->setAlignment(Qt::AlignCenter);

    auto* btnCerrarSesion = new QPushButton("🔒 Cerrar Sesión y Volver a Main", this);
    btnCerrarSesion->setMinimumHeight(40);
    connect(btnCerrarSesion, &QPushButton::clicked, this, [this]() {
        pilaVistas->setCurrentWidget(vistaMain);
    });

    layout->addStretch();
    layout->addWidget(lblTituloDashboard);
    layout->addWidget(lblDetalleCentroDashboard);
    layout->addSpacing(20);
    layout->addWidget(infoCola);
    layout->addSpacing(30);
    layout->addWidget(btnCerrarSesion);
    layout->addStretch();

    pilaVistas->addWidget(vistaDashboardCentro);
}