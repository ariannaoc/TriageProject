// loginwindow.cpp
#include "loginwindow.h"
#include "database.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Acceso Centro de Salud");
    setFixedSize(350, 220);

    txtUsername = new QLineEdit(this);
    txtPassword = new QLineEdit(this);
    txtPassword->setEchoMode(QLineEdit::Password);

    btnLogin = new QPushButton("Iniciar Sesión", this);
    lblError = new QLabel(this);
    lblError->setStyleSheet("color: red;");

    auto form = new QFormLayout;
    form->addRow("Nombre del Centro:", txtUsername);
    form->addRow("Contraseña:", txtPassword);

    auto layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(btnLogin);
    layout->addWidget(lblError);

    connect(btnLogin, &QPushButton::clicked, this, &LoginWindow::attemptLogin);

    // Conexión a BD (usa la ruta de tu archivo .db)
    db = Database::connect("triage.db");
    if (!db) {
        lblError->setText("No se pudo conectar a la base de datos.");
        btnLogin->setEnabled(false);
    }
}

void LoginWindow::attemptLogin() {
    const QString username = txtUsername->text().trimmed();
    const QString password = txtPassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        lblError->setText("Complete todos los campos.");
        return;
    }

    // Autenticación con consulta preparada + bcrypt
    if (Database::authenticateCenter(db, username.toStdString(), password.toStdString())) {
        accept();  // Cierra el diálogo con resultado Accepted
    } else {
        lblError->setText("Credenciales incorrectas.");
        txtPassword->clear();
    }
}
