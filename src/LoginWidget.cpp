#include "LoginWidget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QCryptographicHash>

LoginWidget::LoginWidget(AVLTree<CredencialCentro>* repo, QWidget* parent)
    : QWidget(parent), repositorioCentros(repo) {
    configurarUI();
}

void LoginWidget::configurarUI() {
    auto* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(80, 40, 80, 40);
    layoutPrincipal->setSpacing(15);

    // Encabezado
    auto* titulo = new QLabel("Módulo Centro de Salud — Autenticación", this);
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setStyleSheet("font-size: 22px; font-weight: bold; color: #1565c0;");

    auto* subtitulo = new QLabel("Acceso exclusivo para personal autorizado de la Red Asistencial de Mérida", this);
    subtitulo->setAlignment(Qt::AlignCenter);
    subtitulo->setStyleSheet("font-size: 13px; color: #555;");

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

    auto* btnEntrar = new QPushButton("Iniciar Sesión", this);
    btnEntrar->setMinimumHeight(42);
    btnEntrar->setStyleSheet("background-color: #1976d2; color: white; font-weight: bold; border-radius: 6px;");

    auto* notaPrueba = new QLabel("💡 Usuario de prueba precargado: <b>iahula</b> | Contraseña: <b>1234</b>", this);
    notaPrueba->setStyleSheet("color: #666; font-size: 12px;");

    layoutLogin->addSpacing(15);
    layoutLogin->addLayout(formLogin);
    layoutLogin->addSpacing(10);
    layoutLogin->addWidget(btnEntrar);
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

    pestanasAuth->addTab(tabLogin, "🔑 Iniciar Sesión");
    pestanasAuth->addTab(tabRegistro, "🏥 Registrar Centro");

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
}

void LoginWidget::procesarLogin() {
    QString user = inputLoginUser->text().trimmed();
    QString pass = inputLoginPass->text();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Campos vacíos", "Por favor ingrese usuario y contraseña.");
        return;
    }

    // Hash SHA-256 de la contraseña ingresada
    QString passHash = QString(QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Sha256).toHex());

    // Búsqueda en O(log N) dentro del AVLTree de DeSiGNAR
    CredencialCentro criterioBusqueda{user.toStdString(), "", "", "", 0, 0};
    auto* nodoEncontrado = repositorioCentros->search(criterioBusqueda);

    if (nodoEncontrado != nullptr && nodoEncontrado->password_hash == passHash.toStdString()) {
    inputLoginPass->clear();
    emit loginExitoso(*nodoEncontrado);
    }

   else {
        QMessageBox::critical(this, "Error de Autenticación", "Usuario o contraseña incorrectos.");
    }
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
        "El centro '" + nombre + "' ha sido registrado en el árbol AVL de DeSiGNAR.\nAhora puede iniciar sesión.");

    inputRegNombreCentro->clear();
    inputRegUser->clear();
    inputRegPass->clear();
    pestanasAuth->setCurrentIndex(0); // Cambiar a la pestaña de Login
}