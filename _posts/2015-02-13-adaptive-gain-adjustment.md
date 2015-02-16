---
layout: post
title: Building a Preamplifier with Adaptive Gain Adjustment for Fetal Blood Pressure Measurement
tagline: "not supporting tagline"
tags : [Amplifier, Linear Gain Adjustment]
---

A major problem with triggering off of the fetal blood pressures is that the signal coming from the transducers is relatively small compared to the full scale of the pressure measurement instrument. 

## Specifications

* The Samba Sensor can measure pressures from -38 mmHg to 263 mmHg, which is linearly mapped to a 0-5V output
* Fetal lambs have a typical blood pressure of 70/30 mmHg, which is the best approximation we have for a fetal Yorkshire pig

With the assumed blood pressure range of the pig, only 0.66V of the 5V range will be used for the signal ((70-30)/(263 - (-38))) * 5V = 0.66V

## Building Blocks

To correct for this discrepency, mplification between the Samba instrument and the analog to digital converter <abbr>ADC</abbr>. I chose to go with a digitally adjustable gain amplifier for this problem, because I wanted the microcontroller to be able to adjust the gain automatically without any manual intervention. 

The entire design is based off of this simple Analog Devices whitepaper: [CN0112: Variable Gain Noninverting Amplifier Using the AD5292 Digital Potentiometer and the OP184 Op Amp](http://www.analog.com/en/circuits-from-the-lab/cn0112/vc.html).

![alt text]({{ BASE_PATH }}/images/linear_variable_gain_adjust.jpg)

I chose this circuit because it offered an appropriate gain adjustment range, and because [PJRC already worked out how to interface these digital potentiometers with the Teensy 3.1 through SPI](https://www.pjrc.com/teensy/td_libs_SPI.html).

I started by patching up a simple non-inverting amplifier, on a breadboard like so:

<img src="{{ BASE_PATH }}/images/gain_adjust_amplifier.JPG" alt="Slope Sum function applied to blood pressure waveform" style="width: 600px;"/>

This amplifier had an adjustable gain range of 1.1 V/V to 10 V/V with 256 subdivisions. This is more than enough gain to amplify a 70/30 mmHg signal to the full range of the ADC.

With that all patched up and working, all that remains is to write an efficient algorithm for bringing the signal into full range in real time.

## Algorithm Requirements

1. Must not depend on the triggering algorithm working properly, because in the case of output saturation, or extremely low signal amplitude, this will not work.
2. Must quickly detect and correct when the signal goes above or below the accepable range.
3. Must provide a no signal warning when the signals magnitude is too small for correction by gain adjustment.

## Algorithm

* Four seconds of data will be recorded in a ring buffer. If the average voltage is less than 1V or greater than 2V, the gain will be increased or decreased by an amount proportional to the difference.

## Test Cases:

All testing will be done with an arbitrary waveform generator.

1. Signal with 0.3V peak-to-peak magnitude (<abbr>Vpp</abbr>) input to the amplifier. Passing Criteria: the signal must be amplified to 3Vpp with a response time of less than 15 seconds.
2. After a 0.3Vpp signal has been increased to 