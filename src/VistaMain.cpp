#include "VistaMain.hpp"
#include "ui_VistaMain.h"
#include <QIcon>
#include <QGraphicsOpacityEffect>

VistaMain::VistaMain(QWidget* parent) : QWidget(parent), ui(new Ui::VistaMain) {
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);

    auto* efectoOpacidad = new QGraphicsOpacityEffect(this);
    efectoOpacidad->setOpacity(0.60);
    ui->label_fondo->setGraphicsEffect(efectoOpacidad);
    ui->label_fondo->lower();

    ui->BotonEmergencia->setIcon(QIcon(":/resources/Emergencia.png"));
    ui->BotonEmergencia->setIconSize(QSize(28, 28));
    ui->BotonCentroSalud->setIcon(QIcon(":/resources/Estetoscopio.png"));
    ui->BotonCentroSalud->setIconSize(QSize(28, 28));

    connect(ui->BotonEmergencia, &QPushButton::clicked, this, &VistaMain::pacienteClicked);
    connect(ui->BotonCentroSalud, &QPushButton::clicked, this, &VistaMain::centroClicked);
}

VistaMain::~VistaMain() { delete ui; }
