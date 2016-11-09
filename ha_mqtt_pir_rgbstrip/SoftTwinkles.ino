#include "FastLED.h"

#define LED_PIN     2
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    60
CRGB leds[NUM_LEDS];

#define BRIGHTNESS 120
#define DENSITY     80

void setup() {
  FastLED.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  // Soft, warm white twinkles all along the LEDs
  softtwinkles();
  FastLED.show();
  FastLED.delay(20);
}

// For this animation to work, the Red component of this
// color MUST be nonzero, AND must be an EVEN number.
const CRGB lightcolor(8,7,1);
// If you are using FastLED setCorrection(TypicalLEDStrip), 
// then (8,7,1) gives a very soft, warm yellowish white,
// a little bit like incandescent fairy lights.  If you are
// not using FastLED color correction, try (8,5,1).


// The basic idea behind this animation is that each pixel is 
// black until it's 'bumped' with a small amount of light, at which 
// point it will get brighter and brighter until it maxes out.  
// Then it will switch direction and start getting dimmer and dimmer
// until it's black again.  That's basically the whole thing.
//
// So to implement this, each pixel needs to 'know' whether it should
// be getting brighter or getting dimmer.  We could implement this
// with a parallel array of direction indicators, but that would 
// take up extra memory.
//
// Instead, we use the lowest bit of the red component as a direction
// indicator.  Stop and re-read that previous sentence again.  We're
// using the least significant bit of one of the color components as
// an actual data value.
//
// If the lowest bit of the red component is ZERO (i.e.
// pixel.red is EVEN), it means that this pixel should be BRIGHTENING.  
// If the lowest bit of the red component is ONE (i.e. pixel.red is ODD),
// it means that this pixel should be DIMMING.
//
// In this way, the low bit of the red componetn IS the direction indicator:
// If red is EVEN, it's going up.  If red is ODD, it's coming down.
//
// Now as if that weren't complicated enough, we use the 'saturating'
// properties of CRGB pixel math to avoid having to test for when
// we've hit the maximum brightness, or for having to test for when
// we've hit total black.  
//
// The result is that we can do this whole thing eight lines of code.
//
// Here's how the code works: each pixel can be in one of three states:
//   1. It can be total black, in which case we do nothing to it.
//   2. It can have an EVEN red component, in which case we BRIGHTEN it
//   3. It can have an ODD red component, in which case we DARKEN it
// When we're brightening the red component (because it's EVEN), it will
// saturate at a maximum of 255 -- which means it will automatically 
// switch from EVEN to ODD when it hits the top of the range.  And 
// because it's now an ODD number, we'll DARKEN this pixel each time;
// the numbers start coming back down.  And when it dims all the way 
// down, it will saturate out at total black automatically.
//
void softtwinkles() {
  for( int i = 0; i < NUM_LEDS; i++) {
    if( !leds[i]) continue; // skip black pixels
    if( leds[i].r & 1) { // is red odd?
      leds[i] -= lightcolor; // darken if red is odd
    } else {
      leds[i] += lightcolor; // brighten if red is even
    }
  }
  
  // Randomly choose a pixel, and if it's black, 'bump' it up a little.
  // Since it will now have an EVEN red component, it will start getting
  // brighter over time.
  if( random8() < DENSITY) {
    int j = random16(NUM_LEDS);
    if( !leds[j] ) leds[j] = lightcolor;
  }
}


// If this still all seems confusing, that's OK.  It took me a LONG 
// time to slowly fit all these puzzle pieces together to make it work
// the way I wanted it to.  And in the end I was actually very 
// surprised at how little actual code was needed to make it work.
//   - Mark Kriegsman, December 2014
