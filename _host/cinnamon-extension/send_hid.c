/*
Установить библиотеки, нужные для компиляции:
apt install libhidapi-dev gcc -y

Скомпилировать:
gcc send_hid.c -o send_hid -lhidapi-hidraw -s

Наблюдать трафик:
sudo cat /dev/hidrawN | xxd -g1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hidapi/hidapi.h>

int main(int argc, char *argv[]) {
    // Проверяем количество аргументов
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <HEX_VID> <HEX_PID> <HEX_USAGE_PAGE> <STRING_LAYOUT> <BACKLIGHT_ON>\n", argv[0]);
        fprintf(stderr, "HEX_VID, HEX_PID and HEX_USAGE_PAGE - keyboard USB \"coordinates\"\n");
        fprintf(stderr, "STRING_LAYOUT - two-character language code, like \"RU\" or \"by\"\n");
        fprintf(stderr, "BACKLIGHT_ON - should the keyboard turn on the backlight\n");
        fprintf(stderr, "Example: %s 5954 0003 FF60 ru 1\n", argv[0]);
        return 1;
    }

    // Парсим аргументы (ожидаем шестнадцатеричные значения для VID, PID и USAGE_PAGE)
    unsigned short vid = (unsigned short)strtol(argv[1], NULL, 16);
    unsigned short pid = (unsigned short)strtol(argv[2], NULL, 16);
    unsigned short usage_page = (unsigned short)strtol(argv[3], NULL, 16);
    char *layout_arg = argv[4];
    unsigned char backlight_on = (unsigned char)strtol(argv[5], NULL, 10);


    // Форматируем строку раскладки (аналог layout.padStart(2, " ").substring(0, 2))
    char layout[2] = {' ', ' '};
    size_t len = strlen(layout_arg);
    if (len == 1) {
        layout[0] = ' ';
        layout[1] = layout_arg[0];
    } else if (len >= 2) {
        layout[0] = layout_arg[0];
        layout[1] = layout_arg[1];
    }

    printf("Sending signal to the keyboard about language change: %c%c\n", layout[0], layout[1]);

    // Инициализируем библиотеку hidapi
    int hid_init_result = hid_init();
    if (hid_init_result != 0) {
        fprintf(stderr, "error initializing hidapi: %d\n", hid_init_result);
        return 1;
    }

    // Ищем нужное устройство
    struct hid_device_info *devs, *cur_dev;
    devs = hid_enumerate(vid, pid);
    cur_dev = devs;
    char *target_path = NULL;

    while (cur_dev) {
        if (cur_dev->usage_page == usage_page) {
            target_path = strdup(cur_dev->path);
            break;
        }
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);

    if (!target_path) {
        fprintf(stderr, "USB device or Raw HID interface not found\n");
        hid_exit();
        return 1;
    }

    printf("Found Raw HID interface: %s\n", target_path);

    // Открываем устройство по найденному пути
    hid_device *device = hid_open_path(target_path);
    free(target_path);

    if (!device) {
        fprintf(stderr, "Unable to open the device '%s'. Check access rights (udev).\n", target_path);
        hid_exit();
        return 1;
    }

    printf("Device opened successfully\n");

    // Формируем пакет данных (33 байта: 1 байт Report ID + 32 байта данных)
    unsigned char data[33];
    memset(data, 0, sizeof(data));
    
    data[0] = 0x00;       // Report ID
    data[1] = 0xfc;       // Команда (должна не пересекаться с командами VIAL)
    data[2] = 0x42;       // Команда
    data[3] = layout[0];  // Символ 1
    data[4] = layout[1];  // Символ 2
    data[5] = backlight_on;  // Подсветка

    // Отправляем данные
    int bytes_written = hid_write(device, data, sizeof(data));
    if (bytes_written < 0) {
        fprintf(stderr, "Error writing to device: %ls\n", hid_error(device));
    } else {
        printf("%d bytes sent.\n", bytes_written);
    }

    // Закрываем соединение и очищаем ресурсы
    hid_close(device);
    hid_exit();
    
    printf("Connection closed\n");
    return 0;
}
