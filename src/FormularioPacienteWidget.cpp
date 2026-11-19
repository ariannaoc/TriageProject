#include "FormularioPacienteWidget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QLabel>
#include <QMessageBox>

FormularioPacienteWidget::FormularioPacienteWidget(QWidget* parent) : QWidget(parent) {
    configurarUI();
    aplicarEstilos();
}

void FormularioPacienteWidget::configurarUI() {
    auto* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    auto* widgetContenido = new QWidget();
    auto* layoutContenido = new QVBoxLayout(widgetContenido);
    layoutContenido->setContentsMargins(40, 20, 40, 20);
    layoutContenido->setSpacing(15);

    auto* lblTitulo = new QLabel("Registro de Emergencia y Triaje", this);
    lblTitulo->setAlignment(Qt::AlignCenter);
    lblTitulo->setStyleSheet("font-size: 24px; font-weight: bold; color: #ffffff;");
    layoutContenido->addWidget(lblTitulo);

    // --- ¿Quién tiene la emergencia? ---
    auto* grupoAfectado = new QGroupBox("¿Quién ha tenido la emergencia?", this);
    auto* layoutAfectado = new QVBoxLayout(grupoAfectado);
    comboAfectado = new QComboBox(this);
    comboAfectado->addItems({"Seleccionar...", "Yo", "Un familiar", "Un desconocido"});
    layoutAfectado->addWidget(comboAfectado);
    layoutContenido->addWidget(grupoAfectado);

    // --- Datos Personales ---
    auto* grupoPersonales = new QGroupBox("Datos Personales", this);
    auto* formPersonales = new QFormLayout(grupoPersonales);
    
    comboSexo = new QComboBox(this);
    comboSexo->addItems({"Seleccionar...", "Femenino", "Masculino"});
    spinEdad = new QSpinBox(this);
    spinEdad->setRange(0, 120);
    
    txtNombre = new QLineEdit(this);
    txtApellido = new QLineEdit(this);
    txtUbicacion = new QLineEdit(this);
    txtUbicacion->setPlaceholderText("Obteniendo GPS... (Ej: Centro, Mérida)");

    formPersonales->addRow("Sexo (F/M):", comboSexo);
    formPersonales->addRow("Edad:", spinEdad);
    formPersonales->addRow("Nombre:", txtNombre);
    formPersonales->addRow("Apellido:", txtApellido);
    formPersonales->addRow("Ubicación:", txtUbicacion);
    layoutContenido->addWidget(grupoPersonales);

    // Conexiones lógicas para Embarazo
    connect(comboSexo, &QComboBox::currentTextChanged, this, &FormularioPacienteWidget::verificarEmbarazo);
    connect(spinEdad, &QSpinBox::valueChanged, this, &FormularioPacienteWidget::verificarEmbarazo);

    // --- Factores de Riesgo ---
    auto* grupoRiesgos = new QGroupBox("Condiciones Previas (Factores de Riesgo)", this);
    auto* layoutRiesgos = new QVBoxLayout(grupoRiesgos);

    chkCardio = new QCheckBox("Cardiovascular Severa (+10 pts): Insuficiencia, infarto, arritmias", this);
    chkInmuno = new QCheckBox("Inmunosupresión / Cáncer / Anticoagulación (+10 pts)", this);
    chkRenal = new QCheckBox("Enfermedad Renal Crónica (+10 pts): Diálisis o falla renal", this);
    chkEmbarazo = new QCheckBox("Embarazo o Postparto (+10 pts)", this);
    chkEmbarazo->setEnabled(false); // Inicia deshabilitado
    chkRespiratorio = new QCheckBox("Respiratorio Crónico (+5 pts): Asma severa, EPOC", this);
    chkMetabolico = new QCheckBox("Metabólico / Vascular (+5 pts): Diabetes, Hipertensión", this);
    chkNeuro = new QCheckBox("Compromiso Neurológico Previo (+5 pts): ACV, Epilepsia", this);

    layoutRiesgos->addWidget(chkCardio);
    layoutRiesgos->addWidget(chkInmuno);
    layoutRiesgos->addWidget(chkRenal);
    layoutRiesgos->addWidget(chkEmbarazo);
    layoutRiesgos->addWidget(chkRespiratorio);
    layoutRiesgos->addWidget(chkMetabolico);
    layoutRiesgos->addWidget(chkNeuro);
    layoutContenido->addWidget(grupoRiesgos);

    // --- Síntomas de la Emergencia ---
    auto* grupoSintomas = new QGroupBox("Síntomas de la emergencia", this);
    auto* layoutSintomas = new QVBoxLayout(grupoSintomas);
    comboSintoma = new QComboBox(this);
    comboSintoma->addItems({
        "Seleccione una categoría principal...",
        "Opción A: Traumatismos, golpes, caídas o heridas.",
        "Opción B: Problemas respiratorios o dolor de pecho.",
        "Opción C: Alergias, brotes en la piel o intoxicaciones.",
        "Opción D: Fiebre, dolor abdominal o malestar digestivo.",
        "Opción E: Alteraciones neurológicas o pérdida del conocimiento."
    });
    layoutSintomas->addWidget(comboSintoma);
    layoutContenido->addWidget(grupoSintomas);

    // --- Botones de Acción ---
    auto* layoutBotones = new QHBoxLayout();
    btnVolver = new QPushButton("⬅ Volver a Inicio", this);
    btnVolver->setMinimumHeight(45);
    
    btnEvaluar = new QPushButton("Datos recolectados exitosamente", this);
    btnEvaluar->setMinimumHeight(45);

    layoutBotones->addWidget(btnVolver);
    layoutBotones->addWidget(btnEvaluar);
    layoutContenido->addLayout(layoutBotones);
    
    layoutContenido->addStretch();
    scrollArea->setWidget(widgetContenido);
    layoutPrincipal->addWidget(scrollArea);

    connect(btnVolver, &QPushButton::clicked, this, &FormularioPacienteWidget::volverAlMain);
    connect(btnEvaluar, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "Formulario Completado", 
            "Datos recolectados exitosamente.");
        emit iniciarEvaluacion();
    });
}

