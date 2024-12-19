#include "diagnosticmanager.h"
#include <QDebug>

DiagnosticManager::DiagnosticManager(QObject *parent) 
    : QObject(parent), process(new QProcess(this)), currentProgress(0), overallSuccess(true)
{
    // Настройка процесса
    process->setProcessChannelMode(QProcess::MergedChannels);
}

void DiagnosticManager::runDiagnostics()
{
    // Сброс состояния
    currentProgress = 0;
    overallSuccess = true;

    // Последовательность диагностических проверок
    QTimer::singleShot(0, this, &DiagnosticManager::checkBattery);
}

void DiagnosticManager::checkBattery()
{
    executeSystemCommand(
        "system_profiler SPPowerDataType", 
        "🔋 Проверка состояния батареи..."
    );
}

void DiagnosticManager::checkDiskHealth()
{
    executeSystemCommand(
        "diskutil list", 
        "💽 Проверка состояния дисков..."
    );
}

void DiagnosticManager::checkAppleIDStatus()
{
    executeSystemCommand(
        "dscl . -list /Users", 
        "🍎 Проверка статуса Apple ID..."
    );
}

void DiagnosticManager::checkSystemIntegrity()
{
    executeSystemCommand(
        "diskutil verifyVolume /", 
        "🛡️ Проверка целостности системы..."
    );
}

void DiagnosticManager::executeSystemCommand(const QString &command, const QString &description)
{
    // Обновление прогресса
    currentProgress += 25;
    emit progressUpdated(currentProgress, description);

    // Выполнение системной команды
    process->start("bash", QStringList() << "-c" << command);
    
    // Ожидание завершения
    if (!process->waitForFinished(10000)) {
        // Обработка таймаута
        emit progressUpdated(currentProgress, "❌ Превышено время ожидания команды");
        overallSuccess = false;
        return;
    }

    // Чтение результата
    QString output = QString::fromUtf8(process->readAllStandardOutput());
    emit progressUpdated(currentProgress, output);

    // Проверка статуса
    if (process->exitCode() != 0) {
        overallSuccess = false;
    }

    // Переход к следующей проверке
    if (currentProgress == 25) {
        QTimer::singleShot(0, this, &DiagnosticManager::checkDiskHealth);
    } else if (currentProgress == 50) {
        QTimer::singleShot(0, this, &DiagnosticManager::checkAppleIDStatus);
    } else if (currentProgress == 75) {
        QTimer::singleShot(0, this, &DiagnosticManager::checkSystemIntegrity);
    } else if (currentProgress == 100) {
        emit diagnosticsFinished(overallSuccess);
    }
}
