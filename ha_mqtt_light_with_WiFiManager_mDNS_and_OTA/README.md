# MQTT Light - Home-Assistant
A simple example to control the built-in led connected to a NodeMCU board (ESP8266).

## Advanced features
- WiFiManager:	To list the available Wifi AP and to connecte to one of them
- mDNS:			To look for the IP address and the port of the local MQTT broker
- OTA: 			To update the firmware over-the-air

## Configuration
### MQTT broker server (a RPi, for example)
Steps:
- sudo apt-get install avahi-deamon (installed by default)
- sudo cat /etc/avahi/services/mqtt.service
- Copy/paste the following lines:
```xml
<?xml version="1.0" standalone='no'?>
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
    <name replace-wildcards="yes">%h</name>
    <service>
        <type>_mqtt._tcp</type>
        <port>1883</port>
    </service>
</service-group>
```
- sudo service avahi-daemon restart

### Home-Assistant
configuration.yaml :
```yaml
light:
  platform: mqtt
  name: 'Office light'
  state_topic: '9A6C95/light'
  command_topic: '9A6C95/light/switch'
  optimistic: false
```
