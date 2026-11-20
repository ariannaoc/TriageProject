#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QJsonArray>
#include <QJsonObject>
#include "ModelosApi.hpp"
#include <QResizeEvent>

class FormularioPacienteWidget : public QWidget {
    Q_OBJECT

public:
    explicit FormularioPacienteWidget(QWidget* parent = nullptr);
    void setCentros(const QVector<CentroInfo>& centros);
    void setEnviando(bool enviando);

signals:
    void volverAlMain();
    void pacienteListo(const QJsonObject& datos);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QComboBox* comboCentro;
    QComboBox* comboAfectado;
    QComboBox* comboSexo;
    QSpinBox* spinEdad;
    QLineEdit* txtNombre;
    QLineEdit* txtApellido;
    QLineEdit* txtUbicacion;
    
    QLabel* lblFondo = nullptr;

    QCheckBox* chkCardio;
    QCheckBox* chkInmuno;
    QCheckBox* chkRenal;
    QCheckBox* chkEmbarazo;
    QCheckBox* chkRespiratorio;
    QCheckBox* chkMetabolico;
    QCheckBox* chkNeuro;

    QComboBox* comboSintoma;
    QLabel* lblEstadoFormulario;

    QPushButton* btnEvaluar;
    QPushButton* btnVolver;

    QJsonArray sintomasCatalogo;
    QJsonArray triageNiveles;
    QJsonObject factoresRiesgo;
    bool enviando;

    void configurarUI();
    void aplicarEstilos();
    void cargarCatalogos();
    int puntosFactor(const QString& id) const;
    int nivelParaPuntaje(int puntaje) const;

private slots:
    void verificarEmbarazo();
    void enviarFormulario();


};
