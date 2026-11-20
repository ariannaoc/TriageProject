#pragma once
#include <QWidget>
#include <QString>
#include "ModelosCentro.hpp"
#include "ModelosApi.hpp"

class ApiClient;
class QLabel;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QPushButton;
class QTabWidget;

class LoginWidget : public QWidget {
    Q_OBJECT

private:
    ApiClient* api;
    AVLTree<CredencialCentro>* repositorioCentros;
    bool esperandoLogin;

    // Controles de Iniciar Sesión
    QLineEdit* inputLoginUser;
    QLineEdit* inputLoginPass;
    QPushButton* btnEntrar;
    QLabel* lblEstadoLogin;

    // Controles de Registro de Centro
    QLineEdit* inputRegNombreCentro;
    QComboBox* comboRegTipoCentro;
    QSpinBox* spinRegCapacidad;
    QLineEdit* inputRegUser;
    QLineEdit* inputRegPass;

    QTabWidget* pestanasAuth;

    void configurarUI();
    void setLoginLoading(bool cargando);

private slots:
    void procesarLogin();
    void procesarRegistro();
    void onLoginOk(const SesionCentro& sesion);
    void onRequestFailed(const QString& operation, const QString& message);
    void onRequestFinished();

public:
    explicit LoginWidget(ApiClient* api, AVLTree<CredencialCentro>* repo, QWidget* parent = nullptr);

signals:
    void loginExitoso(const SesionCentro& centroAutenticado);
    void volverAlMain();
};
