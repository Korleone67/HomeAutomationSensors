#include <DigitalIO.h>
#include <DigitalPin.h>
#include <I2cConstants.h>
#include <PinIO.h>
#include <SoftI2cMaster.h>
#include <SoftSPI.h>

#include <SPI.h>
#include <MySensor.h>  
#include <BH1750.h>
#include <Wire.h>
#include <DHT.h>

int BATTERY_SENSE_PIN = A0;
int oldBatteryPcnt = 0;

#define CHILD_ID_LIGHT 2
unsigned long SLEEP_TIME = 90000; // Sleep time between reads (in milliseconds)

#define CHILD_ID_HUM 3
#define CHILD_ID_TEMP 4
#define HUMIDITY_SENSOR_DIGITAL_PIN 4

MySensor gw;

BH1750 lightSensor;
MyMessage msgLux(CHILD_ID_LIGHT, V_LIGHT_LEVEL);
uint16_t lastlux;

DHT dht;
float lastTemp;
float lastHum;
boolean metric = true; 
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);

int reportInterval = 10;
int reportCount = 1;

void setup()  
{ 
  analogReference(INTERNAL);
  gw.begin();
  
  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN); 

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Outdoor temperature + light sensor", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  gw.present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  
  metric = gw.getConfig().isMetric;
  
  lightSensor.begin();
}

void loop()      
{ 
  
  // Battery level
  int sensorValue = analogRead(BATTERY_SENSE_PIN);
  Serial.print("Battery level: ");
  Serial.println(sensorValue);
  // 1M, 470K divider across battery and using internal ADC ref of 1.1V
  // Sense point is bypassed with 0.1 uF cap to reduce noise at that point
  // ((1e6+470e3)/470e3)*1.1 = Vmax = 3.44 Volts
  // 3.44/1023 = Volts per bit = 0.003363075
  float batteryV  = sensorValue * 0.003363075;
  int batteryPcnt = sensorValue / 10;
  Serial.print("Battery Voltage: ");
  Serial.print(batteryV);
  Serial.println(" V");
  Serial.print("Battery percent: ");
  Serial.print(batteryPcnt);
  Serial.println(" %");
  if (oldBatteryPcnt != batteryPcnt) {
    // Power up radio after sleep
    gw.sendBatteryLevel(batteryPcnt);
    oldBatteryPcnt = batteryPcnt;
  }
  
  // Light level
  uint16_t lux = lightSensor.readLightLevel();
  Serial.print("Lux value:");
  Serial.println(lux);
  
  if(lastlux != lux || reportCount == 10) {
    gw.send(msgLux.set(lux));
    lastlux = lux;
  }
  
  // Temperature
  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
      Serial.println("Failed reading temperature from DHT");
  } 
  else {
    Serial.print("T: ");
    Serial.println(temperature);
    if(lastTemp != temperature || reportCount == 10) { 
      lastTemp = temperature;
      gw.send(msgTemp.set(temperature, 1));    
    }
  }

  // Humidity
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
      Serial.println("Failed reading humidity from DHT");
  } 
  else {
    Serial.print("H: ");
    Serial.println(humidity);
    if(lastHum != humidity || reportCount == 10) { 
      lastHum = humidity;
      gw.send(msgHum.set(humidity, 1));
    }
  }
  
  if(reportCount == 10) {
    reportCount = 1;
  }
  else {
    reportCount++;
  }
  
  gw.sleep(SLEEP_TIME);
}

