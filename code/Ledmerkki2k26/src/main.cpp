#include <Arduino.h>
#include <FastLED.h>
#include "header.h"

#define LED_PIN 8
#define NUM_LEDS 45
#define BRIGHTNESS 24

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

uint8_t curBrg = 0;

CRGB leds[NUM_LEDS];

static uint16_t position = 0;
static uint16_t hue = 0;

uint32_t accums[NUM_LEDS];     // used in pendulum wave effect to accumulate sum, to know when to blink a led...
uint16_t led_state[NUM_LEDS];  // used in pendulum wave effect to know which direction the current led should be fading...

void setup() {

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setDither(1);
  fluorescentStartup();
  FastLED.delay(2000);
  //FastLED.setBrightness(0);
  for (int i = 0; i < NUM_LEDS; i++) {
    accums[i] = 0;
    led_state[i] = 0;
  }
}

uint32_t lastMillis = 0;
int phpos = 0;
uint32_t tick = 0;
uint32_t t = 0;

const uint8_t con = 255 / NUM_LEDS;

const uint16_t speed = 40;
int phaseMiddle;

void loop() {
  
  pendulumWave();

  //decay(20, 0, 255, 10, 30, 30);
  /*
  delay(40);
  fadeToBlackBy(leds, NUM_LEDS, 60);
  leds[random(NUM_LEDS)] = CRGB::Blue;
  FastLED.show();
  */
}

void pendulumWave() {
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
  FastLED.delay(5);
}



void rgbSinewave() {
  tick = (millis() & (65535)) / 32;

  t = millis();

  if (FastLED.getBrightness() < BRIGHTNESS) {
    if (millis() - lastMillis > 20) {
      lastMillis = millis();
      FastLED.setBrightness(FastLED.getBrightness() + 1);
    }
  }


  for (int i = 0; i <= NUM_LEDS; i++) {
    //problem: the middle led needs to be lit when going over it horizontally and vertilcally...
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
  /*
  lastMillis = millis();
  while(millis() - lastMillis < 10000){
    for(int i = 0; i < NUM_LEDS; i++){
      leds[i] = CHSV(28, 239, 120 + sin8((255/45)*(i*10+hue*8) / 2));
    }
    FastLED.show();
    hue += 1;
    if(hue >= 1024){
        hue -= 1024;
    }
    FastLED.delay(10);
  }
  */
}

/*
      if(millis() - lastMillis >= 7){
      lastMillis = millis();
      // Fade the existing trail

      fadeToBlackBy(leds, NUM_LEDS, 4);
      
      
      
      phpos = ((position / 6 == NUM_LEDS) ? 22 : position / 6); 
      leds[phpos] = CHSV(hue / 16, 255, 255);

      FastLED.show();

      position++;
      if (position > NUM_LEDS * 6)
          position = 0;

      hue += 1;
      if(hue >= 4096){
          hue -= 4096;
      }
      
      */


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

void decay(byte red, byte green, byte blue, byte meteorSize, byte decaySpeed, int SpeedDelay) {
  FastLED.clear();
  FastLED.show();

  for (int i = 0; i < NUM_LEDS + NUM_LEDS; i++) {
    // fade brightness of leds randomly to create variation
    for (int j = 0; j < NUM_LEDS; j++) {
      if (random(10) > 5) {
        fadeToBlackBy(leds, j, decaySpeed);
      }
    }

    // draw meteor
    for (int j = 0; j < meteorSize; j++) {
      if ((i - j < NUM_LEDS) && (i - j >= 0)) {
        leds[i - j] = CRGB(red, green, blue);
      }
    }

    FastLED.show();
    delay(SpeedDelay);
  }
}
