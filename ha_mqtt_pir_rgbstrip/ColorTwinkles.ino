#include "FastLED.h"

#define LED_PIN     2
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    60
CRGB leds[NUM_LEDS];

//  Twinkling 'holiday' lights that fade up and down in brightness.
//  Colors are chosen from a palette; a few palettes are provided.
//
//  The basic operation is that all pixels stay black until they
//  are 'seeded' with a relatively dim color.  The dim colors
//  are repeatedly brightened until they reach full brightness, then
//  are darkened repeatedly until they are fully black again.
//
//  A set of 'directionFlags' is used to track whether a given
//  pixel is presently brightening up or darkening down.
//
//  For illustration purposes, two implementations of directionFlags
//  are provided: a simple one-byte-per-pixel flag, and a more
//  complicated, more compact one-BIT-per-pixel flag.
//
//  Darkening colors accurately is relatively easy: scale down the 
//  existing color channel values.  Brightening colors is a bit more
//  error prone, as there's some loss of precision.  If your colors
//  aren't coming our 'right' at full brightness, try increasing the
//  STARTING_BRIGHTNESS value.
//
//  -Mark Kriegsman, December 2014
 
#define MASTER_BRIGHTNESS   96

#define STARTING_BRIGHTNESS 64
#define FADE_IN_SPEED       32
#define FADE_OUT_SPEED      20
#define DENSITY            255

void setup() {
  delay(3000);
  FastLED.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(MASTER_BRIGHTNESS);
}

void loop()
{
  chooseColorPalette();
  colortwinkles();
  FastLED.show();  
  FastLED.delay(20);
}


CRGBPalette16 gPalette;

void chooseColorPalette()
{
  uint8_t numberOfPalettes = 5;
  uint8_t secondsPerPalette = 10;
  uint8_t whichPalette = (millis()/(1000*secondsPerPalette)) % numberOfPalettes;

  CRGB r(CRGB::Red), b(CRGB::Blue), w(85,85,85), g(CRGB::Green), W(CRGB::White), l(0xE1A024);

  switch( whichPalette) {  
    case 0: // Red, Green, and White
      gPalette = CRGBPalette16( r,r,r,r, r,r,r,r, g,g,g,g, w,w,w,w ); 
      break;
    case 1: // Blue and White
      //gPalette = CRGBPalette16( b,b,b,b, b,b,b,b, w,w,w,w, w,w,w,w ); 
      gPalette = CloudColors_p; // Blues and whites!
      break;
    case 2: // Rainbow of colors
      gPalette = RainbowColors_p;
      break;
    case 3: // Incandescent "fairy lights"
      gPalette = CRGBPalette16( l,l,l,l, l,l,l,l, l,l,l,l, l,l,l,l );
      break;
    case 4: // Snow
      gPalette = CRGBPalette16( W,W,W,W, w,w,w,w, w,w,w,w, w,w,w,w );
      break;
  }
}

enum { GETTING_DARKER = 0, GETTING_BRIGHTER = 1 };

void colortwinkles()
{
  // Make each pixel brighter or darker, depending on
  // its 'direction' flag.
  brightenOrDarkenEachPixel( FADE_IN_SPEED, FADE_OUT_SPEED);
  
  // Now consider adding a new random twinkle
  if( random8() < DENSITY ) {
    int pos = random16(NUM_LEDS);
    if( !leds[pos]) {
      leds[pos] = ColorFromPalette( gPalette, random8(), STARTING_BRIGHTNESS, NOBLEND);
      setPixelDirection(pos, GETTING_BRIGHTER);
    }
  }
}

void brightenOrDarkenEachPixel( fract8 fadeUpAmount, fract8 fadeDownAmount)
{
 for( uint16_t i = 0; i < NUM_LEDS; i++) {
    if( getPixelDirection(i) == GETTING_DARKER) {
      // This pixel is getting darker
      leds[i] = makeDarker( leds[i], fadeDownAmount);
    } else {
      // This pixel is getting brighter
      leds[i] = makeBrighter( leds[i], fadeUpAmount);
      // now check to see if we've maxxed out the brightness
      if( leds[i].r == 255 || leds[i].g == 255 || leds[i].b == 255) {
        // if so, turn around and start getting darker
        setPixelDirection(i, GETTING_DARKER);
      }
    }
  }
}

CRGB makeBrighter( const CRGB& color, fract8 howMuchBrighter) 
{
  CRGB incrementalColor = color;
  incrementalColor.nscale8( howMuchBrighter);
  return color + incrementalColor;
}

CRGB makeDarker( const CRGB& color, fract8 howMuchDarker) 
{
  CRGB newcolor = color;
  newcolor.nscale8( 255 - howMuchDarker);
  return newcolor;
}


// For illustration purposes, there are two separate implementations
// provided here for the array of 'directionFlags': 
// - a simple one, which uses one byte (8 bits) of RAM for each pixel, and
// - a compact one, which uses just one BIT of RAM for each pixel.

// Set this to 1 or 8 to select which implementation
// of directionFlags is used.  1=more compact, 8=simpler.
#define BITS_PER_DIRECTION_FLAG 1


#if BITS_PER_DIRECTION_FLAG == 8
// Simple implementation of the directionFlags array,
// which takes up one byte (eight bits) per pixel.
uint8_t directionFlags[NUM_LEDS];

bool getPixelDirection( uint16_t i) {
  return directionFlags[i];
}

void setPixelDirection( uint16_t i, bool dir) {
  directionFlags[i] = dir;
}
#endif


#if BITS_PER_DIRECTION_FLAG == 1
// Compact (but more complicated) implementation of
// the directionFlags array, using just one BIT of RAM
// per pixel.  This requires a bunch of bit wrangling,
// but conserves precious RAM.  The cost is a few
// cycles and about 100 bytes of flash program memory.
uint8_t  directionFlags[ (NUM_LEDS+7) / 8];

bool getPixelDirection( uint16_t i) {
  uint16_t index = i / 8;
  uint8_t  bitNum = i & 0x07;
  // using Arduino 'bitRead' function; expanded code below
  return bitRead( directionFlags[index], bitNum);
  // uint8_t  andMask = 1 << bitNum;
  // return (directionFlags[index] & andMask) != 0;
}

void setPixelDirection( uint16_t i, bool dir) {
  uint16_t index = i / 8;
  uint8_t  bitNum = i & 0x07;
  // using Arduino 'bitWrite' function; expanded code below
  bitWrite( directionFlags[index], bitNum, dir);
  //  uint8_t  orMask = 1 << bitNum;
  //  uint8_t andMask = 255 - orMask;
  //  uint8_t value = directionFlags[index] & andMask;
  //  if( dir ) {
  //    value += orMask;
  //  }
  //  directionFlags[index] = value;
}
#endif
