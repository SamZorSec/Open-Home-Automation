/*
  Configuration for Home Assistant :
  
  light:
    - platform: mqtt
      schema: json
      state_topic: 'bedroom/arilux/state'
      command_topic: 'bedroom/arilux/set'
      brightness: true
      rgb: true
      white_value: true
      effect: true
      effect_list: 
        - 'Static'
        - 'Blink'
        - 'Breath'
        - 'Random Color'
        - 'Rainbow'
        - 'Fade'
        - 'Strobe'
        - 'Strobe Rainbow'
        - 'Multi Strobe'
        - 'Blink Rainbow'
        - 'Comet'
        - 'Fire Flicker'
        - 'Halloween'
 */
 
#include <ArduinoOTA.h>
#include <PubSubClient.h>   // https://github.com/knolleary/pubsubclient
#include <ArduinoJson.h>    // https://github.com/bblanchon/ArduinoJson
#include <WS2812FX.h>       // https://github.com/kitesurfer1404/WS2812FX

#include "config.h"

struct RGBW {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t white;
  bool    isWhiteSelected;
  uint8_t brightness;
};

WiFiClient    wifiClient;
PubSubClient  mqttClient(wifiClient);
WS2812FX      ws2812fx = WS2812FX(1, NULL, NEO_RGBW);
RGBW          rgbw = {PWM_MIN,PWM_MIN,PWM_MIN,PWM_MIN,false,PWM_MAX};

long lastReconnectAttempt = 0;
bool newStateToPublish = false;

///////////////////////////////////////////////////////////////////////////////////////////////
// LIGHT
///////////////////////////////////////////////////////////////////////////////////////////////
uint8_t getRedValueFromRGB(uint32_t p_color) {return p_color >> 16;}
uint8_t getGreenValueFromRGB(uint32_t p_color) {return p_color >> 8;}
uint8_t getBlueValueFromRGB(uint32_t p_color) {return p_color;}

void customShow(void) {
  // get the current color 
  uint32 rgb = ws2812fx.getPixelColor(0);
  
  // retrieve the value for each channel
  rgbw.red =    getRedValueFromRGB(rgb);
  rgbw.green =  getGreenValueFromRGB(rgb);
  rgbw.blue =   getBlueValueFromRGB(rgb);
  
  // set each channel with the new value
  analogWrite(RED_PIN,    map(rgbw.red,   PWM_MIN, PWM_MAX, PWM_MIN, rgbw.brightness));
  analogWrite(GREEN_PIN,  map(rgbw.green, PWM_MIN, PWM_MAX, PWM_MIN, rgbw.brightness));
  analogWrite(BLUE_PIN,   map(rgbw.blue,  PWM_MIN, PWM_MAX, PWM_MIN, rgbw.brightness));
  analogWrite(WHITE_PIN,  map(rgbw.white, PWM_MIN, PWM_MAX, PWM_MIN, rgbw.brightness));
}

void setupLight(void) {
  // set the pins as output
  pinMode(RED_PIN,    OUTPUT);
  pinMode(GREEN_PIN,  OUTPUT);
  pinMode(BLUE_PIN,   OUTPUT);
  pinMode(WHITE_PIN,  OUTPUT);
  
  analogWriteFreq(PWM_FREQUENCY);
  analogWriteRange(PWM_MAX);

  ws2812fx.init();
  ws2812fx.setColor(GREEN);
  ws2812fx.stop();
  ws2812fx.setCustomShow(customShow);
}


///////////////////////////////////////////////////////////////////////////////////////////////
// MQTT
///////////////////////////////////////////////////////////////////////////////////////////////

