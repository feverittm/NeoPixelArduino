// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define STRIP_PIN    6
#define RING_PIN 2
#define BUTTON_PIN 4
#define LED_PIN 3
#define POT0_PIN A0
#define POT1_PIN A1

// How many NeoPixels are attached to the Arduino?
#define STRIP_COUNT 10
#define RING_COUNT 12

int Brightness = 100;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(STRIP_COUNT, STRIP_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring(RING_COUNT, RING_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

//Colors
uint32_t black = strip.Color(0, 0, 0);
uint32_t red = strip.Color(255, 0, 0);
uint32_t orange = strip.Color(255, 128, 0);
uint32_t yellow = strip.Color(255, 200, 0);
uint32_t green = strip.Color(0, 255, 0);
uint32_t light_blue = strip.Color(0, 255, 255);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t purple = strip.Color(127, 0, 255);
uint32_t white = strip.Color(255, 255, 255);

// set up some global variables
int buttonState;
int led_state;
int inByte;
int ary[10];
int idx;
int pot0_val;
int pot1_val;
int button0_val;


// setup() function -- runs once at startup --------------------------------

void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.
  // initialize serial communication:
  Serial.begin(9600);
  // initialize the LED pins:
  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(Brightness); // Set BRIGHTNESS to about 40% (max = 255)

  ring.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  ring.show();            // Turn OFF all pixels ASAP
  ring.setBrightness(Brightness); // Set BRIGHTNESS to about 40% (max = 255)

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  led_state = HIGH;
  inByte = 'q';
  idx = 0;
}


// loop() function -- runs repeatedly as long as board is on ---------------

void loop() {
  grabSensors();
  digitalWrite(LED_PIN, led_state);
    
  if (Serial.available() > 0){
    inByte = Serial.read();
    switch (inByte){
      case 'z':
        testloop();
        break;
      case 'W':
        checkButton();
        break;
      case 'x':
        CylonChase(50);
        break;
      case 'k':
        blackout();
        break;
      case 'g':
        colorWipe(strip.Color(0,255,0), 50); // Green
        Serial.println("Green");
        break;
      case 'r':
        colorWipe(strip.Color(255,0,0), 50); // Red
        Serial.println("Red");
        break;
      case 'b':
        colorWipe(strip.Color(0,0,255), 50); // Blue
        Serial.println("Blue");
        break;
      case 'y':
        colorWipe(yellow, 50); // Yellow
        Serial.println("Yellow");
        break;
      case 'w':
        colorWipe(white, 50); // Blue
        Serial.println("White");
        break;
      case 't':
        track0Value(white, 50);
        break;
      case 'B':
        if (Brightness >200) {
          break;
        }
        Brightness += 50;
        strip.setBrightness(Brightness);
        strip.show();
        break;
      case 'D':
        if (Brightness < 50) {
          break;
        }
        Brightness -= 50;
        strip.setBrightness(Brightness);
        strip.show();
        break;
    }
    inByte = 'q';
  }
}

void grabSensors() {
  pot0_val = analogRead(POT0_PIN);
  pot1_val = analogRead(POT1_PIN);
  button0_val = digitalRead(BUTTON_PIN);
}

void blackout() {
  //colorSet(strip.Color(0,   0,   0)); // Blackout
  strip.clear(); // Set all pixel colors to 'off'
  strip.show();
  ring.clear();
  ring.show();
  led_state = LOW;
  Serial.println("Blackout");
}

int avgValue(int val) {
  if (idx > 9) {
    idx = 0;
  }
  ary[idx] = val;
  idx++;
  int b=0;
  for (int j=0; j<=9; j++){
    b += ary[j]; 
  }
  return b/10;
}

void track0Value(uint32_t color, int wait) {
  int oldval=0;
  strip.clear(); // Set all pixel colors to 'off'
  strip.show();
  while(button0_val == HIGH){
    grabSensors();
    track1Value();
    int setpoint = map(pot0_val, 0, 1023, 0, strip.numPixels()-1);
    Serial.print("mapped led =");
    Serial.print(setpoint);
    Serial.print(", Pot1 =");
    Serial.print(pot1_val);
    Serial.print(", Button =");
    Serial.print(button0_val);
    Serial.print(", Average0 = ");
    Serial.println(avgValue(pot1_val));
    strip.setPixelColor(oldval, black);
    strip.setPixelColor(setpoint, color);
    strip.show();
    oldval = setpoint;
    delay(50);
  }
  blackout();
}

void track1Value() {
  int newval = 0;
  int oldval=0;
  ring.clear(); // Set all pixel colors to 'off'
  ring.show();
  Serial.println(avgValue(pot1_val));
  int setpoint = map(avgValue(pot1_val), 0, 1023, 0, ring.numPixels()-1);
  Serial.println(setpoint);
  ring.setPixelColor(oldval, black);
  ring.setPixelColor(setpoint, red);
  ring.show();
  oldval = setpoint;
}



void checkButton() {
  while(buttonState == HIGH){
    buttonState = digitalRead(BUTTON_PIN);
    Serial.print(", Button =");
    Serial.println(buttonState);
    delay(50);
  }
  Serial.println("Done!");
}

void testloop() {
  // Fill along the length of the strip in various colors...
  colorWipe(strip.Color(255,   0,   0), 50); // Red
  colorWipe(strip.Color(  0, 255,   0), 50); // Green
  colorWipe(strip.Color(  0,   0, 255), 50); // Blue

  // Do a theater marquee effect in various colors...
  theaterChase(strip.Color(127, 127, 127), 50); // White, half brightness
  theaterChase(strip.Color(127,   0,   0), 50); // Red, half brightness
  theaterChase(strip.Color(  0,   0, 127), 50); // Blue, half brightness

  rainbow(10);             // Flowing rainbow cycle along the whole strip
  theaterChaseRainbow(50); // Rainbow-enhanced theaterChase variant

  colorSet(strip.Color(0,   0,   0)); // Blackout
}


// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Direcly set the color of the strip to a given color
void colorSet(uint32_t color) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
  }
  strip.show();                          //  Update strip to match
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Chasing lights like a cylon on Battlestar Galactia. Pass in a color 
// (32-bit value, a la strip.Color(r,g,b) as mentioned above), and a 
// delay time (in ms) between frames.
void CylonChase(int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=0; c<=strip.numPixels(); c ++) {
        Serial.println("c");
        strip.setPixelColor(c, white); // Set pixel 'c' to value 'color'
        if (c > 0){
          strip.setPixelColor(c-1, black);
        }
        strip.show();
        delay(wait);
      }
      for(int c=strip.numPixels(); c>= 0; c --) {
        strip.setPixelColor(c, white); // Set pixel 'c' to value 'color'
        if (c<strip.numPixels()){
          strip.setPixelColor(c+1, black);
        }
        strip.show();
        delay(wait);
      }
      strip.setPixelColor(0,black);
      strip.show();
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}
