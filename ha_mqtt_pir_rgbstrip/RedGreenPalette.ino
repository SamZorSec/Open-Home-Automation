#include <FastLED.h>

//  Example showing how to make a simple striped color palette, 
//  and use it to animated blended color bars

#define LED_PIN     2
#define NUM_LEDS    60
CRGB leds[NUM_LEDS];

CRGBPalette16 currentPalette;

void setupStripedPalette( CRGB A, CRGB AB, CRGB B, CRGB BA)
{
  // Sets up a palette with alternating stripes of
  // colors "A" and "B" -- with color "AB" between 
  // where A fades into B, and color "BA" where B fades
  // into A.
  // The stripes of "A" are narrower than the stripes of "B",
  // but an equal-width arrangement is also shown.
  currentPalette = CRGBPalette16( 
        A, A, A, A, AB, B, B, B, B, B, B, B, B, B, B, BA
  //    A, A, A, A, A, A, A, AB, B, B, B, B, B, B, B, BA
  );
}

void setup() {
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<WS2812B,LED_PIN,GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // This example shows stripes with NO gaps between them; the colors
  // cross-fade at the boundaries.  An additional setup alternative is
  // provided that puts a 'black' gap between the colors, so they don't
  // bleed into each other if you prefer that.
  
  // Color stripes with NO gaps between them -- colors will crossfade
  setupStripedPalette( CRGB::Red, CRGB::Red, CRGB::Green, CRGB::Green);

  // Color stripes WITH gaps of space (black) between them
  //setupStripedPalette( CRGB::Red, CRGB::Black, CRGB::Green, CRGB::Black);
}


void loop()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 2; /* higher = faster motion */

  fill_palette( leds, NUM_LEDS, 
                startIndex, 8, /* higher = narrower stripes */ 
                currentPalette, 255, LINEARBLEND);

  FastLED.show();
  FastLED.delay(1000 / 60);
}
