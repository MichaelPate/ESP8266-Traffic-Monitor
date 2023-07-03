#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Replace with your network credentials
const char* ssid = "Hot and Spicy McChicken";
const char* password = "Acute420Knowledge";

#define SDA 4
#define SCL 5
#define BUTTON 0
#define GRN_LED 13
#define YLW_LED 14
#define RED_LED 12

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const long utcOffsetInSeconds = -21600;
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
long startTime; // the display will be on for the first 5 min after power cycle
#define PWR_ON_DISP_TIME 150 //300 = 5 min

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Serial.setDebugOutput(true);

  pinMode(BUTTON, INPUT);
  pinMode(GRN_LED, OUTPUT);
  pinMode(YLW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) {
      yield();
      // Don't proceed, loop forever
      digitalWrite(RED_LED, HIGH);
      delay(300);
      digitalWrite(RED_LED, LOW);
      delay(300);
    }
  }

  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Connecting...");
  display.display();
  delay(2000);

  digitalWrite(GRN_LED, HIGH);
  digitalWrite(YLW_LED, HIGH);
  digitalWrite(RED_LED, HIGH);
  delay(1000);
  digitalWrite(GRN_LED, LOW);
  digitalWrite(YLW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  delay(1000);

  Serial.println();
  Serial.println();
  Serial.println();

  //Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(WiFi.localIP());
  display.display();
  delay(3000);

  timeClient.begin();
  timeClient.update();
  startTime = timeClient.getEpochTime();
}

void loop() {
  // put your main code here, to run repeatedly:

  // Only when we have internet
  if ((WiFi.status() == WL_CONNECTED)) {
    // Prepare the POST request body
    StaticJsonDocument<300> jsonRequestBody;
    char json[] = "{\"origin\": {\"location\": {\"latLng\": {\"latitude\": 40.155543,\"longitude\": -105.048291}}},\"destination\": {\"location\": {\"latLng\": {\"latitude\": 40.560708,\"longitude\": -105.028877}}},\"travelMode\": \"DRIVE\",\"routingPreference\": \"TRAFFIC_AWARE\",\"computeAlternativeRoutes\": false,\"routeModifiers\": {\"avoidTolls\": false,\"avoidHighways\": false,\"avoidFerries\": false},\"languageCode\": \"en-US\",\"units\": \"IMPERIAL\"}";
    deserializeJson(jsonRequestBody, json);
    JsonObject root = jsonRequestBody.as<JsonObject>();

    // Setup the HTTP connection
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    // Ignore SSL certificate validation
    client->setInsecure();
    //create an HTTPClient instance
    HTTPClient https;
    //Initializing an HTTPS communication using the secure client
    Serial.print("Preparing [HTTPS] POST.\n");

    // If we have our HTTP connection we can continue
    if (https.begin(*client, "https://routes.googleapis.com/directions/v2:computeRoutes")) {  // HTTPS
      // Add header information
      https.addHeader("Content-Type", "application/json");
      https.addHeader("X-Goog-Api-Key", "AIzaSyBUdJXMeCrOQZyBVrKHxI_kFj5uY1f8uIs"); //this key is no longer valid so try all you want
      https.addHeader("X-Goog-FieldMask", "routes.duration,routes.distanceMeters,routes.polyline.encodedPolyline");

      // send the POST request
      String requestString;
      serializeJson(jsonRequestBody, requestString);
      https.useHTTP10(true);
      Serial.print("Sending [HTTPS] POST to https://routes.googleapis.com/directions/v2:computeRoutes over HTTP1.0.\n");
      https.POST(requestString);

      // get the response as a stream and parse it for our route
      Stream& response = https.getStream();
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, response);
      JsonArray routes = doc["routes"];
      JsonObject route = routes[0];
      Serial.print("Raw route time: ");
      Serial.println(route["duration"].as<String>());

      // Extract the duration (in seconds) as an integer
      String durSeconds = route["duration"].as<String>();
      int durationSeconds = durSeconds.substring(0, durSeconds.length() - 1).toInt();
      //Serial.println(durationSeconds);

      // Convert to minutes
      int durationMinutes = durationSeconds / 60;
      //Serial.println(durationMinutes);

      // Display everything on the screen only if its the first time after reset or the button is pressed
      timeClient.update();
      if (timeClient.getEpochTime() < (startTime + PWR_ON_DISP_TIME) || digitalRead(BUTTON) == LOW) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println(WiFi.localIP());
        display.setCursor(0, 10);
        display.setTextSize(6);
        display.println(String(durationMinutes));
        display.setTextSize(1);
        display.setCursor(110, 30);
        display.println("min");
        display.setCursor(90, 0);
        display.println(timeClient.getHours());
        display.setCursor(105, 0);
        display.println(":");
        display.setCursor(116, 0);
        display.println(timeClient.getMinutes());
        display.display();
      }
      else {
        // Avoid running the display for too long
        display.clearDisplay();
        display.display();
      }
      

      if (durationMinutes < 40) {
        digitalWrite(GRN_LED, HIGH);
        digitalWrite(YLW_LED, LOW);
        digitalWrite(RED_LED, LOW);
      }
      if (durationMinutes >= 40 && durationMinutes < 50) {
        digitalWrite(GRN_LED, LOW);
        digitalWrite(YLW_LED, HIGH);
        digitalWrite(RED_LED, LOW);
      }
      if (durationMinutes >= 50) {
        digitalWrite(GRN_LED, LOW);
        digitalWrite(YLW_LED, LOW);
        digitalWrite(RED_LED, HIGH);
      }
    }
    // If we dont then let the user know
    else {
      Serial.println("Could not connect.");
    }

  }
  // Wait for a bit before doing it all again
  Serial.println("\nWaiting for a bit to do it again.");
  delay(800);
}
