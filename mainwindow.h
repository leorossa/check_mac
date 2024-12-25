#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QProcess>
#include "diagnosticmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void startDiagnostics();
    void updateLog(const QString &message);
    void diagnosticsCompleted(bool success, const DiagnosticResults &results);
    void openAppleIDSettings();
    void createAdminUser();
    void createRegularUser();

private:
    void createUser(const QString &username, const QString &password, bool isAdmin);
    void executeCommand(const QString &command, const QStringList &args);

    QPushButton *startButton;
    QPushButton *settingsButton;
    QPushButton *createAdminButton;
    QPushButton *createUserButton;
    QTextEdit *logOutput;
    QProcess *process;
    DiagnosticManager *diagnosticManager;
    bool userExists;  // Флаг для отслеживания существующего пользователя
};

#endif // MAINWINDOW_H
