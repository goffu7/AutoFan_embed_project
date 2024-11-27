#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Aman"
#define WIFI_PASSWORD "045781023"
#define APi_KEY "AIzaSyDwVaA472bhGs4-QL46u6YiLTSHwgG2iiY"
#define DATABASE_URL "https://automatic-fan-90da8-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define MICROPHONE_PIN 35
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;


unsigned long sendDataMillis = 0;


bool signUpOk = false;
int idrData = 0;
float voltage = 0.0;

const int SAMPLES = 256; // Number of samples to store in the buffer
int audioBuffer[SAMPLES]; // Buffer to store audio samples

void setup()
{
  // put your setup code here, to run once:

  pinMode(MICROPHONE_PIN, INPUT);
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  unsigned long wifiTimeout = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
    if (millis() - wifiTimeout > 20000) { // 20 seconds timeout
        Serial.println("\nFailed to connect to WiFi.");
        return;
    }

  }
  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
  Serial.println();


  config.database_url = DATABASE_URL;
  config.api_key = APi_KEY; 
  if(Firebase.signUp(&config, &auth, "","")){
    Serial.println("Sign up success");
    signUpOk = true;  
  }else{
    Serial.println("Sign up failed");
    Serial.println("Error: "+ String(config.signer.signupError.message.c_str()));
    signUpOk = false;
  }
  
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop()
{
  // put your main code here, to run repeatedly:
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

    // Check Firebase connection and send data
    if (Firebase.ready() && (millis() - sendDataMillis > 5000 || sendDataMillis == 0)) {
      sendDataMillis = millis();
      if (Firebase.RTDB.setJSON(&firebaseData, "/audioData", &json)) {
        Serial.println("Audio data sent to Firebase");
      } else {
        Serial.print("Failed to send data: ");
        Serial.println(firebaseData.errorReason());
      }
    }
  }

  // Add a small delay to manage sampling rate (e.g., ~8 kHz sampling rate)
  delayMicroseconds(125); // ~8 kHz = 125 µs/sample
}
