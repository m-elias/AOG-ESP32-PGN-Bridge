![AgOpenGPS](https://github.com/m-elias/AOG-AiO-RVC-100hz/blob/main/media/agopengps%20name%20logo.png)
[AOG Download](https://github.com/farmerbriantee/AgOpenGPS/releases)<br>
[AOG Forum](https://discourse.agopengps.com/)<br>
[AOG YouTube](https://youtube.com/@AgOpenGPS)

# XIAO ESP32-C3 Wireless PGN Bridge/forwarder
There is a socket on the AIO v5.0a PCB for a XIAO ESP32-C3 to create a WiFi AP for wireless modules to connect & receive AOG PGNs. The PGNs received by the Teensy via Ethernet, are forwarded by the Teensy via serial to the ESP32, which in turn broadcasts them via WiFi. Modules that receives the wireless PGNs reply back to the ESP32 which forwards the replies back to the Teensy via serial, then the Teensy via Ethernet back to AOG.

AgIO--Eth:8888->Teensy--serial->ESP32--wifi:8888->Modules
Modules-->wifi:9999->ESP32--serial->Teensy--Eth:9999->AgIO

The firmware is in the [ESP32_SerialWifi_PGN_bridge folder](https://github.com/m-elias/AOG-AiO-RVC-100hz/tree/main/ESP32C3%20PGN%20Bridge/ESP32_SerialWifi_PGN_bridge)
I use Arduino IDE v2.3.1, esp32 by Expressif Systems v2.0.14

![ESP32 on v5.0z PCB](https://github.com/m-elias/AOG-ESP32-PGN-Bridge/blob/main/media/ESP32-C3%20on%20v5.0a%20PCB.jpg)
![XIAO ESP32C3 pinout](https://github.com/m-elias/AOG-ESP32-PGN-Bridge/blob/main/media/Seeed-XIAO-ESP32-C3-Pinout-1.jpg)





