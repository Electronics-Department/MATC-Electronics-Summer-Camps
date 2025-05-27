#include <CapacitiveSensor.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PIXELSPIN    1
#define NUMPIXELS 20
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXELSPIN, NEO_GRB + NEO_KHZ800);

CapacitiveSensor   cs_pin = CapacitiveSensor(0,2);        // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired
int reading = LOW;
unsigned long csSum;

unsigned long rainbowInterval=35;  // the time we need to wait
unsigned long pixelsInterval=100;  // the time we need to wait
unsigned long colorWipePreviousMillis=0;
unsigned long theaterChasePreviousMillis=0;
unsigned long theaterChaseRainbowPreviousMillis=0;
unsigned long rainbowPreviousMillis=0;
unsigned long rainbowCyclesPreviousMillis=0;

int theaterChaseQ = 0;
int theaterChaseRainbowQ = 0;
int theaterChaseRainbowCycles = 0;
int rainbowCycles = 0;
int rainbowCycleCycles = 0;

int effect_num = 0;  //this is used to track the current effect
int max_effect = 7;  //increase this number by one for each effect added
uint16_t currentPixel = 0;// what pixel are we operating on

void setup()                    
{  
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
   clock_prescale_set(clock_div_1);
  #endif
  // END of Trinket-specific code.
  
  cs_pin.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example

  pixels.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.show();            // Turn OFF all pixels ASAP
  pixels.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void loop()                    
{
  long start = millis();
  cs_read();  

  if (reading) {
    reading=LOW;
    effect_num++;
    if(effect_num > max_effect){
      effect_num=0;                     //wrap the effect around
    }
  }  
    switch(effect_num) {
      case 0:
        colorFill(pixels.Color(255,   0,   0)); // Red
        break;
      case 1:
        colorFill(pixels.Color(  0, 255,   0)); // Green
        break;
      case 2:
        colorFill(pixels.Color(  0,   0, 255)); // Blue
        break;
      case 3:
        colorFill(pixels.Color(  3, 252,   232)); // Teal
        break;
      case 4:
        colorFill(pixels.Color(  128, 0,   128)); // Purple
        break;
      case 5:
         if ((unsigned long)(millis() - rainbowPreviousMillis) >= rainbowInterval) {
            rainbowPreviousMillis = millis();
            rainbow();
          }
         break;
      case 6:
          if ((unsigned long)(millis() - theaterChasePreviousMillis) >= pixelsInterval) {
            theaterChasePreviousMillis = millis();
            theaterChase(pixels.Color(0, 127, 127)); // White
          }
        break;
      case 7:
          if ((unsigned long)(millis() - theaterChaseRainbowPreviousMillis) >= pixelsInterval) {
            theaterChaseRainbowPreviousMillis = millis();
            theaterChaseRainbow();
          }
        break;
      case 8:
        //colorWipe(pixels.Color(255, 0, 0)); // red
        break;
    }
}

// Fill the dots one after the other with a color
void colorFill(uint32_t c){
  pixels.setPixelColor(currentPixel,c);
  pixels.show();
  currentPixel++;
  if(currentPixel == NUMPIXELS){
    currentPixel = 0;
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c) {
  for (int i=0; i < pixels.numPixels(); i=i+3) {
      pixels.setPixelColor(i+theaterChaseQ, c);    //turn every third pixel on
    }
    pixels.show();
    for (int i=0; i < pixels.numPixels(); i=i+3) {
      pixels.setPixelColor(i+theaterChaseQ, 0);        //turn every third pixel off
    }
    theaterChaseQ++;
    if(theaterChaseQ >= 3) theaterChaseQ = 0;
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow() {
  for (int i=0; i < pixels.numPixels(); i=i+3) {
    pixels.setPixelColor(i+theaterChaseRainbowQ, Wheel( (i+theaterChaseRainbowCycles) % 255));    //turn every third pixel on
  }
      
  pixels.show();
  for (int i=0; i < pixels.numPixels(); i=i+3) {
    pixels.setPixelColor(i+theaterChaseRainbowQ, 0);        //turn every third pixel off        
  }      
  theaterChaseRainbowQ++;
  theaterChaseRainbowCycles++;
  if(theaterChaseRainbowQ >= 3) theaterChaseRainbowQ = 0;
  if(theaterChaseRainbowCycles >= 256) theaterChaseRainbowCycles = 0;
}

void rainbow() {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, Wheel((i+rainbowCycles) & 255));
  }
  pixels.show();
  rainbowCycles++;
  if(rainbowCycles >= 256) rainbowCycles = 0;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void cs_read() {
    long cs = cs_pin.capacitiveSensor(80); //a: Sensor resolution is set to 80
  if (cs > 100) { //b: Arbitrary number
    csSum += cs;
    if (csSum >= 3800) //c: This value is the threshold, a High value means it takes longer to trigger
    {
      reading=HIGH;
      if (csSum > 0) { csSum = 0; } //Reset
      cs_pin.reset_CS_AutoCal(); //Stops readings
    }
  } else {
    csSum = 0; //Timeout caused by bad readings
  }
}
