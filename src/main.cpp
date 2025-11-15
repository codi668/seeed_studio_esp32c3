#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

// ====== USER SETTINGS ======
#define MOTOR_PWM_PIN    D6
#define LED_PIN          10
#define WS2812_COUNT     4

// Compile-Time Default-WiFi (beim ersten Flashen anpassen)
#define WIFI_SSID_DEFAULT "speeed"
#define WIFI_PASS_DEFAULT "The_Integtal_is_Real"

// ====== PWM ======
const uint8_t  PWM_CHANNEL   = 0;
const uint32_t PWM_FREQUENCY = 20000;
const uint8_t  PWM_RES_BITS  = 10;
const uint16_t PWM_MAX       = (1 << PWM_RES_BITS) - 1;

// ====== GLOBALS ======
Preferences prefs;
WebServer server(80);
Adafruit_NeoPixel strip(WS2812_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

enum Effect : uint8_t { EFFECT_SOLID = 0, EFFECT_RAINBOW = 1, EFFECT_BREATH = 2, EFFECT_CHASE = 3 };

struct State {
  uint16_t speed = 0;
  uint8_t  brightness = 128;
  uint32_t color = 0xFF8800;
  Effect   effect = EFFECT_SOLID;
  String   ssid = "";
  String   pass = "";
} S;

uint32_t tPrev = 0;
uint16_t rainbowHue = 0;
int      breathPhase = 0;
uint16_t chaseIndex = 0;

// ====== LED Helpers ======
uint32_t htmlColorToRGB(uint32_t c){ return c & 0xFFFFFF; }
void applyPWM(){ ledcWrite(PWM_CHANNEL, S.speed); }

void applyLEDsSolid(){
  strip.setBrightness(S.brightness);
  for(uint16_t i=0;i<strip.numPixels();i++) strip.setPixelColor(i, S.color);
  strip.show();
}
void applyLEDsRainbow(uint32_t now){
  strip.setBrightness(S.brightness);
  for(uint16_t i=0;i<strip.numPixels();i++){
    uint16_t hue = (rainbowHue + i*65535/strip.numPixels()) & 0xFFFF;
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue)));
  }
  strip.show();
  if(now - tPrev >= 20){ rainbowHue += 256; tPrev = now; }
}
void applyLEDsBreath(uint32_t now){
  static uint32_t last = 0;
  if(now - last >= 12){
    breathPhase = (breathPhase + 1) % 512;
    last = now;
    float x = breathPhase < 256 ? breathPhase/255.0f : (511-breathPhase)/255.0f;
    uint8_t b = (uint8_t)(x*x * S.brightness);
    strip.setBrightness(b);
    for(uint16_t i=0;i<strip.numPixels();i++) strip.setPixelColor(i, S.color);
    strip.show();
  }
}
void applyLEDsChase(uint32_t now){
  static uint32_t last = 0;
  if(now - last >= 60){
    last = now;
    strip.setBrightness(S.brightness);
    for(uint16_t i=0;i<strip.numPixels();i++)
      strip.setPixelColor(i, (i == chaseIndex) ? S.color : 0x000000);
    strip.show();
    chaseIndex = (chaseIndex + 1) % strip.numPixels();
  }
}
void updateLEDs(){
  uint32_t now = millis();
  switch(S.effect){
    case EFFECT_SOLID:  applyLEDsSolid(); break;
    case EFFECT_RAINBOW:applyLEDsRainbow(now); break;
    case EFFECT_BREATH: applyLEDsBreath(now); break;
    case EFFECT_CHASE:  applyLEDsChase(now); break;
  }
}

// ====== Wi-Fi (STA ONLY) ======
bool connectSTA(const String& ssid, const String& pass, uint32_t timeout_ms=15000){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  uint32_t t0 = millis();
  Serial.print("Verbinde mit SSID '"); Serial.print(ssid); Serial.print("' ... ");
  while(WiFi.status()!=WL_CONNECTED && (millis()-t0)<timeout_ms){
    delay(250); Serial.print(".");
  }
  Serial.println();
  return WiFi.status()==WL_CONNECTED;
}

void ensureWiFi(){
  // Lade aus NVS, sonst Defaults
  S.ssid = prefs.getString("ssid", WIFI_SSID_DEFAULT);
  S.pass = prefs.getString("pass", WIFI_PASS_DEFAULT);

  if(!connectSTA(S.ssid, S.pass)){
    Serial.println("WLAN-Verbindung fehlgeschlagen. Erneuter Versuch alle 5 s.");
  }
}

