#include "FastLED.h"

//  MarqueeOverlay
//
//  Code to overlay a 'marquee' effect on top
//  of other animations, e.g. to highlight individual
//  letters of a sign one after another, while
//  still allowing the underlying animations to show through.
//
//  Initially designed for the "YUM cart" for Burning Man 2015.
//  The YUM cart has six segments of 124 LEDs each.  
//  This example code assumes merely one sixty-LED strip,
//  but includes definitions for 6 * 124 segments as well.
//
//  Also initially designed to run on a Teensy 3.1,
//  where SRAM is relatively plentiful.  This code
//  runs fine on an AVR Arduino, but much of the static
//  data could be moved into PROGMEM to save SRAM there.
//
//  Mark Kriegsman, August 2015.

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
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS)
    .setCorrection(TypicalLEDStrip) // cpt-city palettes have different color balance
    .setDither(BRIGHTNESS < 255);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

}


void loop()
{
  DrawRegularPattern(); // here's where you draw your regular animation

  // Then you add this, and applies various 'marquee' effects, from time to time
  // across the various segments of LED strip that you define (below).
  ApplyMarqueeEffect(); 

  FastLED.show();
  FastLED.delay(10);
}



// This is just a stand-in for whatever your regular animations are.
void DrawRegularPattern()
{
  static uint8_t start = 0;
  start -= 1;
  uint8_t hue = start;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV( hue, 240, 192);
    hue += 3;
  }
}



///////////////////  Start of Marquee animation effects //////////////////
//
// Activate these by putting "ApplyMarqueeEffect();" just before
// your call to FastLED.show.
//

// How often to start a new marquee animation, in seconds:
const uint32_t gMarqueeCycleSeconds = 10;

// How long the marquee animation should run, in seconds:
const uint32_t gMarqueeEffectSeconds = 3;

// How long each 'step' of the marquee animation is, in milliseconds:
const uint16_t gMarqueeSequenceStepTimeMsMin =  50;
const uint16_t gMarqueeSequenceStepTimeMsMax = 450;


typedef struct TMarqueeSegment {
  CRGB* mBase;
  uint16_t mLength;
} TMarqueeSegment;

// Definitions for the "segments" of the LED strip(s) to 
// be treated as separate areas.  They can be in 
// different CRGB arrays, or all in the same one, or any combination.
// Each segment is defined as a starting point in an array, and a length.
#ifndef NUM_LEDS_PER_STRIP
// Sixty-pixel demo, with six segments in it.
TMarqueeSegment gMarqueeSegments[] = {
  { leds +  0 * (NUM_LEDS/6), (NUM_LEDS/6)},
  { leds +  1 * (NUM_LEDS/6), (NUM_LEDS/6)},
  { leds +  2 * (NUM_LEDS/6), (NUM_LEDS/6)},
  { leds +  3 * (NUM_LEDS/6), (NUM_LEDS/6)},
  { leds +  4 * (NUM_LEDS/6), (NUM_LEDS/6)},
  { leds +  5 * (NUM_LEDS/6), (NUM_LEDS/6)}
};

#else
// If you just so happen to have six segments 
// with NUM_LEDS_PER_STRIP pixels each, you could do this:
TMarqueeSegment gMarqueeSegments[] = {
  { leds +  (0*NUM_LEDS_PER_STRIP), NUM_LEDS_PER_STRIP},
  { leds +  (1*NUM_LEDS_PER_STRIP), NUM_LEDS_PER_STRIP},
  { leds +  (2*NUM_LEDS_PER_STRIP), NUM_LEDS_PER_STRIP},
  { leds +  (3*NUM_LEDS_PER_STRIP), NUM_LEDS_PER_STRIP},
  { leds +  (4*NUM_LEDS_PER_STRIP), NUM_LEDS_PER_STRIP},
  { leds +  (5*NUM_LEDS_PER_STRIP), NUM_LEDS_PER_STRIP}
};
#endif


// Animated sequence patterns for the marquees.  
// Single digits refer to individual segments, '*' means all segments.
// So to affect segment 1, then segment 2, then segment 3, then all of them,
// you'd write the pattern "123*".
// If you have more physical segments than accounted for in the pattern,
// the pattern repeats across all segments.  So for example, if you have
// six segments, but a pattern only uses the tokens 1, 2, and 3, the pattern
// is duplicated from segments 1..3 onto segments 4..6.
// The patterns listed here are executed in sequence, one after another.
const char* gMarqueeSequencePatterns[] = {
  " 123",
  "123456",
  "112233",
  "123 456 ",
  "1 * 2 * 3 ",
  "* ",
  "1 2 3 ",
  "14253625",
  "123",
  "*  "
};

