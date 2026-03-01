#!/usr/bin/env node

const HID = require('node-hid');

// 1. Укажите VID и PID вашей клавиатуры
const VID = 0x5954; // Замените на ваш VID
const PID = 0x3; // Замените на ваш PID

// В QMK интерфейс Raw HID по умолчанию использует Usage Page 0xFF60
const QMK_RAW_HID_USAGE_PAGE = 0xFF60;

function sendRawHid(layout) {
    console.log(`Поиск устройства ${VID.toString(16)}:${PID.toString(16)}...`);
    
    // Получаем список всех подключенных HID-устройств
    const devices = HID.devices();
    
    // 2. Ищем нужный интерфейс (Raw HID) нашей клавиатуры
    const deviceInfo = devices.find(d => 
        d.vendorId === VID && 
        d.productId === PID && 
        d.usagePage === QMK_RAW_HID_USAGE_PAGE
    );

    if (!deviceInfo) {
        console.error("Устройство или Raw HID интерфейс не найдены!");
        return;
    }

    console.log(`Найден Raw HID интерфейс: ${deviceInfo.path}`);

    try {
      // 3. Подключаемся к устройству по его пути
      const device = new HID.HID(deviceInfo.path);
      console.log("Устройство успешно открыто.");

      // 4. Формируем пакет данных
      // node-hid ожидает массив, где первый элемент — Report ID (0x00).
      // QMK ожидает 32 байта данных. Итого: 33 байта.
      const data = new Array(33).fill(0);
      
      data[0] = 0x00; // Report ID
      data[1] = 0x42; // Ваша команда (например, 0x11)
      data[2] = layout.charCodeAt(0);
      data[3] = layout.charCodeAt(1);

      // 5. Отправляем данные
      const bytesWritten = device.write(data);
      console.log(`Отправлено ${bytesWritten} байт.`);

      // 6. Закрываем соединение
      device.close();
      console.log("Соединение закрыто.");

    } catch (error) {
      console.error("Ошибка при работе с устройством:", error);
    }
}

let layout = process.argv[2];

if (!layout) {
    console.error("Язык не передан!");
    process.exit(1);
}
layout = layout.padStart(2, " ").substring(0, 2)

console.log(`Отправляем на клавиатуру сигнал для языка: ${layout}`);

sendRawHid(layout);
