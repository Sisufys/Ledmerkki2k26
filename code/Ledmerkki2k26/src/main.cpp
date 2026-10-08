#include <Arduino.h>
#include <FastLED.h>
#include "header.h"

#define LED_PIN 8
#define NUM_LEDS 45
#define BRIGHTNESS 75
#define BUTTON 2

//(x, y) coordinates for each led, roughly from (0,0) to (128, 256)
//middle point being the 23rd led (76, 129)
uint8_t coords[45][2] = {
  { 73, 111 },
  { 70, 96 },
  { 68, 81 },
  { 65, 66 },
  { 64, 52 },
  { 61, 36 },
  { 56, 20 },
  { 56, 5 },
  { 67, 18 },
  { 74, 30 },
  { 84, 43 },
  { 93, 54 },
  { 101, 67 },
  { 111, 78 },
  { 120, 90 },
  { 128, 104 },
  { 138, 116 },
  { 147, 128 },
  { 132, 129 },
  { 116, 128 },
  { 104, 129 },
  { 90, 129 },
  { 76, 129 },
  { 62, 129 },
  { 49, 129 },
  { 33, 128 },
  { 18, 128 },
  { 4, 128 },
  { 14, 140 },
  { 22, 153 },
  { 32, 165 },
  { 40, 177 },
  { 50, 190 },
  { 59, 202 },
  { 68, 214 },
  { 77, 226 },
  { 84, 239 },
  { 96, 251 },
  { 94, 235 },
  { 91, 221 },
  { 89, 206 },
  { 85, 191 },
  { 83, 175 },
  { 81, 160 },
  { 79, 146 }
};

//each number represens the sine of the distance of the led coordinates from the middle led
uint8_t radii[NUM_LEDS] = {18,34,49,64,78,94,111,126,111,99,86,77,67,62,59,58,63,71,56,40,28,14,0,14,27,43,58,72,63,59,57,60,66,75,85,97,110,124,108,93,78,63,47,31,17};

uint8_t curBrg = 0;

CRGB leds[NUM_LEDS];

static uint16_t hue = 0;

uint32_t accums[NUM_LEDS];     // used in pendulum wave effect to accumulate sum, to know when to blink a led...
uint16_t led_state[NUM_LEDS];  // used in pendulum wave effect to know which direction the current led should be fading...

uint32_t lastAnimationRefresh = 0;
uint32_t lastButtonPress = 0;
uint32_t lastMillis = 0;
uint32_t tick = 0;
uint32_t t = 0;

int phpos = 0;

const uint8_t con = 255 / NUM_LEDS;
const uint16_t speed = 30;
int phaseMiddle;
uint8_t animation = 0;

const uint8_t debounceDelay = 50;

void setup() {
  pinMode(BUTTON, INPUT);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setDither(1);
  fluorescentStartup();
  FastLED.delay(2000);
  FastLED.setBrightness(0);
  for (int i = 0; i < NUM_LEDS; i++) {
    accums[i] = 0;
    led_state[i] = 0;
  }
}

void loop() {
  //poll button
  
  if(digitalRead(BUTTON) == LOW && millis() - lastButtonPress >= debounceDelay){
    lastButtonPress = millis();
    lastMillis = millis();
    animation = (animation >= 2) ? 0 : animation + 1;
    fadeBrightnessDown();
    clearVars();
    FastLED.delay(500);
  }

  //every n seconds switch animation
  //n for 1 cycle of pendulumWave animation (79500 ms with delay(5) !!HALFWAY,, 102000ms with 2ms delay, full cycle)
  if(millis() - lastMillis >= 102000){
    lastMillis = millis();
    animation = (animation >= 2) ? 0 : animation + 1;
    clearVars();
    fadeBrightnessDown();
    FastLED.delay(500);
  }

  //choose animation function, all are non blocking
  switch(animation){
    case(0):
      diagonalwaves();
      break;
    case(1):
      ripple();
      break;
    case(2):
      pendulumWave();
      break;
  }
  
}

//helper function to clear vars in between animastions
void clearVars(){
  for (int i = 0; i < NUM_LEDS; i++) {
    accums[i] = 0;
    led_state[i] = 0;
  }
}

//fade to black, for switching animations smoothly
//even though the animation wont update during the 200ms that we fade, it wont matter as is not visible
//also blocks for BRIGHTNESS * 10ms... a non blocking solution would be better....
void fadeBrightnessDown(){
  while(FastLED.getBrightness() > 0 ){
    FastLED.setBrightness(FastLED.getBrightness() - 1);
    FastLED.delay(5);
  }
  FastLED.clear();
}

void fadeBrightnessUp(){
  if (FastLED.getBrightness() < BRIGHTNESS) {
    if (millis() - lastAnimationRefresh > 5) {
      lastAnimationRefresh = millis();
      FastLED.setBrightness(FastLED.getBrightness() + 1);
    }
  }
}

//non blocking and timing is millis() bound, so does time between function calls will not affect speed
void diagonalwaves(){
  tick = (millis() & (65535)) / 32;
  t = millis();

  fadeBrightnessUp();

  for (int i = 0; i < NUM_LEDS; i++) {
    //magic values, speed=40
    uint8_t phase = (sin16(- t * speed + ((coords[i][0] - 3 *coords[i][1]) * (110))) + 32768) >> 8;
    phase = (phase < 5) ? 5 : phase;

    leds[i] = CHSV(120 + (tick / 8), 255, phase);
    //sin8(con*(i+tick))
  
    if (i == 22) {
      phaseMiddle = phase;
    }
  }

  FastLED.show();
}

