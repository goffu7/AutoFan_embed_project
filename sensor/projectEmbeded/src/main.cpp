#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// WiFi Credentials
const char* ssid = "Bozzy";        // แก้ไขเป็น Wi-Fi ของคุณ
const char* password = "785963yugi"; // แก้ไขเป็น Wi-Fi Password ของคุณ

// Sensor Pins
const int ldrPin = 32;  // GPIO pin for LDR
const int tempPin = 34; // GPIO pin for ET-ADC Sensor (Temperature)

// Variables
float temperature = 0.0;
int ldrValue = 0;

// Web Server on port 80
AsyncWebServer server(80);

// Function to read temperature
float readTemperature() {
  int adcValue = analogRead(tempPin);
  float voltage = (adcValue / 4095.0) * 3.3; // Assuming 3.3V reference voltage
  float tempC = (voltage - 0.5) * 100.0;     // Convert voltage to temperature
  return tempC;
}

// Function to read LDR value
int readLDR() {
  return analogRead(ldrPin);
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP()); // Print the IP Address

  // Define Web Server Endpoints
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<html><head><title>ESP32 Sensor Dashboard</title></head><body>";
    html += "<h1>ESP32 Sensor Dashboard</h1>";
    html += "<p>LDR Value: " + String(ldrValue) + "</p>";
    html += "<p>Temperature: " + String(temperature, 2) + " &#8451;</p>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  // Start server
  server.begin();
}

void loop() {
  // Read LDR Value
  ldrValue = readLDR();

  // Read Temperature
  temperature = readTemperature();

  // Print to Serial Monitor
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  delay(1000); // Update every 1 second
}
