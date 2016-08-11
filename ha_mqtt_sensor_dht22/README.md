# ha_mqtt_sensor_dht22 - Home-Assistant
A simple example to get temperature and humidity every ten minutes from a DHT22 sensor connected to a nodeMCU board (esp8266)


## Configuration
configuration.yaml :
```yaml
# Weather Prediction
sensor:
  platform: yr

sensor 1:
  platform: mqtt
  state_topic: 'office/sensor1'
  name: 'Temperature'
  unit_of_measurement: '°C'
  value_template: '{{ value_json.temperature }}'

sensor 2:
  platform: mqtt
  state_topic: 'office/sensor1'
  name: 'Humidity'
  unit_of_measurement: '%'
  value_template: '{{ value_json.humidity }}'
```