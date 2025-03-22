
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
//#include "credentials.h"

const char* ssid = "***************";
const char* password = "*****************";

#define LED_PIN 2

WebServer server(80);

void handleRoot() {
    Serial.println("Handling root request");
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
        Serial.println("Failed to open index.html");
        server.send(500, "text/plain", "Failed to open index.html");
        return;
    }
    String html = file.readString();
    server.send(200, "text/html", html);
    file.close();
    Serial.println("Root request handled successfully");
}

void handleLEDOn() {
    Serial.println("Turning LED on");
    digitalWrite(LED_PIN, HIGH);
    server.send(200, "text/plain", "LED is ON");
    Serial.println("LED is ON");
}

void handleLEDOff() {
    Serial.println("Turning LED off");
    digitalWrite(LED_PIN, LOW);
    server.send(200, "text/plain", "LED is OFF");
    Serial.println("LED is OFF");
}
void handleImage() {
    Serial.println("Handling image request");
    File file = LittleFS.open("/grubar_trans.webp", "r");
    if (!file) {
        server.send(500, "text/plain", "Failed to open Grubar.jpg");
        return;
    }
    server.streamFile(file, "image/webp");
    file.close();
    Serial.println("Image request handled successfully");
}

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

    server.on("/", handleRoot);
    server.on("/led/on", handleLEDOn);
    server.on("/led/off", handleLEDOff);
    server.on("/logo",handleImage);
    server.begin();
    Serial.println("HTTP server started");
}
void loop() {
    server.handleClient();
}