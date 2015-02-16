---
layout: post
title: Building a Preamplifier with Adaptive Gain Adjustment
---

A major problem with triggering off of the fetal blood pressures is that the signal coming from the transducers is relatively small compared to the full scale of the pressure measurement instrument. 

The samba sensors instrument and fibre optic pressure transducer that I currently have at hand has a full scale pressure range of -38 mmHg to 263 mmHg, which is linearly mapped to a 0 - 5V analog output. The full scale blood pressure range of fetal lambs was measured by our colleagues in Australia to be a 70/30 mmHg. I have not yet found blood pressure numbers for fetal Yorkshire pigs, but I assume that they should be close enough that 70/30 mmHg is a workable estimate.

This large difference between the full scale range of the instrument is problematic, because it means that only 0.66V of the 5V range will be used for the signal $\frac{70-30}{263 - (-38} * 5V = 0.66V$

To correct for this discrepency, we need to insert an amplifier between the Samba instrument and the analog to digital converter. I choose to go with a digitally adjustable gain amplifier for this problem, because I wanted the microcontroller to be able to adjust the gain automatically without any manual intervention. 

The circuit I built was based on this whitepaper from Analog Devices: [CN0112: Variable Gain Noninverting Amplifier Using the AD5292 Digital Potentiometer and the OP184 Op Amp](http://www.analog.com/en/circuits-from-the-lab/cn0112/vc.html). I chose this circuit because it offered an appropriate gain adjustment range, and because [PJRC already worked out how to interface these digital potentiometers with the Teensy 3.1 through SPI](https://www.pjrc.com/teensy/td_libs_SPI.html).

I started by patching up a simple non-inverting amplifier, with the following schematic:

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