/*
 * PIR sensor setup
 */
 
int inputPin = 2;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status

/*
 * LED Setup
 */
#include <FastLED.h>

#define DATA_PIN     6

// Information about the LED strip itself
#define NUM_LEDS    30
#define CHIPSET     WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define BRIGHTNESS  128

 
void setup() {
	  delay( 3000 ); // power-up safety delay
       FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);

  pinMode(inputPin, INPUT);     // declare sensor as input
 
  Serial.begin(9600);
}
 
void loop(){
  val = digitalRead(inputPin);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
   
 
    if (pirState == LOW) {
      // we have just turned on
      Serial.println("Motion detected!");
      // We only want to print on the output change, not state
      pirState = HIGH;
      for( int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::White;
      FastLED.show();
	  }
      
    }
  } else {
    if (pirState == HIGH){
   
      // we have just turned of
      Serial.println("Motion ended!");
      // We only want to print on the output change, not state
      pirState = LOW;
       	for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
    	FastLED.show();
   	    }
    }
  }
}
