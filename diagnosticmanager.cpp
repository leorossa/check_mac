#include "diagnosticmanager.h"
#include <QDebug>
#include <QRegularExpression>

DiagnosticManager::DiagnosticManager(QObject *parent) 
    : QObject(parent), process(new QProcess(this)), currentStep(0), 
      currentProgress(0), overallSuccess(true)
{
    connect(process, &QProcess::readyReadStandardOutput,
            this, [this]() {
                QString output = process->readAllStandardOutput();
                emit progressUpdated(currentProgress, output);
                
                switch (currentStep) {
                    case 0: // Battery
                        parseBatteryInfo(output);
                        break;
                    case 1: // Disk
                        parseDiskInfo(output);
                        break;
                    case 2: // Apple ID
                        parseAppleIDInfo(output);
                        break;
                }
            });

    connect(process, &QProcess::finished,
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
                    overallSuccess = false;
                }
                currentProgress += 25;
                
                switch (currentStep) {
                    case 0:
                        QTimer::singleShot(0, this, &DiagnosticManager::checkDiskHealth);
                        break;
                    case 1:
                        QTimer::singleShot(0, this, &DiagnosticManager::checkAppleID);
                        break;
                    case 2:
                        QTimer::singleShot(0, this, &DiagnosticManager::checkSystemIntegrity);
                        break;
                    case 3:
                        // Добавляем рекомендации перед завершением
                        if (results.hasAppleID) {
                            results.recommendations.append("Выйдите из Apple ID перед передачей устройства");
                        }
                        if (results.maxCapacity < 80) {
                            results.recommendations.append("Рекомендуется заменить батарею (ёмкость менее 80%)");
                        }
                        emit diagnosticsFinished(overallSuccess, results);
                        break;
                }
                currentStep++;
            });
}

void DiagnosticManager::runDiagnostics()
{
    currentStep = 0;
    currentProgress = 0;
    overallSuccess = true;
    results = DiagnosticResults();

    QTimer::singleShot(0, this, &DiagnosticManager::checkBattery);
}

void DiagnosticManager::checkBattery()
{
    executeSystemCommand(
        "/usr/sbin/system_profiler",
        QStringList() << "SPPowerDataType",
        " Проверка состояния батареи..."
    );
}

void DiagnosticManager::checkDiskHealth()
{
    executeSystemCommand(
        "diskutil verifyVolume /",
        " Проверка состояния дисков..."
    );
}

void DiagnosticManager::checkAppleID()
{
    executeSystemCommand(
        "defaults read MobileMeAccounts",
        " Проверка статуса Apple ID..."
    );
}

void DiagnosticManager::checkSystemIntegrity()
{
    results.diskCheckPassed = overallSuccess;
    emit diagnosticsFinished(overallSuccess, results);
}

void DiagnosticManager::executeSystemCommand(const QString &command, const QStringList &args, const QString &description)
{
    emit progressUpdated(currentProgress, description);
    qDebug() << "Executing command:" << command << args.join(" ");
    
    process->start(command, args);
    if (!process->waitForStarted()) {
        emit progressUpdated(currentProgress, " Ошибка запуска команды: " + command);
        overallSuccess = false;
        emit diagnosticsFinished(false, results);
        return;
    }
}

void DiagnosticManager::executeSystemCommand(const QString &command, const QString &description)
{
    QStringList args = command.split(' ');
    QString program = args.takeFirst();
    
    executeSystemCommand(program, args, description);
}

void DiagnosticManager::parseBatteryInfo(const QString &output)
{
    // Ищем цикл зарядки
    QRegularExpression cycleCountRegex("Cycle Count:\\s*(\\d+)");
    auto cycleMatch = cycleCountRegex.match(output);
    if (cycleMatch.hasMatch()) {
        results.cycleCounts = cycleMatch.captured(1).toInt();
    }
    
    // Ищем максимальную ёмкость, просто находя нужную строку
    QStringList lines = output.split('\n');
    for (const QString &line : lines) {
        if (line.contains("Maximum Capacity:")) {
            // Извлекаем число перед символом %
            QString trimmed = line.trimmed();
            int percentIndex = trimmed.indexOf('%');
            if (percentIndex > 0) {
                QString numberStr = trimmed.mid(trimmed.lastIndexOf(' '), percentIndex - trimmed.lastIndexOf(' ')).trimmed();
                results.maxCapacity = numberStr.toInt();
                break;
            }
        }
    }
}

void DiagnosticManager::parseAppleIDInfo(const QString &output)
{
    qDebug() << "\n=== Parsing Apple ID Info ===";
    qDebug() << "Raw output:";
    qDebug() << output;
    
    if (output.contains("AccountID") || output.contains("AppleID")) {
        results.hasAppleID = true;
        
        // Пытаемся найти email
        QRegularExpression emailRegex("AccountID\\s*=\\s*\"([^\"]+)\"");
        auto match = emailRegex.match(output);
        if (match.hasMatch()) {
            results.appleIDEmail = match.captured(1);
            qDebug() << "Found Apple ID:" << results.appleIDEmail;
        } else {
            results.appleIDEmail = "Найден (email скрыт)";
            qDebug() << "Apple ID found but hidden";
        }
        
        // Проверяем статус Find My Mac
        QStringList lines = output.split('\n');
        bool isFindMyMacSection = false;
        bool enabled = false;
        
        qDebug() << "\nSearching for Find My Mac status:";
        for (const QString &line : lines) {
            QString trimmedLine = line.trimmed();
            qDebug() << "Checking line:" << trimmedLine;
            
            // Если нашли начало новой секции, сбрасываем флаг
            if (trimmedLine.startsWith("{")) {
                isFindMyMacSection = false;
            }
            
            // Проверяем, является ли это секцией Find My Mac
            if (trimmedLine.contains("Name = \"FIND_MY_MAC\"")) {
                qDebug() << "Found Find My Mac section";
                isFindMyMacSection = true;
            }
            
            // Если мы в секции Find My Mac и нашли Enabled
            if (isFindMyMacSection && trimmedLine.contains("Enabled =")) {
                enabled = trimmedLine.contains("Enabled = 1");
                qDebug() << "Found Enabled status:" << enabled;
            }
        }
        
        results.findMyMacEnabled = enabled;
        qDebug() << "Final Find My Mac status:" << results.findMyMacEnabled;
        
    } else {
        results.hasAppleID = false;
        results.findMyMacEnabled = false;
        qDebug() << "No Apple ID found";
    }
    
    qDebug() << "\nFinal results:";
    qDebug() << "Has Apple ID:" << results.hasAppleID;
    qDebug() << "Apple ID Email:" << results.appleIDEmail;
    qDebug() << "Find My Mac Enabled:" << results.findMyMacEnabled;
    qDebug() << "=========================\n";
}

void DiagnosticManager::parseDiskInfo(const QString &output)
{
    results.diskCheckPassed = output.contains("appears to be OK") || 
                             output.contains("No problems found");
    
    if (!results.diskCheckPassed) {
        results.diskStatus = output.split("\n").filter("Error").join(", ");
        if (results.diskStatus.isEmpty()) {
            results.diskStatus = "Неизвестная ошибка";
        }
    }
}
