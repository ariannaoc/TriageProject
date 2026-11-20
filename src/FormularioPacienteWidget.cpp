#include "FormularioPacienteWidget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QLabel>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDateTime>
#include <QResizeEvent>
#include <QGraphicsOpacityEffect>
#include <QPixmap>

namespace {
QJsonObject leerCatalogo(const QString& ruta) {
    QFile archivo(ruta);
    if (!archivo.open(QIODevice::ReadOnly)) {
        return {};
    }
    QJsonParseError error;
    const QJsonDocument documento = QJsonDocument::fromJson(archivo.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return {};
    }
    return documento.object();
}
}

FormularioPacienteWidget::FormularioPacienteWidget(QWidget* parent)
    : QWidget(parent), enviando(false) {
    cargarCatalogos();
    configurarUI();
    aplicarEstilos();
}

void FormularioPacienteWidget::cargarCatalogos() {
    sintomasCatalogo = leerCatalogo(":/resources/data/symptoms.json").value("symptoms").toArray();
    triageNiveles = leerCatalogo(":/resources/data/triage_levels.json").value("triage_levels").toArray();
    factoresRiesgo = leerCatalogo(":/resources/data/risk_factors.json");
}

void FormularioPacienteWidget::configurarUI() {
    setObjectName("FormularioPacienteWidget");

    // Imagen de fondo (la misma de VistaMain)
    lblFondo = new QLabel(this);
    lblFondo->setPixmap(QPixmap(":/resources/fondo.png"));
    lblFondo->setScaledContents(true);
    auto* efectoOpacidad = new QGraphicsOpacityEffect(this);
    efectoOpacidad->setOpacity(0.30); // Ajusta entre 0.20 y 0.45 si la quieres más o menos tenue
    lblFondo->setGraphicsEffect(efectoOpacidad);
    lblFondo->lower();

    auto* layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    scrollArea->viewport()->setStyleSheet("background: transparent;");

    auto* widgetContenido = new QWidget();
    widgetContenido->setStyleSheet("background: transparent;");
    auto* layoutContenido = new QVBoxLayout(widgetContenido);
    layoutContenido->setContentsMargins(40, 20, 40, 20);
    layoutContenido->setSpacing(15);

    auto* lblTitulo = new QLabel("Registro de Emergencia y Triaje", this);
    lblTitulo->setAlignment(Qt::AlignCenter);
    lblTitulo->setStyleSheet("font-size: 24px; font-weight: bold; color: #0E9F91; margin-bottom: 6px; background: transparent;");
    layoutContenido->addWidget(lblTitulo);

    auto* grupoAfectado = new QGroupBox("¿Quién ha tenido la emergencia?", this);
    auto* layoutAfectado = new QVBoxLayout(grupoAfectado);
    comboAfectado = new QComboBox(this);
    comboAfectado->addItems({"Seleccionar...", "Yo", "Un familiar", "Un desconocido"});
    layoutAfectado->addWidget(comboAfectado);
    layoutContenido->addWidget(grupoAfectado);

    auto* grupoPersonales = new QGroupBox("Datos Personales", this);
    auto* formPersonales = new QFormLayout(grupoPersonales);

    comboCentro = new QComboBox(this);
    comboCentro->addItem("Cargando centros…", -1);

    comboSexo = new QComboBox(this);
    comboSexo->addItems({"Seleccionar...", "Femenino", "Masculino"});
    spinEdad = new QSpinBox(this);
    spinEdad->setRange(0, 120);

    txtNombre = new QLineEdit(this);
    txtApellido = new QLineEdit(this);
    txtUbicacion = new QLineEdit(this);
    txtUbicacion->setPlaceholderText("Ej: Centro, Mérida");

    formPersonales->addRow("Centro de destino:", comboCentro);
    formPersonales->addRow("Sexo (F/M):", comboSexo);
    formPersonales->addRow("Edad:", spinEdad);
    formPersonales->addRow("Nombre:", txtNombre);
    formPersonales->addRow("Apellido:", txtApellido);
    formPersonales->addRow("Ubicación:", txtUbicacion);
    layoutContenido->addWidget(grupoPersonales);

    connect(comboSexo, &QComboBox::currentTextChanged, this, &FormularioPacienteWidget::verificarEmbarazo);
    connect(spinEdad, &QSpinBox::valueChanged, this, &FormularioPacienteWidget::verificarEmbarazo);

    auto* grupoRiesgos = new QGroupBox("Condiciones Previas (Factores de Riesgo)", this);
    auto* layoutRiesgos = new QVBoxLayout(grupoRiesgos);

    chkCardio = new QCheckBox("Cardiovascular Severa (+10 pts): Insuficiencia, infarto, arritmias", this);
    chkInmuno = new QCheckBox("Inmunosupresión / Cáncer / Anticoagulación (+10 pts)", this);
    chkRenal = new QCheckBox("Enfermedad Renal Crónica (+10 pts): Diálisis o falla renal", this);
    chkEmbarazo = new QCheckBox("Embarazo o Postparto (+10 pts)", this);
    chkEmbarazo->setEnabled(false);
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

    auto* grupoSintomas = new QGroupBox("Síntomas de la emergencia", this);
    auto* layoutSintomas = new QVBoxLayout(grupoSintomas);
    comboSintoma = new QComboBox(this);
    comboSintoma->addItem("Seleccione un síntoma...", -1);
    for (const QJsonValue& entrada : sintomasCatalogo) {
        const QJsonObject sintoma = entrada.toObject();
        comboSintoma->addItem(sintoma.value("label").toString(), sintoma.value("id").toInt());
    }
    layoutSintomas->addWidget(comboSintoma);
    layoutContenido->addWidget(grupoSintomas);

    lblEstadoFormulario = new QLabel("", this);
    lblEstadoFormulario->setAlignment(Qt::AlignCenter);
    lblEstadoFormulario->setStyleSheet("color: #0E9F91; font-weight: bold; font-size: 13px; background: transparent;");
    layoutContenido->addWidget(lblEstadoFormulario);

    auto* layoutBotones = new QHBoxLayout();
    btnVolver = new QPushButton("⬅ Volver a Inicio", this);
    btnVolver->setMinimumHeight(45);

    btnEvaluar = new QPushButton("Enviar a la cola del centro", this);
    btnEvaluar->setMinimumHeight(45);

    layoutBotones->addWidget(btnVolver);
    layoutBotones->addWidget(btnEvaluar);
    layoutContenido->addLayout(layoutBotones);

    layoutContenido->addStretch();
    scrollArea->setWidget(widgetContenido);
    layoutPrincipal->addWidget(scrollArea);

    connect(btnVolver, &QPushButton::clicked, this, &FormularioPacienteWidget::volverAlMain);
    connect(btnEvaluar, &QPushButton::clicked, this, &FormularioPacienteWidget::enviarFormulario);
}

void FormularioPacienteWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (lblFondo) {
        lblFondo->setGeometry(this->rect());
        lblFondo->lower();
    }
}

void FormularioPacienteWidget::setCentros(const QVector<CentroInfo>& centros) {
    const int seleccionAnterior = comboCentro->currentData().toInt();
    comboCentro->clear();

    if (centros.isEmpty()) {
        comboCentro->addItem("No hay centros disponibles", -1);
    }
    for (const CentroInfo& centro : centros) {
        comboCentro->addItem(QString("%1 (camas: %2)").arg(centro.name).arg(centro.capacity),
                             centro.centerId);
    }

    const int indice = comboCentro->findData(seleccionAnterior);
    if (indice >= 0) {
        comboCentro->setCurrentIndex(indice);
    }
}

void FormularioPacienteWidget::setEnviando(bool cargando) {
    enviando = cargando;
    btnEvaluar->setEnabled(!cargando);
    btnEvaluar->setText(cargando ? "Enviando…" : "Enviar a la cola del centro");
    lblEstadoFormulario->setText(cargando ? "Enviando datos a la API…" : "");
}

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

int FormularioPacienteWidget::puntosFactor(const QString& id) const {
    for (const QJsonValue& entrada : factoresRiesgo.value("risk_factors").toArray()) {
        const QJsonObject factor = entrada.toObject();
        if (factor.value("id").toString() == id) {
            return factor.value("points").toInt();
        }
    }
    for (const QJsonValue& entrada : factoresRiesgo.value("age_risk_rules").toArray()) {
        const QJsonObject regla = entrada.toObject();
        if (regla.value("id").toString() == id) {
            return regla.value("points").toInt();
        }
    }
    return 0;
}

int FormularioPacienteWidget::nivelParaPuntaje(int puntaje) const {
    for (const QJsonValue& entrada : triageNiveles) {
        const QJsonObject nivel = entrada.toObject();
        if (puntaje >= nivel.value("min_score").toInt() && puntaje <= nivel.value("max_score").toInt()) {
            return nivel.value("level").toInt();
        }
    }
    return 5;
}

