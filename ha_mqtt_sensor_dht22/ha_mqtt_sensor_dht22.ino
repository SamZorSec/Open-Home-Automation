/*
   MQTT temperature and humidity sensor (DHT22) for Home-Assistant (esp8266)
   https://home-assistant.io/components/sensor.mqtt/

   Libraries :
    - ESP8266 core for Arduino : https://github.com/esp8266/Arduino
    - PubSubClient : https://github.com/knolleary/pubsubclient
    - DHT : https://github.com/adafruit/DHT-sensor-library
    - ArduinoJson : https://github.com/bblanchon/ArduinoJson

   Sources :
    - File > Examples > ES8266WiFi > WiFiClient
    - File > Examples > PubSubClient > mqtt_auth
    - File > Examples > PubSubClient > mqtt_esp8266
    - File > Examples > DHT sensor library > DHTtester
    - File > Examples > ArduinoJson > JsonGeneratorExample
    - http://www.jerome-bernard.com/blog/2015/10/04/wifi-temperature-sensor-with-nodemcu-esp8266/

   Schematic :
    - https://cdn.odd-one-out.serek.eu/wp-content/uploads/2015/09/ESP8266_DHT22.jpg
    - Resistor : 4.7K Ohm (vcc - data pin of the DHT22
    - Cable between D0 and RST (wake-up purpose)

   Samuel M. - 08.2016
   https://github.com/mertenats
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <ArduinoJson.h>

#define MQTT_VERSION MQTT_VERSION_3_1_1

// WiFi ssid and password
const PROGMEM char* WIFI_SSID = "[Redacted]";
const PROGMEM char* WIFI_PASSWORD = "[Redacted]";

// MQTT server IP and port, username and password
const PROGMEM char* MQTT_CLIENT_ID = "office_sensor1";
const PROGMEM char* MQTT_SERVER_IP = "[Redacted]";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "[Redacted]";
const PROGMEM char* MQTT_PASSWORD = "[Redacted]";

// MQTT topic
const PROGMEM char* MQTT_SENSOR_TOPIC = "office/sensor1";

// sleeping time
const PROGMEM uint16_t SLEEPING_TIME_IN_SECONDS = 600; // 10 minutes x 60 seconds

// DHT: pin 5 / D1 : https://bennthomsen.files.wordpress.com/2015/12/nodemcu_pinout_700-2.png?w=584
#define DHTPIN 5
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// function called to publish the temperature and the humidity
void publishData(float p_temperature, float p_humidity) {
  // create a JSON object
  // doc : https://github.com/bblanchon/ArduinoJson/wiki/API%20Reference
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  // INFO: the data must be converted into a string; a problem occurs when using floats...
  root["temperature"] = (String)p_temperature;
  root["humidity"] = (String)p_humidity;
  root.prettyPrintTo(Serial);
  Serial.println("");
  /*
     {
        "temperature": "23.20" ,
        "humidity": "43.70"
     }
  */
  char data[200];
  root.printTo(data, root.measureLength() + 1);
  client.publish(MQTT_SENSOR_TOPIC, data);
}

// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // init the serial
  Serial.begin(115200);

  dht.begin();

  // init the WiFi connection
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // init the MQTT connection
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  } else {
    Serial.println(h);
    Serial.println(t);
    publishData(t, h);
  }

  Serial.println("Closing the MQTT connection");
  client.disconnect();

  Serial.println("Closing the Wifi connection");
  WiFi.disconnect();

  ESP.deepSleep(SLEEPING_TIME_IN_SECONDS * 1000000, WAKE_RF_DEFAULT);
  delay(500); // wait for deep sleep to happen
}
