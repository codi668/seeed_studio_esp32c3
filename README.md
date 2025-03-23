# ESP32 Webserver

![Last Commit](https://img.shields.io/github/last-commit/codi668/seeed_studio_esp32c3)
![Most Used Language](https://img.shields.io/github/languages/top/codi668/seeed_studio_esp32c3)
![GitHub Repo stars](https://img.shields.io/github/stars/codi668/seeed_studio_esp32c3?style=social)
![GitHub forks](https://img.shields.io/github/forks/codi668/seeed_studio_esp32c3?style=social)
![GitHub issues](https://img.shields.io/github/issues/codi668/seeed_studio_esp32c3)
![GitHub pull requests](https://img.shields.io/github/issues-pr/codi668/seeed_studio_esp32c3)



## Beschreibung
Dieses Projekt zeigt, wie man mit einem ESP32-Mikrocontroller einen einfachen Webserver hostet, über den eine LED ein- und ausgeschaltet werden kann. Der Webserver verwendet das LittleFS-Dateisystem, um HTML-Dateien und Bilder zu verwalten. Dieses Projekt ist ideal für Einsteiger, die lernen möchten, wie man einen Webserver auf einem ESP32 einrichtet und steuert.

## Funktionen
- **Webserver**: Hostet eine Webseite, auf der eine LED gesteuert werden kann.
- **LED-Steuerung**: Die LED kann über die Webseite ein- und ausgeschaltet werden.
- **LittleFS**: Verwendet das LittleFS-Dateisystem, um HTML-Dateien und Bilder zu speichern und zu verwalten.

## Hardware
- **ESP32**: Der Mikrocontroller, der den Webserver hostet.
- **LED**: Eine LED, die über den Webserver gesteuert wird.

## Software
- **PlatformIO**: Eine professionelle Entwicklungsumgebung für Embedded Systems, die in diesem Projekt verwendet wird.
- **LittleFS**: Ein Dateisystem, das auf dem ESP32 verwendet wird, um Dateien zu speichern und zu verwalten.

## Installation
1. **PlatformIO einrichten**:
   - Installiere PlatformIO, falls noch nicht geschehen. Es kann als Plugin für Visual Studio Code oder als eigenständige IDE verwendet werden.

2. **Projekt klonen**:
   - Klone dieses Repository oder lade den Code herunter:
     ```bash
     git clone -b advanced_webserver https://github.com/codi668/seeed_studio_esp32c3.git
     ```

3. **WiFi-Zugangsdaten anpassen**:
   - Öffne die Datei `src/main.cpp` und trage deine WiFi-Zugangsdaten ein:
     ```cpp
     const char* ssid = "DEIN_WIFI_SSID";
     const char* password = "DEIN_WIFI_PASSWORT";
     ```

4. **Dateisystem-Image erstellen und hochladen**:
   - Erstelle ein Dateisystem-Image, das die HTML-Dateien und Bilder enthält.
   - Verwende das `LittleFS Upload Tool`, um das Dateisystem-Image auf den ESP32 hochzuladen.
   - Stelle sicher, dass der ESP32 mit dem Computer verbunden ist.
   - Führe den folgenden Befehl aus, um das Dateisystem-Image hochzuladen:
     ```bash
     pio run --target uploadfs
     ```

5. **Code hochladen**:
   - Verbinde deinen ESP32 mit dem Computer und wähle das richtige Board und den Port in PlatformIO aus.
   - Lade den Code auf den ESP32 hoch:
     ```bash
     pio run --target upload
     ```

## Verwendung
1. **Webseite aufrufen**:
   - Nachdem der ESP32 erfolgreich mit dem WiFi verbunden ist, öffne einen Browser und gib die IP-Adresse des ESP32 ein, die im Serial Monitor angezeigt wird.
   - Die Webseite sollte angezeigt werden, auf der du die LED ein- und ausschalten kannst.

2. **LED steuern**:
   - Klicke auf die Schaltflächen auf der Webseite, um die LED ein- oder auszuschalten.

## Dateistruktur
- **/data/index.html**: Die HTML-Datei für die Webseite.
- **/data/grubar_trans.webp**: Das Bild, das auf der Webseite angezeigt wird.
- **/src/main.cpp**: Der Hauptcode für den ESP32.

## Lizenz
Dieses Projekt ist unter der MIT-Lizenz lizenziert. Weitere Informationen findest du in der [LICENSE](LICENSE)-Datei.

## Autor
- **Thomas Gruber**

## Version
- **1.0.0**

## Erstellt am
- **22.03.2025**

---

Viel Spaß mit deinem ESP32 Webserver-Projekt! 🚀