void FormularioPacienteWidget::enviarFormulario() {
    if (enviando) {
        return;
    }

    const int centroId = comboCentro->currentData().toInt();
    const int idSintoma = comboSintoma->currentData().toInt();
    const QString nombre = txtNombre->text().trimmed();
    const QString apellido = txtApellido->text().trimmed();
    const int edad = spinEdad->value();

    if (centroId <= 0 || idSintoma <= 0 || nombre.isEmpty() || apellido.isEmpty()
        || edad <= 0 || comboSexo->currentIndex() <= 0) {
        QMessageBox::warning(this, "Datos incompletos",
            "Seleccione centro, sexo y síntoma, e ingrese nombre, apellido y edad.");
        return;
    }

    QJsonObject sintomaSeleccionado;
    for (const QJsonValue& entrada : sintomasCatalogo) {
        const QJsonObject sintoma = entrada.toObject();
        if (sintoma.value("id").toInt() == idSintoma) {
            sintomaSeleccionado = sintoma;
            break;
        }
    }

    int puntaje = sintomaSeleccionado.value("base_points").toInt();
    QJsonArray factores;
    auto agregarFactor = [this, &puntaje, &factores](const QString& id) {
        puntaje += puntosFactor(id);
        factores.append(id);
    };

    if (chkCardio->isChecked()) agregarFactor("RF_CARDIO");
    if (chkInmuno->isChecked()) agregarFactor("RF_INMUNO");
    if (chkRenal->isChecked()) agregarFactor("RF_RENAL");
    if (chkEmbarazo->isChecked()) agregarFactor("RF_PREGNANCY");
    if (chkRespiratorio->isChecked()) agregarFactor("RF_RESP");
    if (chkMetabolico->isChecked()) agregarFactor("RF_METABOLIC");
    if (chkNeuro->isChecked()) agregarFactor("RF_NEURO");
    if (edad < 5) agregarFactor("AGE_INFANT");
    if (edad > 65) agregarFactor("AGE_ELDERLY");

    puntaje = qMin(100, puntaje);
    const int nivel = nivelParaPuntaje(puntaje);

    QJsonArray idsSintomas;
    idsSintomas.append(idSintoma);

    QJsonObject datos;
    datos["center_id"] = centroId;
    datos["first_name"] = nombre;
    datos["last_name"] = apellido;
    datos["age"] = edad;
    datos["sex"] = comboSexo->currentText() == "Femenino" ? "F" : "M";
    datos["triage_level"] = nivel;
    datos["triage_score"] = puntaje;
    datos["symptoms"] = sintomaSeleccionado.value("label").toString();
    datos["symptom_ids"] = idsSintomas;
    datos["risk_factor_ids"] = factores;
    datos["ambulance_requested"] = nivel <= 2;
    datos["location"] = txtUbicacion->text().trimmed();
    datos["source"] = "SYSTEM";
    datos["created_at"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    setEnviando(true);
    emit pacienteListo(datos);
}

void FormularioPacienteWidget::aplicarEstilos() {
    setAttribute(Qt::WA_StyledBackground, true);

    this->setStyleSheet(
        "QWidget#FormularioPacienteWidget { background-color: #F8FCFC; }"
        "QGroupBox {"
        "    background-color: rgba(255, 255, 255, 232);"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    border: 1px solid #E2E8F0;"
        "    border-radius: 10px;"
        "    margin-top: 16px;"
        "    padding: 14px 12px 12px 12px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top left;"
        "    left: 12px;"
        "    padding: 2px 8px;"
        "    color: #0E9F91;"
        "    background-color: rgba(248, 252, 252, 240);"
        "    border-radius: 4px;"
        "}"
        "QLabel { background-color: transparent; color: #2D3748; font-size: 14px; }"
        "QLineEdit, QSpinBox, QComboBox {"
        "    background-color: #FFFFFF;"
        "    border: 1.5px solid #CBD5E0;"
        "    border-radius: 6px;"
        "    padding: 6px 10px;"
        "    color: #1A202C;"
        "    font-size: 14px;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus {"
        "    border: 1.5px solid #12C6B5;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background-color: #FFFFFF;"
        "    color: #1A202C;"
        "    selection-background-color: #E6FAF8;"
        "    selection-color: #0E9F91;"
        "}"
        "QCheckBox { background-color: transparent; color: #2D3748; font-size: 14px; padding: 4px; spacing: 8px; }"
        "QCheckBox::indicator { width: 18px; height: 18px; border-radius: 4px; border: 1.5px solid #CBD5E0; background-color: #FFFFFF; }"
        "QCheckBox::indicator:checked { background-color: #12C6B5; border-color: #12C6B5; }"
        "QCheckBox:disabled { color: #A0AEC0; }"
    );

    btnEvaluar->setCursor(Qt::PointingHandCursor);
    btnEvaluar->setStyleSheet(
        "QPushButton { background-color: #12C6B5; color: #FFFFFF; font-weight: bold; font-size: 14px; border-radius: 8px; padding: 10px 18px; border: none; }"
        "QPushButton:hover { background-color: #0EA899; }"
    );

    btnVolver->setCursor(Qt::PointingHandCursor);
    btnVolver->setStyleSheet(
        "QPushButton { background-color: #FFFFFF; color: #2D3748; font-weight: bold; font-size: 14px; border: 2px solid #12C6B5; border-radius: 8px; padding: 10px 18px; }"
        "QPushButton:hover { background-color: #E6FAF8; }"
    );
}