# ha_mqtt_switch - Home-Assistant
A simple example to control a switch connected to a nodeMCU board (esp8266)

## Configuration
configuration.yaml :
```yaml
switch:
  platform: mqtt
  name: 'Office Switch'
  state_topic: 'office/switch1/status'
  command_topic: 'office/switch1/set'
  optimistic: false
```