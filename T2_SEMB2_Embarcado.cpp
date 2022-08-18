#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// Pin that the irrigation sensor is attached to:
#define IRRIGATION_SENSOR_PIN PIN_A0

// Wi-Fi connection data:
const char* ssid = "*********";
const char* password = "**********";

// Cloud endpoint address:
String serverName = "http://3.93.212.178:80/irrigator";

// Loop period parameters:
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

void setup() {
  // Setting up pins for sensor input and irrigation output:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(IRRIGATION_SENSOR_PIN, INPUT);

  Serial.begin(115200); 

  // Connecting to Wi-Fi:
  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {

    // Checking wifi connection before requesting:
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;

      http.begin(client, serverName.c_str());
      
      // Get last command sent by user from the cloud:
      int httpResponseCode = http.GET();
      

      if (httpResponseCode > 0) {
        Serial.print("\nCommand Request - Response code: ");
        Serial.println(httpResponseCode);
        
        String payload = http.getString();
        Serial.print("Command Request - Payload: ");        
        Serial.println(payload);

        // Writing last received command to the output pin:
        digitalWrite(LED_BUILTIN, payload.toInt());
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      
      // Send last value read from the sensor to the cloud:
      int reading = analogRead(IRRIGATION_SENSOR_PIN);
      Serial.print("Sensor reading: ");
      Serial.println(String(reading));
      httpResponseCode = http.POST(String(reading));
      
      if (httpResponseCode > 0) {
        Serial.print("\nSensor data update request - Response code: ");
        Serial.println(httpResponseCode);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }

      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}