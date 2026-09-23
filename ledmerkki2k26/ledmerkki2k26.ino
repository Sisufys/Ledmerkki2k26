#include <FastLED.h>

#define LED_PIN     8
#define NUM_LEDS    45
#define BRIGHTNESS  10

CRGB leds[NUM_LEDS];

void setup()
{
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
}

void loop(){
    // Fade the existing trail
    fadeToBlackBy(leds, NUM_LEDS, 60);

    static uint8_t position = 0;

    leds[position] = CRGB::Blue;

    FastLED.show();

    position++;
    if (position >= NUM_LEDS)
        position = 0;

    delay(40);
}

void decay(byte red, byte green, byte blue, byte meteorSize, byte decaySpeed, int SpeedDelay) {  
  setAll(0,0,0);
 
  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {
    // fade brightness of leds randomly to create variation
    for(int j=0; j<NUM_LEDS; j++) {
      if( (random(10) > 5){
        fadeToBlackBy(j, decaySpeed );        
      }
    }
   
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        leds[i - j] = [red, green, blue];
      }
    }
   
    FastLED.show());
    delay(SpeedDelay);
  }
}