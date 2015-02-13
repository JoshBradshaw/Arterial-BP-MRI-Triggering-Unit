#include "IntervalTimer.h"
#include "pressure_peak_detect.h"

IntervalTimer sampletimer;

// signal processing pathway to transform raw blood pressure input signal into
// MRI triggering signal
// Loosely based on the algorithm developed by Zong et. al at MIT in 
// An Open-source Algorithm to Detect Onset of Arterial Blood Pressure Pulses
// Computers in Cardiology, 2003; 30:259-262

// only affects falling edge of the TTL signal, so millis() polling is accurate enough

const int SERIAL_BAUD_RATE = 115200; // Hz
const int SAMPLE_PERIOD = 4; // milliseconds
const int TRIGGER_PULSE_DURATION = 20; // milliseconds
const int ANALOG_INPUT_PIN = 15;
const int SCANNER_TRIGGER_PIN = 10;
const int LED_PIN = 12;
const int PULSE_DURATION = TRIGGER_PULSE_DURATION / SAMPLE_PERIOD;
volatile int pulseDurationCount;
volatile bool triggerPulseHigh = false;

slopesum ssf;
peakDetect pd;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE); // fastest possible
    //analogReadResolution(12); // The arduino Due has 12 bit ADCs
    pinMode(SCANNER_TRIGGER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    // hardware clock triggers interrupts every x microseconds and calls the given ISR
    sampletimer.begin(sample, SAMPLE_PERIOD * 1000);
}

void loop() {
}

void sample() {
    // signal pathway
    // blood pressure transducer --> Arduino ADC --> low pass filter --> slopesum function --> peak detector
    int sampleVal = analogRead(ANALOG_INPUT_PIN);
    int ssfVal = ssf.step(sampleVal);
    bool sampleIsPeak = pd.isPeak(ssfVal);
    //Serial.printf("%d %d \n", sampleVal, ssfVal);

    if(sampleIsPeak) {
        digitalWrite(SCANNER_TRIGGER_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH);
        triggerPulseHigh = true;
        pulseDurationCount = 0;
    }

    if (triggerPulseHigh && pulseDurationCount >= PULSE_DURATION) {
        digitalWrite(SCANNER_TRIGGER_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
        triggerPulseHigh = false;
    } else if (triggerPulseHigh) {
        pulseDurationCount++;
    } else {
        // do nothing
    }
}
