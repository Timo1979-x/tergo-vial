## cinnamon-to-tergo
Расширение для Cinnamon desktop для отправки сообщений о смене раскладки в клавиатуру T-Ergo на основе прошивки QMK  
## Установка
- установка необходимых библиотек
```bash
sudo apt install libhidapi-hidraw0
```
- Установка nodejs
```bash
apt install nodejs
cd /path/to/git/repo/_cinnamon-extension/cinnamon-to-tergo@tlukashevich.com
npm install

# Скопировать проект в папку расширений cinnamon:
cp -r /path/to/git/repo/_cinnamon-extension/cinnamon-to-tergo@tlukashevich.com ~/.local/share/cinnamon/extensions/
# или сделать ссылку:
ln -s /path/to/git/repo/_cinnamon-extension/cinnamon-to-tergo@tlukashevich.com ~/.local/share/cinnamon/extensions/cinnamon-to-tergo@tlukashevich.com
```
- настройка udev - дать доступ к устройству обычному пользователю. Проверить правильность Vendor ID/Product ID:
```bash
echo -e 'SUBSYSTEM=="hidraw", ATTRS{idVendor}=="5954", ATTRS{idProduct}=="0003", MODE="0666"\nSUBSYSTEM=="usb", ATTRS{idVendor}=="5954", ATTRS{idProduct}=="0003", MODE="0666"' | sudo tee /etc/udev/rules.d/99-qmk-raw-hid.rules
sudo udevadm control --reload-rules
sudo udevadm trigger
```
- **Переподключить клавиатуру!!!**
- Запустить отладчик Looking glass из состава Cinnamon (запустить команду `lg` или назначить и нажать горячую кнопку)
- Активировать расширение: Запустить "Параметры -> Расширения". Там должно быть наше расширение. Выделить его и нажать плюс внизу окна
- При активации Cinnamon может упасть, но потом всё работает
- Перезапустить Cinnamon. Это можно сделать без выхода:
  - из Looking glass -> Actions
  - Нажать Alt+F2 (или Win+R) и запустить команду `r` (да, одна буква)
- Логи можно смотреть в том же Looking glass

