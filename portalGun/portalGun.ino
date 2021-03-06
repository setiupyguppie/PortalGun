// Code to control a Rick and Morty Portal Gun
// Written by Brandon Pomeroy, 2015

/*
******** Required Libraries *************
* ClickEncoder - https://github.com/0xPIT/encoder
* Adafruit_GFX - https://github.com/adafruit/Adafruit-GFX-Library
* Adafruit_LEDBackpack - https://github.com/adafruit/Adafruit-LED-Backpack-Library
*/


/*
********** Required Hardware *********************
* Adafruit Pro Trinket 5V 16MHz - http://www.adafruit.com/product/2000
* LiPoly BackPack - http://www.adafruit.com/product/2124
* LiPoly Battety 3.7V - http://www.adafruit.com/products/1578
* Rotary Encoder - http://www.adafruit.com/products/377
* Metal Knob - http://www.adafruit.com/products/2056
* Quad Alphanumeric Display (Red 0.54") - http://www.adafruit.com/products/1911
* Adafruit Audio FX Sound Board - http://www.adafruit.com/products/2210
* 8 ohm Speaker - http://www.adafruit.com/products/1891
* 10mm Diffused Green LED (x4) - https://www.adafruit.com/products/844
* 10mm Plastic Bevel LED Holder (x4) - https://www.adafruit.com/products/2171
* 150 Ohm Resistor (x4) for LEDs
* Inductive Charging Set - 5V - https://www.adafruit.com/products/1407
* 2.1mm Panel Mount Barrel Jack - http://www.adafruit.com/products/610
* 9VDC Power Supply - http://www.adafruit.com/products/63
*/

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <avr/sleep.h>
#include <avr/power.h>

// Set up our LED display
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();
char displayBuffer[4];
uint8_t dimensionLetter='C';
uint8_t scrollSpeed=100;

// Set up the click encoder
ClickEncoder *encoder;
int16_t last, value;
#define encoderPinA          A1
#define encoderPinB          A0
#define encoderButtonPin     A2

// Steps per notch can be 1, 4, or 8. If your encoder is counting
// to fast or too slow, change this!
#define stepsPerNotch        4

// Comment this line to make the encoder increment in the opposite direction
#define reverseEncoderWheel

//Set up the FX Sound Board outputs
#define portalShootPin       10
#define encoderTickPin       11
#define resetDimensionPin    12
#define dimensionRolloverPin 13
#define powerUpPin           4
#define powerDownPin         8

// FX Board output delay (ms)
const int msDelay = 500;

// Set up the Green LEDs
#define topBulbPin           9
#define frontRightPin        3
#define frontCenterPin       5
#define frontLeftPin         6

// Set up what we need to sleep/wake the Trinket
// Define the pins you'll use for interrupts - CHANGE THESE to match the input pins
// you are using in your project
#define NAV0_PIN A2

//Let us know if our Trinket woke up from sleep
volatile bool justWokeUp;


void timerIsr() {
  encoder->service();
}

void setup() {
  enablePinInterupt(NAV0_PIN);
  
  //Set up pin modes
  pinMode(portalShootPin, OUTPUT);
  pinMode(encoderTickPin, OUTPUT);
  pinMode(resetDimensionPin, OUTPUT);
  pinMode(dimensionRolloverPin, OUTPUT);
  pinMode(powerUpPin, OUTPUT);
  pinMode(powerDownPin, OUTPUT);
  
  pinMode(topBulbPin, OUTPUT);
  pinMode(frontRightPin, OUTPUT);
  pinMode(frontLeftPin, OUTPUT);
  pinMode(frontCenterPin, OUTPUT);
  
  
  digitalWrite(frontRightPin, HIGH);
  digitalWrite(frontLeftPin, HIGH);
  digitalWrite(frontCenterPin, HIGH);
  digitalWrite(topBulbPin, HIGH);
  
  digitalWrite(portalShootPin, HIGH);
  digitalWrite(encoderTickPin, HIGH);
  digitalWrite(resetDimensionPin, HIGH);
  digitalWrite(dimensionRolloverPin, HIGH);
  digitalWrite(powerUpPin, HIGH);
  digitalWrite(powerDownPin, HIGH);
  
  
  encoderSetup();
  alpha4.begin(0x70);  // pass in the address for the LED display
  
  //Play a startup sound
  digitalWrite(powerUpPin, LOW);
  delay(msDelay);
  digitalWrite(powerUpPin, HIGH);
  justWokeUp = false;
  
  //uncomment this to make the display run through a test at startup
  //displayTest();
}

