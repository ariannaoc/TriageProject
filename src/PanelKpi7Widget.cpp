#include "PanelKpi7Widget.hpp"
#include "GraficoLineasTendenciasWidget.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>

PanelKpi7Widget::PanelKpi7Widget(int centerId, QWidget* parent)
    : QWidget(parent), m_centerId(centerId)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    // Cabecera: título + selector de rango
    auto* header = new QHBoxLayout();
    auto* lblTitulo = new QLabel("Tendencias por nivel de triaje", this);
    QFont fSub = lblTitulo->font();
    fSub.setBold(true);
    lblTitulo->setFont(fSub);

    auto* lblRango = new QLabel("Rango:", this);
    m_combo = new QComboBox(this);
    m_combo->addItem("Últimos 7 días", 7);
    m_combo->addItem("Últimos 14 días", 14);
    m_combo->addItem("Últimos 30 días", 30);
    connect(m_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PanelKpi7Widget::onRangeChanged);

    header->addWidget(lblTitulo, 1);
    header->addWidget(lblRango);
    header->addWidget(m_combo);
    layout->addLayout(header);

    // Línea de resumen
    m_lblResumen = new QLabel("Sin datos cargados.", this);
    m_lblResumen->setStyleSheet("color:#424242;");
    layout->addWidget(m_lblResumen);

    // Gráfico
    m_grafico = new GraficoLineasTendenciasWidget(this);
    layout->addWidget(m_grafico, /*stretch=*/2);

    // Tabla
    m_tabla = new QTableWidget(this);
    m_tabla->setColumnCount(8);
    m_tabla->setHorizontalHeaderLabels(
        {"Fecha", "I", "II", "III", "IV", "V", "Total", "Críticos I+II"});
    m_tabla->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tabla->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tabla->setAlternatingRowColors(true);
    layout->addWidget(m_tabla, /*stretch=*/1);

    m_lblSinDatos = new QLabel(
        "El centro aún no tiene registros históricos en este rango.", this);
    m_lblSinDatos->setAlignment(Qt::AlignCenter);
    m_lblSinDatos->setStyleSheet("color:#9e9e9e; padding:12px;");
    m_lblSinDatos->setVisible(false);
    layout->addWidget(m_lblSinDatos);

    reconstruir();
}

void PanelKpi7Widget::setTendencias(const QVector<DatoTendencia>& tendencias)
{
    m_tendencias = tendencias;
    reconstruir();
}

void PanelKpi7Widget::onRangeChanged(int /*index*/)
{
    const int days = m_combo->currentData().toInt();
    emit solicitarTendencias(days);
}

void PanelKpi7Widget::reconstruir()
{
    const bool hayDatos = !m_tendencias.isEmpty();
    m_grafico->setVisible(hayDatos);
    m_tabla->setVisible(hayDatos);
    m_lblSinDatos->setVisible(!hayDatos);

    if (!hayDatos)
    {
        m_lblResumen->setText("Sin datos cargados.");
        m_tabla->setRowCount(0);
        return;
    }

    int totalGeneral = 0;
    int criticosGeneral = 0;
    int maxDia = 0;
    QDate fechaMax;

    m_tabla->setRowCount(m_tendencias.size());
    for (int i = 0; i < m_tendencias.size(); ++i)
    {
        const auto& t = m_tendencias[i];
        const int criticos = t.nivelI + t.nivelII;
        totalGeneral   += t.total;
        criticosGeneral += criticos;
        if (t.total > maxDia) { maxDia = t.total; fechaMax = t.fecha; }

        m_tabla->setItem(i, 0, new QTableWidgetItem(t.fecha.toString("yyyy-MM-dd")));
        m_tabla->setItem(i, 1, new QTableWidgetItem(QString::number(t.nivelI)));
        m_tabla->setItem(i, 2, new QTableWidgetItem(QString::number(t.nivelII)));
        m_tabla->setItem(i, 3, new QTableWidgetItem(QString::number(t.nivelIII)));
        m_tabla->setItem(i, 4, new QTableWidgetItem(QString::number(t.nivelIV)));
        m_tabla->setItem(i, 5, new QTableWidgetItem(QString::number(t.nivelV)));
        m_tabla->setItem(i, 6, new QTableWidgetItem(QString::number(t.total)));
        m_tabla->setItem(i, 7, new QTableWidgetItem(QString::number(criticos)));
    }

    m_grafico->setTendencias(m_tendencias);

    QString resumen = QString(
        "%1 día%2 con registros · Total: %3 triajes · Críticos I+II: %4 · "
        "Día más cargado: %5 (%6)")
        .arg(m_tendencias.size())
        .arg(m_tendencias.size() == 1 ? "" : "s")
        .arg(totalGeneral)
        .arg(criticosGeneral)
        .arg(fechaMax.isValid() ? fechaMax.toString("dd/MM") : "-")
        .arg(maxDia);
    m_lblResumen->setText(resumen);
}
