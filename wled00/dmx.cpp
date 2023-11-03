#include "wled.h"

/*
 * Support for DMX Output via MAX485.
 * Change the output pin in src/dependencies/ESPDMX.cpp, if needed (ESP8266)
 * Change the output pin in src/dependencies/SparkFunDMX.cpp, if needed (ESP32)
 * ESP8266 Library from:
 * https://github.com/Rickgg/ESP-Dmx
 * ESP32 Library from:
 * https://github.com/sparkfun/SparkFunDMX
 */

#ifdef WLED_ENABLE_DMX

void handleDMX()
{
  // don't act, when in DMX Proxy mode
  if (e131ProxyUniverse != 0) return;

  uint8_t brightness = strip.getBrightness();

  bool calc_brightness = true;

  // // NATHAN: Add thse for testing.
  // int DMXframesSinceLastMove = 0;
  // int DMXframesUntilNextMove = 100;
  // bool DMXanmationChangeOkay = true;
  // int DMXverticalCh = 134;
  // int DMXhorizontalCh = 134;

  DMXanmationChangeOkay = false;
  DMXframesSinceLastMove = DMXframesSinceLastMove + 1;
  if(DMXframesSinceLastMove > DMXframesUntilNextMove) {
    DMXframesSinceLastMove = 0;
    DMXframesUntilNextMove = random(20,50);
    DMXanmationChangeOkay = true;
  }

   // check if no shutter channel is set
   for (byte i = 0; i < DMXChannels; i++)
   {
     if (DMXFixtureMap[i] == 5) calc_brightness = false;
   }

  uint16_t len = strip.getLengthTotal();
  for (int i = DMXStartLED; i < len; i++) {        // uses the amount of LEDs as fixture count

    uint32_t in = strip.getPixelColor(i);     // get the colors for the individual fixtures as suggested by Aircoookie in issue #462
    byte w = W(in);
    byte r = R(in);
    byte g = G(in);
    byte b = B(in);

    int DMXFixtureStart = DMXStart + (DMXGap * (i - DMXStartLED));

    //NBARGUSS: We want DMX output AS WELL AS A LOT OF LEDs on the prototype.
    // using long counts of LEDs and multiple channels per led makes the current 
    // implementation really slow!.
    // Assuming minimum of 4 channels per fixture, and 512 possible channels on the 
    // DMX bus, we never want an outer loop greater than 128 
    // if we're past address 512, break out!
    if(DMXFixtureStart > 512) {
      break;
    }

    for (int j = 0; j < DMXChannels; j++) {
      int DMXAddr = DMXFixtureStart + j;
        switch (DMXFixtureMap[j]) {
          case 0:        // Set this channel to 0. Good way to tell strobe- and fade-functions to fuck right off.
            dmx.write(DMXAddr, 0);
            break;
          case 1:        // Red
            dmx.write(DMXAddr, calc_brightness ? (r * brightness) / 255 : r);
            break;
          case 2:        // Green
            dmx.write(DMXAddr, calc_brightness ? (g * brightness) / 255 : g);
            break;
          case 3:        // Blue
            dmx.write(DMXAddr, calc_brightness ? (b * brightness) / 255 : b);
            break;
          case 4:        // White
            dmx.write(DMXAddr, calc_brightness ? (w * brightness) / 255 : w);
            break;
          case 5:        // Shutter channel. Controls the brightness.
            dmx.write(DMXAddr, brightness);
            break;
          case 6:        // Sets this channel to 255. Like 0, but more wholesome.
            dmx.write(DMXAddr, 255);
            break;
        }
    }
  }

  // We're going to HARD CODE the motion light:
  // For the 'test' we're going to hard code the first 15 channels regardless!
  // This is for the moving head bubble light.  
  if(DMXanmationChangeOkay)
  {
    DMXhorizontalCh = random(0,255);
    DMXverticalCh = random(0,255);
  }
  
  // OVERWRITE!!

  uint32_t in = strip.getPixelColor(5);     // get the colors for the individual fixtures as suggested by Aircoookie in issue #462
    byte w = W(in);
    byte r = R(in);
    byte g = G(in);
    byte b = B(in);

  dmx.write(DMXStart + 0, DMXhorizontalCh);  // Ch 1 // Horizonrtal Movement
  dmx.write(DMXStart + 1, 0);                // Ch 2 // Horizontal Refine
  dmx.write(DMXStart + 2, DMXverticalCh);    // Ch 3 // Vertical Movement ~ 134 is 'straight up'
  dmx.write(DMXStart + 3, 0);                // Ch 4 // Vertical Refine
  dmx.write(DMXStart + 4, 0);                // Ch 5 // Slowdown 0 is 'fast' 255 is 'slow'
  dmx.write(DMXStart + 5, 255);              // Ch 6 // Master Dimmer, 255
  dmx.write(DMXStart + 6, 0);                // Ch 7 // Strobe Rate 0 On 1 slow, 255 fast
  dmx.write(DMXStart + 7, calc_brightness ? (r * brightness) / 255 : r);   // Ch 8  // RED
  dmx.write(DMXStart + 8, calc_brightness ? (g * brightness) / 255 : g);   // CH 9  // GREEN
  dmx.write(DMXStart + 9, calc_brightness ? (b * brightness) / 255 : b);   // CH 10 // BLUE
  dmx.write(DMXStart + 10, calc_brightness ? (w * brightness) / 255 : w);  // CH 11 // WHITE 
  dmx.write(DMXStart + 11, 255);             // Ch 12 // Laser Brightness
  dmx.write(DMXStart + 12, 0);               // Front Light Control : 0-30 none ; 31-127 Fast Auto; 128-249 Slow Auto; 250-255 Sound Contro
  dmx.write(DMXStart + 13, 0);             // Ch 14 // Rear light Control : 0-4 Light Ring Off; 5-109 Band Color; 110-255 Self Control
  dmx.write(DMXStart + 14, 0);               // Ch 15 // 0-249 No Effect ; 250-255 System Reset.


  dmx.update();        // update the DMX bus
}

void initDMX() {
 #ifdef ESP8266
  dmx.init(512);        // initialize with bus length
 #else
  dmx.initWrite(512);  // initialize with bus length
 #endif
}

#else
void handleDMX() {}
void initDMX() {}
#endif
