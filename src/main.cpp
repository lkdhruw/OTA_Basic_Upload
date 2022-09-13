#include <Arduino.h>
#include "BluetoothSerial.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


unsigned int BAUD_RATE = 115200;
String ssid_ = "MSI_2020";
String password_ = "12345000";
const char* ssid = ssid_.c_str();
const char* password = password_.c_str();

BluetoothSerial SerialBT;

unsigned int counter=0;
double start_time;

// Operation Modes
enum OP_MODES {OTA_UPDATE, UPDATE_WIFI_SSID, UPDATE_WIFI_PASS,
              TYPE_WIFI_SSID, TYPE_WIFI_PASS, NONE};
OP_MODES op_mode = OTA_UPDATE;

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.setTimeout(100);
  SerialBT.begin("ESP 32");
  
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      SerialBT.println("Start updating " + type);
    })
    .onEnd([]() {
      SerialBT.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      SerialBT.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      SerialBT.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) SerialBT.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) SerialBT.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) SerialBT.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) SerialBT.println("Receive Failed");
      else if (error == OTA_END_ERROR) SerialBT.println("End Failed");
    });
  
  WiFi.mode(WIFI_OFF);
  
  delay(1000);
  
}

void loop() {
  
  if(SerialBT.connected()) {
    //TODO
    digitalWrite(2, HIGH);
  }
  while(SerialBT.available()) {
    // Message received from the Bluetooth serial
    String message = SerialBT.readString();
    if(message == "OTA Update\r\n")
    {
      op_mode = OTA_UPDATE;
      SerialBT.println("Checking WiFi connection...");
      int timeout = 20000;
      int start_time = millis();
      if(!WiFi.isConnected())
      {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        delay(2000);
        while (WiFi.waitForConnectResult() != WL_CONNECTED) {
          op_mode = NONE;
          if (millis() - start_time > timeout) {
            SerialBT.println("Connection timeout!");
            break;
          }
          delay(1000);
        }
      }
      
      if (op_mode == OTA_UPDATE)
      {
        SerialBT.println("Updating...");
        ArduinoOTA.begin();
        SerialBT.print("IP address: ");
        SerialBT.println(WiFi.localIP());
      }
      else
      {
        SerialBT.println("Connection Failed!");
      }
    }
    else if(message == "Show credentials\r\n")
    {
      SerialBT.printf("SSID: %s\r\n", ssid);
      SerialBT.printf("Password: %s\r\n", password);
    }
    else if(message == "WiFi Off\r\n")
    {
      WiFi.mode(WIFI_OFF);
      WiFi.setSleep(true);
      SerialBT.println("WiFi turned Off!");
    }
    else if(message == "Low Power\r\n")
    {
      setCpuFrequencyMhz(40);
      WiFi.mode(WIFI_OFF);
      WiFi.setSleep(true);
    }
    
    else if(message == "Restart\r\n")
    {
      ESP.restart();
    }
  }

  if(op_mode == OTA_UPDATE)
  {
    ArduinoOTA.handle();
    //op_mode = NONE;
  }
  delay(2);
}
