#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "RTClib.h"

FirebaseData fbdo;

#define WIFI_SSID "itera"
#define WIFI_PASSWORD "12345678"

#define FIREBASE_HOST "filterairdesa-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "ghr2tGl9KTAI6MKeU1rfeaZn0KCboPDFzmVZvlaO"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
RTC_DS3231 rtc;

const int pinvoltmeter = 34;
int nilaipotensio = 0;
int nilaifire = 0;

float R1 = 7510.0;
float R2 = 30000.0;
int nilaipinvoltmeter = 0;
float nilaivoltase;

String pesanerr;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup()
{
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  if (!rtc.begin())
  {
    Serial.print("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (rtc.lostPower())
  {
    Serial.print("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.print("SSD1306 allocation failed");
    for (;;)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print("coba firebase");
  display.display();
  delay(3000);
}

void loop()
{
  nilaipinvoltmeter = analogRead(pinvoltmeter);
  nilaivoltase = nilaipinvoltmeter * (5.0 / 1024) * ((R1 + R2) / R2);
  display.clearDisplay();
  DateTime now = rtc.now();

  if (Firebase.getInt(fbdo, "/counter"))
  {
    if (fbdo.dataType() == "int")
    {

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.print("nilai frbs = ");
      display.print(fbdo.intData());
      display.display();
      delay(70);
    }
  }
  else
  {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(fbdo.errorReason());
    display.display();
    delay(70);
  }

  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print(daysOfTheWeek[now.dayOfTheWeek()]);
  display.print(now.hour(), DEC);
  display.print(":");
  display.print(now.minute(), DEC);
  display.print(":");
  display.print(now.second(), DEC);
  display.display();

  display.setTextColor(WHITE);
  display.setCursor(0, 40);
  display.print("volt = ");
  display.print(nilaivoltase);
  display.display();
  delay(500);
}