#pragma once
#include <QWidget>
#include <QResizeEvent>

namespace Ui { class VistaMain; }

class VistaMain : public QWidget {
    Q_OBJECT
public:
    explicit VistaMain(QWidget* parent = nullptr);
    ~VistaMain();

signals:
    void pacienteClicked();
    void centroClicked();

private:
    Ui::VistaMain* ui;
    
protected:
    void resizeEvent(QResizeEvent* event) override;
};