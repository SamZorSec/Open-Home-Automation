/*
  Configuration for Home Assistant:
    mqtt:
      broker: 127.0.0.1
      port: 1883
      client_id: 'ha'
      username: 'ha'
      password: '!ha@OpenHome16'
      
    light:
      # lamp 3 (RGB)
      - platform: mqtt
        name: 'Lamp 3'
        state_topic: 'livingroom/light1/status'
        command_topic: 'livingroom/light1/switch'
        brightness_state_topic: 'livingroom/light1/brightness/status'
        brightness_command_topic: 'livingroom/light1/brightness/set'
        rgb_state_topic: 'livingroom/light1/color/status'
        rgb_command_topic: 'livingroom/light1/color/set'
        optimistic: false
      # lamp 4
      - platform: mqtt
        name: 'Lamp 4'
        state_topic: 'livingroom/light2/status'
        command_topic: 'livingroom/light2/switch'
        optimistic: false
        
    binary_sensor:
      - platform: mqtt
        name: 'TV'
        state_topic: 'livingroom/tv/status'

  Sources:
    - MQTT: File > Examples > PubSubClient > mqtt_esp8266
    - TLS:  https://io.adafruit.com/blog/security/2016/07/05/adafruit-io-security-esp8266/
    - OTA:  File > Examples > ArduinoOTA > BasicOTA
*/

#include <ESP8266WiFi.h>    // https://github.com/esp8266/Arduino (GNUv2.1 licence)
#include <PubSubClient.h>   // https://github.com/knolleary/pubsubclient (no licence)
#include <ArduinoOTA.h>

#define DEBUG
//#define TLS
#define MQTT_VERSION MQTT_VERSION_3_1_1

// Wi-Fi: Access Point SSID and password
const char*       AP_SSID           = "[Redacted]";
const char*       AP_PASSWORD       = "[Redacted]";

// MQTT: client ID, broker IP address, port, username & password
const char*       MQTT_CLIENT_ID    = "livingroom";
const char*       MQTT_SERVER_IP    = "192.168.1.10";
#ifdef TLS
const uint16_t    MQTT_SERVER_PORT  = 8883;
#else
const uint16_t    MQTT_SERVER_PORT  = 1883;
#endif
const char*       MQTT_USERNAME     = "livingroom";
const char*       MQTT_PASSWORD     = "[Redacted]";

// MQTT: topics
// lamp 3
const char*       TOPIC_LIGHT1_STATUS             = "livingroom/light1/status";
const char*       TOPIC_LIGHT1_BRIGHTNESS_STATUS  = "livingroom/light1/brightness/status";
const char*       TOPIC_LIGHT1_COLOR_STATUS       = "livingroom/light1/color/status";
const char*       TOPIC_LIGHT1_COMMAND            = "livingroom/light1/switch";
const char*       TOPIC_LIGHT1_BRIGHTNESS_COMMAND = "livingroom/light1/brightness/set";
const char*       TOPIC_LIGHT1_COLOR_COMMAND      = "livingroom/light1/color/set";
// lamp 4
const char*       TOPIC_LIGHT2_STATUS             = "livingroom/light2/status";
const char*       TOPIC_LIGHT2_COMMAND            = "livingroom/light2/switch";
// brightness sensor
const char*       TOPIC_TV_BRIGHTNESS_STATUS      = "livingroom/tv/status";

// MQTT: payloads
// Lamps 3 & 4: "ON"/"OFF"
const char*       PAYLOAD_ON            = "ON";
const char*       PAYLOAD_OFF           = "OFF";

boolean           g_light1_status       = false; // turn off by default
uint8_t           g_light1_brightness   = 255; // max value by default
uint8_t           g_light1_color_red    = 255;
uint8_t           g_light1_color_green  = 255;
uint8_t           g_light1_color_blue   = 255;
boolean           g_light2_status       = false;
uint16_t          g_photocell_value     = 0;
boolean           g_photocell_status    = false; // the TV is off
uint32_t          g_photocell_last_change = 0;
boolean           g_photocell_status_to_publish = false;

// TV
const uint16_t    PHOTOCELL_THRESHOLD   = 150;
const uint16_t    PHOTOCELL_TIME_OFFSET = 3000; // 3 secondes in the same state before a notification to the controller

// buffer used to send/receive data with MQTT
const uint8_t     MSG_BUFFER_SIZE       = 20;
char              g_msg_buffer[MSG_BUFFER_SIZE];

