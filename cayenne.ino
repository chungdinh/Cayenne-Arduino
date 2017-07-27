/* SMART GARDEN
Copyright (C) 2016 Chung Dinh*/

#include "DHT.h"
//#define CAYENNE_DEBUG         // Uncomment to show debug messages
#define CAYENNE_PRINT Serial  // Comment this out to disable prints and save space
#include <CayenneEthernet.h>

#include <Wire.h>
#include <BH1750.h>
BH1750 lightMeter;

#define DHTPIN 3     // DHT pin
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#define VIRT_TEMP V1
#define VIRT_HUM V2
#define VIRT_DAT1 V3
#define VIRT_DAT2 V4
#define VIRT_CDAS V5
#define VIRT_LL V6

//Token connect Cayenne.
char token[] = "bfcjant0at";

DHT dht(DHTPIN, DHTTYPE);
unsigned long prev_DHT_refresh, interval_DHT_refresh = 1000;

//Flow sensor
volatile int flow_frequency; // Measures flow sensor pulses
unsigned int l_hour; 
unsigned char flowsensor = 2; 
unsigned long currentTime;
unsigned long cloopTime;
void flow () // Interrupt function
{
   flow_frequency++;
}
void setup()
{
  Serial.begin(9600);
  Cayenne.begin(token);

  dht.begin();
  //Soil moisture sensor pins
 
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  lightMeter.begin();
  Serial.println(F("BH1750 Test"));

   pinMode(flowsensor, INPUT);
   digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
   Serial.begin(9600);
   attachInterrupt(0, flow, RISING); // Setup Interrupt
   sei(); // Enable interrupts
   currentTime = millis();
   cloopTime = currentTime;
}

void loop()
{
  Cayenne.run();

  getDhtValues();

}

void getDhtValues() {
    unsigned long now = millis();
  
  if (now - prev_DHT_refresh > interval_DHT_refresh) {
    float h = dht.readHumidity();
    // Temperature
    float t = dht.readTemperature();
    //Soil moisture
    float value1 = analogRead(A0);
    float k1=map(value1,0,1023,100,0);
    float value2 = analogRead(A1);
    float k2=map(value2,0,1023,100,0);
    //Đọc CDAS
    uint16_t lux = lightMeter.readLightLevel();

    currentTime = millis();// Every second, calculate and print litres/hour
   if(currentTime >= (cloopTime + 1000))
   {
      cloopTime = currentTime; // Updates cloopTime
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
      flow_frequency = 0; // Reset Counter
      Serial.print(l_hour, DEC); // Print litres/hour
      Serial.println(" L/hour");
   }
    // Check if any reads failed
    if (!isnan(h) && !isnan(t)) {
      Cayenne.virtualWrite(VIRT_HUM, h);
      Cayenne.celsiusWrite(VIRT_TEMP, t);
      Cayenne.virtualWrite(VIRT_DAT1, k1);
      Cayenne.virtualWrite(VIRT_DAT2, k2);
      Cayenne.virtualWrite(VIRT_CDAS, lux);
       Cayenne.virtualWrite(VIRT_LL, l_hour);
    }
    prev_DHT_refresh = now;
  }
}
