
#include "FastLED.h"


// This sketch shows how to sequence a performance using HH,MM,SS.SSS timecode.
//
// A "ResetPerformance" method is provided so that the performance can be
// restarted from a custom external trigger, e.g., a button or event.
// For demonstration purposes, this Performance is reset if the
// sketch receives a letter "r" on the serial port.
//
// -Mark Kriegsman, January 2015

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    2
//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    60
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120


void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  
  RestartPerformance();
  Serial.begin(57600);
}


uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
uint32_t gTimeCodeBase = 0;
uint32_t gTimeCode = 0;
uint32_t gLastTimeCodeDoneAt = 0;
uint32_t gLastTimeCodeDoneFrom = 0;

#define TC(HOURS,MINUTES,SECONDS) \
  ((uint32_t)(((uint32_t)((HOURS)*(uint32_t)(3600000))) + \
  ((uint32_t)((MINUTES)*(uint32_t)(60000))) + \
  ((uint32_t)((SECONDS)*(uint32_t)(1000)))))


#define AT(HOURS,MINUTES,SECONDS) if( atTC(TC(HOURS,MINUTES,SECONDS)) )
#define FROM(HOURS,MINUTES,SECONDS) if( fromTC(TC(HOURS,MINUTES,SECONDS)) )

static bool atTC( uint32_t tc)
{
  bool maybe = false;
  if( gTimeCode >= tc) {
    if( gLastTimeCodeDoneAt < tc) {
      maybe = true;
      gLastTimeCodeDoneAt = tc;
    }
  }
  return maybe;
}

static bool fromTC( uint32_t tc)
{
  bool maybe = false;
  if( gTimeCode >= tc) {
    if( gLastTimeCodeDoneFrom <= tc) {
      maybe = true;
      gLastTimeCodeDoneFrom = tc;
    }
  }
  return maybe;
}


// There are two kinds of things you can put into this performance:
// "FROM" and "AT".
//
// * "FROM" means starting FROM this time AND CALLING IT REPEATEDLY 
//   until the next "FROM" time comes.
//
// * "AT" means do this ONE TIME ONLY "AT" the designated time.
//
// At least one of the FROM clauses will ALWAYS be executed.
// In the transitional times, TWO pieces of code will be executed back to back.
// For example, if one piece says "FROM(0,0,1.000) {DrawRed()}" and another says
// "FROM(0,0,2.000) {flashblue();}", what you'll get is this:
//   00:00:01.950  -> calls DrawRed
//   00:00:01.975  -> calls DrawRed
//   00:00:02.000  -> calls DrawRed AND calls DrawBlue !
//   00:00:02.025  -> calls DrawBlue
//   00:00:02.050  -> calls DrawBlue
// In most cases, this probably isn't significant in practice, but it's important
// to note.  It could be avoided by listing the sequence steps in reverse
// chronological order, but that makes it hard to read.

void Performance()
{
    AT(0,0,00.001) { FastLED.setBrightness(BRIGHTNESS); }
  FROM(0,0,00.100) { confetti(); }
  FROM(0,0,01.500) { juggle(); }
  FROM(0,0,03.375) { rainbowWithGlitter(); }
  FROM(0,0,04.333) { bpm(); }
  FROM(0,0,06.666) { juggle(); }
  FROM(0,0,08.750) { confetti(); }
    AT(0,0,11.000)   { gHue = HUE_PINK; }
    AT(0,0,12.000)   { fill_solid(leds, NUM_LEDS, CRGB::Red); }
    AT(0,0,15.000)   { fill_solid(leds, NUM_LEDS, CRGB::Blue); }
  FROM(0,0,16.500) { fadeToBlack(); }  
  FROM(0,0,18.000) { applause(); }  
    AT(0,0,19.000)   { FastLED.setBrightness(BRIGHTNESS/2); }
    AT(0,0,20.000)   { FastLED.setBrightness(BRIGHTNESS/4); }
    AT(0,0,21.000)   { FastLED.setBrightness(BRIGHTNESS/8); }
    AT(0,0,22.000)   { FastLED.setBrightness(BRIGHTNESS/16); }
  FROM(0,0,23.000) { fadeToBlack(); }  
}


void loop()
{
  // Set the current timecode, based on when the performance started
  gTimeCode = millis() - gTimeCodeBase;

  Performance();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // For demo purposes, restart the performance any time we read
  // the letter "r" character from the serial port.  Type "r" into 
  // the Arduino serial monitor and press return to restart the performance.
  // In practice, you could have this check a 'restart' button, or something similar.
  if( Serial.read() == 'r') RestartPerformance();

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}


void RestartPerformance()
{
  gLastTimeCodeDoneAt = 0;
  gLastTimeCodeDoneFrom = 0;
  gTimeCodeBase = millis();
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

// An animation to play while the crowd goes wild after the big performance
void applause()
{
  static uint16_t lastPixel = 0;
  fadeToBlackBy( leds, NUM_LEDS, 32);
  leds[lastPixel] = CHSV(random8(HUE_BLUE,HUE_PURPLE),255,255);
  lastPixel = random16(NUM_LEDS);
  leds[lastPixel] = CRGB::White;
}

// An "animation" to just fade to black.  Useful as the last track
// in a non-looping performance.
void fadeToBlack()
{
  fadeToBlackBy( leds, NUM_LEDS, 1);
}
