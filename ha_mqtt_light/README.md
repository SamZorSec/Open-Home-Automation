# ha_mqtt_light - Home-Assistant
A simple example to control the build-in led of the nodeMCU board (esp8266)

## Configuration (MQTT)
configuration.yaml :
```yaml
mqtt:
  broker: [Redacted]
  port: 1883
  client_id: home-assistant-1
  keepalive: 60
```

## Configuration (light)
configuration.yaml :
```yaml
light:
  platform: mqtt
  name: "Office light"
  state_topic: "office/light1/status"
  command_topic: "office/light1/switch"
  optimistic: false
```
