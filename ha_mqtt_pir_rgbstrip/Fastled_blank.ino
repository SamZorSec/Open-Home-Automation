#include <FastLED.h>
#define NUM_LEDS 60
#define DATA_PIN 2

CRGB leds[NUM_LEDS];    //sets up an array that we can manipulate to set/clear led data

//actually setup our leds (declare type of leds, and feed in #s defined above)
void setup() { 
       FastLED.addLeds<WS2812B, DATA_PIN>(leds, NUM_LEDS);
}
   
set the values of the entries in the leds array to whatever colors you want. Then you tell the library to show your data
void loop() {
      // Turn the first led red for 1 second
      leds[0] = CRGB::Red; 
      FastLED.show();
      delay(1000);

      // Set the first led back to black (off) for 1 second
      leds[0] = CRGB::Black;
      FastLED.show();
      delay(1000);
}
