/*
   MQTT RGB Light for Home-Assistant (esp8266)
   https://home-assistant.io/components/light.mqtt/

   Libraries :
    - ESP8266 core for Arduino : https://github.com/esp8266/Arduino
    - PubSubClient : https://github.com/knolleary/pubsubclient

   Sources :
    - File > Examples > ES8266WiFi > WiFiClient
    - File > Examples > PubSubClient > mqtt_auth
    - File > Examples > PubSubClient > mqtt_esp8266
    - http://forum.arduino.cc/index.php?topic=272862.0

   Schematic :
    - https://learn.adafruit.com/adafruit-arduino-lesson-3-rgb-leds/breadboard-layout

   Samuel M. - 07.2016
   https://github.com/mertenats
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_VERSION MQTT_VERSION_3_1_1

// WiFi ssid and password
const char* WIFI_SSID = "[Redacted]";
const char* WIFI_PASSWORD = "[Redacted]";

// MQTT server IP and port, username and password
const char* MQTT_CLIENT_ID = "office_light1";
const char* MQTT_SERVER_IP = "[Redacted]";
const uint16_t MQTT_SERVER_PORT = 1883;
// const char* MQTT_USER = "";
// const char* MQTT_PASSWORD = "";

// MQTT topics
// state
const char* MQTT_LIGHT_STATE_TOPIC = "office/rgb1/light/status";
const char* MQTT_LIGHT_COMMAND_TOPIC = "office/rgb1/light/switch";

// brightness
const char* MQTT_LIGHT_BRIGHTNESS_STATE_TOPIC = "office/rgb1/brightness/status";
const char* MQTT_LIGHT_BRIGHTNESS_COMMAND_TOPIC = "office/rgb1/brightness/set";

// colors (rgb)
const char* MQTT_LIGHT_RGB_STATE_TOPIC = "office/rgb1/rgb/status";
const char* MQTT_LIGHT_RGB_COMMAND_TOPIC = "office/rgb1/rgb/set";

// Light
// the payload that represents enabled/disabled state, by default
const char* LIGHT_ON = "ON";
const char* LIGHT_OFF = "OFF";

// store the state of the rgb light (colors, brightness, ...)
boolean m_rgb_state = false;
uint8_t m_rgb_brightness = 100;
uint8_t m_rgb_red = 255;
uint8_t m_rgb_green = 255;
uint8_t m_rgb_blue = 255;

// the pins used for the rgb led (PWM)
const uint8_t RGB_LIGHT_RED_PIN = 4;
const uint8_t RGB_LIGHT_GREEN_PIN = 0;
const uint8_t RGB_LIGHT_BLUE_PIN = 2;

// buffer used to send/receive data with MQTT
const uint8_t MSG_BUFFER_SIZE = 20;
char m_msg_buffer[MSG_BUFFER_SIZE]; 

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// function called to adapt the brightness and the colors of the led
void setColor(uint8_t p_red, uint8_t p_green, uint8_t p_blue) {
  // convert the brightness in % (0-100%) into a digital value (0-255)
  uint8_t brightness = map(m_rgb_brightness, 0, 100, 0, 255);

  analogWrite(RGB_LIGHT_RED_PIN, map(p_red, 0, 255, 0, brightness));
  analogWrite(RGB_LIGHT_GREEN_PIN, map(p_green, 0, 255, 0, brightness));
  analogWrite(RGB_LIGHT_BLUE_PIN, map(p_blue, 0, 255, 0, brightness));
}

// function called to publish the state of the led (on/off)
void publishRGBState() {
  if (m_rgb_state) {
    client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_ON);
  } else {
    client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_OFF);
  }
}

// function called to publish the brightness of the led (0-100)
void publishRGBBrightness() {
  snprintf(m_msg_buffer, MSG_BUFFER_SIZE, "%d", m_rgb_brightness);
  client.publish(MQTT_LIGHT_BRIGHTNESS_STATE_TOPIC, m_msg_buffer);
}

// function called to publish the colors of the led (xx(x),xx(x),xx(x))
void publishRGBColor() {
  snprintf(m_msg_buffer, MSG_BUFFER_SIZE, "%d,%d,%d", m_rgb_red, m_rgb_green, m_rgb_blue);
  client.publish(MQTT_LIGHT_RGB_STATE_TOPIC, m_msg_buffer);
}

// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  // concat the payload into a string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }
  // handle message topic
  if (String(MQTT_LIGHT_COMMAND_TOPIC).equals(p_topic)) {
    // test if the payload is equal to "ON" or "OFF"
    if (payload.equals(String(LIGHT_ON))) {
      if (m_rgb_state != true) {
        m_rgb_state = true;
        setColor(m_rgb_red, m_rgb_green, m_rgb_blue);
        publishRGBState();
      }
    } else if (payload.equals(String(LIGHT_OFF))) {
      if (m_rgb_state != false) {
        m_rgb_state = false;
        setColor(0, 0, 0);
        publishRGBState();
      }
    }
  } else if (String(MQTT_LIGHT_BRIGHTNESS_COMMAND_TOPIC).equals(p_topic)) {
    uint8_t brightness = payload.toInt();
    if (brightness < 0 || brightness > 100) {
      // do nothing...
      return;
    } else {
      m_rgb_brightness = brightness;
      setColor(m_rgb_red, m_rgb_green, m_rgb_blue);
      publishRGBBrightness();
    }
  } else if (String(MQTT_LIGHT_RGB_COMMAND_TOPIC).equals(p_topic)) {
    // get the position of the first and second commas
    uint8_t firstIndex = payload.indexOf(',');
    uint8_t lastIndex = payload.lastIndexOf(',');
    
    uint8_t rgb_red = payload.substring(0, firstIndex).toInt();
    if (rgb_red < 0 || rgb_red > 255) {
      return;
    } else {
      m_rgb_red = rgb_red;
    }
    
    uint8_t rgb_green = payload.substring(firstIndex + 1, lastIndex).toInt();
    if (rgb_green < 0 || rgb_green > 255) {
      return;
    } else {
      m_rgb_green = rgb_green;
    }
    
    uint8_t rgb_blue = payload.substring(lastIndex + 1).toInt();
    if (rgb_blue < 0 || rgb_blue > 255) {
      return;
    } else {
      m_rgb_blue = rgb_blue;
    }
    
    setColor(m_rgb_red, m_rgb_green, m_rgb_blue);
    publishRGBColor();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID/*, MQTT_USER, MQTT_PASSWORD*/)) {
      Serial.println("connected");
      
      // Once connected, publish an announcement...
      // publish the initial values
      publishRGBState();
      publishRGBBrightness();
      publishRGBColor();

      // ... and resubscribe
      client.subscribe(MQTT_LIGHT_COMMAND_TOPIC);
      client.subscribe(MQTT_LIGHT_BRIGHTNESS_COMMAND_TOPIC);
      client.subscribe(MQTT_LIGHT_RGB_COMMAND_TOPIC);
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

  // init the RGB led
  pinMode(RGB_LIGHT_BLUE_PIN, OUTPUT);
  pinMode(RGB_LIGHT_RED_PIN, OUTPUT);
  pinMode(RGB_LIGHT_GREEN_PIN, OUTPUT);
  setColor(0, 0, 0);

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
}
