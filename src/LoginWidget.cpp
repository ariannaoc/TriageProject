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
#include <QGraphicsOpacityEffect>
#include <QPixmap>
#include <QResizeEvent>

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
    setObjectName("LoginWidget");
    setAttribute(Qt::WA_StyledBackground, true);

    // Imagen de fondo (la misma de VistaMain)
    lblFondo = new QLabel(this);
    lblFondo->setPixmap(QPixmap(":/resources/fondo.png"));
    lblFondo->setScaledContents(true);
    auto* efectoOpacidad = new QGraphicsOpacityEffect(this);
    efectoOpacidad->setOpacity(0.30); // Ajusta entre 0.20 y 0.45 según prefieras
    lblFondo->setGraphicsEffect(efectoOpacidad);
    lblFondo->lower();

    auto* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(80, 40, 80, 40);
    layoutPrincipal->setSpacing(15);

    // Encabezado
    auto* titulo = new QLabel("Módulo Centro de Salud — Autenticación", this);
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setStyleSheet("font-size: 24px; font-weight: bold; color: #0E9F91; background: transparent;");

    auto* subtitulo = new QLabel("Acceso exclusivo para personal autorizado de la Red Asistencial de Mérida", this);
    subtitulo->setAlignment(Qt::AlignCenter);
    subtitulo->setStyleSheet("font-size: 13px; color: #4A5568; background: transparent; margin-bottom: 6px;");

    pestanasAuth = new QTabWidget(this);

    // ==========================================
    // Pestaña 1: Iniciar sesión (LOGIN)
    // ==========================================
    auto* tabLogin = new QWidget(this);
    tabLogin->setStyleSheet("background: transparent;");
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
    btnEntrar->setCursor(Qt::PointingHandCursor);
    btnEntrar->setStyleSheet(
        "QPushButton { background-color: #12C6B5; color: #FFFFFF; font-weight: bold; font-size: 14px; border-radius: 8px; border: none; }"
        "QPushButton:hover { background-color: #0EA899; }"
    );

    auto* notaPrueba = new QLabel("Usuario API de prueba: <b>iahula_admin</b> | Contraseña: <b>admin123</b>", this);
    notaPrueba->setStyleSheet("color: #4A5568; font-size: 12px; background: transparent;");

    lblEstadoLogin = new QLabel("", this);
    lblEstadoLogin->setAlignment(Qt::AlignCenter);
    lblEstadoLogin->setStyleSheet("color: #0E9F91; font-weight: bold; font-size: 12px; background: transparent;");

    layoutLogin->addSpacing(15);
    layoutLogin->addLayout(formLogin);
    layoutLogin->addSpacing(10);
    layoutLogin->addWidget(btnEntrar);
    layoutLogin->addWidget(lblEstadoLogin);
    layoutLogin->addWidget(notaPrueba);
    layoutLogin->addStretch();

    // ==========================================
    // PESTAÑA 2: REGISTRO DE CENTRO DE SALUD
    // ==========================================
    auto* tabRegistro = new QWidget(this);
    tabRegistro->setStyleSheet("background: transparent;");
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
    btnRegistrar->setCursor(Qt::PointingHandCursor);
    btnRegistrar->setStyleSheet(
        "QPushButton { background-color: #0E9F91; color: #FFFFFF; font-weight: bold; font-size: 14px; border-radius: 8px; border: none; }"
        "QPushButton:hover { background-color: #0B7F74; }"
    );

    layoutReg->addLayout(formReg);
    layoutReg->addSpacing(10);
    layoutReg->addWidget(btnRegistrar);
    layoutReg->addStretch();

    pestanasAuth->addTab(tabLogin, QIcon(":/resources/usuario.png"), "Iniciar Sesión");
    pestanasAuth->addTab(tabRegistro, QIcon(":/resources/centro_salud.png"), "Registrar Centro");
    pestanasAuth->setIconSize(QSize(20, 20));

    // Botón para volver a la ventana Main
    auto* btnVolver = new QPushButton("⬅ Volver al Menú Principal (Main)", this);
    btnVolver->setMinimumHeight(40);
    btnVolver->setCursor(Qt::PointingHandCursor);
    btnVolver->setStyleSheet(
        "QPushButton { background-color: #FFFFFF; color: #2D3748; font-weight: bold; font-size: 14px; border: 2px solid #12C6B5; border-radius: 8px; }"
        "QPushButton:hover { background-color: #E6FAF8; }"
    );

    layoutPrincipal->addWidget(titulo);
    layoutPrincipal->addWidget(subtitulo);
    layoutPrincipal->addWidget(pestanasAuth);
    layoutPrincipal->addWidget(btnVolver);

    // Conexiones de eventos
    connect(btnEntrar, &QPushButton::clicked, this, &LoginWidget::procesarLogin);
    connect(btnRegistrar, &QPushButton::clicked, this, &LoginWidget::procesarRegistro);
    connect(btnVolver, &QPushButton::clicked, this, &LoginWidget::volverAlMain);

    setStyleSheet(
        "QWidget#LoginWidget { background-color: #F8FCFC; }"
        "QLabel { color: #2D3748; background: transparent; font-size: 14px; }"
        "QTabWidget::pane {"
        "    background-color: rgba(255, 255, 255, 232);"
        "    border: 1px solid #E2E8F0;"
        "    border-radius: 10px;"
        "    padding: 14px;"
        "}"
        "QTabBar::tab {"
        "    background-color: #E8F9F7;"
        "    color: #4A5568;"
        "    padding: 10px 22px;"
        "    border-top-left-radius: 8px;"
        "    border-top-right-radius: 8px;"
        "    margin-right: 4px;"
        "    font-weight: bold;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: #12C6B5;"
        "    color: #FFFFFF;"
        "}"
        "QLineEdit, QComboBox, QSpinBox {"
        "    background-color: #FFFFFF;"
        "    color: #1A202C;"
        "    border: 1.5px solid #CBD5E0;"
        "    border-radius: 6px;"
        "    padding: 6px 10px;"
        "    font-size: 14px;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus {"
        "    border: 1.5px solid #12C6B5;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background-color: #FFFFFF;"
        "    color: #1A202C;"
        "    selection-background-color: #E6FAF8;"
        "    selection-color: #0E9F91;"
        "}"
    );
}

void LoginWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (lblFondo) {
        lblFondo->setGeometry(this->rect());
        lblFondo->lower();
    }
    bool esMovil = width() < 600;

for (auto* f : findChildren<QFormLayout*>()) {
    f->setRowWrapPolicy(esMovil ? QFormLayout::WrapAllRows : QFormLayout::DontWrapRows);
}
for (auto* c : findChildren<QComboBox*>()) {
    c->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    c->setMinimumWidth(0);
}
if (auto* l = layout()) {
    l->setContentsMargins(esMovil ? 12 : 40, 16, esMovil ? 12 : 40, 16);
}
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