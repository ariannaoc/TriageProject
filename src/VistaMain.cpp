#include "VistaMain.hpp"
#include "ui_VistaMain.h"
#include <QIcon>
#include <QGraphicsOpacityEffect>
#include <QBoxLayout>
#include <QResizeEvent>

VistaMain::VistaMain(QWidget* parent) : QWidget(parent), ui(new Ui::VistaMain) {
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);

    auto* efectoOpacidad = new QGraphicsOpacityEffect(this);
    efectoOpacidad->setOpacity(0.60); // Ajusta entre 0.3 y 0.7 según qué tan tenue la quieras
    ui->label_fondo->setGraphicsEffect(efectoOpacidad);
    ui->label_fondo->lower();

  

    ui->BotonEmergencia->setIcon(QIcon(":/resources/Emergencia.png"));
    ui->BotonEmergencia->setIconSize(QSize(28, 28));
    ui->BotonCentroSalud->setIcon(QIcon(":/resources/Estetoscopio.png"));
    ui->BotonCentroSalud->setIconSize(QSize(28, 28));

    connect(ui->BotonEmergencia, &QPushButton::clicked, this, &VistaMain::pacienteClicked);
    connect(ui->BotonCentroSalud, &QPushButton::clicked, this, &VistaMain::centroClicked);
}


void VistaMain::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

   if (ui->label_fondo) {
        ui->label_fondo->setGeometry(rect());
        ui->label_fondo->lower();
    }

    // 760 px permite activar el modo móvil antes de que los botones bloqueen el ancho mínimo
    bool esMovil = width() < 760;

    // Oculta la foto lateral en celular y la muestra en PC
    ui->label_imagen->setVisible(!esMovil);

    // Apila los botones en vertical en celular y en horizontal en PC
    ui->layoutBotones->setDirection(esMovil ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);

}

VistaMain::~VistaMain() { delete ui; }