void callback(char* p_topic, byte* p_payload, unsigned int p_length) {  
  String topic(p_topic);

  if (topic.equals(MQTT_CMD_TOPIC)) {    
    DynamicJsonBuffer dynamicJsonBuffer;
    JsonObject& root = dynamicJsonBuffer.parseObject(p_payload);
 
    // check if the payload contains a new state
    if (root.containsKey("state")) {
      if (strcmp(root["state"], MQTT_STATE_ON_PAYLOAD) == 0) {
        if (ws2812fx.isRunning() == false) {
          if (rgbw.isWhiteSelected == true) {
            rgbw.white = PWM_MAX;
          }

          ws2812fx.start();
          newStateToPublish = true;
        }
      } else if (strcmp(root["state"], MQTT_STATE_OFF_PAYLOAD) == 0) {
        if (ws2812fx.isRunning() == true) {
          if (rgbw.isWhiteSelected == true) {
            rgbw.white = PWM_MIN;
          }
          
          ws2812fx.stop();
          newStateToPublish = true;
        }
      }
    }

    // check if the payload contains a new color value
    if (root.containsKey("color")) {
      uint8_t red = root["color"]["r"];
      uint8_t green = root["color"]["g"];
      uint8_t blue = root["color"]["b"];
      
      if ((red >= PWM_MIN || red <= PWM_MAX) && (green >= PWM_MIN || green <= PWM_MAX) && (blue >= PWM_MIN || blue <= PWM_MAX)) {
        rgbw.white = PWM_MIN;
        rgbw.isWhiteSelected = false;
        ws2812fx.setMode(FX_MODE_STATIC);
        ws2812fx.setColor(red, green, blue);
        newStateToPublish = true;
      }
    }
    
    // check if the payload contains a new brightness value
    if (root.containsKey("brightness")) {
      uint8_t brightness = root["brightness"];
      
      if (brightness >= PWM_MIN || brightness <= PWM_MAX) {
        rgbw.brightness = brightness;
        newStateToPublish = true;
      }
    }

    // check if the payload contains a new white value
    if (root.containsKey("white_value")) {
      uint8_t white_value = root["white_value"];
      
      if (white_value >= PWM_MIN || white_value <= PWM_MAX) {
        rgbw.white = PWM_MAX;
        rgbw.brightness = white_value;
        rgbw.isWhiteSelected = true;
        ws2812fx.setMode(FX_MODE_STATIC);
        ws2812fx.setColor(0, 0, 0); 
        newStateToPublish = true;
      }
    }

    // check if the payload contains a new effect value
    if (root.containsKey("effect")) {
      const char* effect = root["effect"];

      // TODO check if current effect is different
      rgbw.white = PWM_MIN;
      rgbw.isWhiteSelected = false;
      newStateToPublish = true;

      if (strcmp_P(effect, (const char*)name_0) == 0) {
        ws2812fx.setMode(FX_MODE_STATIC);
      } else if (strcmp_P(effect, (const char*)name_1) == 0) {
        ws2812fx.setMode(FX_MODE_BLINK);
      } else if (strcmp_P(effect, (const char*)name_2) == 0) {
        ws2812fx.setMode(FX_MODE_BREATH);
      } else if (strcmp_P(effect, (const char*)name_8) == 0) {
        ws2812fx.setMode(FX_MODE_RANDOM_COLOR);
      } else if (strcmp_P(effect, (const char*)name_11) == 0) {
        ws2812fx.setMode(FX_MODE_RAINBOW);
      } else if (strcmp_P(effect, (const char*)name_15) == 0) {
        ws2812fx.setMode(FX_MODE_FADE);
      } else if (strcmp_P(effect, (const char*)name_26) == 0) {
        ws2812fx.setMode(FX_MODE_STROBE);
      } else if (strcmp_P(effect, (const char*)name_27) == 0) {
        ws2812fx.setMode(FX_MODE_STROBE_RAINBOW);
      } else if (strcmp_P(effect, (const char*)name_28) == 0) {
        ws2812fx.setMode(FX_MODE_MULTI_STROBE);
      } else if (strcmp_P(effect, (const char*)name_29) == 0) {
        ws2812fx.setMode(FX_MODE_BLINK_RAINBOW);
      } else if (strcmp_P(effect, (const char*)name_44) == 0) {
        ws2812fx.setMode(FX_MODE_COMET);
      } else if (strcmp_P(effect, (const char*)name_48) == 0) {
        ws2812fx.setMode(FX_MODE_FIRE_FLICKER);
      } else if (strcmp_P(effect, (const char*)name_52) == 0) {
        ws2812fx.setMode(FX_MODE_HALLOWEEN);
      } else {
        ws2812fx.setMode(FX_MODE_BLINK);
      }   
    }
  }
}

void sendState(void) {
  DynamicJsonBuffer dynamicJsonBuffer;
  JsonObject& root = dynamicJsonBuffer.createObject();
  root["state"] = ws2812fx.isRunning() ? MQTT_STATE_ON_PAYLOAD : MQTT_STATE_OFF_PAYLOAD;
  root["brightness"] = rgbw.brightness;
  JsonObject& color = root.createNestedObject("color");
  color["r"] = rgbw.red;
  color["g"] = rgbw.green;
  color["b"] = rgbw.blue;
  root["white_value"] = rgbw.white == 0 ? 0 : rgbw.brightness;
  root["effect"] = ws2812fx.getModeName(ws2812fx.getMode());//"Static";

  char tmp[128] = {0};
  root.printTo(tmp);
  mqttClient.publish(MQTT_STATE_TOPIC, tmp);
}

bool reconnect() {
  if (mqttClient.connect(DEVICE_NAME, MQTT_USERNAME, MQTT_PASSWORD, MQTT_AVAILABILITY_TOPIC, 0, 1, MQTT_NOT_AVAILABLE_PAYLOAD)) {
    mqttClient.subscribe(MQTT_CMD_TOPIC);
    mqttClient.publish(MQTT_AVAILABILITY_TOPIC, MQTT_AVAILABLE_PAYLOAD);

    sendState();
  }
  return mqttClient.connected();
}

void setupMQTT(void) {
  mqttClient.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
  mqttClient.setCallback(callback);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// WIFI
///////////////////////////////////////////////////////////////////////////////////////////////

void setupWiFi(void) {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

///////////////////////////////////////////////////////////////////////////////////////////////
// SETUP and LOOP
///////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  delay(10);

  setupWiFi();

  setupMQTT();

  setupLight();
  
  ArduinoOTA.begin();
}


void loop() {
  if (!mqttClient.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      if (reconnect())
        lastReconnectAttempt = 0;
    }
  } else {
    mqttClient.loop();
  }
  
  ws2812fx.service();
  
  yield();

  if (newStateToPublish) {
    newStateToPublish = false;
    sendState();  
  }

  yield();
  
  ArduinoOTA.handle();
}
