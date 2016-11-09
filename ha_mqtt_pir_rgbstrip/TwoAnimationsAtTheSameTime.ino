#include "FastLED.h"

// TwoAnimationsAtTheSameTime
//   Example showing one way to run two different animations on
//   two different parts of one LED array at the same time.
//
// The three keys to success here are:
//
//   1. Move the drawing of each animation into a separate 'draw' function, 
//      each of which is called from 'loop()'.  Each 'draw' function draws
//      just one frame of it's particular animation and then returns.
// 
//   2. The draw functions each take two arguments that tell them
//      where to draw: a starting LED number, and a length.  All updating
//      the leds array done inside the draw function should use these
//      arguments.
//
//   3. Move any 'FastLED.show()' and 'delay()' calls OUT from the draw
//      functions and into 'loop()'.
// 
// -Mark Kriegsman, July 2015

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    2
//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    60
CRGB leds[NUM_LEDS];

#define BRIGHTNESS         96
#define FRAMES_PER_SECOND  100

uint8_t gHue = 0; // rotating "base color" used by both patterns

void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

// The loop function calls each 'draw' function as needed to put
// some color data into some portion of the LED strip.
// The loop function then calls FastLED.show() and delay().
void loop()
{
  // Here's where we decide what two segments of the LED strip
  // each drawing routine will draw into.  You can set up any 
  // ranges of LEDs that you like here.  For this demo, the "left"
  // 1/4th of the LEDs will draw 'confetti', and the "right" 3/4ths
  // will draw a rainbow.  A few pixels are left blank between them.
  int blankPixels = 2;
  
  // "Left" segment will be pixels 0..(NUM_LEDS/4)-1, i.e. the low-numbered quarter.
  // If you have 60 pixels, leftStart = 0, and leftLength = 15.
  int leftStart = 0;
  int leftLength = (NUM_LEDS / 4);
  
  // "Right" segment will be pixels from end of left segment to NUM_PIXELS-1, 
  // i.e. the high-numbered three-quarters of the strip.
  // If you have 60 pixels, rightStart = 10, and rightLength = 45.
  int rightStart = leftStart + leftLength + blankPixels;
  int rightLength = NUM_LEDS - rightStart - blankPixels;


  // Draw confetti on the left side.  This does NOT call FastLED.show() or delay().
  drawConfetti( leftStart, leftLength);
  
  // Draw rainbow on the right side.  This does NOT call FastLED.show() or delay().
  drawRainbow( rightStart, rightLength);

  
  // Now, here's where we call FastLED.show() and delay() -- only from loop() directly.
  
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 10 ) { gHue--; } // slowly cycle the "base color" through the rainbow
}

// Animation 'draw' functions
// Each animation draw function takes a starting pixel number
// and a count of how many pixels to draw.
// These functions do NOT call FastLED.show() or delay().
//
// For purposes of illustration, the 'classic' way of writing the code
// has been included, commented out, and the new parameterized version
// is included below.

void drawRainbow(int startpixel, int pixelcount) 
{
  // FastLED's built-in rainbow generator
  
  //fill_rainbow( leds             , NUM_LEDS,   gHue, 17);
    fill_rainbow( leds + startpixel, pixelcount, gHue, 17);
}

void drawConfetti(int startpixel, int pixelcount) 
{
  // random colored speckles that blink in and fade smoothly
  
  //fadeToBlackBy( leds             , NUM_LEDS  , 30);
    fadeToBlackBy( leds + startpixel, pixelcount, 30);
    
  //int pos = random16( NUM_LEDS);
    int pos = random16( pixelcount);
    
  //leds[pos             ] += CHSV( gHue -32 + random8(64), 200, 255);
    leds[pos + startpixel] += CHSV( gHue -32 + random8(64), 200, 255);
}