// Lógica condicional del requerimiento: Sexo == Femenino && 10 < edad < 55
void FormularioPacienteWidget::verificarEmbarazo() {
    bool esFemenino = (comboSexo->currentText() == "Femenino");
    int edad = spinEdad->value();
    
    if (esFemenino && edad > 10 && edad < 55) {
        chkEmbarazo->setEnabled(true);
    } else {
        chkEmbarazo->setEnabled(false);
        chkEmbarazo->setChecked(false);
    }
}

void FormularioPacienteWidget::aplicarEstilos() {
    // Estilos para encajar perfectamente con la pantalla oscura de tu proyecto
    this->setStyleSheet(
        "QWidget { background-color: #2b2b2b; color: #ffffff; font-size: 14px; }"
        "QGroupBox { font-weight: bold; border: 1px solid #555555; border-radius: 6px; margin-top: 12px; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 5px; color: #90caf9; }"
        "QLineEdit, QSpinBox, QComboBox { background-color: #3c3c3c; border: 1px solid #555; border-radius: 4px; padding: 6px; color: white; }"
        "QCheckBox { padding: 4px; }"
        "QCheckBox::indicator { width: 18px; height: 18px; }"
        "QCheckBox:disabled { color: #777777; }"
    );

    btnEvaluar->setStyleSheet("QPushButton { background-color: #d32f2f; color: white; font-weight: bold; border-radius: 6px; }"
                              "QPushButton:hover { background-color: #b71c1c; }");
    btnVolver->setStyleSheet("QPushButton { background-color: #555555; color: white; font-weight: bold; border-radius: 6px; }"
                             "QPushButton:hover { background-color: #444444; }");
}