// ====== HTTP ======
void sendIndex(){
  File f = LittleFS.open("/index.htm", "r");
  if(!f){ f = LittleFS.open("/index.html", "r"); }
  if(!f){ server.send(500, "text/plain", "UI not found in LittleFS"); return; }
  server.streamFile(f, "text/html; charset=utf-8");
  f.close();
}

void handleStateGET(){
  StaticJsonDocument<512> doc;
  doc["speed"] = S.speed;
  doc["brightness"] = S.brightness;
  doc["color"] = S.color;
  doc["effect"] = (uint8_t)S.effect;
  doc["ssid"] = S.ssid;
  doc["ip"] = WiFi.localIP().toString();
  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleStatePOST(){
  if (server.hasArg("plain")){
    StaticJsonDocument<512> doc;
    if(deserializeJson(doc, server.arg("plain"))==DeserializationError::Ok){
      if(doc.containsKey("speed")){
        uint16_t sp = doc["speed"];
        if(sp > PWM_MAX) sp = PWM_MAX;
        S.speed = sp; applyPWM();
      }
      if(doc.containsKey("brightness")) S.brightness = (uint8_t)doc["brightness"];
      if(doc.containsKey("color"))      S.color = htmlColorToRGB((uint32_t)doc["color"]);
      if(doc.containsKey("effect")){
        uint8_t e = doc["effect"];
        if(e <= EFFECT_CHASE) S.effect = (Effect)e;
      }
      server.send(200, "application/json", "{\"ok\":true}");
      return;
    }
  }
  server.send(400, "application/json", "{\"ok\":false}");
}

// WLAN-Daten ändern (nur STA; speichert in NVS und rebootet)
void handleWiFiPOST(){
  if (server.hasArg("plain")){
    StaticJsonDocument<384> doc;
    if(deserializeJson(doc, server.arg("plain"))==DeserializationError::Ok){
      String ssid = doc["ssid"] | "";
      String pass = doc["pass"] | "";
      if(ssid.length()){
        prefs.putString("ssid", ssid);
        prefs.putString("pass", pass);
        server.send(200, "application/json", "{\"msg\":\"WLAN gespeichert. Neustart…\"}");
        delay(400);
        ESP.restart();
        return;
      }
    }
  }
  server.send(400, "application/json", "{\"msg\":\"Bad request\"}");
}

// ====== SETUP / LOOP ======
void setup(){
  Serial.begin(115200); delay(100);

  if(!LittleFS.begin()){
    Serial.println("LittleFS mount failed!");
  }

  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RES_BITS);
  ledcAttachPin(MOTOR_PWM_PIN, PWM_CHANNEL);
  applyPWM();

  strip.begin(); strip.show();
  strip.setBrightness(S.brightness);

  prefs.begin("turntable", false);

  ensureWiFi();

  server.on("/", HTTP_GET, sendIndex);
  server.on("/index.htm", HTTP_GET, sendIndex);
  server.on("/api/state", HTTP_GET, handleStateGET);
  server.on("/api/state", HTTP_POST, handleStatePOST);
  server.on("/api/wifi",  HTTP_POST, handleWiFiPOST);
  server.onNotFound([](){ server.send(404, "application/json", "{\"error\":\"not found\"}"); });
  server.begin();

  Serial.println("=================================");
  Serial.println("Turntable Controller (STA only)");
  Serial.print("SSID: "); Serial.println(S.ssid);
  if(WiFi.status()==WL_CONNECTED){
    Serial.print("IP-Adresse: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("Nicht verbunden. Prüfe SSID/Passwort.");
  }
  Serial.println("=================================");
}

void loop(){
  // Falls Verbindung weg ist: alle 5s neuer Versuch
  static uint32_t lastRetry = 0;
  if(WiFi.status()!=WL_CONNECTED && millis()-lastRetry > 5000){
    lastRetry = millis();
    connectSTA(S.ssid, S.pass, 7000);
    if(WiFi.status()==WL_CONNECTED){
      Serial.print("Wieder verbunden. IP: "); Serial.println(WiFi.localIP());
    }
  }

  server.handleClient();
  updateLEDs();
}
