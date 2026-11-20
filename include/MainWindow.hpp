#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include "ModelosCentro.hpp"
#include "LoginWidget.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    QStackedWidget* pilaVistas;

    // Vistas del sistema
    QWidget* vistaMain;
    QWidget* vistaModuloPaciente;
    LoginWidget* vistaLoginCentro;
    QWidget* vistaDashboardCentro;

    // Etiquetas dinámicas del Dashboard para mostrar el centro autenticado
    QLabel* lblTituloDashboard;
    QLabel* lblDetalleCentroDashboard;

    // Árbol AVL de DeSiGNAR con las credenciales de los centros de salud de Mérida
    AVLTree<CredencialCentro> arbolCentros;

    void precargarCentrosMerida();
    void configurarVistaMain();
    void configurarVistaPacientePlaceholder();
    void configurarVistaDashboardPlaceholder();

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;
};