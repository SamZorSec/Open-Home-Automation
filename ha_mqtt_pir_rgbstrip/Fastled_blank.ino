//from https://github.com/FastLED/FastLED/wiki/Basic-usage
#include <FastLED.h>
#define DATA_PIN    2
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    60
CRGB leds[NUM_LEDS];    //sets up an array that we can manipulate to set/clear led data

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

//actually setup our leds (declare type of leds, and feed in #s defined above)
void setup() { 
       delay(3000); // 3 second delay for recovery
       // tell FastLED about the LED strip configuration
       FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
}

//ifi its not going to be just one patter, then define a list of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { candycane, redmoves, blueupline }; //and more to list here, or re-arange

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current (start at beginning of list when first turned on)
uint8_t gHue = 0; // rotating "base color" used by many of the patterns (remove???????)

//set the values of the entries in the leds array to whatever colors you want. Then you tell the library to show your data
void loop() {
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically

}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void redmoves() 
{
  // Turn the first led red for 1/2 second
      leds[0] = CRGB::Red; 
      FastLED.show();
      millis(500);

      // Set the first led back to black (off) for 1/2 second
      leds[0] = CRGB::Black;
      FastLED.show();
      millis(500);
}

void blueupline() 
{
  // have a blue dot move up the line
      for(int dot = 0; dot < NUM_LEDS; dot++) { 
            leds[dot] = CRGB::Blue;
            FastLED.show();
            // clear this led for the next time around the loop
            leds[dot] = CRGB::Black;
            millis(30);
       }
}
      
void candycane()
{
   //alternating red and white
       for(int dot = 0; dot < NUM_LEDS; dot++) { 
            leds[dot] = CRGB::Red;
            leds[dot+1] = CRGB::White;
            FastLED.show();
            // clear this led for the next time around the loop
            delay(100);
       }
}

