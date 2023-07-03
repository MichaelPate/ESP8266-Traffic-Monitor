#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Wire.h>
#include <ACROBOTIC_SSD1306.h>

// Replace with your network credentials
const char* ssid = "Hot and Spicy McChicken";
const char* password = "Acute420Knowledge";

#define SDA 2
#define SCL 0
#define BUTTON 14
#define GRN_LED 4
#define YLW_LED 5
#define RED_LED 12

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Serial.setDebugOutput(true);

  Wire.begin(SDA, SCL);
  oled.init();
  oled.clearDisplay();
  oled.setTextXY(0, 0);
  oled.putString("Loading");
  oled.setTextXY(1, 0);         // row 1 col 0
  oled.putString("Please Wait");

  pinMode(BUTTON, INPUT);
  pinMode(GRN_LED, OUTPUT);
  pinMode(YLW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

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
    oled.setTextXY(2, 0);         // row 1 col 0
    oled.putString("Connecting");
    delay(1000);
  }
  oled.setTextXY(2, 0);         // row 1 col 0
  oled.putString("Connected!   ");
  delay(1000);
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
      https.addHeader("X-Goog-Api-Key", "AIzaSyBUdJXMeCrOQZyBVrKHxI_kFj5uY1f8uIs");
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

      // Display everything on the screen
      oled.clearDisplay();
      oled.setTextXY(0, 0);
      oled.putString(String(durationMinutes));
    }
    // If we dont then let the user know
    else {
      Serial.println("Could not connect.");
    }

  }
  // Wait for a bit before doing it all again
  Serial.println("\nWaiting for a bit to do it again.");
  delay(1200);
}
