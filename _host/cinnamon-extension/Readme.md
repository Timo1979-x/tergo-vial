## cinnamon-to-tergo2
Расширение для Cinnamon desktop для отправки сообщений о смене раскладки в клавиатуру T-Ergo на основе прошивки QMK  
### Установка
- при желании можно заново скомпилировать программу для общения с клавиатурой:
```bash
apt install libhidapi-dev gcc -y 
gcc send_hid.c -o send_hid -lhidapi-hidraw -s
```
- в файле `cinnamon.yaml` в секции переменных указать "координаты" клавиатуры: vid (5954), pid(0003) и hid_raw_usage_page (FF60 по умолчанию для клавиатур на QMK)
- запустить `ansible-playbook cinnamon.yaml -K`


