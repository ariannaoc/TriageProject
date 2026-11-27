#include "GraficoLineasTendenciasWidget.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QPolygon>

GraficoLineasTendenciasWidget::GraficoLineasTendenciasWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(260);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void GraficoLineasTendenciasWidget::setTendencias(const QVector<DatoTendencia>& tendencias)
{
    m_tendencias = tendencias;
    update();
}

QSize GraficoLineasTendenciasWidget::sizeHint() const { return QSize(600, 300); }

void GraficoLineasTendenciasWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    if (m_tendencias.isEmpty())
    {
        p.setPen(QColor("#9e9e9e"));
        p.drawText(rect(), Qt::AlignCenter,
                   "Sin datos para el rango seleccionado.");
        return;
    }

    const int margen      = 46;
    const int altoXAxis   = 30;
    const int margenSup   = 20;

    const int anchoArea = width()  - 2 * margen;
    const int altoArea  = height() - margenSup - altoXAxis - 10;

    // Escala Y (redondeo a múltiplo de 5)
    int maxY = 1;
    for (const auto& t : m_tendencias)
        for (int i = 0; i < 5; ++i)
            if (t.nivel(i) > maxY) maxY = t.nivel(i);
    maxY = ((maxY + 4) / 5) * 5;
    if (maxY < 5) maxY = 5;

    // Ejes
    p.setPen(QPen(QColor("#616161"), 1));
    p.drawLine(margen, margenSup + altoArea, margen + anchoArea, margenSup + altoArea);
    p.drawLine(margen, margenSup,            margen,                margenSup + altoArea);

    // Ticks Y + grid horizontal
    QFont f = p.font(); f.setPointSize(8);
    p.setFont(f);
    for (int i = 0; i <= 5; ++i)
    {
        const int y = margenSup + altoArea - (i * altoArea / 5);
        const int val = maxY * i / 5;

        p.setPen(QColor("#757575"));
        p.drawText(QRect(0, y - 10, margen - 6, 20),
                   Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(val));

        p.setPen(QPen(QColor("#e0e0e0"), 1, Qt::DotLine));
        p.drawLine(margen, y, margen + anchoArea, y);
    }

    // Ticks X
    const int n = m_tendencias.size();
    const int step = (n > 1) ? anchoArea / (n - 1) : 0;
    for (int i = 0; i < n; ++i)
    {
        const int x = margen + i * step;
        p.setPen(QColor("#424242"));
        p.drawText(QRect(x - 30, margenSup + altoArea + 6, 60, 20),
                   Qt::AlignCenter,
                   m_tendencias[i].fecha.toString("dd/MM"));
    }

    // Colores de triaje
    const QVector<QColor> colores = {
        QColor("#b71c1c"), QColor("#e53935"), QColor("#fb8c00"),
        QColor("#fdd835"), QColor("#43a047")
    };
    const QStringList etiquetas = {"I","II","III","IV","V"};

    // 5 series
    for (int nivel = 0; nivel < 5; ++nivel)
    {
        QPolygon poly;
        for (int i = 0; i < n; ++i)
        {
            const int val = m_tendencias[i].nivel(nivel);
            const int x = margen + i * step;
            const int y = margenSup + altoArea - (val * altoArea / maxY);
            poly << QPoint(x, y);
        }

        p.setPen(QPen(colores[nivel], 2));
        p.setBrush(Qt::NoBrush);
        p.drawPolyline(poly);

        // Puntos
        p.setPen(Qt::NoPen);
        p.setBrush(colores[nivel]);
        for (const QPoint& pt : poly)
            p.drawEllipse(pt, 3, 3);
    }

    // Leyenda
    int lx = margen + 8;
    int ly = margenSup + 4;
    for (int i = 0; i < 5; ++i)
    {
        p.setPen(Qt::NoPen);
        p.setBrush(colores[i]);
        p.drawRect(lx, ly + i * 16, 12, 12);

        p.setPen(QColor("#212121"));
        p.drawText(lx + 18, ly + i * 16 + 11, QString("Nivel %1").arg(etiquetas[i]));
    }
}
