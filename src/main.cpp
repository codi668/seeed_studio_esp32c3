#include <WiFi.h>
#include <Arduino.h>
//#include "cridentials.h" // wifi ssid and password

const char* ssid = "**************";
const char* password = "**************";

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
  } else {
    Serial.println("\nWLAN-Verbindung fehlgeschlagen");
  }
}

void loop() {
  // Hier könnte weitere Logik stehen, z. B. eine Wiederverbindung bei Verbindungsverlust
}
