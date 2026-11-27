#include "FormularioPacienteWidget.hpp"

#include <QDateTime>
#include <QFile>
#include <QFormLayout>
#include <QGraphicsOpacityEffect>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMessageBox>
#include <QPixmap>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStringList>
#include <QVBoxLayout>

#include <algorithm>

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
    : QWidget(parent) {
    cargarCatalogos();
    bosqueTriaje_.construirDesdeCategorias(catalogoCategorias);
    motorTriaje_ = std::make_unique<MotorTriaje>(bosqueTriaje_, factoresTriaje);
    configurarUI();
    aplicarEstilos();
    configurarGps();
}

void FormularioPacienteWidget::cargarCatalogos() {
    const QJsonObject sintomasDoc = leerCatalogo(":/resources/data/symptoms.json");
    const QJsonObject riesgosDoc = leerCatalogo(":/resources/data/risk_factors.json");
    triageNiveles = leerCatalogo(":/resources/data/triage_levels.json").value("triage_levels").toArray();

    std::map<int, std::vector<OpcionSintoma>> porCategoria;
    for (const QJsonValue& entrada : sintomasDoc.value("symptoms").toArray()) {
        const QJsonObject sintoma = entrada.toObject();
        OpcionSintoma opcion;
        opcion.id = sintoma.value("id").toInt();
        opcion.etiqueta = sintoma.value("label").toString().toStdString();
        opcion.base_points = sintoma.value("base_points").toInt();
        int nivel = sintoma.value("estimated_level").toInt();
        if (nivel < 1 || nivel > 5) {
            nivel = 5;
        }
        opcion.nivel_estimado = static_cast<NivelTriaje>(nivel);
        porCategoria[sintoma.value("category_id").toInt()].push_back(opcion);
    }

    catalogoCategorias.clear();
    for (const QJsonValue& entrada : sintomasDoc.value("categories").toArray()) {
        const QJsonObject categoria = entrada.toObject();
        const int id = categoria.value("id").toInt();
        catalogoCategorias.push_back({id,
                                      categoria.value("name").toString().toStdString(),
                                      porCategoria[id]});
    }

    factoresTriaje.clear();
    for (const QJsonValue& entrada : riesgosDoc.value("age_risk_rules").toArray()) {
        const QJsonObject regla = entrada.toObject();
        factoresTriaje.push_back({regla.value("id").toString().toStdString(),
                                  regla.value("label").toString().toStdString(),
                                  regla.value("points").toInt(),
                                  true,
                                  regla.value("condition").toString().toStdString()});
    }
    for (const QJsonValue& entrada : riesgosDoc.value("risk_factors").toArray()) {
        const QJsonObject factor = entrada.toObject();
        factoresTriaje.push_back({factor.value("id").toString().toStdString(),
                                  factor.value("name").toString().toStdString(),
                                  factor.value("points").toInt(),
                                  false,
                                  ""});
    }
}

