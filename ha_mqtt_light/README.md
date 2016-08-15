# MQTT Light - Home-Assistant
A simple example to control a led connected to a NodeMCU board (ESP8266).

## Configuration
configuration.yaml :
```yaml
light:
  platform: mqtt
  name: Office light'
  state_topic: 'office/light1/status'
  command_topic: 'office/light1/switch'
  optimistic: false
```

## Schematic
- GND - LED - Resistor 220 Ohms - D1/GPIO5

![Schematic](Schematic.png)