const uint8_t     LIGHT1_RED_PIN        = D3;
const uint8_t     LIGHT1_GREEN_PIN      = D2;
const uint8_t     LIGHT1_BLUE_PIN       = D1;
const uint8_t     LIGHT2_PIN            = D5; // D4 seems to be also connected to a internal LED...
const uint8_t     PHOTOCELL_PIN         = A0;

// TLS: The fingerprint of the MQTT broker certificate (SHA1)
#ifdef TLS
// openssl x509 -fingerprint -in  <certificate>.crt
const char*       CA_FINGERPRINT    = "[Redacted]";
// openssl x509 -subject -in  <certificate>.crt
const char*       CA_SUBJECT        = "[Redacted]";
#endif

// Fixed IP address: IP address, IP gateway, subnet, dns
const IPAddress   IP                (192, 168,   1, 101);
const IPAddress   IP_GATEWAY        (192, 168,   1,   1);
const IPAddress   IP_SUBNET         (255, 255, 255,   0);
const IPAddress   IP_DNS            (192, 168,   1,   1);

// OTA: Hostname (MQTT_CLIENT_ID) & password
const char*       OTA_PASSWORD      = "[Redacted]";

// WiFiFlientSecure instead of WiFiClient, for SSL/TLS support
#ifdef TLS
WiFiClientSecure  g_wifiClient;
#else
WiFiClient  g_wifiClient;
#endif
PubSubClient      g_mqttClient(g_wifiClient);

///////////////////////////////////////////////////////////////////////////
//
// LIGHT 1 (RGB)
//
///////////////////////////////////////////////////////////////////////////

/*
  Function called to publish the status of the light 1
*/
void publishLight1Status() {
  if (g_light1_status) {
    if (g_mqttClient.publish(TOPIC_LIGHT1_STATUS, PAYLOAD_ON, true)) {
#ifdef DEBUG
      Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
      Serial.print(TOPIC_LIGHT1_STATUS);
      Serial.print(F(". Payload: "));
      Serial.println(PAYLOAD_ON);
#endif
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
    }
  } else {
    if (g_mqttClient.publish(TOPIC_LIGHT1_STATUS, PAYLOAD_OFF, true)) {
#ifdef DEBUG
      Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
      Serial.print(TOPIC_LIGHT1_STATUS);
      Serial.print(F(". Payload: "));
      Serial.println(PAYLOAD_OFF);
#endif
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
    }
  }
}

/*
  Function called to publish the brightness status of the light 1
*/
void publishLight1BrightnessStatus() {
  snprintf(g_msg_buffer, MSG_BUFFER_SIZE, "%d", g_light1_brightness);
  if (g_mqttClient.publish(TOPIC_LIGHT1_BRIGHTNESS_STATUS, g_msg_buffer, true)) {
#ifdef DEBUG
    Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
    Serial.print(TOPIC_LIGHT1_BRIGHTNESS_STATUS);
    Serial.print(F(". Payload: "));
    Serial.println(g_msg_buffer);
#endif
  } else {
#ifdef DEBUG
    Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
  }
}

/*
  Function called to publish the color status of the light 1
*/
void publishLight1ColorStatus() {
  snprintf(g_msg_buffer, MSG_BUFFER_SIZE, "%d,%d,%d", g_light1_color_red, g_light1_color_green, g_light1_color_blue);
  if (g_mqttClient.publish(TOPIC_LIGHT1_COLOR_STATUS, g_msg_buffer, true)) {
#ifdef DEBUG
    Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
    Serial.print(TOPIC_LIGHT1_COLOR_STATUS);
    Serial.print(F(". Payload: "));
    Serial.println(g_msg_buffer);
#endif
  } else {
#ifdef DEBUG
    Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
  }
}

/*
  Function called to set the color of the light 1
  @param p_light1_color_red   The value for the red channel
  @param p_light1_color_green The value for the green channel
  @param p_light1_color_blue  The value for the blue channel
*/

void setLight1Color(uint8_t p_light1_color_red, uint8_t p_light1_color_green, uint8_t p_light1_color_blue) {
  analogWrite(LIGHT1_RED_PIN,   map(p_light1_color_red,   0, 255, 0, g_light1_brightness));
  analogWrite(LIGHT1_GREEN_PIN, map(p_light1_color_green, 0, 255, 0, g_light1_brightness));
  analogWrite(LIGHT1_BLUE_PIN,  map(p_light1_color_blue,  0, 255, 0, g_light1_brightness));
}

///////////////////////////////////////////////////////////////////////////
//
// LIGHT 2
//
///////////////////////////////////////////////////////////////////////////

