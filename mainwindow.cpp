#include "mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // Создание центрального виджета и основного layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Кнопка запуска диагностики
    startButton = new QPushButton("Начать диагностику", this);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::startDiagnostics);
    mainLayout->addWidget(startButton);

    // Прогресс-бар
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    mainLayout->addWidget(progressBar);

    // Область логирования
    logDisplay = new QTextEdit(this);
    logDisplay->setReadOnly(true);
    mainLayout->addWidget(logDisplay);

    // Создание менеджера диагностики
    diagnosticManager = new DiagnosticManager(this);
    
    // Подключение сигналов
    connect(diagnosticManager, &DiagnosticManager::progressUpdated, 
            this, &MainWindow::updateDiagnosticProgress);
    connect(diagnosticManager, &DiagnosticManager::diagnosticsFinished, 
            this, &MainWindow::diagnosticsCompleted);

    setCentralWidget(centralWidget);
}

void MainWindow::startDiagnostics()
{
    // Очистка предыдущих результатов
    logDisplay->clear();
    progressBar->setValue(0);
    startButton->setEnabled(false);

    // Начало диагностики
    logDisplay->append("🔍 Начало диагностики Mac...");
    diagnosticManager->runDiagnostics();
}

void MainWindow::updateDiagnosticProgress(int progress, const QString &message)
{
    // Обновление прогресс-бара и логов
    progressBar->setValue(progress);
    logDisplay->append(message);
}

void MainWindow::diagnosticsCompleted(bool success)
{
    // Завершение диагностики
    startButton->setEnabled(true);
    
    if (success) {
        logDisplay->append("✅ Диагностика завершена успешно!");
        QMessageBox::information(this, "Диагностика", "Проверка оборудования Mac завершена.");
    } else {
        logDisplay->append("❌ Диагностика завершена с ошибками!");
        QMessageBox::warning(this, "Диагностика", "В ходе проверки обнаружены проблемы.");
    }
}
