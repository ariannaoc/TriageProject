#include "BarraOcupacionWidget.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>

BarraOcupacionWidget::BarraOcupacionWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(320, 80);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void BarraOcupacionWidget::setDatos(int ocupado, int enCamino, int capacidadTotal)
{
    m_ocupado   = qMax(0, ocupado);
    m_enCamino  = qMax(0, enCamino);
    m_capacidad = qMax(0, capacidadTotal);
    update();
}

QSize BarraOcupacionWidget::sizeHint() const { return QSize(360, 80); }

QColor BarraOcupacionWidget::colorSemaforo() const
{
    if (m_capacidad <= 0) return QColor("#9e9e9e");
    const double ratio = static_cast<double>(m_ocupado + m_enCamino) / m_capacidad;
    if (ratio < 0.50) return QColor("#2e7d32");
    if (ratio < 0.75) return QColor("#f9a825");
    return QColor("#c62828");
}

void BarraOcupacionWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int margen     = 6;
    const int altoTexto  = 18;
    const int altoBarra  = 18;
    const int altoEstado = 16;
    const int yTexto  = margen;
    const int yBarra  = yTexto + altoTexto + 2;
    const int yEstado = yBarra + altoBarra + 2;
    const int anchoBarra = width() - 2 * margen;

    QRect barraRect(margen, yBarra, anchoBarra, altoBarra);
    p.setPen(QPen(QColor("#bdbdbd"), 1));
    p.setBrush(QColor("#eeeeee"));
    p.drawRoundedRect(barraRect, 3, 3);

    if (m_capacidad <= 0)
    {
        p.setPen(QColor("#555555"));
        p.drawText(barraRect, Qt::AlignCenter, "Sin capacidad configurada");
        return;
    }

    const double propOcupado  = static_cast<double>(m_ocupado)  / m_capacidad;
    const double propEnCamino = static_cast<double>(m_enCamino) / m_capacidad;
    const double propTotal    = propOcupado + propEnCamino;

    double factorEscala = 1.0;
    if (propTotal > 1.0) factorEscala = 1.0 / propTotal;

    double anchoOcupado  = anchoBarra * propOcupado  * factorEscala;
    double anchoEnCamino = anchoBarra * propEnCamino * factorEscala;

    // Forzar un mínimo visual de 3px para que 1 solo paciente sea visible en capacidades grandes
    if (m_ocupado > 0 && anchoOcupado > 0.0 && anchoOcupado < 3.0) anchoOcupado = 3.0;
    if (m_enCamino > 0 && anchoEnCamino > 0.0 && anchoEnCamino < 3.0) anchoEnCamino = 3.0;
    const QColor color = colorSemaforo();

    if (anchoOcupado > 0.5)
    {
        QRect r(barraRect.x(), barraRect.y(),
                static_cast<int>(anchoOcupado), barraRect.height());
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        p.drawRoundedRect(r, 3, 3);
    }

    if (anchoEnCamino > 0.5)
    {
        QRect r(barraRect.x() + static_cast<int>(anchoOcupado), barraRect.y(),
                static_cast<int>(anchoEnCamino), barraRect.height());
        QPainterPath clip;
        clip.addRoundedRect(barraRect, 3, 3);
        p.save();
        p.setClipPath(clip);
        p.setPen(Qt::NoPen);
        QBrush rayado(color.lighter(115), Qt::BDiagPattern);
        p.setBrush(rayado);
        p.drawRect(r);
        p.restore();
    }

    const double porc = (static_cast<double>(m_ocupado + m_enCamino) / m_capacidad) * 100.0;
    QString textoSuperior = QString("%1/%2 · %3% · +%4 en camino")
        .arg(m_ocupado).arg(m_capacidad)
        .arg(QString::number(porc, 'f', 1)).arg(m_enCamino);
    if (propTotal > 1.0)
    {
        const int exceso = (m_ocupado + m_enCamino) - m_capacidad;
        textoSuperior += QString("  (+%1 sobre capacidad)").arg(exceso);
    }

    p.setPen(QColor("#212121"));
    QFont f = p.font();
    f.setPointSize(9);
    f.setBold(true);
    p.setFont(f);
    p.drawText(QRect(margen, yTexto, anchoBarra, altoTexto),
               Qt::AlignLeft | Qt::AlignVCenter, textoSuperior);

    // --- Texto inferior: Leyendas ---
    f.setPointSize(8);
    f.setBold(false);
    p.setFont(f);

    int xLeyenda = margen;
    int yLeyenda = yEstado + 2;

    // Leyenda 1: Ocupado (Sólido)
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawRoundedRect(xLeyenda, yLeyenda, 12, 12, 2, 2);
    p.setPen(QColor("#424242"));
    p.drawText(xLeyenda + 16, yLeyenda + 10, "Capacidad ocupada");

    // Leyenda 2: Comprometido (Rayado)
    xLeyenda += 130; // Desplazar a la derecha
    p.setPen(Qt::NoPen);
    QBrush rayadoLeyenda(color.lighter(115), Qt::BDiagPattern);
    p.setBrush(rayadoLeyenda);
    p.drawRoundedRect(xLeyenda, yLeyenda, 12, 12, 2, 2);
    p.setPen(QColor("#424242"));
    p.drawText(xLeyenda + 16, yLeyenda + 10, "Capacidad comprometida");
}
