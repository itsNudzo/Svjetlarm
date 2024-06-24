#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "YOUR_WIFI"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define API_KEY "AIzaSyCYCqDwxkw8Zs5CxOoECtbTFa0sTO6dSeM"
#define DATABASE_URL "https://svjetlarm-2f17c-default-rtdb.europe-west1.firebasedatabase.app/" 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
int sensor = A0; 
int alarm = D1;  

void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; 
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  pinMode(sensor, INPUT);
  pinMode(alarm, OUTPUT);

}

void loop(){
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
     // Read sensor data
     int pirValue = analogRead(sensor); 
     Serial.println(pirValue);
     Firebase.RTDB.setInt(&fbdo, "test/pirValue", pirValue);
     if(pirValue > 999 && Firebase.RTDB.getInt(&fbdo, "test/systemOn")) {    
        if(Firebase.RTDB.getString(&fbdo, "test/alarm") && fbdo.stringData() != "iskljucen") {
          tone(alarm, 1000);  
          delay(1000);           
          noTone(alarm);      
          delay(1000);            
          Serial.println("pisti");
        }
      }

      if (Firebase.RTDB.getString(&fbdo, "test/alarm") && fbdo.stringData() == "iskljucen") {
        noTone(alarm);
        Serial.println("alarm iskljucen");
     }
  }
}