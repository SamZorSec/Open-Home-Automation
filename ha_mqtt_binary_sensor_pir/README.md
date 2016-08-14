# ha_mqtt_binary_sensor_pir - Home-Assistant
A simple example to use a PIR motion sensor connected to a nodeMCU board (esp8266)

## Configuration
configuration.yaml :
```yaml
binary_sensor:
  platform: mqtt
  state_topic: 'office/motion/status'
  name: 'Motion'
  sensor_class: motion
```