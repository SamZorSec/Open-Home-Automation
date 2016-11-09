#include "FastLED.h"

// MapTwinkle
// Designed to illuminate a 'map' of pixels, each of which randomly
// sometimes twinkles brighter and then back down to it's base color again.
//
// Parameters include: background color, peak twinkle color, and speed
// of brightening and dimming.
//
// Mark Kriegsman, August 2015

#define LED_PIN     2
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    60
CRGB leds[NUM_LEDS];

#define MASTER_BRIGHTNESS   255

// Base background color
#define BASE_COLOR       CRGB(32,0,32)

// Peak color to twinkle up to
#define PEAK_COLOR       CRGB(64,0,64)


// Currently set to brighten up a bit faster than it dims down, 
// but this can be adjusted.

// Amount to increment the color by each loop as it gets brighter:
#define DELTA_COLOR_UP   CRGB(4,0,4)

// Amount to decrement the color by each loop as it gets dimmer:
#define DELTA_COLOR_DOWN CRGB(1,0,1)


// Chance of each pixel starting to brighten up.  
// 1 or 2 = a few brightening pixels at a time.
// 10 = lots of pixels brightening at a time.
#define CHANCE_OF_TWINKLE 1

void setup() {
  delay(3000);
  FastLED.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(MASTER_BRIGHTNESS);

  InitPixelStates();
}

void loop()
{
  TwinkleMapPixels();
  FastLED.show();  
  FastLED.delay(20);
}

enum { SteadyDim, GettingBrighter, GettingDimmerAgain };
uint8_t PixelState[NUM_LEDS];

void InitPixelStates()
{
  memset( PixelState, sizeof(PixelState), SteadyDim); // initialize all the pixels to SteadyDim.
  fill_solid( leds, NUM_LEDS, BASE_COLOR);
}

void TwinkleMapPixels()
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    if( PixelState[i] == SteadyDim) {
      // this pixels is currently: SteadyDim
      // so we randomly consider making it start getting brighter
      if( random8() < CHANCE_OF_TWINKLE) {
        PixelState[i] = GettingBrighter;
      }
      
    } else if( PixelState[i] == GettingBrighter ) {
      // this pixels is currently: GettingBrighter
      // so if it's at peak color, switch it to getting dimmer again
      if( leds[i] >= PEAK_COLOR ) {
        PixelState[i] = GettingDimmerAgain;
      } else {
        // otherwise, just keep brightening it:
        leds[i] += DELTA_COLOR_UP;
      }
      
    } else { // getting dimmer again
      // this pixels is currently: GettingDimmerAgain
      // so if it's back to base color, switch it to steady dim
      if( leds[i] <= BASE_COLOR ) {
        leds[i] = BASE_COLOR; // reset to exact base color, in case we overshot
        PixelState[i] = SteadyDim;
      } else {
        // otherwise, just keep dimming it down:
        leds[i] -= DELTA_COLOR_DOWN;
      }
    }
  }
}
