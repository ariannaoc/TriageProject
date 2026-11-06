// loginwindow.h
#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class LoginWindow : public QDialog {
    Q_OBJECT
public:
    explicit LoginWindow(QWidget* parent = nullptr);
private slots:
    void attemptLogin();
private:
    QLineEdit* txtUsername;
    QLineEdit* txtPassword;
    QPushButton* btnLogin;
    QLabel* lblError;
    sqlite3* db;
};