// There are multiple visual overlay effects used to create 
// the marquee animation.  
// If you don't like one of the effects, just comment out that line.
// You can also write your own overlay effects and just add them 
// to this array.
// Visual effects are chosen at random.
typedef void (*TMarqueeSegmentEffect)( const struct TMarqueeSegment& segment);
const TMarqueeSegmentEffect gMarqueeSegmentEffects[] = {
  
  // Effects that sprinkle 'glitter' on the background pattern
  MarqueeEffectGlitterWhite,
  MarqueeEffectGlitterBlack,
  MarqueeEffectGlitterColor,

  // Effects that create a crawling marquee on the background pattern
  MarqueeEffectCrawlWhite,
  MarqueeEffectCrawlBlack,
  MarqueeEffectCrawlColor,

  // Effects that display a steady, solid color
  MarqueeEffectSolidWhite,
  MarqueeEffectSolidBlack,
  MarqueeEffectSolidColor,

  // Effects that rapidly flicker a solid color
  MarqueeEffectFlashWhite,
  MarqueeEffectFlashBlack,
  MarqueeEffectFlashColor,

  // Effect that changes the hue of the background pattern
  MarqueeEffectRotateColor
};

const uint8_t kMarqueeSequenceCount = sizeof( gMarqueeSequencePatterns ) / sizeof( const char*);
const uint8_t kMarqueeSegmentCount = sizeof(gMarqueeSegments) / sizeof( TMarqueeSegment);
const uint8_t kMarqueeSegmentEffectCount = sizeof( gMarqueeSegmentEffects) / sizeof( TMarqueeSegmentEffect);

uint8_t gMarqueeCurrentPatternNumber = 0;
uint8_t gMarqueeSegmentEffectNumber = 0;



void ApplyMarqueeEffect()
{
  static bool running = true;
  static uint32_t posStartTimeMs = 0;
  static uint8_t seqpos = 0;
  
  uint32_t curTimeMs = millis();
  uint32_t cycletime = curTimeMs % (uint32_t)(gMarqueeCycleSeconds * 1000L);
  bool shouldRun = cycletime < (uint32_t)(gMarqueeEffectSeconds * 1000L);

  // Step 1: figure out if we're "off duty" right now, and if so,
  // is it time to start up again?
  if( !running ) {
    if( shouldRun ) {
      // start
      running = true;
      posStartTimeMs = curTimeMs;
      seqpos = 0;
      MarqueeChooseNewSegmentEffect();
      gMarqueeCurrentPatternNumber = addmod8( gMarqueeCurrentPatternNumber, 1, kMarqueeSequenceCount);
    } else {
      return;
    }
  }
  

  // Step 2: figure out which where we are in the current Marquee sequence
  uint8_t len = MarqueeGetSequenceLength();
  uint16_t gMarqueeSequenceStepTimeMs = beatsin88( 555, gMarqueeSequenceStepTimeMsMin, gMarqueeSequenceStepTimeMsMax);
  
  if( (curTimeMs - posStartTimeMs) >= gMarqueeSequenceStepTimeMs) {
    seqpos++;
    posStartTimeMs = curTimeMs;
  }
  if( seqpos >= len) {
    seqpos = 0;
    if( !shouldRun) {
      running = false;
      return;
    }
  }
  

  // Step 3: figure out which LED strip segment(s) to apply
  // the effect to.
  const char* cp = gMarqueeSequencePatterns[gMarqueeCurrentPatternNumber];
  char c = cp[seqpos];
  if( c == ' ') return;

  uint8_t segno = 0;
  if( c >= '0' && c <= '9') {
    segno = c - '0';
  } else if (c == '*') {
    segno = 255; // meaning "all"
  }

  // junk characters = ignore;
  if( segno == 0) return;
  
  // apply to all segments
  if( segno == 255) {
    for( uint8_t s = 0; s < kMarqueeSegmentCount; s++) {
      ApplyEffectToSegment( gMarqueeSegments[ s]);
    }
  } else {
    // just one segment phase
    segno--; // adjust for zero-index
    uint8_t maxNamedSegment = MarqueeGetMax();
    for( uint8_t s = segno; s < kMarqueeSegmentCount; s += maxNamedSegment) {
      ApplyEffectToSegment( gMarqueeSegments[ s]);
    }
  }
}

uint8_t MarqueeGetSequenceLength()
{
  return strlen( gMarqueeSequencePatterns[gMarqueeCurrentPatternNumber]);
}