//non blocking and timing is millis() bound, so does time between function calls will not affect speed
void ripple(){
  tick = (millis() & (65535)) / 32;
  t = millis();

  fadeBrightnessUp();


  for (int i = 0; i < NUM_LEDS; i++) {

    uint8_t phase = (sin16(-t * speed + (radii[i] * (500))) + 32768) >> 8;
    phase = (phase < 5) ? 5 : phase;

    leds[i] = CHSV(120 + (tick / 8), 255, phase);
    //sin8(con*(i+tick))
  
    if (i == 22) {
      phaseMiddle = phase;
    }
  }

  FastLED.show();
}

//timing is NOT millis() bound
void pendulumWave() {
  //fade in brightness
  if (FastLED.getBrightness() < BRIGHTNESS) {
    if (millis() - lastAnimationRefresh > 20) {
      lastAnimationRefresh = millis();
      FastLED.setBrightness(FastLED.getBrightness() + 1);
    }
  }

  //the actual animation logic
  for (int i = 0; i < NUM_LEDS; i++) {  //go through all the leds
    accums[i] += 88 - (i);               //increment the corresponding accumulator different value per led
    if (accums[i] >= 16384) {//16384
      accums[i] -= 16384;
      led_state[i] = 4;  // start the cycle
      hue = 170;
    }
    //if brightness cycle has finished, zero brightness
    if (led_state[i] > 506) {
      led_state[i] = 0;
    }
    //if brightness cycle has not finished and is ongoing, increment brightness
    if (led_state[i] > 0) {
      led_state[i] += 4;
    }
    uint16_t s = led_state[i];
    //calculate brightess
    uint8_t brg = (s > 255) ? 510 - s : s;
    //blue, second half fade to yellow (roughly 170 -> 35)

    leds[i] = CHSV(HUE_BLUE, 255, brg); //pride mode: brg = 255, hue = (s / 2), blue to yellow: ((s > 230) ? 170 : 35)
  }

  FastLED.show();
  FastLED.delay(2);
}


/*
A 1 dimensional sinewave that slowly shifts in color
non blocking and timing is millis() bound, so does time between function calls will not affect speed
*/
void rgbSinewave() {
  tick = (millis() & (65535)) / 32;
  t = millis();

  fadeBrightnessUp();


  for (int i = 0; i <= NUM_LEDS; i++) {
    //problem: the middle led needs to be lit when going over it horizontally and vertically...
    //check if the middle led is brighter horizontally or vertically, and light it up the way it is brighter
    uint8_t phase = 128 + (sin16(-t * speed + (i * (65535 / (NUM_LEDS + 1)))) >> 8);
    phase = (phase < 5) ? 5 : phase;
    if (i == NUM_LEDS) {
      if (phase >= phaseMiddle) {
        leds[22] = CHSV(120 + (tick / 8), 255, phase);
      } else {
        continue;
      }
    } else {
      //130 magic value to get blue color at startup, add tick/ 8 to slowly increment hue
      leds[i] = CHSV(120 + (tick / 8), 255, phase);
      //sin8(con*(i+tick))
    }
    if (i == 22) {
      phaseMiddle = phase;
    }
  }

  FastLED.show();
}

void fluorescentStartup() {
  FastLED.setBrightness(25);
  fill_solid(leds, NUM_LEDS, CRGB::DarkGoldenrod);
  FastLED.delay(100);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.delay(700);
  fill_solid(leds, NUM_LEDS, CRGB::DarkGoldenrod);
  FastLED.delay(100);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.delay(50);
  fill_solid(leds, NUM_LEDS, CRGB::DarkGoldenrod);
  FastLED.delay(100);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.delay(50);
  fill_solid(leds, NUM_LEDS, CRGB::DarkGoldenrod);
  FastLED.delay(100);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.delay(50);
  fill_solid(leds, NUM_LEDS, CRGB::DarkGoldenrod);
  FastLED.delay(400);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.delay(100);
  fill_solid(leds, NUM_LEDS, CRGB::DarkGoldenrod);
  FastLED.setBrightness(50);
  for (int i = 0; i <= 255; i++) {
    int v = 255 - i;
    fill_solid(leds, NUM_LEDS,
               CRGB(
                 ((uint16_t)184 * v) / 255,
                 ((uint16_t)134 * v) / 255,
                 ((uint16_t)11 * v) / 255));
    FastLED.delay(((i < 200) ? 12 : 3));
  }
  FastLED.setBrightness(BRIGHTNESS);
}

/*
//decaymaxx, ei hyvä
    if(millis() - lastMillis >= 7){
      lastMillis = millis();
      // Fade the existing trail
      for(int i = 0; i < NUM_LEDS; i++){
        leds[i] -= CHSV(0, 0, random(20));;
        //fadeToBlackBy(leds[i], NUM_LEDS, 5 + random(15));
      }
      
      
      phpos = ((position / 6 == NUM_LEDS) ? 22 : position / 6); 
      leds[phpos] = CHSV(hue / 4, 255, 255);

      FastLED.show();

      position++;
      if (position > NUM_LEDS * 6)
          position = 0;

      hue += 1;
      if(hue >= 1024){
          hue -= 1024;
      }
    }
    */

