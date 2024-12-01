#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Wi-Fi Credentials
#define ssid "Bozzy"          // Your Wi-Fi SSID
#define password "785963yugi" // Your Wi-Fi Password

// Firebase Credentials
#define FIREBASE_HOST "https://embed-firebase-default-rtdb.asia-southeast1.firebasedatabase.app/" // Replace with your Firebase database URL
#define FIREBASE_AUTH "AIzaSyCa-1IqoEQLZ_pNlhllAY6le3Rw9ME6PAc"                                   // Replace with your Firebase secret or web API key

// Sensor Pins
const int LDR_PIN = 32;        // GPIO pin for LDR
const int TEMP_PIN = 34;       // GPIO pin for Temperature Sensor
const int MICROPHONE_PIN = 35; // GPIO pin for Microphone

// Firebase
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Variables
unsigned long sendDataPrevMillis = 0;
bool signupOk = false;
float temperature = 0.0;
int ldrValue = 0;
int micValue = 0;

// Function to read temperature
float readTemperature()
{
  int adcValue = analogRead(TEMP_PIN);
  float voltage = (adcValue / 3048.0) * 3.3; // Assuming 3.3V reference voltage
  float tempC = voltage  * 100.0;     // Convert voltage to temperature
  return tempC;
}

void setup()
{
  Serial.begin(115200);

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Configure Firebase
  config.database_url = FIREBASE_HOST;
  config.api_key = FIREBASE_AUTH;
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("signup successful");
    signupOk = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop()
{
  if (Firebase.ready() && signupOk && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
  { // delay 5 seconds
    sendDataPrevMillis = millis();
    // Read Sensor Data
    ldrValue = analogRead(LDR_PIN);
    temperature = readTemperature();
    micValue = analogRead(MICROPHONE_PIN);
    if (Firebase.RTDB.setInt(&firebaseData, "/sensorData/ldrValue", ldrValue))
    {
      Serial.println(ldrValue);
      Serial.println("LDR Value sent to Firebase");
    }
    else
    {
      Serial.println(firebaseData.errorReason());
    }

    if (Firebase.RTDB.setFloat(&firebaseData, "/sensorData/temperature", temperature))
    {
      Serial.println(temperature);
      Serial.println("Temperature sent to Firebase");
    }
    else
    {
      Serial.println(firebaseData.errorReason());
    }

    if (Firebase.RTDB.setInt(&firebaseData, "/sensorData/micValue", micValue))
    {
      Serial.println(micValue);
      Serial.println("Microphone Value sent to Firebase");
    }
    else
    {
      Serial.println(firebaseData.errorReason());
    }
  }
}