#pragma once

#include <QWidget>
#include <QColor>

class BarraOcupacionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BarraOcupacionWidget(QWidget* parent = nullptr);
    void setDatos(int ocupado, int enCamino, int capacidadTotal);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int m_ocupado    = 0;
    int m_enCamino   = 0;
    int m_capacidad  = 0;
    QColor colorSemaforo() const;
};
