#include "FastLED.h"

// PaletteBeat
// Shows how to pulse back and forth between two color palettes
// at a given number of beats per minute.
// 
// The two color palettes in the pulse cycle each can
// also independently morph and shift -- although choosing
// new target palettes is not shown here.
//
// Mark Kriegsman, August 2015

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    2
//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    60
#define BRIGHTNESS  255

CRGB leds[NUM_LEDS];



void setup() {
  delay(3000); // 3 second delay for recovery
  
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS)
    .setCorrection(TypicalLEDStrip) 
    .setDither(BRIGHTNESS < 255);

  FastLED.setBrightness(BRIGHTNESS);
}

void loop()
{
  PeriodicallyChooseNewColorPalettes(); // currently does nothing

  MixBeatPalette(); // mix up the new 'beat palette' to draw with

  DrawOneFrameUsingBeatPalette(); // draw a simple animation using the 'beat palette'
  
  FastLED.show();
  FastLED.delay(30);
}



CRGBPalette16 gCurrentPaletteA( CRGB::Black);
CRGBPalette16 gCurrentPaletteB( CRGB::Black);

CRGBPalette16 gTargetPaletteA( RainbowColors_p);
CRGBPalette16 gTargetPaletteB( CRGB::Red );

CRGBPalette16 gBeatPalette;

// Here's where the magic happens.  Since internally palettes are 
// nothing more than an array of CRGB, we can use the FastLED "blend" 
// function to mix up a NEW palette, which is a blend of our two current
// palettes, with the amount of blending controlled by a 'mixer' that
// pulses back and forth smoothly between the two, at a given BPM.
//
void MixBeatPalette()
{
  uint8_t paletteBeatsPerMinute = 60;

  uint8_t beat = beat8( paletteBeatsPerMinute); // repeats from 0..255 

  // 'cubicwave8' spends more time at each end than sin8, and less time
  // in the middle.  Try others: triwave8, quadwave8, sin8, cubicwave8
  uint8_t mixer = cubicwave8( beat);

  // Mix a new palette, gBeatPalette, with a varying amount of contribution
  // from gCurrentPaletteA and gCurrentPaletteB, depending on 'mixer'.
  // The 'beat palette' is then used to draw onto the LEDs.
  uint8_t palettesize = sizeof( gBeatPalette) / sizeof(gBeatPalette[0]); // = 16
  blend( gCurrentPaletteA, gCurrentPaletteB, gBeatPalette, palettesize, mixer);
}


// Sample draw function to draw some pixels using the colors in gBeatPalette
void DrawOneFrameUsingBeatPalette()
{
  uint8_t startindex = millis() / 2;
  uint8_t incindex = 7;
  fill_palette(leds, NUM_LEDS, startindex, incindex, gBeatPalette, 255, LINEARBLEND);
}


// If you wanted to change the two palettes from 'rainbow' and 'red'
// to something else, here's where you'd do it.
void PeriodicallyChooseNewColorPalettes()
{
  EVERY_N_SECONDS(11) {
    // choose new gTargetPaletteA
    // (currently not implemented)
  }

  EVERY_N_SECONDS(17) {
    // choose new gTargetPaletteB
    // (currently not implemented)
  }
  
  EVERY_N_MILLISECONDS(20) {
    nblendPaletteTowardPalette( gCurrentPaletteA, gTargetPaletteA);
    nblendPaletteTowardPalette( gCurrentPaletteB, gTargetPaletteB);
  }
}
