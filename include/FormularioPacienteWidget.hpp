#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>

class FormularioPacienteWidget : public QWidget {
    Q_OBJECT

public:
    explicit FormularioPacienteWidget(QWidget* parent = nullptr);

signals:
    void volverAlMain();
    void iniciarEvaluacion(); // Se usará cuando el Árbol exista

private:
    QComboBox* comboAfectado;
    QComboBox* comboSexo;
    QSpinBox* spinEdad;
    QLineEdit* txtNombre;
    QLineEdit* txtApellido;
    QLineEdit* txtUbicacion;

    QCheckBox* chkCardio;
    QCheckBox* chkInmuno;
    QCheckBox* chkRenal;
    QCheckBox* chkEmbarazo;
    QCheckBox* chkRespiratorio;
    QCheckBox* chkMetabolico;
    QCheckBox* chkNeuro;

    QComboBox* comboSintoma;

    QPushButton* btnEvaluar;
    QPushButton* btnVolver;

    void configurarUI();
    void aplicarEstilos();

private slots:
    void verificarEmbarazo();
};