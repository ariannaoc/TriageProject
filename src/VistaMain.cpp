#include "VistaMain.hpp"
#include "ui_VistaMain.h"
#include <QIcon>

VistaMain::VistaMain(QWidget* parent) : QWidget(parent), ui(new Ui::VistaMain) {
    ui->setupUi(this);
    
    setAttribute(Qt::WA_StyledBackground, true);
    ui->BotonEmergencia->setIcon(QIcon(":/resources/Emergencia.png"));
    ui->BotonEmergencia->setIconSize(QSize(28, 28));
    ui->BotonCentroSalud->setIcon(QIcon(":/resources/Estetoscopio.png"));
    ui->BotonCentroSalud->setIconSize(QSize(28, 28));

    connect(ui->BotonEmergencia, &QPushButton::clicked, this, &VistaMain::pacienteClicked);
    connect(ui->BotonCentroSalud, &QPushButton::clicked, this, &VistaMain::centroClicked);
}

VistaMain::~VistaMain() { delete ui; }