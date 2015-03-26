#include "IntervalTimer.h"
#include "PressurePeakDetect.h"
#include "AutoGainAdjust.h"

IntervalTimer sampletimer;

// signal processing pathway to transform raw blood pressure input signal into
// MRI triggering signal
// Loosely based on the algorithm developed by Zong et. al at MIT in
// An Open-source Algorithm to Detect Onset of Arterial Blood Pressure Pulses
// Computers in Cardiology, 2003; 30:259-262

// only affects falling edge of the TTL signal, so millis() polling is accurate enough

const int SAMPLING_PERIOD = 4; // milliseconds
const int TRIGGER_PULSE_DURATION = 20; // milliseconds
const int SCANNER_TRIGGER_PIN = 19;
const int LED_PIN = 18;
int ANALOG_INPUT_PIN;
const int PULSE_DURATION = TRIGGER_PULSE_DURATION / SAMPLING_PERIOD;
const int GAIN_ADJUST_PERIOD = 100; // milliseconds
const int gainAdjustDuration = GAIN_ADJUST_PERIOD / SAMPLING_PERIOD;
volatile int pulseDurationCount;
volatile int gainAdjustCount = 0;
volatile bool triggerPulseHigh = false;
const int SERIAL_UPDATE_PERIOD = 4;
int serial_update_count = 0;
const int TRIGGER_SENT_CODE = 100000;

slopesum ssf;
peakDetect pd;

void setup() {
    Serial.begin(115200); // fastest stable BAUD rate
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
    int ssfVal = ssf.step(sampleVal);
    bool sampleIsPeak = pd.isPeak(ssfVal);

    if(sampleIsPeak) {
        digitalWrite(SCANNER_TRIGGER_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH);
        triggerPulseHigh = true;
        pulseDurationCount = 0;
        Serial.println(TRIGGER_SENT_CODE); // signal to the monitoring software that trigger sent
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

    if (gainAdjustCount >= gainAdjustDuration) {
        adjustGain(sampleVal);
        gainAdjustCount = 0;
    } else {
        gainAdjustCount++;
    }

    if (serial_update_count < SERIAL_UPDATE_PERIOD) {
        serial_update_count++;
    } else {
        Serial.println(ssfVal);
        serial_update_count = 0;
    }
}
