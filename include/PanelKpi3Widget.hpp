#pragma once

// =============================================================================
// PanelKpi3Widget.hpp
// Panel del KPI 3: "Pacientes Nivel I/II en espera".
// =============================================================================

#include <QWidget>
#include <QVector>

#include "ModelosApi.hpp"

class QLabel;
class QTableWidget;
class GraficoBarrasNivelesWidget;

class PanelKpi3Widget : public QWidget
{
    Q_OBJECT
public:
    explicit PanelKpi3Widget(QWidget* parent = nullptr);
    void setPendientes(const QVector<PacienteTemp>& pendientes);

private:
    void reconstruir();

    QLabel* m_lblGrande   = nullptr;
    QLabel* m_lblDetalle  = nullptr;
    QLabel* m_lblSinDatos = nullptr;
    GraficoBarrasNivelesWidget* m_grafico = nullptr;
    QTableWidget* m_tabla = nullptr;

    QVector<PacienteTemp> m_pendientes;
    QVector<int> m_conteos = QVector<int>(5, 0);
};
