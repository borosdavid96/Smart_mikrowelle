/*
https://randomnerdtutorials.com/solved-reconnect-esp32-to-wifi/

//wifi ip
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


*/

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <iostream>


#include <WiFi.h>
#include "time.h"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define uS_TO_M 60000000  //Conversion micro seconds to seconds
unsigned long time_now = 0;
unsigned long timer = 0;
#define BUTTON_PIN 33  // GIOP21 pin connected to button
#define relay1 5
#define relay2 18
#define relay3 26

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool mikroison = false;

const char* ssid = "Boros";
const char* password = "boroszoltan";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

const int REFRESH_INTERVAL = 4000;  // Update rate in milliseconds

int mp = 0;
int p = 0;


void time_reset() {
  mp = 0;
  p = 0;
}

void time_add() {
  mp += 30;
  //mikro szabalyok
  if (p >= 7) {
    p += 1;
    mp = 0;
  } else {
    if (mp / 60 == 1) {
      p += 1;
      mp = 0;
    }
  }

  if (p > 30) {
    p = 0;
  }
}

void mikrotime() {

  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(10, 30);
  // Display static text
  display.println(p);
  display.display();

  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(50, 30);
  // Display static text
  display.println(":");
  display.display();

  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(70, 30);
  // Display static text
  display.println(mp);
  display.display();
}

void relays_off() {
  digitalWrite(relay1, HIGH);  //mikro //fordított logika miatt magasra állítás (tehát kikapcsolt állapotban tartás
  digitalWrite(relay2, HIGH);  //lampa
  digitalWrite(relay3, HIGH);
}



void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to ask the time");

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10,0);
    // Display static text
    display.println("NO TIME");
    display.display();
    return;
  }

  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(&timeinfo, "%B%d");



  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 40);
  // Display static text
  display.println(&timeinfo, "%H:%M:%S");
  display.display();
}



void setup() {
  Serial.begin(115200);



  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
  }
  

  
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // config GIOP21 as input pin and enable the internal pull-up resistor
  pinMode(relay1, OUTPUT);            //relay
  pinMode(relay2, OUTPUT);            //relay
  pinMode(relay3, OUTPUT);            //relay

  relays_off();

  //connect to WiFi
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("OKOS MIKRO v1.1     w/OTA");
  display.display();  
  
  delay(100);
  
  Serial.printf("Connecting to %s ", ssid);
  
  WiFi.begin(ssid, password);
  
  /* while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
   */
  time_now = millis();
  //varakozas wifire egy ideig ha nem megy akkor tovabblepes
   while (millis() < time_now + 5000 || WiFi.status() != WL_CONNECTED ){
     
   Serial.println("Wifi CONNECTTING");
   display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 10);
  // Display static text
  display.println("Wifi CONNECTING");
  display.display();
   }
  
  if(WiFi.status() == WL_CONNECTED){
  Serial.println("Wifi OK");
   display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 10);
  // Display static text
  display.println("Wifi OK");
  display.display();
    
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 30);
  // Display static text
  display.println("Loading..");
  display.display();

  //////////////////////////////////////////////////OTA///////////////////////////////////////////////////
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);
  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("esp32 okosmikro");
  // No authentication by default
  ArduinoOTA.setPassword("admin");
  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
        type = "filesystem";
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  ArduinoOTA.begin();
  //////////////////////////////////////////////////OTA///////////////////////////////////////////////////



  time_now = millis();
}

void loop() {

  if (!(touchRead(4) > 55)) {
    time_now = millis();
    mikrotime();  //ido hozzaadas kijelzes
    delay(100);  //lassitas a tul hamari hozzaadas miatt

    while (millis() < time_now + 2000) {

      if (!(touchRead(4) > 55)) {
        time_add();  //30mp hozzaadasa

        mikrotime();  //ido hozzaadas kijelzes
        time_now = millis();
      }
      delay(300);  //lassitas a tul gyors hozzaadasa miatt
    }
    if (mp > 1 || p >= 1) {
      //mikrózás
      while (digitalRead(BUTTON_PIN)) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0, 10);
        // Display static text
        display.println("Csukd     be  az     ajtot!");
        display.display();
      }
      int i = (p * 60000) + (mp * 1000);
      timer = millis() + i;
      Serial.println("Timer be:");
      Serial.println(i);
      Serial.println(p);
      Serial.println(mp);
      Serial.println(timer);
      Serial.println(millis());
      digitalWrite(relay1, LOW);
      mikroison = true;
    }
  }
  //ha kinyilik az ajto
  while (digitalRead(BUTTON_PIN)) {  //fordított logika volt a kapcsolonal ezert nincs negálás
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, LOW);
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5, 10);
    // Display static text
    display.println(" Ajto       nyitva");
    display.display();
    mikroison = false;
    time_reset();
  }
  //ha lejart az ido leall
  if (millis() > timer && mikroison) {
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    mikroison = false;
    Serial.println("Timer ki:");
    Serial.println(p);
    Serial.println(mp);
    Serial.println(timer);
    Serial.println(millis());
    time_reset();
  }
  //ha mikro megy lampa be
  if (mikroison) {
    digitalWrite(relay2, LOW);
  } else {
    digitalWrite(relay2, HIGH);
  }
  
  //wifi reconnect
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("Reconnecting to WIFI network");
   // WiFi.disconnect();
    WiFi.reconnect();
  }
  
  display.clearDisplay();
  printLocalTime();
  ArduinoOTA.handle();
  delay(10);
}
