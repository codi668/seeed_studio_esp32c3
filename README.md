# ESP32 Secure Web Server & HTTPS Client

![Last Commit](https://img.shields.io/github/last-commit/codi668/seeed_studio_esp32c3)
![Most Used Language](https://img.shields.io/github/languages/top/codi668/seeed_studio_esp32c3)
![GitHub Repo stars](https://img.shields.io/github/stars/codi668/seeed_studio_esp32c3?style=social)
![GitHub forks](https://img.shields.io/github/forks/codi668/seeed_studio_esp32c3?style=social)
![GitHub issues](https://img.shields.io/github/issues/codi668/seeed_studio_esp32c3)
![GitHub pull requests](https://img.shields.io/github/issues-pr/codi668/seeed_studio_esp32c3)


![Version](https://img.shields.io/badge/version-1.2.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## 📋 Überblick

Dieser Code stellt einen ESP32 als HTTPS-Client ein, um mit einem sicheren Server zu kommunizieren. Der ESP32 wird mit einem WLAN-Netzwerk verbunden, stellt eine sichere HTTPS-Verbindung zu einem externen Server her und sendet eine GET-Anfrage. Anschließend wird die Antwort des Servers im seriellen Monitor ausgegeben.

## 1. Einbinden von Bibliotheken

```cpp
#include <WiFi.h>
#include <WebServer.h> // Muss manuell installiert werden, falls nicht enthalten
#include <WiFiClientSecure.h>
#include "credentials.h" // Enthält WLAN-Zugangsdaten
```

- **WiFi.h**: Ermöglicht die WLAN-Verbindung.
- **WebServer.h**: Wird für Webserver-Funktionalität verwendet (in diesem Code aber nicht aktiv).
- **WiFiClientSecure.h**: Wird genutzt, um eine sichere HTTPS-Verbindung aufzubauen.
- **credentials.h**: Enthält WLAN-Zugangsdaten (SSID und Passwort).

## 2. WLAN-Verbindung herstellen

```cpp
Serial.println("Verbinde mit WLAN...");
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
}
Serial.println("
Verbunden!");
```

- Der Code verbindet den ESP32 mit einem WLAN-Netzwerk mit den in `credentials.h` hinterlegten Zugangsdaten.
- Wenn die Verbindung erfolgreich ist, wird dies im seriellen Monitor angezeigt.

## 3. Sichere Verbindung zu einem Server aufbauen

```cpp
WiFiClientSecure client;
client.setInsecure(); // ⚠️ Unsicher, aber nützlich für Tests
```

- **WiFiClientSecure** erstellt einen sicheren Client.
- `client.setInsecure()` deaktiviert die Zertifikatsprüfung (unsicher, aber nützlich für Tests).

## 4. Verbindung zum Zielserver herstellen

```cpp
Serial.print("Verbinde mit https://");
Serial.println(host);

if (!client.connect(host, httpsPort)) {
    Serial.println("Verbindung fehlgeschlagen!");
    return;
}
```

- Der ESP32 verbindet sich mit dem Server (z.B. `howsmyssl.com`).
- Wenn die Verbindung fehlschlägt, wird eine Fehlermeldung angezeigt.

## 5. HTTP GET-Anfrage senden

```cpp
client.print(String("GET ") + url + " HTTP/1.1
" +
             "Host: " + host + "
" +
             "User-Agent: ESP32
" +
             "Connection: close

");
```

- Der ESP32 sendet eine HTTP GET-Anfrage an die URL `/a/check` auf dem Server.
- `User-Agent: ESP32` wird hinzugefügt, um den Server zu informieren, dass die Anfrage von einem ESP32 stammt.

## 6. Antwort des Servers lesen

```cpp
while (client.connected()) {
    String line = client.readStringUntil('
');
    if (line == "
") break; // Header zu Ende
}

String payload = client.readString();
Serial.println("Antwort:");
Serial.println(payload);
```

- Der ESP32 liest die Antwort des Servers.
- Zuerst werden die Header übersprungen, dann wird der Payload (Inhalt der Antwort) in `payload` gespeichert und im seriellen Monitor angezeigt.



- In dieser Funktion passiert nichts, da der Code nur einmal ausgeführt wird. Der ESP32 bleibt nach der Verbindung zum Server in der `setup()`-Funktion und wartet auf eine Antwort.

## **Zusammenfassung**

- Der ESP32 verbindet sich mit einem WLAN und stellt eine HTTPS-Verbindung zu einem Server her.
- Eine GET-Anfrage wird gesendet und die Antwort wird im seriellen Monitor ausgegeben.
- Die `WiFiClientSecure`-Bibliothek wird verwendet, um die HTTPS-Verbindung sicher zu gestalten.

## **Verbesserungspotential**

- **Zertifikatsprüfung**: Statt `setInsecure()` sollte ein valides Root-Zertifikat verwendet werden.
- **Datenverarbeitung**: Die Serverantwort könnte weiterverarbeitet werden (z.B. JSON extrahieren).
- **Fehlerbehandlung**: Der Code könnte robuster gemacht werden, um Verbindungsprobleme besser zu handhaben.


## 📋 Komplette Installationsanleitung

### Voraussetzungen
1. **Hardware**:
    - ESP32 Board (z.B. ESP32 DevKitC)
    - Micro-USB Kabel
    - LED mit 220Ω Widerstand (optional)

2. **Software**:
    - [Visual Studio Code](https://code.visualstudio.com/)
    - [PlatformIO Extension](https://platformio.org/install/ide?install=vscode)

### 🛠 Schritt-für-Schritt Installation

1. **Projekt klonen**:
   ```bash
   git clone https://github.com/codi668/seeed_studio_esp32c3.git
   cd seeed_studio_esp32c3
   ```

2. **WLAN konfigurieren**:
   Erstelle eine neue Datei `include/credentials.h` mit:
   ```cpp
   const char* ssid = "DEIN_WLAN_NAME";
   const char* password = "DEIN_WLAN_PASSWORT";
   ```

3. **PlatformIO einrichten**:
    - Öffne das Projekt in VS Code.
    - Wähle das richtige Board in `platformio.ini`:
   ```ini
   [env:esp32dev]
   platform = espressif32
   board = esp32dev
   ```

4. **Dateisystem uploaden**:
   Platziere Webdateien im `data` Ordner.

   Führe aus:
   ```bash
   pio run --target uploadfs
   ```

5. **Firmware flashen**:
   Flashen Sie die Firmware auf das ESP32-Board:
   ```bash
   pio run --target upload
   ```

6. **Seriellen Monitor starten**:
   Um die Ausgabe zu überwachen und sicherzustellen, dass das Gerät mit dem WLAN verbunden ist, starten Sie den seriellen Monitor:
   ```bash
   pio device monitor
   ```

### 🔧 Troubleshooting

| Problem | Lösung |
| ------- | ------ |
| Upload fehlgeschlagen | Boot-Taste beim Upload gedrückt halten |
| WLAN verbindet nicht | `credentials.h` überprüfen |
| Certificate verify failed | Root-Zertifikat in Code aktivieren |

### 📂 Dateistruktur
```
├── data/                # Webdateien
│   ├── index.html
│   └── style.css
├── include/
│   └── credentials.h    # WLAN Einstellungen
├── lib/
├── src/
│   └── main.cpp         # Hauptprogramm
└── platformio.ini       # Build-Konfiguration
```

### 🌟 Erste Schritte
- Seriellen Monitor öffnen
- Warten auf WLAN-Verbindung
- IP-Adresse im Monitor notieren
- Im Browser öffnen: `http://<ESP_IP>`

### 📜 Lizenz
MIT License - Details in der LICENSE Datei


