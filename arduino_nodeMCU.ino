#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTTYPE DHT11
#define DHTPIN 19
DHT dht(DHTPIN, DHTTYPE);

float Temperature;
float Humidity;

const char* ssid = ""; // Tu Wifi SSID
const char* password = ""; // Tu contraseña

WiFiServer server(80);

const char* ntpServer = "europe.pool.ntp.org";
const int timeZone = 7200;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, timeZone);

char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

unsigned long previousMillis = 0;
const unsigned long interval = 300000;

String Token = "";
void setup() {

  Serial.begin(115200);
  pinMode(DHTPIN, INPUT);

  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  display.display();
  delay(2000);
  display.clearDisplay();

  WiFi.begin(ssid, password);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("  ROOM - STATION v2  ");
  display.setCursor(0, 7);
  display.print("---------------------");
  display.setCursor(0, 16);
  display.println("    Connecting to    ");
  display.setCursor(0, 26);
  display.println("      CW-75383_1     ");

  if (WiFi.status() != WL_CONNECTED) {
    delay(500);
  } else {
    server.begin();
  }
  delay(500);


  timeClient.begin();
  delay(500);

  display.setCursor(0, 36);
  display.println("    Connecting to    ");
  display.setCursor(0, 47);
  display.println(" europe.pool.ntp.org ");
  display.display();

  delay(700);
  timeClient.update();
  delay(500);

  Token = getToken();
  delay(500);
}

void loop() {
  Temperature = dht.readTemperature();
  Humidity = dht.readHumidity();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sendSensorData(Token, Temperature, Humidity);
  }

  unsigned long timeRemaining = (interval - (currentMillis - previousMillis)) / 1000;  // Calcula el tiempo restante en segundos
  int minutes = timeRemaining / 60;                                                    // Calcula los minutos restantes
  int seconds = timeRemaining % 60;                                                    // Calcula los segundos restantes


  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1.0);
  display.setTextColor(SSD1306_WHITE);
  display.print("  ROOM - STATION v2  ");
  display.setCursor(0, 7);
  display.print("---------------------");

  display.setCursor(0, 16);
  display.print("   ");
  display.print(daysOfTheWeek[timeClient.getDay()]);
  display.print(" ");
  display.print(timeClient.getFormattedTime());
  display.print("   ");

  display.setTextSize(1);
  display.setCursor(0, 28);
  display.print(" T ");
  display.print(String(Temperature) + ((char)247) + "C");
  display.print("  H ");
  display.print(String(Humidity) + "% \n");

  display.setCursor(0, 38);
  display.print("Send in: ");
  display.print(minutes);
  display.print(" min ");
  display.print(seconds);
  display.println(" sec");

  display.setCursor(0, 48);
  display.println("---------------------");

  display.setCursor(0, 56);
  display.print("   ");
  display.print(WiFi.localIP());
  display.println("   ");
  display.display();
}
String getToken() {
  HTTPClient http;
  const char* host = ""; // dirección de tu api
  const int port = 0; // Puerto de tu api
  const char* endpoint = ""; // Endpoint de tu api
  String url = "http://" + String(host) + ":" + String(port) + String(endpoint);
  int httpResponseCode = http.begin(url);
  httpResponseCode = http.GET();
  if (httpResponseCode == HTTP_CODE_OK) {
    String response = http.getString();
    Serial.println("GET request successful");
    Serial.print("Token: ");

    int tokenIndex = response.indexOf("{\"token\":\"");
    if (tokenIndex != -1) {
      int endIndex = response.indexOf("\"", tokenIndex + 10);
      String token = response.substring(tokenIndex + 10, endIndex);
      Serial.println(token);
      return token;
    } else {
      Serial.println("Token not found");
    }
  } else {
    Serial.print("Error in GET request. Response code: ");
    Serial.println(httpResponseCode);
  }

  return "";
}

void sendSensorData(String token, float temp, float hum) {
  if (token != "") {
    HTTPClient http;
    const char* host = ""; // dirección de tu api
    const int port = 0; // Puerto de tu api
    const char* endpoint = ""; // Endpoint de tu api

    String url = "http://" + String(host) + ":" + String(port) + String(endpoint);

    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", token);

    String body = "temp=" + String(int(temp)) + "&hum=" + String(int(hum));
    Serial.println(body);
    int httpResponseCode = http.POST(body);

    if (httpResponseCode == HTTP_CODE_OK) {
      Serial.println("POST request sent successfully");
    } else {
      Serial.print("Error in POST request. Response code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
}

