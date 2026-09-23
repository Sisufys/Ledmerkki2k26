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

void loop()
{
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