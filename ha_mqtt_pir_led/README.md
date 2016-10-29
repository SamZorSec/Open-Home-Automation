# MQTT Binary Sensor - Motion - LED -  Home Assistant
A simple example to use a PIR motion sensor cnnected to a NodeMCU board (ESP8266), activating an LED 

## Configuration
configuration.yaml :
```yaml
binary_sensor:
  platform: mqtt
  state_topic: 'office/motion/status'
  name: 'Motion'
  sensor_class: motion
```

## Schematic
- PIR leg 1 - VCC
- PIR leg 2 - D1/GPIO5
- PIR leg 3 - GND
- GND - LED - Resistor 220 Ohms - D1/GPIO5

![Schematic](Schematic.png)
