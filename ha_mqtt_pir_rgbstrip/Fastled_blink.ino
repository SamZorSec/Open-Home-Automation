#include "FastLED.h"

        CRGB leds[1];
        void setup() { FastLED.addLeds<NEOPIXEL, 6>(leds, 1); }
        void loop() { 
                leds[0] = CRGB::White; FastLED.show(); delay(30); 
                leds[0] = CRGB::Black; FastLED.show(); delay(30);
        }
