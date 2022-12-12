
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#include <WiFi.h>
#include "time.h"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define uS_TO_M 60000  //Conversion micro seconds to seconds
unsigned long time_now = 0;
unsigned long time_fromlastuse = 0;
unsigned long timer = 0;
unsigned long currentMillis = 0;
#define BUTTON_PIN 33   // GIOP21 pin connected to button (ajto biztonsagi kapcsolo)
#define BUTTON_PIN1 32  // (ido hozzaado)
#define BUTTON_PIN2 17  //  egyeb

#define relay1 5
#define relay2 18
#define relay3 26

struct tm timeinfo;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
bool mikroison = false;
bool mikroison1 = false;

const char* ssid = "Boros";
const char* password = "boroszoltan";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

const int REFRESH_INTERVAL = 60000;  // Update rate in milliseconds
unsigned long previousMillis = 0;

bool wifiwasdisconnected = 0;


int mp = 0;
int p = 0;
int mp_s = 0;
int p_s = 0;



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


void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wake_up_source;

  wake_up_source = esp_sleep_get_wakeup_cause();

  switch (wake_up_source) {
    case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wake-up from external signal with RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wake-up from external signal with RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wake up caused by a timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wake up caused by a touchpad"); break;
    default: Serial.printf("Wake up not caused by Deep Sleep: %d\n", wake_up_source); break;
  }
}

void deepsleep() { //az idomerest a leallitas utan nezzük, 10 óra utan alszik
  if (millis() - time_fromlastuse >  600 * uS_TO_M) {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 20);
    // Display static text
    display.println("Sleeping..ZzZzZ");
    display.display();

    esp_deep_sleep_start();
  }
}


void wifireconnect() {

  //WiFi.disconnect();
  WiFi.reconnect();

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Wifi      reconnect");
  display.display();
  delay(300);
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

  // if (WiFi.status() == WL_CONNECTED) {


  if (WiFi.status() != WL_CONNECTED) {
    /*display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 10);
    // Display static text
    display.println("Ask for  time");
    display.display();
    */
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 20);
    // Display static text
    display.println("NO TIME");
    display.display();
  } else {


    if (!getLocalTime(&timeinfo)) {  //if no time

      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(10, 20);
      // Display static text
      display.println("NO TIME");
      display.display();

      return;
    } else {

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
  }
  /* } else {
    //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(60, 40);
    // Display static text
    display.println("xxxx");
    display.display();

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 20);
    // Display static text
    display.println("NO TIME");
    display.display();
  }
  */
}




void setup() {
  Serial.begin(115200);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, LOW);

  //Displays the wake-up source
  print_wakeup_reason();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
  }


  pinMode(BUTTON_PIN, INPUT_PULLUP);   // config GIOP21 as input pin and enable the internal pull-up resistor
  pinMode(BUTTON_PIN1, INPUT_PULLUP);  // config GIOP21 as input pin and enable the internal pull-up resistor
  pinMode(BUTTON_PIN2, INPUT_PULLUP);  // config GIOP21 as input pin and enable the internal pull-up resistor
  pinMode(relay1, OUTPUT);             //relay
  pinMode(relay2, OUTPUT);             //relay
  pinMode(relay3, OUTPUT);             //relay

  relays_off();

  //connect to WiFi
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("OKOS MIKRO v1.3");
  display.display();
  delay(400);

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  /* while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wifi CONNECTED");
  */


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


  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  // Display static text
  display.println("Asking   for time");
  display.display();
  display.clearDisplay();

  getLocalTime(&timeinfo);

  time_now = millis();
}

void loop() {



  if (!(digitalRead(BUTTON_PIN1)) && !mikroison1) {  //ha a felso gomb megnyomodik
    time_now = millis();
    delay(200);  //lassitas a tul hamari hozzaadas miatt

    if (!(digitalRead(BUTTON_PIN2)) && !(digitalRead(BUTTON_PIN2))) {
      ESP.restart();
    }

    while (millis() < time_now + 2000) {
      if (!(digitalRead(BUTTON_PIN1))) {


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

      if (!mikroison) {
        timer = millis() + i;
      } else {
        timer += i;
      }
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
    mikroison1 = false;
    time_reset();
    time_fromlastuse = millis();
  }

  //ha lejart az ido leall
  if (millis() > timer && (mikroison || mikroison1)) {
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    mikroison = false;
    mikroison1 = false;
    Serial.println("Timer ki:");
    Serial.println(p);
    Serial.println(mp);
    Serial.println(timer);
    Serial.println(millis());
    time_reset();
    time_fromlastuse = millis();
  }

  //ha mikro megy lampa be
  if (mikroison || mikroison1) {
    digitalWrite(relay2, LOW);
  } else {
    digitalWrite(relay2, HIGH);
  }

  if ((mikroison || mikroison1) && (!digitalRead(BUTTON_PIN2))) {  //az also gombotol megall
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    mikroison = false;
    mikroison1 = false;
    time_reset();
    time_fromlastuse = millis();
    delay(700);
  }

  if (!digitalRead(BUTTON_PIN2) && (!(mikroison) && !(mikroison1))) {  //popcorn

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    // Display static text
    display.println("Popcorn");
    display.display();
    delay(600);

    if (digitalRead(BUTTON_PIN2)) {



      mp = 0;
      p = 3;

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
      mikroison1 = true;

      time_reset();
    }
  }



  if ((!(mikroison) && !(mikroison1))) {  // ha egy program sem megy

    currentMillis = millis();
    if (currentMillis - previousMillis >= REFRESH_INTERVAL) {  //intervall események
      if (WiFi.status() != WL_CONNECTED) {
        wifireconnect();
      }
    }


    deepsleep(); //csa akkor alszik ha nincs tevekenyseg 
    previousMillis = currentMillis;
  }




  printLocalTime();
  display.clearDisplay();
  ArduinoOTA.handle();
  delay(10);
}
