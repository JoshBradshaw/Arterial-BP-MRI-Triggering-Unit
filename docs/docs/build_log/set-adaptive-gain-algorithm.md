## Algorithm for Adjusting Signal Gain in Real Time

The motivation for this algorithm is described in the document: Variable Gain Amplifier with SPI Interface

### Algorithm Requirements

1. Must not depend on the triggering algorithm working properly, because in the case of output saturation, or extremely low signal amplitude, triggering will not be functional.
3. When gain adjustments are made, they must be gradual, to avoid creating spikes in the input signal.
4. Changes to signal gain must be gradual, to avoid over-correcting for the insignificant amplitude spikes that occasionally occur when the pressure catheter tip comes into contact with the vessel wall. 
5. In steady state, the signal must have some headroom to avoid output saturation if the baseline drifts, or the signal magnitude increases.

Note: The instrument used for this study provides a minimum voltage of 0V and performs offset correction automatically, so offset correction is not addressed here.

## Attempt 1: Simplest Possible Algorithm

This algorithm is based on how a person would manually set the gain level. Starting from a low gain level, the gain is slowly increased until it reaches a set ampliftude threshold.

* Amplifier starts at unity gain, to ensure that even a maximum amplitude will not clip at t = 0
* In three second windows, we check whether or not the signal amplitude has reached the threshold value


Algorithm code:

A summarized version of the implementation is given here [Full Code Here](https://gist.github.com/JoshBradshaw/2dee769803f95edf20b8):


{% highlight c %}
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
{% endhighlight %}


For such a simple algorithm this actually works surprisingly well. There are a few potential problems though:

Ignored cases and problems to fix:
1. Signals that start off with high amplitude, then gradually lose amplitude will never be corrected.
2. A signals with noise content that produces large, instantaneous amplitude spikes.
3. The sudden amplitude changes brought about by shifting the potentiometer code by 10 counts could interfere with the peak detection and filtering algorithms if they occur at an inopportune time.
4. There is no margin of acceptable values above the threshold in this algorithm.

### Attempt 2

To address issues 1-3 I made the following modifications to the algorithm

* Replaced single threshold with three thresholds: minimum, target and maximum
* If the signal's amplitude drops below minium, or exceeds maximum the amplitude is changed until it reaches target
* The thresholds are checked over a 5s interval.

The advantage of this approach is that the signal's amplitude can fluctuate within a ~4V range without crossing the maximum or minimum threshold. When the signal does cross the threshold, the algorithm attempts to center it within the min-max range. This ensures that gain correction will only be triggered when there is a significant change in input signal amplitude, and provided that the signal is in a steady state, corrections should last a long time.

The code for the modified algorithm is given below [full code here](https://gist.github.com/JoshBradshaw/d48509c063174f4c5d18):
    
{% highlight c %}
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
{% endhighlight %}

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

On the rolling oscilloscope I had running I watched the maximum signal amplitude increase from 0.6V to 3.2V. This was a successful test.

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

1. The algorithm's centering functionality depends on the signal being relatively consistent in magnitude from beat to beat. The consequences of this assumption need to be tested using real blood pressure waveforms taken from humans and animals.
2. Changes in gain are still perform in 10 count leaps which cause signal distortion. To fix this I have two options:
--* Time the changes in amplitude such that they will not interfere with peak detection.
--* Change the gain gradually in increments of 1 count.
I've opted not to tackle this in this version of the algorithm, because the testing the output waveform will require a better oscilliscope than I currently have available.

### Attempt 3

I modified the algorithm to perform changes in gain gradually. I tested this algorithm with the high resolution oscilloscope at the Toronto Center for Phenogenetics, and I'm very confident that the gain adjustment is sufficiently smooth to avoid disrupting the triggering algorithm.

{% highlight c %}
void adjustGain(const int sensor_value) {
    // seek state 0 = equilibrium
    // seek state 1 = increasing gain to meet threshold
    // seek state 2 = decreasing gain to meet threshold
    // starts in increasing state to ensure that the signal is amplified to an optimal level at startup
    volatile int seekState = 1;

    // adjust potentiometer value if required (done in steps of 1 count for smoothness)

    // increase the pot value by one count
    if (potentiometerValue < targetPotentiometerValue) {
        potentiometerValue += 1;
        digitalPotWrite(GAIN_POT, potentiometerValue);
    }
    // decrease the pot value by one count
    if (potentiometerValue > targetPotentiometerValue) {
        potentiometerValue -= 1;
        digitalPotWrite(GAIN_POT, potentiometerValue);
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
            } else if (potentiometerValue > 0 + CORRECTION) {
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
{% endhighlight %}

## Related Questions and Uncertainty
1. What will a fetal pig's blood pressure be in uturo? Current estimate is 70/30 mmHg based on measurements taken by an Australian research group.
2. If we switch to using a Transonic unit, what is the Transonic units reference pressure? Can it be zeroed? Will the module ever use the negative portion of the -5V to 5V analog output range if we zero the unit at ambient atmospheric pressure?