/*
  Function called to publish the status of the light 2
*/
void publishLight2Status() {
  if (g_light2_status) {
    if (g_mqttClient.publish(TOPIC_LIGHT2_STATUS, PAYLOAD_ON, true)) {
#ifdef DEBUG
      Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
      Serial.print(TOPIC_LIGHT2_STATUS);
      Serial.print(F(". Payload: "));
      Serial.println(PAYLOAD_ON);
#endif
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
    }
  } else {
    if (g_mqttClient.publish(TOPIC_LIGHT2_STATUS, PAYLOAD_OFF, true)) {
#ifdef DEBUG
      Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
      Serial.print(TOPIC_LIGHT2_STATUS);
      Serial.print(F(". Payload: "));
      Serial.println(PAYLOAD_OFF);
#endif
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
    }
  }
}

/*
  Function called to switch the status of the light 2
*/
void setLight2Status() {
  if (g_light2_status) {
    digitalWrite(LIGHT2_PIN, HIGH);
  } else {
    digitalWrite(LIGHT2_PIN, LOW);
  }
}

///////////////////////////////////////////////////////////////////////////
//
// TV BRIGHTNESS SENSOR
//
///////////////////////////////////////////////////////////////////////////

/*
  Function called to read the photocell attached to the TV
 */
void readTVBrightness() {
  // read the value from the photocell
  g_photocell_value = analogRead(PHOTOCELL_PIN);

  if (g_photocell_value > PHOTOCELL_THRESHOLD) {
    // if the TV was alread switched on...
    if (g_photocell_status) {
      // test if it was switched on for the specified duration
      if (millis() > g_photocell_last_change + PHOTOCELL_TIME_OFFSET) {
        if (g_photocell_status_to_publish) {
          g_photocell_status_to_publish = false;
          publishTVStatus();
        }
      }
    } else {
      g_photocell_last_change = millis();
      g_photocell_status = true;
      g_photocell_status_to_publish = true;
    }
  } else {
    if (g_photocell_status) {
      g_photocell_last_change = millis();
      g_photocell_status = false;
      g_photocell_status_to_publish = true;
    } else {
      if (millis() > g_photocell_last_change + PHOTOCELL_TIME_OFFSET) {
        if (g_photocell_status_to_publish) {
          g_photocell_status_to_publish = false;
          publishTVStatus();
        }
      }
    }
  }

#ifdef DEBUG
  Serial.print(F("INFO: Photocell read value: "));
  Serial.println(g_photocell_value);
#endif
}

