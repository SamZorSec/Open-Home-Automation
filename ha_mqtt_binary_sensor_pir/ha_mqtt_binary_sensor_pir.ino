/*
   MQTT PIR binary sensor for Home-Assistant (esp8266)
   https://home-assistant.io/components/binary_sensor.mqtt/

   Config : 
   binary_sensor:
    platform: mqtt
    state_topic: 'office/motion/status'
    name: 'Motion'
    sensor_class: motion

   Libraries :
    - ESP8266 core for Arduino : https://github.com/esp8266/Arduino
    - PubSubClient : https://github.com/knolleary/pubsubclient

   Sources :
    - File > Examples > ES8266WiFi > WiFiClient
    - File > Examples > PubSubClient > mqtt_auth
    - File > Examples > PubSubClient > mqtt_esp8266
    - https://learn.adafruit.com/pir-passive-infrared-proximity-motion-sensor/using-a-pir

   Schematic :
    - https://learn.adafruit.com/pir-passive-infrared-proximity-motion-sensor/using-a-pir

    TODO :
    - Use the interrupts instead of constinously polling the sensor

   Samuel M. - 08.2016
   If you like this example, please add a star! Thank you!
   https://github.com/mertenats/open-home-automation
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_VERSION MQTT_VERSION_3_1_1

// WiFi ssid and password
const PROGMEM char* WIFI_SSID = "[Redacted]";
const PROGMEM char* WIFI_PASSWORD = "[Redacted]";

// MQTT server IP and port, username and password
const PROGMEM char* MQTT_CLIENT_ID = "office_motion";
const PROGMEM char* MQTT_SERVER_IP = "[Redacted]";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "[Redacted]";
const PROGMEM char* MQTT_PASSWORD = "[Redacted]";

// MQTT topic
const PROGMEM char* MQTT_MOTION_STATUS_TOPIC = "office/motion/status";

// default payload
const PROGMEM char* MOTION_ON = "ON";
const PROGMEM char* MOTION_OFF = "OFF";

// PIR : D2 / 4 : https://bennthomsen.files.wordpress.com/2015/12/nodemcu_pinout_700-2.png?w=584
const PROGMEM uint8_t PIR_PIN = 4;
uint8_t m_pir_state = LOW; // no motion detected
uint8_t m_pir_value = 0;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// function called to publish the state of the pir sensor
void publishPirSensorState() {
  if (m_pir_state) {
    client.publish(MQTT_MOTION_STATUS_TOPIC, MOTION_OFF);
  } else {
    client.publish(MQTT_MOTION_STATUS_TOPIC, MOTION_ON);
  }
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

  // read the PIR sensor
  m_pir_value = digitalRead(PIR_PIN);
  if (m_pir_value == HIGH) {
    if (m_pir_state == LOW) {
      // a motion is detected
      Serial.println("INFO: Motion detected");
      publishPirSensorState();
      m_pir_state = HIGH;
    }
  } else {
    if (m_pir_state == HIGH) {
      publishPirSensorState();
      Serial.println("INFO: Motion ended");
      m_pir_state = LOW;
    }
  }
}
