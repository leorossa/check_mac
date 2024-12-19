#ifndef DIAGNOSTICMANAGER_H
#define DIAGNOSTICMANAGER_H

#include <QObject>
#include <QProcess>

class DiagnosticManager : public QObject
{
    Q_OBJECT

public:
    explicit DiagnosticManager(QObject *parent = nullptr);
    void runDiagnostics();

signals:
    void progressUpdated(int progress, const QString &message);
    void diagnosticsFinished(bool success);

private slots:
    void checkBattery();
    void checkDiskHealth();
    void checkAppleIDStatus();
    void checkSystemIntegrity();

private:
    QProcess *process;
    int currentProgress;
    bool overallSuccess;

    void executeSystemCommand(const QString &command, const QString &description);
};
