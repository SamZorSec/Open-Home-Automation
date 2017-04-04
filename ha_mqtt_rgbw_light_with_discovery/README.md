# MQTT JSON Light - Home Assistant
A simple example to control a RGB led and a white led connected to a NodeMCU board (ESP8266).

## Features
- MQTT discovery
- MQTT JSON Light
  - State
  - RGB
  - White
  - Brightness
  - Color temperature
  - Effect
    - Rainbow

## How to
1. Rename `config.example.h`to `config.h`
2. Define your WiFi SSID and password
3. Define your MQTT settings
4. Install the external libraries (PubSubClient, ArduinoJson)
4. Define `MQTT_MAX_PACKET_SIZE` to `256`in `Arduino/libraries/pubsubclient/src/PubSubClient.h`
