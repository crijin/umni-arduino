# umni-arduino

Included in this folder is 1 ".h" file and 2 ".ino" files. The "umni_scale.ino" is for use on production ESP8266 chips in tandem with the smart scale. "umni_no_scale.ino" is for testing use in the absence of a physical scale.

To enable usage of an Arduino ESP8266 chip, first download the Arduino IDE (https://www.arduino.cc/en/main/software). Then, once you've opened one of the files, navigate to "Arduino > Preferences > Additional Boards Manager URLs" and paste in "http://arduino.esp8266.com/stable/package_esp8266com_index.json"

You now have access to ESP8266 libraries. Under "Tools > Board" select "LOLIN(WEMOS) D1 R2 & mini".

Install needed libraries by going to "Tools > Manage Libraries..." Install the following libraries:
- "WifiManager" by tzapu
- "HX711" by Bogdan Necula
- "ArduinoHttpClient" by Arduino
- "ArduinoJson" by Benoit Blanchon

Within the WiFiManager library's files, replace "WiFiManager.h" with the provided WiFiManager.h file found in this repo to get the Umni branded wifi configuration page.

If you have your Arduino board handy, connect it to your PC via micro-USB cable and select the correct port under "Tools > Port". You're now able to flash the sketch file onto your Arduino!
