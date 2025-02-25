#include <sys/_stdint.h>
#include "IPAddress.h"
#include "WiFi.h"
#include "AsyncUDP.h"
#include "esp_wifi.h"

// written for XIAO ESP32-C3/S3

// *************************************************************************************
// ******************************* USER SETTINGS/OPTIONS *******************************
// *************************************************************************************
// Choose one option, set Wifi details accordingly below
#define AP      // use AP mode for AIO v5.0 Proto Wifi Bridge
//#define STN

#ifdef AP
const char* ssid = "AgOpenGPS_net";
const char* password = "";
IPAddress myIP = { 192, 168, 137, 1 }; // IP of ESP32 AccessPoint, default: 192.168.137.1 to match Windows Hotspot scheme
#elif defined(STN)
const char* ssid = "AgOpenGPS_net";
const char* password = "";
IPAddress myIP = { 192, 168, 137, 79 }; // IP of ESP32 stn/client, default: 192.168.137.79 to match Windows Hotspot scheme
#endif
// *************************************************************************************
// ******************************* END OF USER SETTINGS ********************************
// *************************************************************************************


IPAddress netmask = { 255, 255, 255, 0 };
IPAddress udpSendIP;                   // assigned in wifi.ino, myIP.255

AsyncUDP UDPforModules;                // UDP object to send/receive PGNs
uint16_t udpListenPort = 9999;         // UDP port to listen for Module replies
uint16_t udpSendPort = 8888;           // UDP port to send to Modules listening

HardwareSerial SerialTeensy(1);        // Use Serial1 to avoid Serial0's debug/boot output msgs
byte SerialTeensyRX = D7;  // ESP RX pin connected to Teensy TX pin, D7 is Serial0 default, we'll remap to Serial1 to avoid extra Serial0 debug msgs
byte SerialTeensyTX = D6;  // ESP TX pin connected to Teensy RX pin, D6 is Serial0 default, we'll remap to Serial1 to avoid extra Serial0 debug msgs

void setup()
{
  delay(500);           // time for ESP32 power to stabilize
  pinMode(D8, OUTPUT);
  Serial.begin(115200);
  while (millis() < 3000 && !Serial);
  Serial.print("\r\n*******************************************\r\nESP32 Async UDP<->Serial Forwarder/Bridge for AoG PGNs - " __DATE__ " " __TIME__);
  Serial.print("\r\n - to be used on AiO v5.0 Proto\r\n");

  // ESP32-C3 default is 128, setRxBufferSize returns "0" if unsuccessful, otherwise returns the size of the new buffer
  uint16_t bufSize = SerialTeensy.setRxBufferSize(256);   // 128 should be plenty but why not use more
  Serial.print((String)"\r\nSerialTeensy RX buffer size: " + (bufSize == 0 ? 128 : bufSize));

  setupWifi();
  setupUDP();
  Serial.print("\r\n\nSetup complete\r\n*******************************************\r\n\n");

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

  // send AP WiFi telem to Teensy for displaying in Web UI
  static uint32_t lastHelloTime = 0;
  if ( (millis() - lastHelloTime) > 5000 ) {
    lastHelloTime = millis();
    uint8_t helloFromESP32[] = { 0x80, 0x81, 90, 90, 5, 0, 0, 0, 0, 0, 71 };

    union {   // both variables in the union share the same memory space
      byte array[4];
      uint32_t millis;
    } runtime;
    runtime.millis = millis();

    helloFromESP32[5] = runtime.array[0];
    helloFromESP32[6] = runtime.array[1];
    helloFromESP32[7] = runtime.array[2];
    helloFromESP32[8] = runtime.array[3];
    helloFromESP32[9] = numStns;

    SerialTeensy.write(helloFromESP32, sizeof(helloFromESP32));
    SerialTeensy.println();  // to signal end of PGN
  }

  #endif


  static uint32_t lastLEDTime = 0;
  static bool ledOn = true;
  if ( (millis() - lastLEDTime) > 500 ) {
    lastLEDTime = millis();
    digitalWrite(D8, ledOn);
    ledOn = !ledOn;
  }

  // AgIO--UDP:8888-->Teensy
  //     Teensy--serial-->ESP32
  //          ESP32--wifi:8888-->Modules
  if (SerialTeensy.available())
  {

    // Read serial bytes one at a time into buffer
    static uint8_t incomingBytes[50];
    static uint8_t incomingIndex;
    incomingBytes[incomingIndex++] = SerialTeensy.read();   // advance index counter after storing new serial byte
    /*Serial.print("\r\nindex: "); Serial.print(incomingIndex);
    Serial.print(" ");
    for (byte i = 0; i < incomingIndex; i++) {
      Serial.print(incomingBytes[i]);
      Serial.print(" ");
    }*/

    // Check for End-Of-PGN bytes [CR] [LF]
    if (incomingBytes[incomingIndex - 2] == 13 && incomingBytes[incomingIndex - 1] == 10)
    {
      // Verify the first two bytes are AOG PGN header bytes
      if (incomingBytes[0] == 128 && incomingBytes[1] == 129)
      {
        // ESP32--wifi:8888->Modules
        UDPforModules.writeTo(incomingBytes, incomingIndex - 2, udpSendIP, udpSendPort);  // repeat AOG:8888 PGNs from Teensy(AgIO) to WiFi modules

        // Pass data to USB for debug
        Serial.print("\r\nT41-s->E32-w:8888->Modules ");
        for (byte i = 0; i < incomingIndex - 2; i++) {
          Serial.print(incomingBytes[i]);
          Serial.print(" ");
        }
        Serial.print((String)" (" + SerialTeensy.available() + ")");  // usually 0 except with high data volume at low baud
      } else {
        Serial.print("\r\n\nCR/LF detected but NOT valid PGN ([0]/[1] bytes != 128/129)\r\n");
      }
      incomingIndex = 0;  // "reset" buffer
    }
  }
}

void clearBuffers() {
  SerialTeensy.flush();
  while (SerialTeensy.available()) SerialTeensy.read();
}
