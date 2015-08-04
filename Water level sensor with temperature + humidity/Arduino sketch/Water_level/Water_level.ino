#include <SPI.h>
#include <MySensor.h>
#include <DHT.h>

// Define the pins where the sensors are connected to
#define moistureSensorPin A0
#define dht11SensorPin 4
#define BATTERY_SENSE_PIN A6

// Setup the child ids for the sensors
#define CHILD_ID_WATER_ALARM 0
#define CHILD_ID_MOISTURE_LEVEL 1
#define CHILD_ID_TEMP 2
#define CHILD_ID_HUMIDITY 3

// Initialize MySensors
MySensor gw;
unsigned long SLEEP_TIME = 10000; // Sleep time between reads (in milliseconds)

// Setup messages for the different sensors
MyMessage msgWater(CHILD_ID_WATER_ALARM, V_TRIPPED);
MyMessage msgMoisture(CHILD_ID_MOISTURE_LEVEL, V_DISTANCE);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgHum(CHILD_ID_HUMIDITY, V_HUM);

// Initialize the dht sensor
DHT dht;
float lastTemp;
float lastHum;
boolean metric = true; 

// Set report interval
int reportInterval = 10; // Report at minimum every 10 measurements;
int reportCount = 1;

float lastWaterLevel;
int waterSensorValue = 0;

void setup() {
  analogReference(INTERNAL);
//  gw.begin();
  dht.setup(dht11SensorPin);
  gw.sendSketchInfo("Water alarm CV", "1.0");
  
  // Present sensors to controller
  gw.present(CHILD_ID_WATER_ALARM, S_MOTION);
  gw.present(CHILD_ID_MOISTURE_LEVEL, S_DISTANCE);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_HUMIDITY, S_HUM);
  
  metric = gw.getConfig().isMetric;
}

void loop() {
  delay(dht.getMinimumSamplingPeriod());
  
  // Get battery level
//  int sensorValue = analogRead(BATTERY_SENSE_PIN);
//  Serial.print("Battery level: ");
//  Serial.println(sensorValue);
//  // 1M, 470K divider across battery and using internal ADC ref of 1.1V
//  // Sense point is bypassed with 0.1 uF cap to reduce noise at that point
//  // ((1e6+470e3)/470e3)*1.1 = Vmax = 3.44 Volts
//  // 3.44/1023 = Volts per bit = 0.003363075
//  float batteryV  = sensorValue * 0.003363075;
//  int batteryPcnt = sensorValue / 10;
//  Serial.print("Battery Voltage: ");
//  Serial.print(batteryV);
//  Serial.println(" V");
//  Serial.print("Battery percent: ");
//  Serial.print(batteryPcnt);
//  Serial.println(" %");
//  gw.sendBatteryLevel(batteryPcnt); 

  
  // Read water sensor level
  waterSensorValue = analogRead(moistureSensorPin);
  // Convert value to level
  // 543 is the maximum value measured on 3 volts. 
  // This reports level in cm. My sensor is 4.2 cm long.
  float waterLevel = (waterSensorValue / (float)543) * (float)4.2; 
  Serial.println(waterLevel);
  if(waterLevel > 0.05 && lastWaterLevel != waterLevel) {
    Serial.print("Water detected. Level: ");
    Serial.print(waterLevel);
    Serial.println();
    // Water detected, reporting
    gw.send(msgWater.set(1));
    gw.send(msgMoisture.set(waterLevel, 1));
  }
  if(lastWaterLevel != waterLevel && waterLevel <= 0.05) {
    Serial.print("It's dry again. Level: ");
    Serial.println(waterLevel);
    // Report dry so alert can be turned off
    gw.send(msgWater.set(0));
    gw.send(msgMoisture.set(waterLevel, 1));
  }
  lastWaterLevel = waterLevel;
  
  // Read DHT11 for temperature and humidity;
  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();
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
  
  
  // Increase report count or set to 1 after 10 measurements
  if(reportCount == 10) {
    reportCount = 1;
  }
  else {
    reportCount++;
  }
  
  gw.sleep(SLEEP_TIME);
}
