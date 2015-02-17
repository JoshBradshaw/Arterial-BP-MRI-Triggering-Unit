---
layout: post
title: Designing an Algorithm for Adjusting Signal Gain in Real Time
---

## Algorithm Requirements

1. Must not depend on the triggering algorithm working properly, because in the case of output saturation, or extremely low signal amplitude, this will not work.
2. Must quickly detect and correct when the signal goes above or below the accepable range.
3. Must provide a no signal warning when the signals magnitude is too small for correction by gain adjustment.

Note: The instrument used for this study provides a minimum voltage of 0V and performs offset correction automatically, so offset correction is not addressed here.

## Attempt 1: Simplest Possible Algorithm

This algorithm is based on how a person would manually set the gain level. Starting from a low gain level, the gain is slowly increased until it reaches a set ampliftude threshold.

* Amplifier starts at unity gain, to ensure that even a maximum amplitude will not clip at t = 0
* In three second windows, we check whether or not the signal amplitude has reached the threshold value


Algorithm code:

A summarized version of the implementation is given here [Full Code Here](https://gist.github.com/JoshBradshaw/2dee769803f95edf20b8):

    volatile int potentiometer_code = 0; // range 0-256 where 0 -> ~84 Ohm and 256 -> ~50 KOhm
    const int MAX_SIGNAL_AMPLITUDE = 28900;
    const int CORRECTION = 10; // number of potentiomter codes to correct by if signal is out of range
    volatile bool threshold_achieved = false;
    volatile int WAITING_PERIOD = 30;
    volatile int waiting_count = 0;
    volatile const int SAMBA_GAIN_POT = 2;

    void sample() {
      int val = analogRead(14);
      //Serial.printf("sample taken, value: %d \n", val);
      
      if (val > MAX_SIGNAL_AMPLITUDE) {
        threshold_achieved = true;
        change_pot_code(LOW);
        Serial.printf("threshold reached, new pot code: %d \n", potentiometer_code);
      } 
      
      if (waiting_count >= WAITING_PERIOD && !threshold_achieved) {
        waiting_count = 0;
        change_pot_code(HIGH);
        Serial.printf("threshold not yet reached, new pot code: %d \n", potentiometer_code);
      }
      waiting_count++;
    }

    void change_pot_code(const bool increase) {
      // change the potentiometer code if the new code fits in 0 - 256 range
      
      if (increase && potentiometer_code < 256 - CORRECTION) {
        potentiometer_code += CORRECTION;
        digitalPotWrite(SAMBA_GAIN_POT, potentiometer_code);
      }
      if (!increase && potentiometer_code - CORRECTION > 0 + CORRECTION) {
        potentiometer_code -= CORRECTION;
        digitalPotWrite(SAMBA_GAIN_POT, potentiometer_code);
      }
    }



For such a simple algorithm this actually works surprisingly well. There are a few potential problems though:

Ignored cases and problems to fix:
1. Signals that start off with high amplitude, then gradually lose amplitude will never be corrected.
2. A signals with noise content that produces large, instantaneous amplitude spikes.
3. The sudden amplitude changes brought about by shifting the potentiometer code by 10 counts could interfere with the peak detection and filtering algorithms if they occur at an inopportune time.
4. There is no margin of acceptable values above the threshold in this algorithm.

To address issues 1-3 I made the following modifications to the algorithm

* Replaced single threshold with three thresholds: minimum, target and maximum
* If the signal's amplitude drops below minium, or exceeds maximum the amplitude is changed until it reaches target
* The thresholds are checked over a 5s interval.

The advantage of this approach is that the signal's amplitude can fluctuate within a ~4V range without crossing the maximum or minimum threshold. When the signal does cross the threshold, the algorithm attempts to center it within the min-max range. This ensures that gain correction will only be triggered when there is a significant change in input signal amplitude, and provided that the signal is in a steady state, corrections should last a long time.

The code for the modified algorithm is given below [full code here](https://gist.github.com/JoshBradshaw/d48509c063174f4c5d18):
    
    // for the purposes of this algorithm LOW -> lower gain HIGH -> raise gain

    void adjust_gain(const int sensor_value) {
      if (sensor_value > MIN_SIGNAL_AMPLITUDE) {
        minExceeded = true;
      } 
      if (sensor_value > MAX_SIGNAL_AMPLITUDE) {
        maxExceeded = true;
      }
      if (sensor_value > TARGET_AMPLITUDE) {
        targetExceeded = true;
      }
      
      // every 5 seconds check the amplitude levels
      if (windowCount > WINDOW_PERIOD) {
        // check if min or max have been violated
        if (!minExceeded) {
          seekState = 1;
        }
        if (maxExceeded) {
          seekState = 2;
        }
          
        // if unit was increasing gain, check if threshold reached
        if (seekState == 1) {
          if (targetExceeded) {
            seekState = 0;
          }
          else {
            changeGain(SAMBA_GAIN_POT, HIGH);
          }
        }
        // if unit was decreasing gain, check if threshold reached
        if (seekState == 2) {
          if (!targetExceeded) {
            seekState = 0;
          } else {
            changeGain(SAMBA_GAIN_POT, LOW);
          }
        }
        
        Serial.printf("state: %d, minExceeded: %d, targetExceeded: %d, maxExceeded: %d,  pot value: %d \n", seekState, minExceeded, targetExceeded, maxExceeded, potentiometerValue);
        
        // reset state
        minExceeded = false;
        targetExceeded = false;
        maxExceeded = false;
        windowCount = 0;
      }
      windowCount++;
    }

## Testing

So far the testing on this algorithm has been pretty light. I connected the teensy board to an arbitrary funciton generator and decreased the amplitude until it dropped below the minimum threshold. The Teensy's debug output was:

    state: 1, min_exceeded: 0, target_exceeded: 0, max_exceeded: 0,  pot value: 20 
    state: 1, min_exceeded: 0, target_exceeded: 0, max_exceeded: 0,  pot value: 30 
    state: 1, min_exceeded: 0, target_exceeded: 0, max_exceeded: 0,  pot value: 40 
    state: 1, min_exceeded: 0, target_exceeded: 0, max_exceeded: 0,  pot value: 50 
    state: 1, min_exceeded: 0, target_exceeded: 0, max_exceeded: 0,  pot value: 60 
    state: 1, min_exceeded: 0, target_exceeded: 0, max_exceeded: 0,  pot value: 70 
    state: 1, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 80 
    state: 1, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 90 
    state: 1, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 100 
    state: 1, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 110 
    state: 1, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 120 
    state: 1, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 130 
    state: 1, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 140 
    state: 0, min_exceeded: 1, target_exceeded: 1, max_exceeded: 0,  pot value: 140 
    state: 0, min_exceeded: 1, target_exceeded: 1, max_exceeded: 0,  pot value: 140 
    state: 0, min_exceeded: 1, target_exceeded: 1, max_exceeded: 0,  pot value: 140
    INFINITE REPEAT

On the rolling oscilliscope I had running I watched the maximum signal amplitude increase from 0.6V to 3.2V. This was a successful test.

During the same run of the device, I increased the output of the funciton generator until it exceeded the upper threshold (I set the function generator to 4.8V).

    state: 2, min_exceeded: 1, target_exceeded: 1, max_exceeded: 1,  pot value: 70 
    state: 2, min_exceeded: 1, target_exceeded: 1, max_exceeded: 0,  pot value: 60 
    state: 2, min_exceeded: 1, target_exceeded: 1, max_exceeded: 0,  pot value: 50 
    state: 2, min_exceeded: 1, target_exceeded: 1, max_exceeded: 0,  pot value: 40 
    state: 2, min_exceeded: 1, target_exceeded: 1, max_exceeded: 0,  pot value: 30 
    state: 0, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 30 
    state: 0, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 30 
    state: 0, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 30 
    state: 0, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 30 
    state: 0, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 30 
    state: 0, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 30 
    state: 0, min_exceeded: 1, target_exceeded: 0, max_exceeded: 0,  pot value: 30 

Once again the Teensy centered the signal on the target threshold as expected.

## Remaining Issues

1. The algorithm's centering funcitionality depends on the signal being relatively consistent in magnitude from beat to beat. The consequences of this assumption need to be tested using real blood pressure waveforms taken from humans and animals.
2. Changes in gain are still perform in 10 count leaps which cause signal distortion. To fix this I have two options:
--* Time the changes in amplitude such that they will not interfere with peak detection.
--* Change the gain gradually in incrememnts of 1 count.
I've opted not to tackle this in this version of the algorithm, because the testing the output waveform will require a better oscilliscope than I currently have available.
3. The case where the gain can not get high enough or low enough to center the signal is not handled very well. Ideally the Teensy should issue a status runtime warning or turn on an LED indicator.