#include "IntervalTimer.h"
#include "PressurePeakDetect.h"
#include "AutoGainAdjust.h"

IntervalTimer sampletimer;

// written for Teensy 3.1 running at 96 MHz

const int SAMPLING_PERIOD = 4; // milliseconds
const int TRIGGER_PULSE_DURATION = 20; // milliseconds, given in the scanner's external triggering timing table
const int SCANNER_TRIGGER_PIN = 19;
const int LED_PIN = 18;

const int PULSE_DURATION = TRIGGER_PULSE_DURATION / SAMPLING_PERIOD;
const int GAIN_ADJUST_PERIOD = 100; // milliseconds
const int gainAdjustDuration = GAIN_ADJUST_PERIOD / SAMPLING_PERIOD;
volatile int pulseDurationCount;
volatile int gainAdjustCount = 0;
volatile bool triggerPulseHigh = false;

volatile int ANALOG_INPUT_PIN; // set at startup depending on switch position
volatile int serial_update_count = 0;
const int SAMPLE_SEND_PERIOD = 3;
volatile int sampleSendCount = 0;

lowPassFilter filt;
slopeSumFilter ssf;
peakDetect pd;

void setup() {
    Serial.begin(115200); // fastest stable BAUD rate (Hz)
    analogReadRes(16);  // the teensy has 16 bit ADCs
    pinMode(SCANNER_TRIGGER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(INPUT_SELECT_PIN, INPUT);

    bool analogInputSelect = digitalRead(INPUT_SELECT_PIN);

    if(analogInputSelect) { // switch right --> transonic
        ANALOG_INPUT_PIN = 14;
    } else { // switch left --> samba
        ANALOG_INPUT_PIN = 15;
    }

    // hardware clock interrupt service routine calls the sample routine every x microseconds
    sampletimer.begin(sample, SAMPLING_PERIOD * 1000);
    setupGainAdjustment();
}

void loop() {

}

void sample() {
    // signal pathway
    // blood pressure transducer --> Arduino ADC --> low pass filter --> slopesum function --> peak detector
    int sampleVal = analogRead(ANALOG_INPUT_PIN);
    int lpfVal = filt.step(sampleVal);
    int ssfVal = ssf.step(lpfVal);
    bool sampleIsPeak = pd.isPeak(ssfVal);

    if(sampleIsPeak) {
        // when a peak is detected, sent a TTL pulse to the scanner
        digitalWrite(SCANNER_TRIGGER_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH);
        triggerPulseHigh = true;
        pulseDurationCount = 0;
    }

    if (triggerPulseHigh && pulseDurationCount >= PULSE_DURATION) {
        // keep the TTL pulse at logic high (3.3V) until pulse duration exceeded 
        digitalWrite(SCANNER_TRIGGER_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
        triggerPulseHigh = false;
    } else if (triggerPulseHigh) {
        pulseDurationCount++;
    } else {
    }

    if (gainAdjustCount >= gainAdjustDuration) {
        adjustGain(sampleVal);
        gainAdjustCount = 0;
    } else {
        gainAdjustCount++;
    }
    // send data to the plotting/logging program
    if (sampleSendCount < SAMPLE_SEND_PERIOD) {
        sampleSendCount += 1;
    } else {
        Serial.printf("%d %d\n", sampleVal, triggerPulseHigh);
        sampleSendCount = 0;
    }
}
