#include "LoginWidget.hpp"
#include "ApiClient.hpp"
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QCryptographicHash>

LoginWidget::LoginWidget(ApiClient* api, AVLTree<CredencialCentro>* repo, QWidget* parent)
    : QWidget(parent), api(api), repositorioCentros(repo), esperandoLogin(false) {
    configurarUI();

    connect(api, &ApiClient::loginOk, this, &LoginWidget::onLoginOk);
    connect(api, &ApiClient::requestStarted, this, [this](const QString& operation) {
        if (operation == "login") {
            esperandoLogin = true;
            setLoginLoading(true);
        }
    });
    connect(api, &ApiClient::failed, this, &LoginWidget::onRequestFailed);
    connect(api, &ApiClient::requestFinished, this, &LoginWidget::onRequestFinished);
}

void LoginWidget::configurarUI() {
    auto* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(80, 40, 80, 40);
    layoutPrincipal->setSpacing(15);

    // Encabezado
    auto* titulo = new QLabel("Módulo Centro de Salud — Autenticación", this);
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setStyleSheet("font-size: 22px; font-weight: bold; color: #64B5F6;");

    auto* subtitulo = new QLabel("Acceso exclusivo para personal autorizado de la Red Asistencial de Mérida", this);
    subtitulo->setAlignment(Qt::AlignCenter);
    subtitulo->setStyleSheet("font-size: 13px; color: #ffffff;");

    pestanasAuth = new QTabWidget(this);

    
    // Pestaña 1: Iniciar sesión (LOGIN)
    
    auto* tabLogin = new QWidget(this);
    auto* layoutLogin = new QVBoxLayout(tabLogin);
    auto* formLogin = new QFormLayout();
    formLogin->setSpacing(12);

    inputLoginUser = new QLineEdit(this);
    inputLoginUser->setPlaceholderText("Ej: Iahula, Sorjuana, El llano");
    inputLoginPass = new QLineEdit(this);
    inputLoginPass->setEchoMode(QLineEdit::Password);
    inputLoginPass->setPlaceholderText("Ingrese su contraseña");

    formLogin->addRow("Usuario del Centro:", inputLoginUser);
    formLogin->addRow("Contraseña:", inputLoginPass);

    btnEntrar = new QPushButton("Iniciar Sesión", this);
    btnEntrar->setMinimumHeight(42);
    btnEntrar->setStyleSheet("background-color: #1976d2; color: white; font-weight: bold; border-radius: 6px;");

    auto* notaPrueba = new QLabel("Usuario API de prueba: <b>iahula_admin</b> | Contraseña: <b>admin123</b>", this);
    notaPrueba->setStyleSheet("color: #ffffff; font-size: 12px;");

    lblEstadoLogin = new QLabel("", this);
    lblEstadoLogin->setAlignment(Qt::AlignCenter);
    lblEstadoLogin->setStyleSheet("color: #90caf9; font-size: 12px;");

    layoutLogin->addSpacing(15);
    layoutLogin->addLayout(formLogin);
    layoutLogin->addSpacing(10);
    layoutLogin->addWidget(btnEntrar);
    layoutLogin->addWidget(lblEstadoLogin);
    layoutLogin->addWidget(notaPrueba);
    layoutLogin->addStretch();


    // PESTAÑA 2: REGISTRO DE CENTRO DE SALUD
   
    auto* tabRegistro = new QWidget(this);
    auto* layoutReg = new QVBoxLayout(tabRegistro);
    auto* formReg = new QFormLayout();
    formReg->setSpacing(10);

    inputRegNombreCentro = new QLineEdit(this);
    inputRegNombreCentro->setPlaceholderText("Ej: Ambulatorio Urbano III Belén");

    comboRegTipoCentro = new QComboBox(this);
    comboRegTipoCentro->addItem("Hospital Tipo IV (Nivel I a V)", 1);
    comboRegTipoCentro->addItem("Hospital Tipo II / III (Nivel II a V)", 2);
    comboRegTipoCentro->addItem("Ambulatorio Urbano Tipo III / Especial (Nivel III a V)", 3);
    comboRegTipoCentro->addItem("Ambulatorio Urbano Tipo I (Nivel IV a V)", 4);

    spinRegCapacidad = new QSpinBox(this);
    spinRegCapacidad->setRange(1, 200);
    spinRegCapacidad->setValue(20);

    inputRegUser = new QLineEdit(this);
    inputRegUser->setPlaceholderText("Nombre de usuario único");
    inputRegPass = new QLineEdit(this);
    inputRegPass->setEchoMode(QLineEdit::Password);
    inputRegPass->setPlaceholderText("Contraseña de acceso");

    formReg->addRow("Nombre del Centro:", inputRegNombreCentro);
    formReg->addRow("Clasificación MPPS:", comboRegTipoCentro);
    formReg->addRow("Capacidad (Camas):", spinRegCapacidad);
    formReg->addRow("Usuario nuevo:", inputRegUser);
    formReg->addRow("Contraseña:", inputRegPass);

    auto* btnRegistrar = new QPushButton("Registrar Centro de Salud", this);
    btnRegistrar->setMinimumHeight(42);
    btnRegistrar->setStyleSheet("background-color: #2e7d32; color: white; font-weight: bold; border-radius: 6px;");

    layoutReg->addLayout(formReg);
    layoutReg->addSpacing(10);
    layoutReg->addWidget(btnRegistrar);
    layoutReg->addStretch();

    pestanasAuth->addTab(tabLogin, QIcon(":/resources/usuario.png"), "Iniciar Sesión");
    pestanasAuth->addTab(tabRegistro, QIcon(":/resources/centro_salud.png"), "Registrar Centro");
    pestanasAuth->setIconSize(QSize(20, 20));

   

    // Botón para volver a la ventana Main
    auto* btnVolver = new QPushButton("⬅ Volver al Menú Principal (Main)", this);
    btnVolver->setMinimumHeight(36);

    layoutPrincipal->addWidget(titulo);
    layoutPrincipal->addWidget(subtitulo);
    layoutPrincipal->addWidget(pestanasAuth);
    layoutPrincipal->addWidget(btnVolver);

    // Conexiones de eventos
    connect(btnEntrar, &QPushButton::clicked, this, &LoginWidget::procesarLogin);
    connect(btnRegistrar, &QPushButton::clicked, this, &LoginWidget::procesarRegistro);
    connect(btnVolver, &QPushButton::clicked, this, &LoginWidget::volverAlMain);

        setObjectName("LoginWidget");
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(
        "QWidget#LoginWidget { background-color: #313131; }"
        "QLabel { color: #ffffff; background: transparent; }"
        "QTabWidget::pane { background-color: #3a3a3a; border: 1px solid #555; }"
        "QTabBar::tab { background: #3a3a3a; color: #ffffff; padding: 8px 16px; }"
        "QTabBar::tab:selected { background: #1976d2; font-weight: bold; }"
        "QLineEdit { background-color: #ffffff; color: #1b263b; border: 1px solid #aaa; border-radius: 4px; padding: 4px; }"
    );
}

