#include "PanelKpi3Widget.hpp"
#include "GraficoBarrasNivelesWidget.hpp"

#include <QLabel>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QFrame>

PanelKpi3Widget::PanelKpi3Widget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* tarjeta = new QFrame(this);
    tarjeta->setFrameShape(QFrame::StyledPanel);
    tarjeta->setStyleSheet("QFrame { background:#f5f5f5; border-radius:8px; }");

    auto* layTarjeta = new QVBoxLayout(tarjeta);
    layTarjeta->setContentsMargins(16, 12, 16, 12);
    layTarjeta->setSpacing(4);

    m_lblGrande = new QLabel("0 pacientes Nivel I/II en espera", tarjeta);
    QFont fGrande = m_lblGrande->font();
    fGrande.setPointSize(20); fGrande.setBold(true);
    m_lblGrande->setFont(fGrande);
    m_lblGrande->setAlignment(Qt::AlignCenter);

    m_lblDetalle = new QLabel("", tarjeta);
    m_lblDetalle->setAlignment(Qt::AlignCenter);
    m_lblDetalle->setStyleSheet("color:#616161;");

    layTarjeta->addWidget(m_lblGrande);
    layTarjeta->addWidget(m_lblDetalle);
    layout->addWidget(tarjeta);

    auto* lblGrafico = new QLabel("Distribución por nivel de triaje", this);
    QFont fSub = lblGrafico->font();
    fSub.setBold(true);
    lblGrafico->setFont(fSub);
    layout->addWidget(lblGrafico);

    m_grafico = new GraficoBarrasNivelesWidget(this);
    layout->addWidget(m_grafico);

    auto* lblTabla = new QLabel("Detalle de pacientes Nivel I/II", this);
    lblTabla->setFont(fSub);
    layout->addWidget(lblTabla);

    m_tabla = new QTableWidget(this);
    m_tabla->setColumnCount(4);
    m_tabla->setHorizontalHeaderLabels({"Cédula", "Nombre", "Nivel", "Síntomas"});
    m_tabla->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tabla->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tabla->setAlternatingRowColors(true);
    layout->addWidget(m_tabla, /*stretch=*/1);

    m_lblSinDatos = new QLabel("Sin pacientes pendientes en este momento.", this);
    m_lblSinDatos->setAlignment(Qt::AlignCenter);
    m_lblSinDatos->setStyleSheet("color:#9e9e9e; padding:12px;");
    m_lblSinDatos->setVisible(false);
    layout->addWidget(m_lblSinDatos);

    reconstruir();
}

void PanelKpi3Widget::setPendientes(const QVector<PacienteTemp>& pendientes)
{
    m_pendientes = pendientes;
    reconstruir();
}

void PanelKpi3Widget::reconstruir()
{
    m_conteos = QVector<int>(5, 0);
    int sinClasificar = 0;
    for (const auto& p : m_pendientes)
    {
        if (p.triageLevel >= 1 && p.triageLevel <= 5)
            m_conteos[p.triageLevel - 1]++;
        else
            sinClasificar++;
    }

    const int nivelI  = m_conteos[0];
    const int nivelII = m_conteos[1];
    const int criticos = nivelI + nivelII;

    m_lblGrande->setText(
        QString("%1 paciente%2 Nivel I/II en espera")
            .arg(criticos).arg(criticos == 1 ? "" : "s"));

    QString bg, fg;
    if (criticos == 0)      { bg = "#e8f5e9"; fg = "#1b5e20"; }
    else if (criticos <= 2) { bg = "#fff8e1"; fg = "#e65100"; }
    else                    { bg = "#ffebee"; fg = "#b71c1c"; }
    m_lblGrande->setStyleSheet(
        QString("background:%1; color:%2; padding:8px; border-radius:6px;")
            .arg(bg, fg));

    QString detalle = QString("Nivel I: %1 · Nivel II: %2 · Total pendientes: %3")
                        .arg(nivelI).arg(nivelII).arg(m_pendientes.size());
    if (sinClasificar > 0)
        detalle += QString("  (%1 sin clasificar)").arg(sinClasificar);
    m_lblDetalle->setText(detalle);

    m_grafico->setConteos(m_conteos);

    QVector<PacienteTemp> criticosLista;
    for (const auto& p : m_pendientes)
        if (p.triageLevel == 1 || p.triageLevel == 2)
            criticosLista.append(p);

    m_tabla->setRowCount(criticosLista.size());
    for (int i = 0; i < criticosLista.size(); ++i)
    {
        const auto& p = criticosLista[i];
        m_tabla->setItem(i, 0, new QTableWidgetItem(p.cedula));
        m_tabla->setItem(i, 1, new QTableWidgetItem(
            QString("%1 %2").arg(p.firstName, p.lastName).trimmed()));
        m_tabla->setItem(i, 2, new QTableWidgetItem(
            p.triageLevel == 1 ? "I" : "II"));
        QString sint = p.symptoms;
        if (sint.size() > 60) sint = sint.left(57) + "...";
        m_tabla->setItem(i, 3, new QTableWidgetItem(sint));
    }

    const bool hayCriticos = !criticosLista.isEmpty();
    m_tabla->setVisible(hayCriticos);
    m_lblSinDatos->setVisible(!hayCriticos);
}