void FormularioPacienteWidget::configurarUI() {
    setObjectName("FormularioPacienteWidget");

    lblFondo = new QLabel(this);
    lblFondo->setPixmap(QPixmap(":/resources/fondo.png"));
    lblFondo->setScaledContents(true);
    auto* efectoOpacidad = new QGraphicsOpacityEffect(this);
    efectoOpacidad->setOpacity(0.30);
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
    comboSexo = new QComboBox(this);
    comboSexo->addItems({"Seleccionar...", "Femenino", "Masculino"});
    spinEdad = new QSpinBox(this);
    spinEdad->setRange(0, 120);
    txtNombre = new QLineEdit(this);
    txtApellido = new QLineEdit(this);
    formPersonales->addRow("Sexo:", comboSexo);
    formPersonales->addRow("Edad:", spinEdad);
    formPersonales->addRow("Nombre:", txtNombre);
    formPersonales->addRow("Apellido:", txtApellido);
    layoutContenido->addWidget(grupoPersonales);

    auto* grupoUbicacion = new QGroupBox("Ubicación de la emergencia", this);
    auto* formUbicacion = new QFormLayout(grupoUbicacion);
    txtUbicacion = new QLineEdit(this);
    txtUbicacion->setPlaceholderText("Ej: Sector Belén, Mérida");
    btnUbicacion = new QPushButton("Usar mi ubicación GPS", this);
    btnUbicacion->setMinimumHeight(36);
    auto* layoutReferencia = new QHBoxLayout();
    layoutReferencia->setSpacing(8);
    layoutReferencia->addWidget(txtUbicacion, 1);
    layoutReferencia->addWidget(btnUbicacion);
    formUbicacion->addRow("Referencia / dirección:", layoutReferencia);
    lblUbicacionEstado = new QLabel("Ingresa una referencia o usa el GPS; si no hay datos se usará Mérida centro.", this);
    lblUbicacionEstado->setWordWrap(true);
    formUbicacion->addRow(lblUbicacionEstado);
    layoutContenido->addWidget(grupoUbicacion);

    auto* grupoRiesgos = new QGroupBox("Paso 1 - Factores de Riesgo", this);
    auto* layoutRiesgos = new QVBoxLayout(grupoRiesgos);
    layoutRiesgos->setSpacing(6);
    for (const auto& factor : factoresTriaje) {
        if (factor.esReglaEdad) {
            auto* lblRegla = new QLabel(
                QString::fromStdString(factor.nombre) +
                    QString(" (+%1 pts, se aplica automáticamente)").arg(factor.puntos),
                this);
            lblRegla->setWordWrap(true);
            layoutRiesgos->addWidget(lblRegla);
        }
    }
    const QJsonObject riesgosDoc = leerCatalogo(":/resources/data/risk_factors.json");
    for (const QJsonValue& entrada : riesgosDoc.value("risk_factors").toArray()) {
        const QJsonObject factor = entrada.toObject();
        const QString id = factor.value("id").toString();
        QString etiqueta = QString("%1 (+%2 pts)")
                               .arg(factor.value("name").toString())
                               .arg(factor.value("points").toInt());
        const QString descripcion = factor.value("description").toString();
        if (!descripcion.isEmpty()) {
            etiqueta += ": " + descripcion;
        }
        auto* chk = new QCheckBox(etiqueta, this);
        const QJsonObject restricciones = factor.value("constraints").toObject();
        chk->setProperty("sexoRequerido", restricciones.value("sex").toString());
        chk->setProperty("edadMin", restricciones.value("min_age").toInt());
        chk->setProperty("edadMax", restricciones.value("max_age").toInt());
        chk->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        checkFactores[id] = chk;
        layoutRiesgos->addWidget(chk);
    }
    layoutContenido->addWidget(grupoRiesgos);

    auto* grupoClinico = new QGroupBox("Paso 2 - Área y Síntomas", this);
    auto* layoutClinico = new QVBoxLayout(grupoClinico);
    layoutClinico->setSpacing(8);
    layoutClinico->addWidget(new QLabel("Área principal de la emergencia:", this));
    comboCategoria = new QComboBox(this);
    comboCategoria->addItem("Seleccione un área o categoría...", -1);
    for (int id : bosqueTriaje_.categorias()) {
        comboCategoria->addItem(QString::fromStdString(bosqueTriaje_.nombreCategoria(id)), id);
    }
    layoutClinico->addWidget(comboCategoria);

    lblGuiaSintomas = new QLabel("Seleccione un área o categoría para ver los síntomas relacionados.", this);
    lblGuiaSintomas->setWordWrap(true);
    lblGuiaSintomas->setStyleSheet("color: #4A5568; font-style: italic; background: transparent;");
    layoutClinico->addWidget(lblGuiaSintomas);

    widgetSintomas = new QWidget(this);
    widgetSintomas->setObjectName("contenedorSintomas");
    layoutSintomas = new QVBoxLayout(widgetSintomas);
    layoutSintomas->setContentsMargins(10, 10, 10, 10);
    layoutSintomas->setSpacing(6);
    layoutClinico->addWidget(widgetSintomas);
    widgetSintomas->hide();
    layoutContenido->addWidget(grupoClinico);

    // TODO(DEBUG): Eliminar sección temporal de selección manual de centro
    auto* grupoDebug = new QGroupBox("Modo Depuración / Prueba (Temporal - Eliminar al integrar BD)", this);
    grupoDebug->setObjectName("debugGroup");
    auto* layoutDebug = new QVBoxLayout(grupoDebug);
    layoutDebug->setSpacing(8);
    chkForzarCentroDebug = new QCheckBox(
        "Forzar centro de destino manualmente (omitir asignación automática)", this);
    comboCentroDebug = new QComboBox(this);
    comboCentroDebug->addItem("Sin centros disponibles", -1);
    comboCentroDebug->setEnabled(false);
    layoutDebug->addWidget(chkForzarCentroDebug);
    layoutDebug->addWidget(comboCentroDebug);
    layoutContenido->addWidget(grupoDebug);
    connect(chkForzarCentroDebug, &QCheckBox::toggled,
            this, [this](bool marcado) { comboCentroDebug->setEnabled(marcado); });
    // TODO(DEBUG): Eliminar sección temporal de selección manual de centro

    lblEstadoFormulario = new QLabel("", this);
    lblEstadoFormulario->setAlignment(Qt::AlignCenter);
    lblEstadoFormulario->setWordWrap(true);
    lblEstadoFormulario->setStyleSheet("color: #0E9F91; font-weight: bold; font-size: 13px; background: transparent;");
    layoutContenido->addWidget(lblEstadoFormulario);

    auto* layoutBotones = new QHBoxLayout();
    layoutBotones->setSpacing(10);
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

    connect(comboSexo, &QComboBox::currentTextChanged, this, &FormularioPacienteWidget::verificarEmbarazo);
    connect(spinEdad, &QSpinBox::valueChanged, this, &FormularioPacienteWidget::verificarEmbarazo);
    connect(comboCategoria, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FormularioPacienteWidget::onCategoriaCambiada);
    connect(btnUbicacion, &QPushButton::clicked, this, &FormularioPacienteWidget::detectarUbicacion);
    connect(btnVolver, &QPushButton::clicked, this, &FormularioPacienteWidget::volverAlMain);
    connect(btnEvaluar, &QPushButton::clicked, this, &FormularioPacienteWidget::enviarFormulario);
}

void FormularioPacienteWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (lblFondo) {
        lblFondo->setGeometry(rect());
        lblFondo->lower();
    }
}

void FormularioPacienteWidget::configurarGps() {
    fuenteGps = QGeoPositionInfoSource::createDefaultSource(this);
    if (fuenteGps == nullptr) {
        lblUbicacionEstado->setText("GPS no disponible: ingresa una referencia o se usará Mérida centro.");
        return;
    }

    connect(fuenteGps, &QGeoPositionInfoSource::positionUpdated,
            this, &FormularioPacienteWidget::onPosicionActualizada);
    connect(fuenteGps, &QGeoPositionInfoSource::errorOccurred, this,
            [this](QGeoPositionInfoSource::Error) {
                if (!ubicacionResuelta) {
                    lblUbicacionEstado->setText("GPS sin señal: ingresa una referencia o se usará Mérida centro.");
                }
            });
    fuenteGps->setUpdateInterval(10000);
    fuenteGps->startUpdates();
}

void FormularioPacienteWidget::setApiClient(ApiClient* client) {
    if (api == client) {
        return;
    }
    api = client;
    if (api) {
        connect(api, &ApiClient::geocodificado, this, &FormularioPacienteWidget::onGeocodificado);
        connect(api, &ApiClient::failed, this, &FormularioPacienteWidget::onErrorGeocodificacion);
    }
}

