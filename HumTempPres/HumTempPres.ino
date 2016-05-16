/**
 * Temperature - Humidity - Pressure
 * Measurement interval : 30 minutes
 * Samuel M. - 05.2016
 * Sources : - sketch HumiditySensor (MySensors)
 *           - sketch BatteryPoweredSensor (MySensors)
 */

#include <SPI.h>
#include <MySensor.h>  
#include <DHT.h>  
#include <Wire.h>
#include <Adafruit_BMP085.h>

#define CHILD_ID_HUMIDITY 0
#define CHILD_ID_TEMPERATURE 1
#define CHILD_ID_PRESSURE 2

#define HUMIDITY_SENSOR_DIGITAL_PIN 3
#define BATTERY_SENSOR_ANALOG_PIN A0

unsigned long SLEEP_TIME = 1800000; // [ms] 30 minutes : 30 x 60 x 1000 = 1 800 000

// 1M, 470K divider across battery and using internal ADC ref of 1.1V
// Sense point is bypassed with 0.1 uF cap to reduce noise at that point
// ((1e6+470e3)/470e3)*1.1 = Vmax = 3.44 Volts
// 3.44/1023 = Volts per bit = 0.003363075
#define VBAT_PER_BITS 0.003363075
#define VMIN 1.9  // Battery monitor lower level. Vmin_radio=1.9V
#define VMAX 3.3  //  " " " high level. Vmin<Vmax<=3.44
int lastBatteryPcnt = -1;

MySensor gw;
DHT dht;
Adafruit_BMP085 bmp = Adafruit_BMP085();

float lastTemperature = -1;
float lastHumidity = -1;
float lastPressure = -1;

boolean metric = true; 
MyMessage msgHum(CHILD_ID_HUMIDITY, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMPERATURE, V_TEMP);
MyMessage msgPres(CHILD_ID_PRESSURE, V_PRESSURE);

void setup() { 
  // use the 1.1 V internal reference for battery level measuring
  analogReference(INTERNAL);  

  gw.begin();
  
  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN);
  
  if (!bmp.begin()) {
    #ifdef DEBUG
      Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    #endif
    while (1) {}
  }

  // Send the Sketch Version Information to the Gateway
  gw.sendSketchInfo("HumTempPres", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID_HUMIDITY, S_HUM);
  gw.present(CHILD_ID_TEMPERATURE, S_TEMP);
  gw.present(CHILD_ID_PRESSURE, S_BARO);
  
  metric = gw.getConfig().isMetric;
}

void loop() {  
  delay(dht.getMinimumSamplingPeriod());
  
  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
    #ifdef DEBUG
      Serial.println("Failed reading temperature from DHT");
    #endif
  } else if (temperature != lastTemperature) {
    lastTemperature = temperature;
    if (!metric) {
      temperature = dht.toFahrenheit(temperature);
    }
    gw.send(msgTemp.set(temperature, 1));
    
    #ifdef DEBUG
      Serial.print("T: ");
      Serial.println(temperature);
    #endif
  }
  
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
    #ifdef DEBUG
      Serial.println("Failed reading humidity from DHT");
    #endif
  } else if (humidity != lastHumidity) {
      lastHumidity = humidity;
      gw.send(msgHum.set(humidity, 1));
      #ifdef DEBUG
        Serial.print("H: ");
        Serial.println(humidity);
      #endif
  }
  
  float pressure = bmp.readPressure()/100;
  if (pressure != lastPressure) {
    lastPressure = pressure;
    gw.send(msgPres.set(pressure, 0));
    #ifdef DEBUG
      Serial.print("P: ");
      Serial.println(pressure);
    #endif
  }
  
  // Battery monitoring reading
  int sensorValue = analogRead(BATTERY_SENSOR_ANALOG_PIN);   
  float Vbat  = sensorValue * VBAT_PER_BITS;
  int batteryPcnt = static_cast<int>(((Vbat-VMIN)/(VMAX-VMIN))*100.);
  
  #ifdef DEBUG
    Serial.print("Battery Voltage: ");
    Serial.print(Vbat);
    Serial.println(" V");
    Serial.print("Battery percent: ");
    Serial.print(batteryPcnt);
    Serial.println(" %");
  #endif

  if (lastBatteryPcnt != batteryPcnt) {
    lastBatteryPcnt = batteryPcnt;
    gw.sendBatteryLevel(batteryPcnt);
  }

  gw.sleep(SLEEP_TIME); //sleep a bit
}
