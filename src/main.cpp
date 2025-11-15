#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

// ---------- USER SETTINGS (PINS / LED COUNT) ----------
#define MOTOR_PWM_PIN    6    // PWM-fähiger GPIO des ESP32-C3 -> Motortreiber
#define LED_PIN          7    // WS2812 Datenpin
#define WS2812_COUNT     4

// ---------- PWM/LEDC SETTINGS ----------
const uint8_t  PWM_CHANNEL   = 0;
const uint32_t PWM_FREQUENCY = 20000;  // 20 kHz
const uint8_t  PWM_RES_BITS  = 10;     // 0..1023
const uint16_t PWM_MAX       = (1 << PWM_RES_BITS) - 1;

// ---------- GLOBALS ----------
Preferences prefs;
WebServer server(80);
Adafruit_NeoPixel strip(WS2812_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

enum Effect : uint8_t { EFFECT_SOLID = 0, EFFECT_RAINBOW = 1, EFFECT_BREATH = 2, EFFECT_CHASE = 3 };

struct State {
  uint16_t speed = 0;         // 0..1023
  uint8_t  brightness = 128;  // 0..255
  uint32_t color = 0xFF8800;  // RGB 24-bit
  Effect   effect = EFFECT_SOLID;
  bool     wifiAP = true;     // true=AP, false=STA
  String   ssid = "speeed";
  String   pass = "The_Integtal_is_Real";
} S;

uint32_t tPrev = 0;
uint16_t rainbowHue = 0;
int      breathPhase = 0;
uint16_t chaseIndex = 0;

// ---------- HELPERS ----------
uint32_t htmlColorToRGB(uint32_t c) { return c & 0xFFFFFF; }

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
    for(uint16_t i=0;i<strip.numPixels();i++){
      strip.setPixelColor(i, (i == chaseIndex) ? S.color : 0x000000);
    }
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

// ---------- WIFI ----------
void startAP(){
  String apSsid = "Turntable-" + String((uint32_t)ESP.getEfuseMac(), HEX).substring(4).substring(0,4);
  const char* apPass = "turntable";
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid.c_str(), apPass);
}

bool startSTA(const String& ssid, const String& pass){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  unsigned long start = millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-start < 10000){ delay(200); }
  return WiFi.status()==WL_CONNECTED;
}

void applyWiFiMode(){
  if(S.wifiAP){ startAP(); }
  else {
    if(!startSTA(S.ssid, S.pass)){
      startAP(); S.wifiAP = true;
    }
  }
}

// ---------- HTTP ----------
void sendIndex(){
  // Versuche index.htm, dann index.html
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
  doc["wifiMode"] = S.wifiAP ? "AP" : "STA";
  doc["ssid"] = S.ssid;
  doc["pass"] = "";
  doc["ip"] = S.wifiAP ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
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

void handleWiFiPOST(){
  if (server.hasArg("plain")){
    StaticJsonDocument<512> doc;
    if(deserializeJson(doc, server.arg("plain"))==DeserializationError::Ok){
      String mode = doc["mode"] | "AP";
      String ssid = doc["ssid"] | "";
      String pass = doc["pass"] | "";

      S.wifiAP = (mode != "STA");
      if(mode=="STA"){
        S.ssid = ssid; S.pass = pass;
        prefs.putBool("wifiAP", false);
        prefs.putString("ssid", S.ssid);
        prefs.putString("pass", S.pass);
      } else {
        prefs.putBool("wifiAP", true);
      }
      server.send(200, "application/json", "{\"msg\":\"Neu verbinden…\"}");
      delay(500);
      ESP.restart();
      return;
    }
  }
  server.send(400, "application/json", "{\"msg\":\"Bad request\"}");
}

void setup(){
  Serial.begin(115200); delay(100);

  // Filesystem
  if(!LittleFS.begin()){
    Serial.println("LittleFS mount failed!");
  }

  // PWM
  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RES_BITS);
  ledcAttachPin(MOTOR_PWM_PIN, PWM_CHANNEL);
  applyPWM();

  // LEDs
  strip.begin(); strip.show();
  strip.setBrightness(S.brightness);

  // NVS
  prefs.begin("turntable", false);
  S.wifiAP = prefs.getBool("wifiAP", true);
  S.ssid   = prefs.getString("ssid", "");
  S.pass   = prefs.getString("pass", "");

  // WiFi
  applyWiFiMode();

  // HTTP
  server.on("/", HTTP_GET, sendIndex);
  server.on("/index.htm", HTTP_GET, sendIndex);
  server.on("/api/state", HTTP_GET, handleStateGET);
  server.on("/api/state", HTTP_POST, handleStatePOST);
  server.on("/api/wifi",  HTTP_POST, handleWiFiPOST);
  server.onNotFound([](){ server.send(404, "application/json", "{\"error\":\"not found\"}"); });
  server.begin();

  // --- NEU: Serieller Statusausgabe ---
  Serial.println("=================================");
  Serial.println("Turntable Controller gestartet!");
  if(S.wifiAP){
    Serial.println("Modus: Access Point");
    Serial.print("SSID: Turntable-");
    Serial.println(String((uint32_t)ESP.getEfuseMac(), HEX).substring(4).substring(0,4));
    Serial.println("Passwort: turntable");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Modus: Station (WLAN)");
    Serial.print("Verbunden mit SSID: ");
    Serial.println(S.ssid);
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());
  }
  Serial.println("Webinterface öffnen im Browser → http://<IP>");
  Serial.println("=================================");
}


void loop(){
  server.handleClient();
  updateLEDs();
}