void FormularioPacienteWidget::setCentros(const QVector<CentroInfo>& centros) {
    std::vector<CentroSalud> cargados;
    cargados.reserve(static_cast<std::size_t>(centros.size()));
    for (const CentroInfo& info : centros) {
        CentroSalud centro;
        centro.id = info.centerId;
        centro.nombre = info.name.toStdString();
        centro.nivel_complejidad = info.nivelComplejidad;
        centro.capacidad_total = info.capacidadTotal;
        centro.capacidad_disponible = info.capacidadDisponible;
        centro.cola_actual = info.colaActual;
        centro.latitud = info.latitude;
        centro.longitud = info.longitude;
        cargados.push_back(centro);
    }
    gestorCentros_.cargarCentros(cargados);

    // TODO(DEBUG): Eliminar sección temporal de selección manual de centro
    const int seleccionAnterior = comboCentroDebug->currentData().toInt();
    comboCentroDebug->clear();
    if (centros.isEmpty()) {
        comboCentroDebug->addItem("Sin centros disponibles", -1);
    }
    for (const CentroInfo& info : centros) {
        comboCentroDebug->addItem(
            QString("%1 (ID %2, camas: %3)")
                .arg(info.name)
                .arg(info.centerId)
                .arg(info.capacidadDisponible),
            info.centerId);
    }
    const int indiceAnterior = comboCentroDebug->findData(seleccionAnterior);
    if (indiceAnterior >= 0) {
        comboCentroDebug->setCurrentIndex(indiceAnterior);
    }
}

void FormularioPacienteWidget::setEnviando(bool cargando) {
    enviando = cargando;
    btnEvaluar->setEnabled(!cargando);
    btnEvaluar->setText(cargando ? "Enviando…" : "Enviar a la cola del centro");
    lblEstadoFormulario->setText(cargando ? "Enviando datos a la API…" : ultimoEstado);
}

void FormularioPacienteWidget::verificarEmbarazo() {
    const int indiceSexo = comboSexo->currentIndex();
    const QString sexo = indiceSexo == 1 ? "F" : (indiceSexo == 2 ? "M" : "");
    const int edad = spinEdad->value();

    for (auto it = checkFactores.cbegin(); it != checkFactores.cend(); ++it) {
        QCheckBox* chk = it->second;
        bool okMin = false;
        bool okMax = false;
        const int min = chk->property("edadMin").toInt(&okMin);
        const int max = chk->property("edadMax").toInt(&okMax);
        const QString sexoRequerido = chk->property("sexoRequerido").toString();

        bool habilitado = true;
        if (!sexoRequerido.isEmpty() && sexo != sexoRequerido) {
            habilitado = false;
        }
        if (habilitado && okMin && min > 0 && edad < min) {
            habilitado = false;
        }
        if (habilitado && okMax && max > 0 && edad > max) {
            habilitado = false;
        }

        chk->setEnabled(habilitado);
        if (!habilitado) {
            chk->setChecked(false);
        }
    }
}

void FormularioPacienteWidget::reconstruirSintomas() {
    QLayoutItem* item = nullptr;
    while ((item = layoutSintomas->takeAt(0)) != nullptr) {
        if (item->widget() != nullptr) {
            delete item->widget();
        }
        delete item;
    }
    checkSintomas.clear();
}