void LoginWidget::procesarLogin() {
    QString user = inputLoginUser->text().trimmed();
    QString pass = inputLoginPass->text();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Campos vacíos", "Por favor ingrese usuario y contraseña.");
        return;
    }

    api->login(user, pass);
}

void LoginWidget::setLoginLoading(bool cargando) {
    btnEntrar->setEnabled(!cargando);
    btnEntrar->setText(cargando ? "Iniciando sesión…" : "Iniciar Sesión");
    lblEstadoLogin->setText(cargando ? "Conectando con la API…" : "");
}

void LoginWidget::onLoginOk(const SesionCentro& sesion) {
    esperandoLogin = false;
    inputLoginPass->clear();
    emit loginExitoso(sesion);
}

void LoginWidget::onRequestFailed(const QString& operation, const QString& message) {
    if (operation != "login" || !esperandoLogin) {
        return;
    }
    esperandoLogin = false;
    QMessageBox::critical(this, "Error de autenticación", message);
}

void LoginWidget::onRequestFinished() {
    if (esperandoLogin) {
        return;
    }
    setLoginLoading(false);
}

void LoginWidget::procesarRegistro() {
    QString nombre = inputRegNombreCentro->text().trimmed();
    QString user = inputRegUser->text().trimmed();
    QString pass = inputRegPass->text();
    int nivelMax = comboRegTipoCentro->currentData().toInt();
    QString tipoTexto = comboRegTipoCentro->currentText();
    int capacidad = spinRegCapacidad->value();

    if (nombre.isEmpty() || user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Datos incompletos", "Complete todos los campos para registrar el centro.");
        return;
    }

    CredencialCentro nuevaCredencial{
        user.toStdString(),
        QString(QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Sha256).toHex()).toStdString(),
        nombre.toStdString(),
        tipoTexto.toStdString(),
        nivelMax,
        capacidad
    };

    // Verificar en DeSiGNAR si el usuario ya existe
    if (repositorioCentros->search(nuevaCredencial) != nullptr) {
        QMessageBox::warning(this, "Usuario existente", "Ese nombre de usuario ya está registrado en el sistema.");
        return;
    }

    repositorioCentros->insert(nuevaCredencial);
    QMessageBox::information(this, "Registro Exitoso",
        "El centro '" + nombre + "' se registró solo en el árbol AVL local de DeSiGNAR.\n"
        "No crea un usuario en la API; use las credenciales del seed para iniciar sesión.");

    inputRegNombreCentro->clear();
    inputRegUser->clear();
    inputRegPass->clear();
    pestanasAuth->setCurrentIndex(0); // Cambiar a la pestaña de Login
}