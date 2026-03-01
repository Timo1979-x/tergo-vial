const Gio = imports.gi.Gio;
const GLib = imports.gi.GLib;

// Пытаемся подключить библиотеку XApp, которая отвечает за раскладки в Cinnamon
let XApp;
try {
    XApp = imports.gi.XApp;
} catch (e) {
    XApp = null;
}

let instance = null;

class TergoLayout {
    constructor(meta) {
        this.meta = meta;
        this.logFilePath = GLib.build_filenamev([GLib.get_home_dir(), 'cinnamon-layout-changes.log']);
        
        this.kbdController = null;
        this.signalId = 0;
    }

    enable() {
        this._log("=== Расширение включено. Начинаем отслеживание ===");
        
        // for (let key in global.keys) {
        //     this._log(`\tkey: ${key}`)
        // }
        if (XApp && XApp.KbdLayoutController) {
            try {
                // Создаем экземпляр контроллера раскладок
                this.kbdController = new XApp.KbdLayoutController();
                
                // Подписываемся на реальный сигнал изменения раскладки
                this.signalId = this.kbdController.connect('layout-changed', () => this._onLayoutChanged());
                
                this._log("Успешно подключен XApp.KbdLayoutController.");
            } catch (e) {
                global.logError(`[TergoLayout] Ошибка инициализации XApp: ${e.message}`);
                this._log(`Ошибка инициализации XApp: ${e.message}`);
            }
        } else {
            this._log("Библиотека XApp.KbdLayoutController не найдена в этой системе.");
        }
    }

    disable() {
        if (this.signalId > 0 && this.kbdController) {
            this.kbdController.disconnect(this.signalId);
            this.signalId = 0;
        }
        this._log("=== Расширение отключено ===");
    }
  
  _onLayoutChanged() {
        try {
            // Получаем индекс текущей группы (0, 1, 2...)
            let group = this.kbdController.get_current_group();
            // Получаем имя иконки (обычно содержит код языка, например "ru", "us")
            let iconName = this.kbdController.get_current_icon_name();
            // Очищаем имя (иногда оно может приходить в формате "xapp-kbd-ru")
            let layoutName = iconName ? iconName.replace('xapp-kbd-', '') : `Группа ${group}`;
            
            // this._log(`Раскладка изменена. Индекс: ${group}, Язык: ${layoutName.toUpperCase()}`);
            
            // === НОВЫЙ КОД: Запуск Node.js скрипта ===
            
            // Получаем путь к папке нашего расширения
            let extensionDir = this.meta.path; 
            
            // Формируем полный путь к скрипту
            let scriptPath = GLib.build_filenamev([extensionDir, 'send_hid.js']);
            
            // Формируем команду: node /путь/к/скрипту/keyboard-usb.js ru
            // Используем абсолютный путь к node на всякий случай
            let command = `/usr/bin/node "${scriptPath}" ${layoutName}`;
            
            // Запускаем асинхронно
            let success = GLib.spawn_command_line_async(command);
            
            // if (success) {
            //   this._log(`Успешно запущен процесс: ${command}`);
            // }

        } catch (e) {
            global.logError(`[TergoLayout] Ошибка при получении раскладки: ${e.message}`);
        }
    }

    _log(message) {
        global.log(`TergoLayout: ${message}`)
        // let now = GLib.DateTime.new_now_local().format('%Y-%m-%d %H:%M:%S');
        // let logLine = `[${now}] ${message}\n`;
        
        // try {
        //     let file = Gio.File.new_for_path(this.logFilePath);
        //     let stream;
            
        //     if (file.query_exists(null)) {
        //         stream = file.append_to(Gio.FileCreateFlags.NONE, null);
        //     } else {
        //         stream = file.create(Gio.FileCreateFlags.NONE, null);
        //     }
            
        //     let dataStream = new Gio.DataOutputStream({ base_stream: stream });
        //     dataStream.put_string(logLine, null);
        //     dataStream.close(null);
        // } catch (e) {
        //     global.logError(`[LayoutReporter] Ошибка записи в файл: ${e.message}`);
        // }
    }
}

// --- Обязательные глобальные функции ---

function init(meta) {
    instance = new TergoLayout(meta);
}

function enable() {
    if (instance) {
        instance.enable();
    }
}

function disable() {
    if (instance) {
        instance.disable();
    }
}