#pragma once

// =============================================================================
// GraficoBarrasNivelesWidget.hpp
// Gráfico de barras simple (QPainter puro) para la distribución de niveles
// de triaje I..V. Sin dependencias externas.
// =============================================================================

#include <QWidget>
#include <QVector>

class GraficoBarrasNivelesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GraficoBarrasNivelesWidget(QWidget* parent = nullptr);

    /// conteos[0]=Nivel I, conteos[1]=Nivel II, ..., conteos[4]=Nivel V
    void setConteos(const QVector<int>& conteos);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QVector<int> m_conteos = QVector<int>(5, 0);
};