uint8_t MarqueeGetMax()
{
  uint8_t mx = 1;
  const char* cp = gMarqueeSequencePatterns[gMarqueeCurrentPatternNumber];
  char c;
  while( (c = *cp)) {
    if( c >= '0' && c <= '9') {
      uint8_t n = c - '0';
      if( n > mx ) {
        mx = n;
      }
    }
    cp++;
  }
  return mx;
}


void MarqueeChooseNewSegmentEffect()
{
  gMarqueeSegmentEffectNumber = random8( kMarqueeSegmentEffectCount );
}

void ApplyEffectToSegment( const struct TMarqueeSegment& segment)
{
  TMarqueeSegmentEffect effect = gMarqueeSegmentEffects[ gMarqueeSegmentEffectNumber ];
  (effect)(segment);
}

void MarqueeEffectDim( const struct TMarqueeSegment& segment)
{
  fadeToBlackBy( segment.mBase, segment.mLength, 192);
}

void MarqueeEffectFlashBlack( const struct TMarqueeSegment& segment)
{
  MarqueeEffectFlashX( segment, CRGB::Black);
}

void MarqueeEffectFlashWhite( const struct TMarqueeSegment& segment)
{
  MarqueeEffectFlashX( segment, CRGB(120,120,160)); // not FFFFFF for power reasons!
}

void MarqueeEffectFlashColor( const struct TMarqueeSegment& segment)
{
  MarqueeEffectFlashX( segment, CHSV( millis() / 16, 200, 255));
}

void MarqueeEffectFlashX( const struct TMarqueeSegment& segment, const CRGB& color)
{
  uint8_t m = millis() & 0x20;
  if( m ) {
    fill_solid( segment.mBase, segment.mLength, color);
  }
}


void MarqueeEffectSolidBlack( const struct TMarqueeSegment& segment)
{
  MarqueeEffectSolidX( segment, CRGB::Black);
}

void MarqueeEffectSolidWhite( const struct TMarqueeSegment& segment)
{
  MarqueeEffectSolidX( segment, CRGB(120,120,160)); // not FFFFFF for power reasons!
}

void MarqueeEffectSolidColor( const struct TMarqueeSegment& segment)
{
  MarqueeEffectSolidX( segment, CHSV( millis() / 16, 200, 255));
}

void MarqueeEffectSolidX( const struct TMarqueeSegment& segment, const CRGB& color)
{
  fill_solid( segment.mBase, segment.mLength, color);
}



void MarqueeEffectGlitterBlack( const struct TMarqueeSegment& segment)
{
  MarqueeEffectGlitterX( segment, CRGB::Black, 2);
}

void MarqueeEffectGlitterColor( const struct TMarqueeSegment& segment)
{
  MarqueeEffectGlitterX( segment, CHSV( millis() / 16, 200, 255), 3);
}

void MarqueeEffectGlitterWhite( const struct TMarqueeSegment& segment)
{
  MarqueeEffectGlitterX( segment, CRGB::White, 7);
}

void MarqueeEffectGlitterX( const struct TMarqueeSegment& segment, const CRGB& color, uint8_t glitFactor)
{
  CRGB* segleds = segment.mBase;
  uint16_t len = segment.mLength;
  for( uint16_t i = random8(glitFactor); i < len; i += (random8(glitFactor) + 1)) {
    segleds[i] = color;
  }
}

void MarqueeEffectRotateColor( const struct TMarqueeSegment& segment)
{
  CRGB* segleds = segment.mBase;
  uint16_t len = segment.mLength;
  for( uint16_t i = 0; i < len; i++) {
      uint8_t t = segleds[i].r;
      segleds[i].r = segleds[i].g;
      segleds[i].g = segleds[i].b;
      segleds[i].b = t;
  }
}

void MarqueeEffectCrawlWhite( const struct TMarqueeSegment& segment)
{
  MarqueeEffectCrawlX( segment, CRGB::White);
}

void MarqueeEffectCrawlBlack( const struct TMarqueeSegment& segment)
{
  MarqueeEffectCrawlX( segment, CRGB::Black);
}

void MarqueeEffectCrawlColor( const struct TMarqueeSegment& segment)
{
  MarqueeEffectCrawlX( segment, CHSV( millis() / 16, 200, 255));
}

void MarqueeEffectCrawlX( const struct TMarqueeSegment& segment, const CRGB& color)
{
  CRGB* segleds = segment.mBase;
  uint16_t len = segment.mLength;
//  static uint8_t start = 0;
  //start = addmod8( start, 1, 4);
  uint8_t start = millis();
  start = (start >> 5) & 0x03;
  for( uint16_t i = start; i < len; i += 4) {
    segleds[i] = color;
    if( (i + 1) < len) {
      segleds[i+1] = color;
    }
  }
}

///////////////////  End of Marquee animation effects //////////////////
