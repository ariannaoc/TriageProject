#pragma once

// =============================================================================
// PanelKpi7Widget.hpp
// Panel del KPI 7: tendencias históricas por día y nivel de triaje.
// =============================================================================

#include <QWidget>
#include <QVector>

#include "ModelosApi.hpp"

class QComboBox;
class QLabel;
class QTableWidget;
class GraficoLineasTendenciasWidget;

class PanelKpi7Widget : public QWidget
{
    Q_OBJECT
public:
    explicit PanelKpi7Widget(int centerId, QWidget* parent = nullptr);

    int centerId() const { return m_centerId; }
    void setTendencias(const QVector<DatoTendencia>& tendencias);

signals:
    /// La pestaña solicita al MainWindow que dispare ApiClient::fetchTrends(days).
    void solicitarTendencias(int days);

private slots:
    void onRangeChanged(int index);

private:
    void reconstruir();

    int m_centerId;

    QComboBox* m_combo    = nullptr;
    QLabel* m_lblResumen  = nullptr;
    QLabel* m_lblSinDatos = nullptr;
    GraficoLineasTendenciasWidget* m_grafico = nullptr;
    QTableWidget* m_tabla = nullptr;

    QVector<DatoTendencia> m_tendencias;
};
