# Mac Diagnostic Tool

## Описание
Программа для диагностики Mac при увольнении сотрудника.

## Функциональность
- Проверка состояния батареи
- Диагностика дисков
- Проверка статуса Apple ID
- Проверка целостности системы

## Требования
- Qt 5.x или выше
- Компилятор C++11
- macOS

## Сборка и установка

### Предварительные требования
- Qt 5.x или выше
- Компилятор C++11
- macOS
- Xcode Command Line Tools
- Homebrew (пакетный менеджер)

### Шаги по установке

1. Установите Homebrew (если еще не установлен):
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

2. Установите Xcode Command Line Tools:
```bash
xcode-select --install
```

3. Установите Qt через Homebrew:
```bash
brew install qt@5
```

4. Добавьте Qt в PATH (выберите один из способов):
   - Временно (только для текущей сессии терминала):
   ```bash
   export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"
   ```
   
   - Постоянно (добавьте эту строку в ~/.zshrc):
   ```bash
   echo 'export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"' >> ~/.zshrc
   ```
   После этого откройте новое окно терминала или выполните:
   ```bash
   source ~/.zshrc
   ```

5. Проверьте установку Qt:
```bash
which qmake
qmake --version
```

6. Клонируйте репозиторий:
```bash
git clone https://github.com/yourusername/check_mac.git
cd check_mac
```

7. Соберите проект:
```bash
# Если у вас SDK версии > 13, используйте:
qmake mac_diagnostic.pro "CONFIG+=sdk_no_version_check"
# Или для SDK версии 13:
# qmake mac_diagnostic.pro

make
```

8. Запустите программу:
```bash
./mac_diagnostic
```

### Возможные проблемы

Если при сборке возникают ошибки:
1. Убедитесь, что путь к Qt добавлен в PATH
2. Проверьте версию Qt и компилятора
3. Убедитесь, что все зависимости установлены

## Использование
1. Запустите исполняемый файл
2. Нажмите "Начать диагностику"
3. Просмотрите результаты в интерфейсе

## Лицензия
Частное использование

## Контакты
Служба поддержки IT-отдела
