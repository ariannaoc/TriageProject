#include "GraficoBarrasNivelesWidget.hpp"

#include <QPainter>
#include <QPaintEvent>

GraficoBarrasNivelesWidget::GraficoBarrasNivelesWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void GraficoBarrasNivelesWidget::setConteos(const QVector<int>& conteos)
{
    m_conteos = conteos;
    while (m_conteos.size() < 5) m_conteos.append(0);
    update();
}

QSize GraficoBarrasNivelesWidget::sizeHint() const { return QSize(400, 220); }

void GraficoBarrasNivelesWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int margen       = 16;
    const int altoContador = 18;
    const int altoEtiqueta = 20;
    const int espacio      = 12;

    int maxConteo = 1;
    for (int c : m_conteos) if (c > maxConteo) maxConteo = c;

    const int anchoArea  = width() - 2 * margen;
    const int anchoBarra = (anchoArea - 4 * espacio) / 5;
    const int altoArea   = height() - 2 * margen - altoContador - altoEtiqueta;

    const QVector<QColor> colores = {
        QColor("#b71c1c"), QColor("#e53935"), QColor("#fb8c00"),
        QColor("#fdd835"), QColor("#43a047")
    };
    const QStringList etiquetas = {"I", "II", "III", "IV", "V"};

    p.setPen(QPen(QColor("#bdbdbd"), 1));
    p.drawLine(margen, margen + altoContador + altoArea,
               width() - margen, margen + altoContador + altoArea);

    for (int i = 0; i < 5; ++i)
    {
        const int x = margen + i * (anchoBarra + espacio);
        const int altoBarra = (m_conteos[i] * altoArea) / maxConteo;
        const int yBarra = margen + altoContador + (altoArea - altoBarra);

        p.setPen(Qt::NoPen);
        p.setBrush(colores[i]);
        p.drawRoundedRect(x, yBarra, anchoBarra, altoBarra, 4, 4);

        p.setPen(QColor("#212121"));
        QFont f = p.font();
        f.setPointSize(9); f.setBold(true);
        p.setFont(f);
        p.drawText(QRect(x, yBarra - altoContador, anchoBarra, altoContador),
                   Qt::AlignCenter, QString::number(m_conteos[i]));

        p.setPen(QColor("#424242"));
        f.setBold(false); f.setPointSize(9);
        p.setFont(f);
        p.drawText(QRect(x, margen + altoContador + altoArea, anchoBarra, altoEtiqueta),
                   Qt::AlignCenter, etiquetas[i]);
    }
}
