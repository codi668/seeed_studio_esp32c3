#include <WiFi.h>  // Für ESP32, verwende <ESP8266WiFi.h> für ESP8266
#include <WebServer.h>
#include <Arduino.h>
//#include "credentials.h"

const char* ssid = "DEIN_SSID";
const char* password = "DEIN_PASSWORT";

WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", "<h1>Hallo von deinem ESP Webserver!</h1>");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starte WLAN-Verbindung...");

  WiFi.begin(ssid, password);

  int maxTries = 20;
  while (WiFi.status() != WL_CONNECTED && maxTries > 0) {
    delay(1000);
    Serial.print(".");
    maxTries--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nVerbunden mit dem WLAN");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.begin();
    Serial.println("Webserver gestartet");
  } else {
    Serial.println("\nWLAN-Verbindung fehlgeschlagen");
  }
}

void loop() {
  server.handleClient();
}
