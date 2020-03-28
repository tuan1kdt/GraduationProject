
/*
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2019 mobizt
 *
*/


#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <DHT.h>
#include "MQ135.h"

#define PIN_MQ135 A0

//prosess sensor temperture
const int DHTPIN = D1;
const int DHTTYPE = DHT22;

DHT dht(DHTPIN, DHTTYPE);
MQ135 mq135_sensor = MQ135(PIN_MQ135);

#define FIREBASE_HOST "wateringsystem-48fbc.firebaseio.com"
#define FIREBASE_AUTH "gChHKpBsXMICPlsELMr3FvGpSuah3ppV9Oe4b2NJ"
#define WIFI_SSID "Bee_IoT"
#define WIFI_PASSWORD "tuandaik"

//
//
//#include <iostream>
//#include <chrono>
//#include <ctime>   

//Define FirebaseESP8266 data object
FirebaseData firebaseData;
int ledYellow = D3;
int ledGreen = D6;
int ledOrange = D5;
int flame_sensor = D2;
int flying_Fish = A0;
int flame_detected;                     
void setup()
{

  Serial.begin(115200);
  pinMode(2, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(flame_sensor,INPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  dht.begin();
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");

}

void loop()
{
  
  float h = dht.readHumidity(); //read Humidity
  float t = dht.readTemperature(); //read Temperature
  Firebase.setDouble(firebaseData,"/TEMPERATURE", t);         //set value
  Firebase.setDouble(firebaseData,"/HUMIDITY", h);            //set value
  Serial.printf("value temperature: %.2f\n",t);
  Serial.printf("value humidity: %.2f\n",h);
  Serial.println(); //Xuống hàng
  delay(1000);
  //==========FLAME_SENSOR==========
  flame_detected = digitalRead(flame_sensor);
  Firebase.setInt(firebaseData,"/FLAME_DETECTED", flame_detected); //set value
 
  //-------endflame--------

   delay(1000);
  //==========FLYING_FISH==========
  double PPM = mq135_sensor.getCorrectedPPM(t, h);
  Firebase.setDouble(firebaseData,"/PPM", PPM);             //set value
  Serial.print("\t Corrected PPM: ");
  Serial.print(PPM);
  Serial.println("ppm");
  //-------endflying_fish--------

  int fireStatus= -1,temperature,ppm;
  
  if(Firebase.getInt(firebaseData, "/LED_STATUS"))
      fireStatus = firebaseData.intData();
  if(Firebase.getInt(firebaseData, "/TEMPERATURE"))
      temperature = firebaseData.intData();
  if(Firebase.getInt(firebaseData, "/PPM"))
      ppm = firebaseData.intData();
  if(Firebase.getInt(firebaseData, "/FLAME_DETECTED"))
      flame_detected = firebaseData.intData();

  if(flame_detected==1)
    fireStatus = 3;
  else if(temperature>60)
    fireStatus = 3;
  else if(ppm>=15)
    fireStatus = 3;
  else if(ppm>=10&&ppm<15)
    fireStatus = 2;
  else if(temperature>=50&&temperature<=60)
    fireStatus = 2;
  else if(temperature>=40)
    fireStatus = 1;
  else if(ppm>6)
    fireStatus = 1;
  else
    fireStatus = 0;
Firebase.setInt(firebaseData, "/WARNING",fireStatus);
switch (fireStatus){
    case 0:
          digitalWrite(ledGreen, LOW);
          digitalWrite(ledYellow, LOW);
          digitalWrite(ledOrange, LOW);
          break;
  
      case 1:
          digitalWrite(ledGreen, HIGH);
          digitalWrite(ledYellow, LOW);
          digitalWrite(ledOrange, LOW);
          break;
      case 2:
          digitalWrite(ledGreen, HIGH);
          digitalWrite(ledYellow, HIGH);
          digitalWrite(ledOrange, LOW);
          break;
      case 3:
          digitalWrite(ledGreen, HIGH);
          digitalWrite(ledYellow, HIGH);
          digitalWrite(ledOrange, HIGH);
          break;
  }

   
}
