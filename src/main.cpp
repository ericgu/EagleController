#include <Arduino.h>
#include "Task.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include <NeoPixelBus.h>

extern "C" {
#include "user_interface.h"
}

#include "PersistentStorage.h"
#include "WifiHandler.h"
#include "PixelHandler.h"
#include "taskProcessMessages.h"
#include "webServer.h"

PersistentStorage persistentStorage;

const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266
const char* Version = "1.3";

PixelHandler pixelHandler(&persistentStorage, PixelPin);
WifiHandler wifiHandler;
TaskManager taskManager;

TaskProcessMessages taskProcessMessages(LED_BUILTIN, MsToTaskTime(10), &pixelHandler, &wifiHandler);

WebServer webServer;

#define NODE_TYPE_PRODUCTION 1
#define NODE_TYPE_LOCAL_NET 2
#define NODE_TYPE_AP_NET_FORGETFUL 3

int nodeType = 2;

DNSServer dnsServer;

void setup() { 
  Serial.begin(115200);
  Serial.print("EagleDecorations Controller Version ");
  Serial.println(Version);
  //Serial.setDebugOutput(true);

  if (nodeType == NODE_TYPE_AP_NET_FORGETFUL)
  {
    strcpy(persistentStorage._ssid, "junkjunkjunk");  // set bad wifi params for testing...
    strcpy(persistentStorage._storedAnimation, "invalid");  // set bad animation. 
    persistentStorage._ledCount = 33;
    persistentStorage.Save();
  }
  persistentStorage.Load();
  
  Serial.print("Starting: ");
  Serial.println(persistentStorage._ssid);

  pixelHandler.Init();

  taskProcessMessages.Init();

  webServer.SetHandlers(&wifiHandler, &pixelHandler);
  webServer.Init(); 

  taskManager.StartTask(&taskProcessMessages);

  wifiHandler.LoadConfiguration(&persistentStorage);
  if (nodeType == NODE_TYPE_LOCAL_NET)
  {
    wifiHandler.setParamsForDebug("ASUS", "Gunnerson", "EDP44");
  }
  
  if (nodeType == NODE_TYPE_AP_NET_FORGETFUL)
  {
    wifiHandler.SetSaveAfterProvisioning(false);
  }

  wifiHandler.loadNetworks();

  //wifiHandler.Init();
}

void loop() {
  taskManager.Loop();
}


