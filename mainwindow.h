#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProgressBar>
#include "diagnosticmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void startDiagnostics();
    void updateDiagnosticProgress(int progress, const QString &message);
    void diagnosticsCompleted(bool success);

private:
    QTextEdit *logDisplay;
    QPushButton *startButton;
    QProgressBar *progressBar;
    DiagnosticManager *diagnosticManager;
};
