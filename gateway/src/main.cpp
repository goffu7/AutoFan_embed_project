#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Wi-Fi Credentials
#define WIFI_SSID "Bozzy"
#define WIFI_PASSWORD "785963yugi"

// Firebase Credentials
#define FIREBASE_HOST "https://embed-firebase-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "AIzaSyCa-1IqoEQLZ_pNlhllAY6le3Rw9ME6PAc"

// Gateway Pins
const int MQ2_AO_PIN = 36;  // MQ-2 Analog Output
const int MQ2_DO_PIN = 27;  // MQ-2 Digital Output
const int T447_PIN = 13;    // T447 Sensor
const int PIEZO_PIN = 14;   // Piezo Buzzer

// Firebase
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Thresholds
unsigned char signupOk = false;
float tempThreshold = 30.0;   // Temperature threshold
int ldrThreshold = 800;       // LDR threshold
int micThreshold = 1900;       // Microphone threshold
int mq2Threshold = 2000;       // MQ2 Analog threshold

void setup() {
  Serial.begin(115200);

  pinMode(MQ2_DO_PIN, INPUT);
  pinMode(T447_PIN, INPUT);
  pinMode(PIEZO_PIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");

  config.database_url = FIREBASE_HOST;
  config.api_key = FIREBASE_AUTH;
  if(Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("signup successful");
    signupOk = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Fetch sensor data from Firebase
  float temperature = 0;
  int ldrValue = 0;
  int micValue = 0;
  Serial.println("Fetching sensor data from Firebase...");

  if (Firebase.ready()) {
    // Read temperature
    if (Firebase.RTDB.getFloat(&firebaseData, "/sensorData/temperature")) {
      temperature = firebaseData.floatData();
      Serial.printf("Temperature: %.2f\n", temperature);
    }

    // Read LDR value
    if (Firebase.RTDB.getInt(&firebaseData, "/sensorData/ldrValue")) {
      ldrValue = firebaseData.intData();
      Serial.printf("LDR Value: %d\n", ldrValue);
    }

    // Read Microphone value
    if (Firebase.RTDB.getInt(&firebaseData, "/sensorData/micValue")) {
      micValue = firebaseData.intData();
      Serial.printf("Microphone Value: %d\n", micValue);
    }

    // Read local sensor data
    int mq2Analog = analogRead(MQ2_AO_PIN);
    int mq2Digital = digitalRead(MQ2_DO_PIN);
    int t447Value = digitalRead(T447_PIN);

    Serial.printf("MQ2 Analog: %d, MQ2 Digital: %d, T447: %d\n", mq2Analog, mq2Digital, t447Value);

    // Trigger Piezo Buzzer based on thresholds
    if (temperature > tempThreshold || ldrValue > ldrThreshold || micValue > micThreshold ||
        mq2Analog > mq2Threshold || mq2Digital == 1 || t447Value == 1) {
      digitalWrite(PIEZO_PIN, HIGH);
      delay(500);
      digitalWrite(PIEZO_PIN, LOW);
      delay(500);
    } else {
      digitalWrite(PIEZO_PIN, LOW);
    }
  }

  delay(5000);  // Small delay for stability
}
