#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Types.h>
#include <WiFiManager.h>
#include <BaroManager.h>
#include <GPSManager.h>
#include <LEDManager.h>
#include <MessagesManager.h>

/* Settings */
const bool DEBUG = true;
const char FORMAT_MSG_GPS[] = "%lf%lf%f%f%f%f%f%f%lf";
const char FORMAT_MSG_BARO[] = "%f%lf";
const int THRESHOLD_HORIZONTAL_ACC = 20e3; // [mm]
const int TRIGGER_WIFI = 14;
const int LED_PIN = 5; // 5 or 16
const char* host = "192.168.42.1";
const uint32_t port = 5152;

WiFiManager wifiManager;
LEDManager led(LED_PIN);

BaroManager baro;
BaroData_t baroData;

GPSManager gps;
GPSData_t gpsData;

MessagesManager msg;

template <typename Generic>
void debugLog(Generic text) {
  if (DEBUG) {
    Serial1.print(text);
  }
}

void setWiFi()
{
  if (digitalRead(TRIGGER_WIFI) == LOW) {
    debugLog("Button pushed\n");
    led.blink(1,0);
    delay(500);
    wifiManager.setMinimumSignalQuality(50);
    if(!wifiManager.startCustomConfigPortal())
    {
      debugLog("Failed to connect\n");
    }
    msg.init(host,port);
    while(WiFi.status() != WL_CONNECTED) {
      delay(100);
    }
    led.blink(0,0);
  }
}
void checkWifiStatus()
{
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin();
    while(WiFi.status() != WL_CONNECTED) {
      delay(100); // on veut rester bloqué si on perd la connexion
    }
    msg.init(host,port);
  }
}

void sendAllMessages()
{
  if(gps.isReady())
  {
    gpsData = gps.getData();

    msg.send(MSG_GPS,
      FORMAT_MSG_GPS,
      gpsData.latitude/1e7,
      gpsData.longitude/1e7,
      gpsData.altitude/1000.0,
      gpsData.horizontalAcc/1000.0,
      gpsData.verticalAcc/1000.0,
      gpsData.northSpeed/1000.0,
      gpsData.eastSpeed/1000.0,
      gpsData.downSpeed/1000.0,
      gpsData.numberSV);
    debugLog("GPS  : lat=");
    debugLog(gpsData.latitude/1e7);
    debugLog("deg lon=");
    debugLog(gpsData.longitude/1e7);
    debugLog("deg height=");
    debugLog(gpsData.altitude);
    debugLog("mm hAcc=");
    debugLog(gpsData.horizontalAcc);
    debugLog("mm vAcc=");
    debugLog(gpsData.verticalAcc);
    debugLog("mm velN=");
    debugLog(gpsData.northSpeed);
    debugLog("mm/s velE=");
    debugLog(gpsData.eastSpeed);
    debugLog("mm/s velD=");
    debugLog(gpsData.downSpeed);
    debugLog("mm/s");
    debugLog(" numSV=");
    debugLog(gpsData.numberSV);
    debugLog("\n");

    baroData = baro.getData();

    msg.send(MSG_BARO,
      FORMAT_MSG_BARO,
      (float) baroData.pressureFiltered, // TODO : avoid cast
      (float) baroData.timestamp);
    debugLog("BARO : pressure =");
    debugLog(baroData.pressureFiltered);
    debugLog("Pa timestamp = ");
    debugLog(baroData.timestamp);
    debugLog("\n");

    gps.prepareNextMeasure();
    baro.updateTimestamp();
  }
}

void setup() {
  Serial.begin(230400);   // Setup GPS connection
  Serial.swap();
  Serial1.begin(230400);  // Setup UART-USB connection
  debugLog("******* BOOT *******\n");
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIGGER_WIFI,INPUT_PULLUP);
  Wire.begin(4,12);       // Setup Baro connection
  Wire.setClock(400000);
  //TODO : validate gps.flash();
  baro.init();
  setWiFi();
}

void loop() {
  checkWifiStatus();
  baro.process();
  gps.process();
  sendAllMessages();
  led.status(WiFi.status() == WL_CONNECTED,gpsData.horizontalAcc < THRESHOLD_HORIZONTAL_ACC);
}
