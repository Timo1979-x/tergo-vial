## QML Layout monitor
Агент для Windows, отслеживающий изменения раскладки и передающий эту информацию в T-Ergo keyboard
### Установка
- при желании можно заново скомпилировать.
  - Потребуется MS Visual Studio 2022 с включенной опцией "Windows C++ development"
  - Запустить Developer Command Prompt for Visual Studio 2022
  - перейти в папку с проектом и собрать:
```cmd
cd e:\temp\QMKLayoutMonitor\QMKLayoutMonitor
cl /EHsc *.cpp /Fe:QMKLayoutMonitor.exe /link /SUBSYSTEM:WINDOWS
# или с оптимизацией:
cl /EHsc /O1 /MD *.cpp /Fe:QMKLayoutMonitor.exe /link /SUBSYSTEM:WINDOWS
```
- для установки запустить так:
```cmd
QmkLayoutMonitor.exe <VID> <PID> <USAGE_PAGE> install
```
Программа скопирует себя в %APPDATA% (c:/Users/tim/AppData/Roaming/QMKLayoutMonitor/QMKLayoutMonitor.exe) и создаст ярлык в автозапуске (`shell:startup`) с указанными параметрами VID, PID и USAGE_PAGE.

### Прочие заметки
- Чтобы писать отладочные сообщения в файл `layout_monitor.log` в папке с программой, надо прописать в коде `#define DEBUG` и перекомпилировать.
- просто запустить:
```cmd
QmkLayoutMonitor.exe <VID> <PID> <USAGE_PAGE>
# Пример:
QmkLayoutMonitor.exe 0x5954 0x0003 0xFF60
```
- Проверить связи с клавиатурой:
```cmd
QmkLayoutMonitor.exe 0x5954 0x0003 0xff60 test
```
программа просто отправит в клавиатуру текущую раскладку и завершится.
- Программе не всегда приходят уведомления о смене раскладки. Например, при смене раскладки с EN на RU через иконку в system tray. При смене раскладки в обратную сторону через system tray - обновления приходят.
- С помощью ИИ перепробовал кучу вариантов:
  - Через Text Services Framework - но оказалось, что тут уведомления приходят только о смене раскладки для нашего приложения, а не для глобально
  - Через периодический опрос - это мне не нравится, хотя так сделано в клавиатуре stront ([вот](https://github.com/zzeneg/qmk-hid-host) проект на Rust)
  - Через перехват WM_SHELL - это по словам ИИ не работает начиная с Windows 8
  - Комбинированный метод, который у нас тут и используется: отслеживаем изменение активного окна плюс отпускание кнопок, которые применяются в переключении языков (Ctrl, Shift, Alt, Win...). Это работает почти хорошо.
