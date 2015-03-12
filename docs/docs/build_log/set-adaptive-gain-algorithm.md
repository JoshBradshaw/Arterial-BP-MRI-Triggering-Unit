## Real Time Gain Adjustment

### Algorithm Requirements

1. Must not depend on the triggering algorithm working properly, because in the case of output saturation, or extremely low signal amplitude, triggering will not be functional.
3. When gain adjustments are made, they must be gradual, to avoid creating spikes in the input signal.
4. Changes to signal gain must be gradual, to avoid over-correcting for the insignificant amplitude spikes that occasionally occur when the pressure catheter tip comes into contact with the vessel wall. 
5. In steady state, the signal must have some headroom to avoid output saturation if the baseline drifts, or the signal magnitude increases.