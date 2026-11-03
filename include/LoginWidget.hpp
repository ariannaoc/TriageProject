#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTabWidget>
#include "ModelosCentro.hpp"

class LoginWidget : public QWidget {
    Q_OBJECT

private:
    AVLTree<CredencialCentro>* repositorioCentros;

    // Controles de Iniciar Sesión
    QLineEdit* inputLoginUser;
    QLineEdit* inputLoginPass;

    // Controles de Registro de Centro
    QLineEdit* inputRegNombreCentro;
    QComboBox* comboRegTipoCentro;
    QSpinBox* spinRegCapacidad;
    QLineEdit* inputRegUser;
    QLineEdit* inputRegPass;

    QTabWidget* pestanasAuth;

    void configurarUI();
    void procesarLogin();
    void procesarRegistro();

public:
    explicit LoginWidget(AVLTree<CredencialCentro>* repo, QWidget* parent = nullptr);

signals:
    void loginExitoso(const CredencialCentro& centroAutenticado);
    void volverAlMain();
};