void FormularioPacienteWidget::onCategoriaCambiada(int indice) {
    if (categoriaActual >= 0) {
        seleccionPorCategoria[categoriaActual].clear();
    }

    const int id = comboCategoria->itemData(indice).toInt();
    categoriaActual = (indice > 0 && id > 0) ? id : -1;
    reconstruirSintomas();
    opcionesCategoriaActual.clear();

    if (categoriaActual < 0) {
        lblGuiaSintomas->show();
        widgetSintomas->hide();
        return;
    }

    opcionesCategoriaActual = bosqueTriaje_.opcionesCategoria(categoriaActual);
    lblGuiaSintomas->hide();
    widgetSintomas->show();

    for (const OpcionSintoma& opcion : opcionesCategoriaActual) {
        auto* chk = new QCheckBox(
            QString::fromStdString(opcion.etiqueta) +
                QString(" (+%1 pts)").arg(opcion.base_points),
            widgetSintomas);
        chk->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        layoutSintomas->addWidget(chk);
        checkSintomas[opcion.id] = chk;
        const int idOpcion = opcion.id;
        connect(chk, &QCheckBox::toggled, this, [this, idOpcion](bool marcado) {
            auto& ids = seleccionPorCategoria[categoriaActual];
            if (marcado) {
                if (std::find(ids.begin(), ids.end(), idOpcion) == ids.end()) {
                    ids.push_back(idOpcion);
                }
            } else {
                ids.erase(std::remove(ids.begin(), ids.end(), idOpcion), ids.end());
            }
        });
    }
}

void FormularioPacienteWidget::detectarUbicacion() {
    if (fuenteGps == nullptr) {
        lblUbicacionEstado->setText("GPS no disponible: ingresa una referencia o se usará Mérida centro.");
        return;
    }
    lblUbicacionEstado->setText("Buscando señal GPS…");
    fuenteGps->requestUpdate(8000);
}

void FormularioPacienteWidget::onPosicionActualizada(const QGeoPositionInfo& info) {
    const QGeoCoordinate coord = info.coordinate();
    if (!coord.isValid()) {
        return;
    }
    latPaciente = coord.latitude();
    lngPaciente = coord.longitude();
    ubicacionResuelta = true;
    lblUbicacionEstado->setText(QString("Ubicación GPS obtenida: %1, %2")
                                    .arg(latPaciente, 0, 'f', 6)
                                    .arg(lngPaciente, 0, 'f', 6));
}

void FormularioPacienteWidget::onGeocodificado(double latitud, double longitud) {
    esperandoGeocodificacion = false;
    latPaciente = latitud;
    lngPaciente = longitud;
    ubicacionResuelta = true;
    lblUbicacionEstado->setText(QString("Ubicación resuelta: %1, %2")
                                    .arg(latPaciente, 0, 'f', 6)
                                    .arg(lngPaciente, 0, 'f', 6));
    enviarFormulario();
}

void FormularioPacienteWidget::onErrorGeocodificacion(const QString& operacion, const QString& mensaje) {
    if (operacion != "geocode" || !esperandoGeocodificacion) {
        return;
    }
    (void)mensaje;
    esperandoGeocodificacion = false;
    ubicacionResuelta = true;
    lblUbicacionEstado->setText("No se pudo geocodificar la referencia; se usa Mérida centro.");
    enviarFormulario();
}

bool FormularioPacienteWidget::resolveUbicacion() {
    if (ubicacionResuelta) {
        return true;
    }

    const QString referencia = txtUbicacion->text().trimmed();
    if (!referencia.isEmpty() && api != nullptr && !esperandoGeocodificacion) {
        esperandoGeocodificacion = true;
        lblUbicacionEstado->setText("Resolviendo la dirección ingresada…");
        api->geocodeAddress(referencia);
        return false;
    }

    latPaciente = 8.5839;
    lngPaciente = -71.1578;
    ubicacionResuelta = true;
    lblUbicacionEstado->setText("Ubicación no resuelta: se usa Mérida centro como referencia.");
    return true;
}

bool FormularioPacienteWidget::allowAmbulancia(NivelTriaje nivel) const {
    const bool porDefecto = nivel <= NivelTriaje::II;
    for (const QJsonValue& entrada : triageNiveles) {
        const QJsonObject objeto = entrada.toObject();
        if (objeto.value("level").toInt() == static_cast<int>(nivel)) {
            const QJsonValue valor = objeto.value("allow_ambulance");
            return valor.isBool() ? valor.toBool() : porDefecto;
        }
    }
    return porDefecto;
}

