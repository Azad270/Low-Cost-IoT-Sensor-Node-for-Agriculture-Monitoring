#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

// Pin Definitions
#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define SOIL_MOISTURE_PIN A0
#define RELAY_PIN D1

// WiFi Credentials
const char* ssid = "NEMz"; // Your WiFi SSID
const char* password = "password"; // Your WiFi Password
const char* serverName = "http://192.168.126.89:8000/board1_data"; // Replace with Flask server IP

// Calibration values for soil moisture sensor
#define SOIL_MOISTURE_DRY 1023  // Replace with value for completely dry soil
#define SOIL_MOISTURE_WET 300   // Replace with value for fully wet soil

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Ensure the pump is off at the start

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  dht.begin();
}

void loop() {
  int soilMoistureRaw = analogRead(SOIL_MOISTURE_PIN);

  // Convert raw soil moisture value to percentage
  int soilMoisturePercent = map(soilMoistureRaw, SOIL_MOISTURE_DRY, SOIL_MOISTURE_WET, 0, 100);
  soilMoisturePercent = constrain(soilMoisturePercent, 0, 100); // Ensure the value stays between 0 and 100

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Debugging logs
  Serial.print("Soil Moisture (%): ");
  Serial.println(soilMoisturePercent);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Temperature: ");
  Serial.println(temperature);

  // Control the relay based on soil moisture percentage
  if (soilMoisturePercent < 40) { // Threshold for turning on the pump
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Water pump ON");
  } else {
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Water pump OFF");
  }

  // Send data to the server
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");

    // Prepare JSON data
    String jsonData = "{\"soil_moisture\": \"" + String(soilMoisturePercent) + "%\",";
    jsonData += "\"humidity\": " + String(humidity) + ",";
    jsonData += "\"temperature\": " + String(temperature) + "}";
    Serial.print("JSON Data: ");
    Serial.println(jsonData);

    int httpResponseCode = http.POST(jsonData);
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending data: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }

  delay(5000); // Send data every 5 seconds
}
