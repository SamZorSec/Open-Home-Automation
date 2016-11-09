#include "FastLED.h"
//DOESNT WORK RIGHT NOW< https://gist.github.com/kriegsman/841c8cd66ed40c6ecaae
// This sketch shows one way to define a 'timed playlist'
// of animations that automatically rotate on a custom schedule.
//
// A "ResetPlaylist" method is provided so that the playlist can be
// restarted from a custom external trigger, e.g., a button or event.
// For demonstration purposes, the playlist is reset if the
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


// List of patterns to cycle through.  Each is defined as a separate function below.

typedef void (*SimplePattern)();
typedef SimplePattern SimplePatternList[];
typedef struct { SimplePattern mPattern;  uint16_t mTime; } PatternAndTime;
typedef PatternAndTime PatternAndTimeList[];

// These times are in seconds, but could be changed to milliseconds if desired;
// there's some discussion further below.

const PatternAndTimeList gPlaylist = { 
  { confetti,                5 },
  { juggle,                 10 },
  { bpm,                    10 },
  { rainbowWithGlitter,      5 },
  { juggle,                  5 },
  { applause,               10 },
  { fadeToBlack,             3 }
};

// If you want the playlist to loop forever, set this to true.
// If you want the playlist to play once, and then stay on the final pattern 
// until the playlist is reset, set this to false.
bool gLoopPlaylist = true;

void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  
  RestartPlaylist();
  Serial.begin(57600);
}


uint8_t gCurrentTrackNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
bool gRestartPlaylistFlag = false;

void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  gPlaylist[gCurrentTrackNumber].mPattern();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // For demo purposes, restart the playlist any time we read
  // the letter "r" character from the serial port.  Type "r" into 
  // the Arduino serial monitor and press return to restart the playlist.
  // In practice, you could have this check a 'restart' button, or something similar.
  if( Serial.read() == 'r') RestartPlaylist();

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow

  // Here's where we do two things: switch patterns, and also set the
  // 'virtual timer' for how long until the NEXT pattern switch.
  //
  // Instead of EVERY_N_SECONDS(10) { nextPattern(); }, we use a special
  // variation that allows us to get at the pattern timer object itself,
  // and change the timer period every time we change the pattern.
  //
  // You could also do this with EVERY_N_MILLISECONDS_I and have the 
  // times be expressed in milliseconds instead of seconds.
  {
    EVERY_N_SECONDS_I(patternTimer,gPlaylist[gCurrentTrackNumber].mTime) { 
      nextPattern(); 
      patternTimer.setPeriod( gPlaylist[gCurrentTrackNumber].mTime);
    }

    // Here's where we handle restarting the playlist if the 'reset' flag
    // has been set. There are a few steps:  
    if( gRestartPlaylistFlag ) {
      
      // Set the 'current pattern number' back to zero
      gCurrentTrackNumber = 0;
      
      // Set the playback duration for this patter to it's correct time
      patternTimer.setPeriod( gPlaylist[gCurrentTrackNumber].mTime);
      // Reset the pattern timer so that we start marking time from right now
      patternTimer.reset();
      
      // Finally, clear the gRestartPlaylistFlag flag
      gRestartPlaylistFlag = false;
    }
  }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number
  gCurrentTrackNumber = gCurrentTrackNumber + 1;
  
  // If we've come to the end of the playlist, we can either 
  // automatically restart it at the beginning, or just stay at the end.
  if( gCurrentTrackNumber == ARRAY_SIZE( gPlaylist) ) {
    if( gLoopPlaylist == true) {
      // restart at beginning
      gCurrentTrackNumber = 0;
    } else {
      // stay on the last track
      gCurrentTrackNumber--;
    }
  }
}

void RestartPlaylist()
{
  gRestartPlaylistFlag = true;
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
// in a non-looping performance-oriented playlist.
void fadeToBlack()
{
  fadeToBlackBy( leds, NUM_LEDS, 10);
}