/*
  Function called to publish the status of the TV
*/
void publishTVStatus() {
    if (g_photocell_status) {
    if (g_mqttClient.publish(TOPIC_TV_BRIGHTNESS_STATUS, PAYLOAD_ON, true)) {
#ifdef DEBUG
      Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
      Serial.print(TOPIC_TV_BRIGHTNESS_STATUS);
      Serial.print(F(". Payload: "));
      Serial.println(PAYLOAD_ON);
#endif
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
    }
  } else {
    if (g_mqttClient.publish(TOPIC_TV_BRIGHTNESS_STATUS, PAYLOAD_OFF, true)) {
#ifdef DEBUG
      Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
      Serial.print(TOPIC_TV_BRIGHTNESS_STATUS);
      Serial.print(F(". Payload: "));
      Serial.println(PAYLOAD_OFF);
#endif
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//
// WIFI and TLS
//
///////////////////////////////////////////////////////////////////////////

/*
   Function called to setup the connection to the Wi-Fi Access Point
*/
void setupWifi() {
  delay(10);
  // attempt to connect to the Wi-Fi AP
  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASSWORD);
  // define the fixed IP address
  WiFi.config(IP, IP_GATEWAY, IP_SUBNET, IP_DNS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

#ifdef DEBUG
  Serial.println(F("INFO: Client is now connected to the Wi-Fi AP"));
  Serial.print(F("INFO: IP address: "));
  Serial.println(WiFi.localIP());
#endif

#ifdef TLS
  verifyFingerprint();
#endif
}

/*
   Function called to verify the fingerprint of the MQTT server and establish a secure connection
*/
#ifdef TLS
void verifyFingerprint() {
  if (!g_wifiClient.connect(MQTT_SERVER_IP, MQTT_SERVER_PORT)) {
#ifdef DEBUG
    Serial.println(F("ERROR: The connection failed to the secure MQTT server"));
#endif
    return;
  }

  if (g_wifiClient.verify(CA_FINGERPRINT, CA_SUBJECT)) {
#ifdef DEBUG
    Serial.println(F("INFO: The connection is secure"));
#endif
  } else {
#ifdef DEBUG
    Serial.println(F("ERROR: The given certificate does't match"));
#endif
  }
}
#endif

///////////////////////////////////////////////////////////////////////////
//
// MQTT
//
///////////////////////////////////////////////////////////////////////////

/*
   Function called when a MQTT message arrived
   @param p_topic   The topic of the MQTT message
   @param p_payload The payload of the MQTT message
   @param p_length  The length of the payload
*/
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
#ifdef DEBUG
  Serial.println(F("INFO: A new MQTT message arrived"));
  Serial.print(F("INFO: Topic: "));
  Serial.println(p_topic);
  Serial.print(F("INFO: Payload: "));
  for (int i = 0; i < p_length; i++) {
    Serial.print((char)p_payload[i]);
  }
  Serial.println();
  Serial.print(F("INFO: Length: "));
  Serial.println(p_length);
#endif
  // handle the MQTT topic of the received message
  if (String(TOPIC_LIGHT1_COMMAND).equals(p_topic)) {
    // concat the payload into a string
    String payload;
    for (uint8_t i = 0; i < p_length; i++) {
      payload.concat((char)p_payload[i]);
    }
    if (payload.equals(String(PAYLOAD_ON))) {
      if (g_light1_status != true) {
        g_light1_status = true;
        setLight1Color(g_light1_color_red, g_light1_color_green, g_light1_color_blue);
        publishLight1Status();
      }
    } else if (payload.equals(String(PAYLOAD_OFF))) {
      if (g_light1_status != false) {
        g_light1_status = false;
        setLight1Color(LOW, LOW, LOW); // switch off the light
        publishLight1Status();
      }
    }
  } else if (String(TOPIC_LIGHT1_BRIGHTNESS_COMMAND).equals(p_topic)) {
    // concat the payload into a string
    String payload;
    for (uint8_t i = 0; i < p_length; i++) {
      payload.concat((char)p_payload[i]);
    }
    uint8_t brightness = payload.toInt();
    if (brightness < 0 || brightness > 255) {
      // do nothing...
      return;
    } else {
      g_light1_brightness = brightness;
      setLight1Color(g_light1_color_red, g_light1_color_green, g_light1_color_blue);
      publishLight1BrightnessStatus();
    }
  } else if (String(TOPIC_LIGHT1_COLOR_COMMAND).equals(p_topic)) {
    // concat the payload into a string
    String payload;
    for (uint8_t i = 0; i < p_length; i++) {
      payload.concat((char)p_payload[i]);
    }
    // get the position of the first and second comma
    uint8_t firstIndex = payload.indexOf(',');
    uint8_t lastIndex = payload.lastIndexOf(',');

    // get the value for the red color
    uint8_t red_color = payload.substring(0, firstIndex).toInt();
    if (red_color < 0 || red_color > 255) {
      return;
    } else {
      g_light1_color_red = red_color;
    }
    
    // get the value for the green color
    uint8_t green_color = payload.substring(firstIndex + 1, lastIndex).toInt();
    if (green_color < 0 || green_color > 255) {
      return;
    } else {
      g_light1_color_green = green_color;
    }
    
    // get the value for the blue color
    uint8_t blue_color = payload.substring(lastIndex + 1).toInt();
    if (blue_color < 0 || blue_color > 255) {
      return;
    } else {
      g_light1_color_blue = blue_color;
    }

    setLight1Color(g_light1_color_red, g_light1_color_green, g_light1_color_blue);
    publishLight1ColorStatus();
  } else if (String(TOPIC_LIGHT2_COMMAND).equals(p_topic)) {
    // concat the payload into a string
    String payload;
    for (uint8_t i = 0; i < p_length; i++) {
      payload.concat((char)p_payload[i]);
    }
    if (payload.equals(String(PAYLOAD_ON))) {
      g_light2_status = true;
      setLight2Status();
      publishLight2Status();
    } else if (payload.equals(String(PAYLOAD_OFF))) {
      g_light2_status = false;
      setLight2Status();
      publishLight2Status();
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: The payload of the MQTT message is not valid"));
#endif
    }
  } else {
    // do nothing.....
#ifdef DEBUG
    Serial.println(F("INFO: The received MQTT message was not used"));
#endif
  }
}

/*
   Function called to reconnect the client to the MQTT broker and publish/subscribe to/from some MQTT topics
*/
void reconnect() {
  while (!g_mqttClient.connected()) {
    if (g_mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
#ifdef DEBUG
      Serial.println(F("INFO: The client is successfully connected to the MQTT broker"));
#endif
      // subscribe to the light1 command topic
      if (g_mqttClient.subscribe(TOPIC_LIGHT1_COMMAND)) {
#ifdef DEBUG
        Serial.print(F("INFO: Sending the MQTT subscribe succeeded. Topic: "));
        Serial.println(TOPIC_LIGHT1_COMMAND);
#endif
      } else {
#ifdef DEBUG
        Serial.print(F("ERROR: Sending the MQTT subscribe failed. Topic: "));
        Serial.println(TOPIC_LIGHT1_COMMAND);
#endif
      }

      // subscribe to the light1 brightness command topic
      if (g_mqttClient.subscribe(TOPIC_LIGHT1_BRIGHTNESS_COMMAND)) {
#ifdef DEBUG
        Serial.print(F("INFO: Sending the MQTT subscribe succeeded. Topic: "));
        Serial.println(TOPIC_LIGHT1_BRIGHTNESS_COMMAND);
#endif
      } else {
#ifdef DEBUG
        Serial.print(F("ERROR: Sending the MQTT subscribe failed. Topic: "));
        Serial.println(TOPIC_LIGHT1_BRIGHTNESS_COMMAND);
#endif
      }

      // subscribe to the light1 brightness command topic
      if (g_mqttClient.subscribe(TOPIC_LIGHT1_COLOR_COMMAND)) {
#ifdef DEBUG
        Serial.print(F("INFO: Sending the MQTT subscribe succeeded. Topic: "));
        Serial.println(TOPIC_LIGHT1_COLOR_COMMAND);
#endif
      } else {
#ifdef DEBUG
        Serial.print(F("ERROR: Sending the MQTT subscribe failed. Topic: "));
        Serial.println(TOPIC_LIGHT1_COLOR_COMMAND);
#endif
      }

      // subscribe to the light2 command topic
      if (g_mqttClient.subscribe(TOPIC_LIGHT2_COMMAND)) {
#ifdef DEBUG
        Serial.print(F("INFO: Sending the MQTT subscribe succeeded. Topic: "));
        Serial.println(TOPIC_LIGHT2_COMMAND);
#endif
      } else {
#ifdef DEBUG
        Serial.print(F("ERROR: Sending the MQTT subscribe failed. Topic: "));
        Serial.println(TOPIC_LIGHT2_COMMAND);
#endif
      }

      // set the initial status of lights 1 & 2
      setLight1Color(LOW, LOW, LOW);
      setLight2Status();

      // publish the initial status of lights 1 & 2
      publishLight1Status();
      publishLight1BrightnessStatus();
      publishLight1ColorStatus();
      publishLight2Status();
      publishTVStatus();
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: The connection failed with the MQTT broker"));
      Serial.print("ERROR: rc: ");
      Serial.println(g_mqttClient.state());
      // wait 5 seconds before retrying
      delay(5000);
#endif
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//
// SETUP and LOOP
//
///////////////////////////////////////////////////////////////////////////

/*
   Function called once to initialize the board
*/
void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println(F("\nINFO: The Wi-Fi module is starting..."));
#endif

  // init the LEDs as output, the TV brightness sensor as input
  pinMode(LIGHT1_RED_PIN,     OUTPUT);
  pinMode(LIGHT1_GREEN_PIN,   OUTPUT);
  pinMode(LIGHT1_BLUE_PIN,    OUTPUT);
  pinMode(LIGHT2_PIN,         OUTPUT);
  pinMode(PHOTOCELL_PIN,      INPUT);

  setupWifi();

  // set the MQTT broker IP address and port
  g_mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  // set the MQTT callback function
  g_mqttClient.setCallback(callback);

  // set the hostname & password for OTA
  ArduinoOTA.setHostname(MQTT_CLIENT_ID);
  ArduinoOTA.setPassword(OTA_PASSWORD);

#ifdef DEBUG
  ArduinoOTA.onStart([]() {
    Serial.println(F("INFO: OTA starts"));
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("INFO: OTA ends"));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.println(F("INFO: An error occurs during OTA"));
  });
#endif
  ArduinoOTA.begin();

  // blink the internal LED to indicate the end of the startup
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
  delay(100);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(100);
  digitalWrite(BUILTIN_LED, LOW);
  delay(100);
  digitalWrite(BUILTIN_LED, HIGH);
}

/*
   Function called infinitely after the setup function
*/
void loop() {
  // keep the MQTT client connected to the broker
  if (!g_mqttClient.connected()) {
    reconnect();
  }
  g_mqttClient.loop();
  
  // a delay is necessary if the debug is disabled
  // without a delay, there is some latency to switch on/off a light
  yield();
  //delay(100); 
  
  readTVBrightness();

  yield();

  ArduinoOTA.handle();

  yield();
}
