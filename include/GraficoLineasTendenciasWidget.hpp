#pragma once

// =============================================================================
// GraficoLineasTendenciasWidget.hpp
// Gráfico de líneas (5 series: Nivel I..V) sobre una serie diaria.
// =============================================================================

#include <QWidget>
#include <QVector>

#include "ModelosApi.hpp"

class GraficoLineasTendenciasWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GraficoLineasTendenciasWidget(QWidget* parent = nullptr);
    void setTendencias(const QVector<DatoTendencia>& tendencias);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QVector<DatoTendencia> m_tendencias;
};