void FormularioPacienteWidget::enviarFormulario() {
    if (enviando) {
        return;
    }

    const QString nombre = txtNombre->text().trimmed();
    const QString apellido = txtApellido->text().trimmed();
    const int edad = spinEdad->value();
    if (nombre.isEmpty() || apellido.isEmpty() || edad <= 0 ||
        comboAfectado->currentIndex() <= 0 || comboSexo->currentIndex() <= 0) {
        QMessageBox::warning(this, "Datos incompletos",
                             "Complete quién tuvo la emergencia, nombre, apellido, edad y sexo.");
        return;
    }

    if (categoriaActual < 0 || checkSintomas.empty()) {
        QMessageBox::warning(this, "Datos incompletos",
                             "Seleccione un área principal y al menos un síntoma de esa rama.");
        return;
    }

    std::vector<int> idsSintomas;
    for (const auto& [id, chk] : checkSintomas) {
        if (chk->isChecked()) {
            idsSintomas.push_back(id);
        }
    }
    seleccionPorCategoria.clear();
    if (!idsSintomas.empty()) {
        seleccionPorCategoria[categoriaActual] = idsSintomas;
    }
    if (!bosqueTriaje_.tieneSintomasSeleccionados(seleccionPorCategoria)) {
        QMessageBox::warning(this, "Sin síntomas",
                             "Marque al menos un síntoma del área seleccionada para evaluar el triaje.");
        return;
    }

    if (!resolveUbicacion()) {
        return;
    }

    std::vector<std::string> idsFactoresActivos;
    for (const auto& [id, chk] : checkFactores) {
        if (chk->isChecked()) {
            idsFactoresActivos.push_back(id.toStdString());
        }
    }

    const char sexo = comboSexo->currentIndex() == 1 ? 'F' : 'M';
    const ResultadoTriaje resTriaje = motorTriaje_->evaluar(
        seleccionPorCategoria, edad, sexo, idsFactoresActivos);
    if (resTriaje.sin_sintomas) {
        QMessageBox::warning(this, "Sin síntomas", "No hay síntomas válidos para evaluar.");
        return;
    }

    const ResultadoEnrutamiento resRuta = motorEnrutamiento_.recomendar(
        resTriaje, latPaciente, lngPaciente, gestorCentros_);

    int centroAsignado = resRuta.centro_recomendado.id;
    QString nombreCentro = QString::fromStdString(resRuta.centro_recomendado.nombre);
    double scoreRuta = resRuta.score;

    // TODO(DEBUG): Eliminar sección temporal de selección manual de centro
    const bool forzarCentroDebug = chkForzarCentroDebug->isChecked();
    if (!forzarCentroDebug && resRuta.sin_centro_disponible) {
        QMessageBox::warning(this, "Sin centro disponible",
            QString("No hay un centro de salud apto para el nivel de triaje %1. "
                    "Coordenadas usadas: lat=%2, lng=%3. "
                    "Revise la ubicación o los datos geográficos de los centros.")
                .arg(QString::fromStdString(to_string(resTriaje.nivel_final)))
                .arg(latPaciente, 0, 'f', 6)
                .arg(lngPaciente, 0, 'f', 6));
        return;
    }
    if (forzarCentroDebug) {
        centroAsignado = comboCentroDebug->currentData().toInt();
        if (centroAsignado <= 0) {
            QMessageBox::warning(this, "Centro de prueba inválido",
                                 "Seleccione un centro válido en el modo depuración.");
            return;
        }
        nombreCentro = comboCentroDebug->currentText();
    }
    // TODO(DEBUG): Eliminar sección temporal de selección manual de centro

    const int nivel = static_cast<int>(resTriaje.nivel_final);
    const int puntaje = resTriaje.puntuacion_categoria_ganadora +
                        resTriaje.puntuacion_factores_riesgo;

    QJsonArray sintomasJson;
    QStringList etiquetasSintomas;
    for (int id : idsSintomas) {
        sintomasJson.append(id);
        auto it = std::find_if(opcionesCategoriaActual.begin(), opcionesCategoriaActual.end(),
                               [id](const OpcionSintoma& opcion) { return opcion.id == id; });
        if (it != opcionesCategoriaActual.end()) {
            etiquetasSintomas.append(QString::fromStdString(it->etiqueta));
        }
    }
    QJsonArray factoresJson;
    for (const std::string& id : idsFactoresActivos) {
        factoresJson.append(QString::fromStdString(id));
    }

    QJsonObject datos;
    datos["center_id"] = centroAsignado;
    datos["center_name"] = nombreCentro;
    datos["route_score"] = scoreRuta;
    datos["first_name"] = nombre;
    datos["last_name"] = apellido;
    datos["age"] = edad;
    datos["sex"] = sexo == 'F' ? "F" : "M";
    datos["triage_level"] = nivel;
    datos["triage_score"] = puntaje;
    datos["winning_category"] = QString::fromStdString(resTriaje.categoria_ganadora);
    datos["symptoms"] = etiquetasSintomas.join(" | ");
    datos["symptom_ids"] = sintomasJson;
    datos["risk_factor_ids"] = factoresJson;
    datos["ambulance_requested"] = allowAmbulancia(resTriaje.nivel_final);
    datos["location"] = txtUbicacion->text().trimmed();
    datos["latitude"] = latPaciente;
    datos["longitude"] = lngPaciente;
    datos["source"] = "SYSTEM";
    datos["created_at"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    ultimoEstado = QString("Triaje %1 (%2 pts) · Centro asignado: %3")
                       .arg(QString::fromStdString(to_string(resTriaje.nivel_final)))
                       .arg(puntaje)
                       .arg(nombreCentro);
    lblEstadoFormulario->setText(ultimoEstado);
    setEnviando(true);
    emit pacienteListo(datos);
}

void FormularioPacienteWidget::aplicarEstilos() {
    setAttribute(Qt::WA_StyledBackground, true);

    setStyleSheet(
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
        "QWidget#contenedorSintomas {"
        "    background-color: rgba(248, 252, 252, 220);"
        "    border: 1px dashed #B2DFDB;"
        "    border-radius: 8px;"
        "}"
        "QGroupBox#debugGroup {"
        "    border: 1px solid #F6AD55;"
        "    background-color: rgba(255, 250, 240, 235);"
        "}"
        "QGroupBox#debugGroup::title { color: #C05621; }"
        "QGroupBox#debugGroup QLabel { color: #744210; }"
        "QGroupBox#debugGroup QCheckBox { color: #744210; }"
    );

    btnEvaluar->setCursor(Qt::PointingHandCursor);
    btnEvaluar->setStyleSheet(
        "QPushButton { background-color: #12C6B5; color: #FFFFFF; font-weight: bold; font-size: 14px; border-radius: 8px; padding: 10px 18px; border: none; }"
        "QPushButton:hover { background-color: #0EA899; }"
        "QPushButton:disabled { background-color: #A0AEC0; }"
    );

    btnVolver->setCursor(Qt::PointingHandCursor);
    btnVolver->setStyleSheet(
        "QPushButton { background-color: #FFFFFF; color: #2D3748; font-weight: bold; font-size: 14px; border: 2px solid #12C6B5; border-radius: 8px; padding: 10px 18px; }"
        "QPushButton:hover { background-color: #E6FAF8; }"
    );

    if (btnUbicacion) {
        btnUbicacion->setCursor(Qt::PointingHandCursor);
        btnUbicacion->setStyleSheet(
            "QPushButton { background-color: #FFFFFF; color: #0E9F91; font-weight: bold; font-size: 13px; border: 1.5px solid #12C6B5; border-radius: 6px; padding: 6px 12px; }"
            "QPushButton:hover { background-color: #E6FAF8; }"
        );
    }
}
