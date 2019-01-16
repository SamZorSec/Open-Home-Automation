# MQTT Light - Arilux - Home Assistant
This sketch is an alternative firmware for Arilux LED controllers, based on the [WS2812FX](https://github.com/kitesurfer1404/WS2812FX) library and is compatible with `Home Assistant` through the `MQTT` protocol. 

## Configuration
To configure this sketch, you have to rename the header file `example.config.h` in `config.h`. Then, it is possible to modify the `name` and the `location` of the device or edit your Wi-Fi and MQTT credentials. Note that the device `name` and `location` are used to create the MQTT topics.

### Device location and name
```
#define DEVICE_LOCATION "bedroom"
#define DEVICE_NAME     "arilux" // also used as MQTT ID
```

### Credentials
Wi-Fi :

```
#define WIFI_SSID     ""
#define WIFI_PASSWORD ""
```
MQTT :

```
#define MQTT_USERNAME     ""
#define MQTT_PASSWORD     ""
#define MQTT_SERVER       ""
#define MQTT_SERVER_PORT  1883
```

### Home Assistant
To integrate the light in Home Assistant, please edit and add this snippet into your configuration.

```yaml
# Example configuration.yml entry
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
```

## Flash the firmware and control
More information available [here](https://github.com/mertenats/Arilux_AL-LC0X).

Example of a returned state :
```
{
  "state" : "ON",
  "brightness" : 255,
  "color" : {
    "r" : 0,
    "g" : 250,
    "b" : 0
  },
  "white_value" : 0,
  "effect" : "Static"
}
```

## Licence
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*If you like the content of this repo, please add a star! Thank you!*
