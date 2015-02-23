/*
 This code adjusts the gain of a non-inverting amplifier to bring a pressure
 sensor's signal level into the ideal range for measurement with the 
 Teensy microntroller's ADC.
  
 The AD5206 is SPI-compatible,and to command it, you send two bytes, 
 one with the channel number (0 - 5) and one with the resistance value for the
 channel (0 - 255).  

 LMC6464 is an instrumentation grade micropower rail-to-rail op-amp
 
 The circuit:
 AD5206
  * CS - to digital pin 10  (SS pin) on Teensy
  * SDI - to digital pin 11 (MOSI pin) on Teensy
  * CLK - to digital pin 13 (SCK pin) on Teensy
 LMC6464
  * IN (-) A - connected to B3 on AD5206
  * IN (+) A - connected to Samba Sensor's analog signal output
  * OUT A - connected to W3 on AD5206 and pin 14 on Teensy
 
*/

#include "spi4teensy3.h"

// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin = 10;
const int SAMBA_SENSOR_PIN = 14;

const int WINDOW_PERIOD = 50; // number of 100ms increments
const int MIN_SIGNAL_AMPLITUDE = 10000;
const int TARGET_AMPLITUDE = 35000;
const int MAX_SIGNAL_AMPLITUDE = 55000;
const int CORRECTION = 10; // number of potentiomter codes to correct by if signal is out of range

volatile int potentiometerValue = 0; // range 0-256 where 0 -> ~84 Ohm and 256 -> ~50 KOhm
volatile int targetPotentiometerValue = 0;
volatile int windowCount = 0;
const int SAMBA_GAIN_POT = 2; // corresponds to pins B3 and W3 on the AD5204 or AD5206

volatile bool minExceeded = false;
volatile bool maxExceeded = false;
volatile bool targetExceeded = false;

// 0 = equilibrium
// 1 = increasing gain to meet threshold
// 2 = decreasing gain to meet threshold
volatile int seekState = 1;

void digitalPotWrite(const int address, int value) {
  // write the new potentiometer value to the digital pot chip through SPI
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin, LOW);
  //  send in the address and value via SPI:
  spi4teensy3::send(address);
  spi4teensy3::send(potentiometerValue);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin, HIGH); 
}

void setupGainAdjustment() {
  pinMode (slaveSelectPin, OUTPUT);
  spi4teensy3::init();
  digitalPotWrite(SAMBA_GAIN_POT, 0); // start with unity gain to avoid clipping
}

// algorithm:
// have minimum maximum and target signal threshold
// during a 5 second window signal must exceed minimum and not exceed maximum
// if the signal doesn't meet these criteria, it will be increased or decreased until its max
// ampitude passes TARGET_AMPLITUDE, or the amplifier reaches maximum or minimum gain
void adjustGain(const int sensor_value) {
  // adjust potentiometer value if required (done in steps of 1 count for smoothness)
  if (potentiometerValue < targetPotentiometerValue) {
    potentiometerValue += 1;
    digitalPotWrite(SAMBA_GAIN_POT, potentiometerValue);
  }
  if (potentiometerValue > targetPotentiometerValue) {
    potentiometerValue -= 1;
    digitalPotWrite(SAMBA_GAIN_POT, potentiometerValue);
  }
  // check if this sample is out of bounds
  if (sensor_value > MIN_SIGNAL_AMPLITUDE) {
    minExceeded = true;
  } 
  if (sensor_value > MAX_SIGNAL_AMPLITUDE) {
    maxExceeded = true;
  }
  if (sensor_value > TARGET_AMPLITUDE) {
    targetExceeded = true;
  }
  // this section runs every 5 seconds
  // checks if gain needs to be adjusted, and sets the new gain value if required
  if (windowCount > WINDOW_PERIOD) {  
    // check if min or max have been violated
    if (!minExceeded) {
      seekState = 1;
    }
    if (maxExceeded) {
      seekState = 2;
    }
    // if unit was increasing gain, check if the threshold magnitude was reached
    if (seekState == 1) {
      if (targetExceeded) {
        seekState = 0;
      } else if (potentiometerValue <= 256 - CORRECTION) {
        targetPotentiometerValue += CORRECTION;
      }
      else {
        return;
      }
    }
    // if unit was decreasing gain, check if threshold reached
    if (seekState == 2) {
      if (!targetExceeded) {
        seekState = 0;
      } else if (potentiometerValue >= 0 + CORRECTION) {
        targetPotentiometerValue -= CORRECTION;
      } else {
        return;
      }
    }
    
    // reset state
    minExceeded = false;
    targetExceeded = false;
    maxExceeded = false;
    windowCount = 0;
  }
  windowCount++;
}