void loop() {
  if (justWokeUp) {
    //Play a startup sound
    digitalWrite(powerUpPin, LOW);
    delay(msDelay);
    digitalWrite(powerUpPin, HIGH);    
    digitalWrite(frontRightPin, HIGH);
    digitalWrite(frontLeftPin, HIGH);
    digitalWrite(frontCenterPin, HIGH);
    digitalWrite(topBulbPin, HIGH);
    justWokeUp = false;
  }
  
  ClickEncoder::Button b = encoder->getButton();
  switch (b) {
    case ClickEncoder::Held:
      // Holding the button will put your trinket to sleep.
      // The trinket will wake on the next button press
      
      //Play a PowerDown sound
      digitalWrite(powerDownPin, LOW);
      delay(msDelay);
      digitalWrite(powerDownPin, HIGH);
      
      //alpha4.clear();
      //alpha4.writeDigitAscii(0, 'R');
      //alpha4.writeDigitAscii(1, 'I');
      //alpha4.writeDigitAscii(2, 'C');
      //alpha4.writeDigitAscii(3, 'K');
      char myStr = 'SCHWIFTY';
      digitalWrite(frontRightPin, LOW);
      digitalWrite(frontLeftPin, LOW);
      digitalWrite(frontCenterPin, LOW);
      digitalWrite(topBulbPin, LOW);
      //alpha4.writeDisplay();
      scroll(myStr);
      delay(500);
      alpha4.clear();
      alpha4.writeDisplay();
      delay(2500);
      justWokeUp = true;
      goToSleep();
    break;
    case ClickEncoder::Clicked:
      // Play a portal shot sound
      digitalWrite(portalShootPin, LOW);
      delay(msDelay);
      digitalWrite(portalShootPin, HIGH);
    break;
    case ClickEncoder::DoubleClicked:
      //If you double click the button, it sets the dimension to C137
      dimensionLetter = 'C';
      value = 137;
      // Play a reset dimension sound
      digitalWrite(resetDimensionPin, LOW);
      delay(msDelay);
      digitalWrite(resetDimensionPin, HIGH);
      
    break;
    case ClickEncoder::Open:
      // The dimension will increment from 0-999, then roll over to the next
      // letter. (A999 -> B000)
      updateDimension();
    break;
  }
}


void encoderSetup(){
    // set up encoder
    encoder = new ClickEncoder(encoderPinA, encoderPinB, encoderButtonPin, stepsPerNotch);
    encoder->setAccelerationEnabled(true);
  
    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr); 
    last = -1;
    value = 137;
}


void updateDimension(){
  #ifdef reverseEncoderWheel
  value -= encoder->getValue();
  #endif
  
  #ifndef reverseEncoderWheel
  value += encoder->getValue();
  #endif
  
  if (value != last) {
    if (value > 999){
      value = 0;
      if (dimensionLetter == 'Z') {
        dimensionLetter = 'A';
        // Play a dimension rollover sound
        digitalWrite(dimensionRolloverPin, LOW);
        delay(msDelay);
        digitalWrite(dimensionRolloverPin, HIGH);
      } else {
        dimensionLetter ++;
        // Play an encoder tick sound
        digitalWrite(encoderTickPin, LOW);
        delay(msDelay);
        digitalWrite(encoderTickPin, HIGH);
        
      }
    } else if ( value < 0 ) {
      value = 999;
      if (dimensionLetter == 'A') {
        dimensionLetter = 'Z';
        // Play a dimension rollover sound
        digitalWrite(dimensionRolloverPin, LOW);
        delay(msDelay);
        digitalWrite(dimensionRolloverPin, HIGH);
      } else {
        dimensionLetter --;
        // Play an encoder tick sound
        digitalWrite(encoderTickPin, LOW);
        delay(msDelay);
        digitalWrite(encoderTickPin, HIGH);
      }
    }
    last = value;
  }
  
  sprintf(displayBuffer, "%03i", value);
  alpha4.clear();
  alpha4.writeDigitAscii(0, dimensionLetter);
  alpha4.writeDigitAscii(1, displayBuffer[0]);
  alpha4.writeDigitAscii(2, displayBuffer[1]);
  alpha4.writeDigitAscii(3, displayBuffer[2]);
  alpha4.writeDisplay();
}


