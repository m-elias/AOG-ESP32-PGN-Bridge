#include <sys/_stdint.h>
#include "IPAddress.h"
#include "WiFi.h"
#include "AsyncUDP.h"

// written for XIAO ESP32-C3

// *************************************************************************************
// ******************************* USER SETTINGS/OPTIONS *******************************
// *************************************************************************************
// Choose one option, set Wifi details accordingly below
#define AP      // AP for AIO v5.0 Wifi Bridge
//#define STN

#ifdef AP
const char* ssid = "AgOpenGPS_net";
const char* password = "";
IPAddress myIP = { 192, 168, 137, 1 }; // IP of ESP32 AccessPoint, default: 192.168.137.1 to match Windows Hotspot scheme
#elif defined(STN)
//const char* ssid = "other";
//const char* password = "PW";
const char* ssid = "AgOpenGPS_net";
const char* password = "";
IPAddress myIP = { 192, 168, 137, 79 }; // IP of ESP32 stn/client, default: 192.168.137.79 to match Windows Hotspot scheme
#endif
// *************************************************************************************
// ********************************** END OF SETTINGS **********************************
// *************************************************************************************


IPAddress netmask = { 255, 255, 255, 0 };
IPAddress udpSendIP;                   // assigned in wifi.ino, myIP.255

AsyncUDP UDPforModules;                // UDP object to send/receive PGNs
uint16_t udpListenPort = 9999;         // UDP port to listen for Module replies
uint16_t udpSendPort = 8888;           // UDP port to send to Modules listening

HardwareSerial SerialTeensy(1);        // Use Serial1 to avoid Serial0's debug/boot output msgs
byte SerialTeensyRX = D7;  // ESP RX pin connected to Teensy TX pin, D7 is Serial0 default, we'll remap to Serial1 to avoid extra Serial0 debug msgs
byte SerialTeensyTX = D6;  // ESP TX pin connected to Teensy RX pin, D6 is Serial0 default, we'll remap to Serial1 to avoid extra Serial0 debug msgs

bool debug = true;
uint8_t ver = 11;

void setup()
{
  delay(250);           // time for power to stabilize
  Serial.begin(115200);
  Serial.print("\r\n*******************************************\r\nESP32 Async UDP<->Serial Forwarder/Bridge for AoG PGNs - " __DATE__ " v");
  Serial.print(ver);
  Serial.print("\r\n - to be used on AiO v5.0a\r\n");

  // ESP32-C3 already uses 128, setRxBufferSize returns "0" if unsuccesful, otherwise returns the size of the new buffer
  uint16_t bufSize = SerialTeensy.setRxBufferSize(256);   // 128 should be plenty but why not use more
  Serial.print((String)"\r\nSerialTeensy RX buffer size: " + (bufSize == 0 ? 128 : bufSize));

  setupWifi();
  setupUDP();
  Serial.print("\r\n\nSetup complete\r\n*******************************************\r\n\n\n\n\n");

  SerialTeensy.begin(460800, SERIAL_8N1, SerialTeensyRX, SerialTeensyTX);
  delay(50);
  clearBuffers();   // clear out SerialTeensy buffers for a clean(er) start
}



void loop()
{
  yield();
  if (Serial.available()) Serial.write(Serial.read());  // just for testing

  #ifdef AP
  static uint8_t numStns = 0;
  if (WiFi.softAPgetStationNum() != numStns) {
    Serial.print("\r\nNum Stns: ");
    Serial.println(WiFi.softAPgetStationNum());
    numStns = WiFi.softAPgetStationNum();
  }
  #endif

  // AgIO--ethernet:8888-->Teensy--serial-->ESP32--wifi:8888-->Modules
  if (SerialTeensy.available())
  {
    static uint8_t incomingBytes[50];
    static uint8_t incomingIndex;
    incomingBytes[incomingIndex++] = SerialTeensy.read();
    /*Serial.print("\r\nindex: "); Serial.print(incomingIndex);
    Serial.print(" ");
    for (byte i = 0; i < incomingIndex; i++) {
      Serial.print(incomingBytes[i]);
      Serial.print(" ");
    }*/

    if (incomingBytes[incomingIndex - 2] == 13 && incomingBytes[incomingIndex - 1] == 10)
    {
      if (incomingBytes[0] == 128 && incomingBytes[1] == 129)
      {
        // ESP32--wifi:8888->Modules
        UDPforModules.writeTo(incomingBytes, incomingIndex - 2, udpSendIP, udpSendPort);  // repeat AOG:8888 PGNs from Teensy(AgIO) to WiFi modules

        //pass data to USB for debug
        Serial.print("\r\nT41-s->E32-w:8888->Modules ");
        for (byte i = 0; i < incomingIndex - 2; i++) {
          Serial.print(incomingBytes[i]);
          Serial.print(" ");
        }
        Serial.print((String)" (" + SerialTeensy.available() + ")");  // usually 0 except with high data volume at low baud
      } else {
        Serial.print("\r\n\nCR/LF detected but [0]/[1] bytes != 128/129\r\n");
      }
      incomingIndex = 0;  // "reset" buffer
    }
  }
}

void clearBuffers() {
  SerialTeensy.flush();
  while (SerialTeensy.available()) SerialTeensy.read();
}
