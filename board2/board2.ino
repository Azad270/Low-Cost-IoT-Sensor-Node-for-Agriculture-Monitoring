#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Pin Definitions
#define ONE_WIRE_BUS D2   // Pin for DS18B20 sensor
#define MQ2_PIN A0        // Pin for MQ-2 sensor

// WiFi Credentials
const char* ssid = "NEMz"; // Your WiFi SSID
const char* password = "password"; // Your WiFi Password
const char* serverName = "http://192.168.126.89:8000/board2_data"; // Flask server IP

// Setup OneWire and DallasTemperature libraries
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Function to map raw sensor value to AQI
int calculateAQI(float sensorValue) {
  // Define the AQI breakpoints (adjust as per your calibration)
  int aqiLow = 0;
  int aqiHigh = 500;
  
  // Define the concentration breakpoints for AQI calculation (example values)
  float concentrationLow = 0;
  float concentrationHigh = 1023;  // Max analog value (10-bit ADC, 0-1023)
  
  // Map sensor value to AQI (simplified approach, adjust if necessary)
  int aqi = (int)((sensorValue - concentrationLow) / (concentrationHigh - concentrationLow) * (aqiHigh - aqiLow) + aqiLow);
  aqi = constrain(aqi, 0, 500);  // Ensure AQI stays within 0-500
  return aqi;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  sensors.begin();  // Start DS18B20 sensor
}

void loop() {
  sensors.requestTemperatures();  // Request temperature reading
  float temperature = sensors.getTempCByIndex(0);  // Get temperature from DS18B20 sensor
  
  // Read raw value from MQ-2 sensor
  int rawMQ2Value = analogRead(MQ2_PIN);
  
  // Calculate AQI based on raw value
  int airQualityAQI = calculateAQI(rawMQ2Value);

  // Debugging logs
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Raw MQ-2 Value: ");
  Serial.println(rawMQ2Value);
  Serial.print("Calculated AQI: ");
  Serial.println(airQualityAQI);

  // Send data to the server
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");

    // Prepare JSON data
    String jsonData = "{\"soil_temp\": " + String(temperature) + ",";
    jsonData += "\"air_quality\": " + String(airQualityAQI) + "}";

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
