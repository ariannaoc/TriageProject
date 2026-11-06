// mainwindow.h
#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
private slots:
    void onEmergencyClicked();
    void onHealthCenterClicked();
private:
    QPushButton* btnEmergency;
    QPushButton* btnHealthCenter;
};
