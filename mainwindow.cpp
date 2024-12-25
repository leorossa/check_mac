#include "mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), process(new QProcess(this)), userExists(false)
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Кнопки в горизонтальном layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    startButton = new QPushButton("Начать диагностику", this);
    buttonLayout->addWidget(startButton);
    
    settingsButton = new QPushButton("Настройки Apple ID", this);
    settingsButton->setEnabled(false);
    buttonLayout->addWidget(settingsButton);
    
    createAdminButton = new QPushButton("Создать админа", this);
    buttonLayout->addWidget(createAdminButton);
    
    createUserButton = new QPushButton("Создать пользователя", this);
    buttonLayout->addWidget(createUserButton);
    
    mainLayout->addLayout(buttonLayout);

    // Область логов
    logOutput = new QTextEdit(this);
    logOutput->setReadOnly(true);
    mainLayout->addWidget(logOutput);

    setCentralWidget(centralWidget);
    setWindowTitle("Mac Diagnostic Tool");
    resize(800, 600);

    // Создание менеджера диагностики
    diagnosticManager = new DiagnosticManager(this);
    
    // Подключение сигналов
    connect(startButton, &QPushButton::clicked, this, &MainWindow::startDiagnostics);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::openAppleIDSettings);
    connect(createAdminButton, &QPushButton::clicked, this, &MainWindow::createAdminUser);
    connect(createUserButton, &QPushButton::clicked, this, &MainWindow::createRegularUser);
    connect(diagnosticManager, &DiagnosticManager::progressUpdated, 
            this, [this](int, const QString &message) { updateLog(message); });
    connect(diagnosticManager, &DiagnosticManager::diagnosticsFinished, 
            this, &MainWindow::diagnosticsCompleted);
            
    connect(process, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = process->readAllStandardOutput();
        updateLog(output);
    });
    
    connect(process, &QProcess::readyReadStandardError, this, [this]() {
        QString error = process->readAllStandardError();
        updateLog("Ошибка: " + error);
    });
}

void MainWindow::startDiagnostics()
{
    startButton->setEnabled(false);
    settingsButton->setEnabled(false);
    logOutput->clear();
    
    updateLog(" Начало диагностики Mac...\n");
    diagnosticManager->runDiagnostics();
}

void MainWindow::updateLog(const QString &message)
{
    logOutput->append(message);
}

void MainWindow::diagnosticsCompleted(bool success, const DiagnosticResults &results)
{
    startButton->setEnabled(true);
    settingsButton->setEnabled(results.hasAppleID);
    
    // Добавляем итоговый отчет
    updateLog("\n" + results.toString());
    
    if (success) {
        QMessageBox::information(this, "Диагностика", "Проверка оборудования Mac завершена успешно.");
    } else {
        QMessageBox::warning(this, "Диагностика", "В ходе проверки обнаружены проблемы. "
                                                "Просмотрите отчет для подробной информации.");
    }
}

void MainWindow::openAppleIDSettings()
{
    updateLog("\n🔧 Открываем настройки Apple ID...");
    
    QProcess *settingsProcess = new QProcess(this);
    connect(settingsProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            updateLog("✅ Настройки Apple ID открыты");
        } else {
            updateLog("❌ Ошибка при открытии настроек");
        }
        settingsProcess->deleteLater();
    });

    QStringList args;
    args << "x-apple.systempreferences:com.apple.preferences.AppleIDPrefPane";
    settingsProcess->start("open", args);
}

void MainWindow::createAdminUser()
{
    int result = QMessageBox::warning(this, "Создание администратора",
                                    "Вы уверены, что хотите создать пользователя admin с правами администратора?",
                                    QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        createUser("admin", "dveri123x", true);
    }
}

void MainWindow::createRegularUser()
{
    int result = QMessageBox::warning(this, "Создание пользователя",
                                    "Вы уверены, что хотите создать обычного пользователя user с паролем 1111?",
                                    QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        createUser("user", "1111", false);
    }
}

void MainWindow::createUser(const QString &username, const QString &password, bool isAdmin)
{
    bool ok;
    QString sudoPass = QInputDialog::getText(this, "Аутентификация",
                                           "Введите пароль администратора:",
                                           QLineEdit::Password,
                                           QString(), &ok);
    if (!ok || sudoPass.isEmpty()) {
        updateLog("❌ Операция отменена пользователем");
        return;
    }

    updateLog(QString("\n👤 Создание пользователя %1...").arg(username));
    userExists = false;  // Сбрасываем флаг перед началом
    
    // Создаем процесс для sudo
    QProcess *sudoProcess = new QProcess(this);
    
    connect(sudoProcess, &QProcess::started, [this, sudoProcess, sudoPass]() {
        updateLog("⏳ Выполняется...");
        sudoProcess->write(QString(sudoPass + "\n").toUtf8());
    });
    
    connect(sudoProcess, &QProcess::readyReadStandardOutput, [this, sudoProcess]() {
        QString output = sudoProcess->readAllStandardOutput();
        if (!output.contains("Password:")) {
            updateLog("📝 " + output.trimmed());
        }
    });
    
    connect(sudoProcess, &QProcess::readyReadStandardError, [this, sudoProcess]() {
        QString error = sudoProcess->readAllStandardError();
        if (error.contains("already exists")) {
            userExists = true;
            updateLog("❌ Ошибка: Пользователь уже существует");
        } else if (!error.contains("Password:")) {
            updateLog("❌ " + error.trimmed());
        }
    });
    
    connect(sudoProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, username, sudoProcess](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0 && exitStatus == QProcess::NormalExit && !userExists) {
            updateLog(QString("✅ Пользователь %1 создан успешно").arg(username));
        } else if (!userExists && exitCode != 0) {
            updateLog(QString("❌ Ошибка создания пользователя %1 (код: %2)").arg(username).arg(exitCode));
        }
        sudoProcess->deleteLater();
    });

    QStringList args;
    args << "-S" << "sysadminctl"
         << "-addUser" << username
         << "-password" << password;
    
    if (isAdmin) {
        args << "-admin";
    }

    sudoProcess->start("sudo", args);
}

void MainWindow::executeCommand(const QString &command, const QStringList &args)
{
    process->start(command, args);
    if (!process->waitForStarted()) {
        updateLog(" Ошибка запуска команды");
        return;
    }
    
    process->waitForFinished();
    
    if (process->exitCode() != 0) {
        updateLog(" Ошибка выполнения команды");
    } else {
        updateLog(" Команда выполнена успешно");
    }
}
