#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// WiFi Credentials
const char* ssid = "Aman";        // Your Wi-Fi SSID
const char* password = "045781023"; // Your Wi-Fi Password
// Firebase Credentials
#define FIREBASE_HOST "https://automatic-fan-90da8-default-rtdb.asia-southeast1.firebasedatabase.app/" // Replace with your Firebase database URL
#define FIREBASE_AUTH "AIzaSyDwVaA472bhGs4-QL46u6YiLTSHwgG2iiY"         // Replace with your Firebase secret or web API key

// Sensor Pins
const int LDR_PIN = 32;  // GPIO pin for LDR
const int TEMP_PIN = 34; // GPIO pin for ET-ADC Sensor (Temperature)
const int MICROPHONE_PIN = 35; // GPIO

FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;
float voltage = 0.0;

const int SAMPLES = 256; // Number of samples to store in the buffer
int audioBuffer[SAMPLES]; // Buffer to store audio samples

// Variables
float temperature = 0.0;
int ldrValue = 0;

// Function to read temperature
float readTemperature() {
  int adcValue = analogRead(TEMP_PIN);
  float voltage = (adcValue / 4095.0) * 3.3; // Assuming 3.3V reference voltage
  float tempC = (voltage - 0.5) * 100.0;     // Convert voltage to temperature
  return tempC;
}


float readAudio(){
  static int sampleIndex = 0; // Keep track of samples in the buffer
  // Sample audio and store in the buffer
  audioBuffer[sampleIndex] = analogRead(MICROPHONE_PIN);
  sampleIndex++;

  // If the buffer is full, send to Firebase
  if (sampleIndex >= SAMPLES) {
    sampleIndex = 0; // Reset buffer index

    // Create JSON to send
    FirebaseJson json;
    for (int i = 0; i < SAMPLES; i++) {
      json.add(String(i), audioBuffer[i]); // Add each sample to the JSON object
    }

  }
  // Add a small delay to manage sampling rate (e.g., ~8 kHz sampling rate)
  delayMicroseconds(125); // ~8 kHz = 125 µs/sample
}

// Function to read LDR value
int readLDR() {
  return analogRead(LDR_PIN);
}

void setup() {
  Serial.begin(115200);
  pinMode(MICROPHONE_PIN, INPUT);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP()); // Print the IP Address

  // Configure Firebase
  config.database_url = FIREBASE_HOST;
  config.api_key = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Read Sensor Data
  ldrValue = readLDR();
  temperature = readTemperature();

  // Create JSON object
  FirebaseJson json;
  json.set("ldr_value", ldrValue);
  json.set("Temperature(°C)", temperature);

  // Send Data to Firebase
  if (Firebase.RTDB.setJSON(&firebaseData, "/sensorData", &json)) {
    Serial.println("Data sent successfully!");
  } else {
    Serial.println("Failed to send data to Firebase");
    Serial.println(firebaseData.errorReason());
  }

  // Print to Serial Monitor
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  delay(1000); // Update every 1 second
}
