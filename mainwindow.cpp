// mainwindow.cpp
#include "mainwindow.h"
#include "loginwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    auto central = new QWidget(this);
    auto layout = new QVBoxLayout(central);

    btnEmergency = new QPushButton("🚨 Emergencia", central);
    btnHealthCenter = new QPushButton("🏥 Centro de Salud", central);

    layout->addWidget(btnEmergency);
    layout->addWidget(btnHealthCenter);
    setCentralWidget(central);

    connect(btnEmergency, &QPushButton::clicked, this, &MainWindow::onEmergencyClicked);
    connect(btnHealthCenter, &QPushButton::clicked, this, &MainWindow::onHealthCenterClicked);
}

void MainWindow::onEmergencyClicked() {
    // Aquí abrirás el módulo de paciente (formulario de triaje)
    QMessageBox::information(this, "Módulo Paciente", "Aquí se abrirá el formulario de emergencia.");
}

void MainWindow::onHealthCenterClicked() {
    LoginWindow login;
    if (login.exec() == QDialog::Accepted) {
        // Login exitoso: abrir dashboard del centro de salud
        QMessageBox::information(this, "Bienvenido", "Acceso concedido al panel del centro de salud.");
    }
}
