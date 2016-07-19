# ha_mqtt_light - Home-Assistant
A simple example to control the built-in led of the nodeMCU board (esp8266)

## Configuration
configuration.yaml :
```yaml
light:
  platform: mqtt
  name: "Office light"
  state_topic: "office/light1/status"
  command_topic: "office/light1/switch"
  optimistic: false
```