void scroll(char* in, uint8_t speed=scrollSpeed, uint8_t displayLen=sizeof(displayBuffer)){
  uint8_t i, dpos, tindex;
  char charToPrint;
  uint8_t inLen = strlen(in);
  alpha4.clear(); // Clear the display
  for (i=0; i < inLen + (displayLen * 2); i++){
    for (dpos=0; dpos < displayLen; dpos++){
      tindex = dpos + i;
      if (tindex < displayLen || tindex >= (displayLen + inLen)) {
        charToPrint = ' ';
      } else {
        charToPrint = in[tindex - displayLen];
      }
      //writeCharacter(charToPrint,dpos);//this is the command that write the char on screen
      alpha4.writeDigitAscii(dpos, charToPrint);
      alpha4.writeDisplay();
    }
    delay(speed);
  }
}


/*
============== Sleep/Wake Methods ==================
====================================================
*/

// Most of this code comes from seanahrens on the adafruit forums
// http://forums.adafruit.com/viewtopic.php?f=25&t=59392#p329418


void enablePinInterupt(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

void goToSleep()   
{
// The ATmega328 has five different sleep states.
// See the ATmega 328 datasheet for more information.
// SLEEP_MODE_IDLE -the least power savings 
// SLEEP_MODE_ADC
// SLEEP_MODE_PWR_SAVE
// SLEEP_MODE_STANDBY
// SLEEP_MODE_PWR_DOWN -the most power savings
// I am using the deepest sleep mode from which a
// watchdog timer interrupt can wake the ATMega328

 


set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Set sleep mode.
sleep_enable(); // Enable sleep mode.
sleep_mode(); // Enter sleep mode.
// After waking the code continues
// to execute from this point.

sleep_disable(); // Disable sleep mode after waking.                   
}

ISR (PCINT0_vect) // handle pin change interrupt for D8 to D13 here
{    
  // if I wired up D8-D13 then I'd need some code here
} 

ISR (PCINT1_vect) // handle pin change interrupt for A0 to A5 here // NAV0
{
    /* This will bring us back from sleep. */
  
  /* We detach the interrupt to stop it from 
   * continuously firing while the interrupt pin
   * is low.
   */
  
  detachInterrupt(0);

}

ISR (PCINT2_vect) // handle pin change interrupt for D0 to D7 here // NAV1, NAV2
{
  // Check it was NAV1 or NAV2 and nothing else
}




/*
============== Testing Methods ==================
=================================================
*/

void displayTest() {
  
  alpha4.writeDigitRaw(3, 0x0);
  alpha4.writeDigitRaw(0, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(0, 0x0);
  alpha4.writeDigitRaw(1, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(1, 0x0);
  alpha4.writeDigitRaw(2, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(2, 0x0);
  alpha4.writeDigitRaw(3, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  
  alpha4.clear();
  alpha4.writeDisplay();

  // display every character, 
  for (uint8_t i='!'; i<='z'; i++) {
    alpha4.writeDigitAscii(0, i);
    alpha4.writeDigitAscii(1, i+1);
    alpha4.writeDigitAscii(2, i+2);
    alpha4.writeDigitAscii(3, i+3);
    alpha4.writeDisplay();
    delay(300);
  }
}
