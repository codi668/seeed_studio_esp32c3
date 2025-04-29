#include <WiFi.h>
#include <WebServer.h> // <- Muss manuell installiert werden, wenn nicht dabei

#include <WiFiClientSecure.h>
#include "credentials.h" // Enthält WLAN-Zugangsdaten

// WLAN-Zugang
//const char* ssid = "********";
//const char* password = "**************";

// Zielserver und Pfad
const char* host = "www.howsmyssl.com";
const char* url = "/a/check";
const int httpsPort = 443;

// Root-Zertifikat (z. B. von ISRG Root X1 für Let's Encrypt)

/*const char* root_ca = R"EOF(
-----BEGIN CERTIFICATE-----
MIIEKzCCAxOgAwIBAgIUM5E73MesV7C3fVVwsISYCcTvhHcwDQYJKoZIhvcNAQEL und so weiter
-----END CERTIFICATE-----
)EOF";*/


void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Verbinde mit WLAN...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nVerbunden!");

    WiFiClientSecure client;
    client.setInsecure(); // ⚠️ Unsicher, aber nützlich für Tests

    Serial.print("Verbinde mit https://");
    Serial.println(host);

    if (!client.connect(host, httpsPort)) {
        Serial.println("Verbindung fehlgeschlagen!");
        return;
    }

    // HTTP GET Anfrage senden
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: ESP32\r\n" +
                 "Connection: close\r\n\r\n");

    // Antwort lesen
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") break; // Header zu Ende
    }

    String payload = client.readString();
    Serial.println("Antwort:");
    Serial.println(payload);
}

void loop() {
    // nichts tun
}
