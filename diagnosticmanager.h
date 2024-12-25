#ifndef DIAGNOSTICMANAGER_H
#define DIAGNOSTICMANAGER_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QDebug>

struct DiagnosticResults {
    // Результаты батареи
    int cycleCounts = 0;
    int maxCapacity = 0;
    
    // Результаты Apple ID
    bool hasAppleID = false;
    QString appleIDEmail;
    
    // Find My Mac
    bool findMyMacEnabled = false;
    
    // Результаты проверки диска
    bool diskCheckPassed = false;
    QString diskStatus;
    
    // Список рекомендаций
    QStringList recommendations;
    
    QString toString() const {
        QString result = "📊 Итоги диагностики:\n\n";
        QStringList allRecommendations = recommendations;
        
        // Батарея
        result += "🔋 Батарея:\n";
        result += QString("   • Циклы заряда: %1\n").arg(cycleCounts);
        result += QString("   • Максимальная ёмкость: %1%\n\n").arg(maxCapacity);
        
        // Apple ID
        result += "🍎 Apple ID:\n";
        if (hasAppleID) {
            result += QString("   • Аккаунт: %1\n").arg(appleIDEmail);
            if (findMyMacEnabled) {
                allRecommendations << "Отключите Find My Mac перед передачей устройства";
            }
        } else {
            result += "   • Аккаунт не найден\n";
        }
        result += "\n";
        
        // Диск
        result += "💽 Проверка диска:\n";
        if (diskCheckPassed) {
            result += "   • Проверка успешна\n";
        } else {
            result += QString("   • Обнаружены проблемы: %1\n").arg(diskStatus);
        }
        result += "\n";
        
        // Рекомендации
        if (!allRecommendations.isEmpty()) {
            result += "⚠️ Рекомендации:\n";
            for (const QString &rec : allRecommendations) {
                result += QString("   • %1\n").arg(rec);
            }
        }
        
        return result;
    }
};

class DiagnosticManager : public QObject
{
    Q_OBJECT
public:
    explicit DiagnosticManager(QObject *parent = nullptr);
    void runDiagnostics();

signals:
    void progressUpdated(int progress, const QString &message);
    void diagnosticsFinished(bool success, const DiagnosticResults &results);

private:
    void checkBattery();
    void checkDiskHealth();
    void checkAppleID();
    void checkSystemIntegrity();
    void parseBatteryInfo(const QString &output);
    void parseAppleIDInfo(const QString &output);
    void parseDiskInfo(const QString &output);
    void executeSystemCommand(const QString &command, const QString &description);
    void executeSystemCommand(const QString &command, const QStringList &args, const QString &description);

    QProcess *process;
    int currentStep;
    int currentProgress;
    bool overallSuccess;
    DiagnosticResults results;
};

#endif // DIAGNOSTICMANAGER_H
