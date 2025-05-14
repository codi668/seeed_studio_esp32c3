
/*********************************************************************************************************
 * ===================================================================================================== *
 *                                                                  |  |                                 *
 *                           ██████╗  ██████╗  ██╗   ██╗ ██████╗  █████╗   ██████╗                       *
 *                          ██╔════╝  ██╔══██╗ ██║   ██║ ██╔══██╗ ██╔══██╗ ██╔══██╗                      *
 *                          ██║  ███╗ ██████╔╝ ██║   ██║ ██████╔╝ ███████║ ██████╔╝                      *
 *                          ██║   ██║ ██╔══██╗ ██║   ██║ ██╔══██╗ ██╔══██║ ██╔══██╗                      *
 *                          ╚██████╔╝ ██║  ██║ ╚██████╔╝ ███████║ ██║  ██║ ██║  ██║                      *
 *                           ╚═════╝  ╚═╝  ╚═╝  ╚═════╝  ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝                      *
 * ===================================================================================================== *
 *                                             G R U B Ä R                                               *
 * ===================================================================================================== *
 *                                      Version: 1.0.0                                                   *
 *                                      Autor: Thomas Gruber                                             *
 *                                      Beschreibung: Webserver controlled LED using littleFS            *
 *                                      Erstellt am: 22.3.2025                                           *
 * ===================================================================================================== *
 *                                    Willkommen bei Grubär!                                             *
 *                                    Diese Programm hostet einen Webserver auf dem ESP32,               *
 *                                    Auf der webseite kann mam eine LED ein- und ausschalten.           *
 *                                    Dafür wird die LittleFS verwendet, die Dateien zu managen          *
 * ===================================================================================================== *
 *********************************************************************************************************/


#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <esp_bt.h>

//#include "credentials.h"

const char* ssid = "HTL-WLAN-IoT";
const char* password = "HTL2IoT!";

#define LED_PIN 2

AsyncWebServer server(80);


void setup() {
    Serial.begin(115200);
    Serial.println("Setup started");
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    if (!LittleFS.begin()) {
        Serial.println("An error has occurred while mounting LittleFS");
        return;
    }
    Serial.println("LittleFS mounted successfully");

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.println("\nIP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });
    server.on("/led/on", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(LED_PIN, HIGH);
        request->send(LittleFS, "/index.html", "text/html");
    });
    server.on("/led/off", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(LED_PIN, LOW);
        request->send(LittleFS, "/index.html", "text/html");
    });

    server.on("/logo", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/grubar_trans.webp", "image/webp");
    });

    server.begin();
    Serial.println("HTTP server started");
}
void loop() {
}