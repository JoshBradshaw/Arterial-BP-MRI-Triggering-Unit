#ifndef __PRESSUREPEAKDETECTH__
#define __PRESSUREPEAKDETECTH__

const int BUFFER_LEN = 15; // determines how many samples will be stored at a time
const int PEAK_BUFFER_LEN = 5; // must be <= than BUFFER_LEN, determines how many peaks threshold average uses
const int THRESHOLD_RESET_PERIOD = 1250; // reset magnitude thresholds after 5 seconds without heartbeat
const int ROLLING_POINT_SPACING = 2; // must be < than BUFFER_LEN

// # of samples to wait before searching for peaks again, determines maximum possible BPM
// should be kept as short as possible, because many problems arise when refractory period approaches beat period
const int REFRACTORY_PERIOD = 20;
volatile int THRESHOLD_SCALE = 3 * PEAK_BUFFER_LEN / 2;

class ringBuffer {
// when buffer is full, oldest item is overwritten by the new item
public:
    ringBuffer() {
        // initialize to all zeros, so that the moving averages on sames initialize to reasonable values
        for(int ii=0; ii < BUFFER_LEN; ii++) {
            addSample(0);
        }
    }
private:
    volatile int buff[BUFFER_LEN];
    volatile int validItems = 0;
    volatile int first = 0;
    volatile int last = 0;
public:
    volatile int& operator[] (const int nIndex) {
        // python style indexing, negative indices count back from the end
        return buff[(first + validItems + nIndex)%BUFFER_LEN];
    }
    void addSample(const int newSample) {
        // add samples until array full, then begin overwriting oldest
        // samples, advancing the first/last indexes to keep track
        if(validItems==BUFFER_LEN) {
            buff[last] = newSample;
            first = (first+1)%BUFFER_LEN;
        } else {
            validItems++;
            buff[last] = newSample;
        }
        last = (last+1)%BUFFER_LEN;
    }
};


class lowPassFilter {
// single pole, non-causal, recursive first order low pass filter
// 3db cutoff frequency: 25Hz (chosen through trial and error)
// http://www.mathworks.com/help/dsp/examples/designing-low-pass-fir-filters.html
private:
    int x_n_1 = 0;
    int y_n_1 = 0;
public:
    int step(int x_n) {
        int y_n = (x_n/20) + (x_n_1/20) + 9*y_n_1/10;
        x_n_1 = x_n;
        y_n_1 = y_n;
        return(y_n);
    }
};

class slopeSumFilter {
// implements the slope-sum function as defined in "An Open-source Algorithm to Detect Onset of Arterial Blood Pressure Pulses" by W. Zong et al
// The buffer length is equivalent to the value k in the mathematical definition of the slope sum function. 
// shorter butter lengths work better for slower heart rates. Always stay within the range of 10-25, 15 works best for fetal
public:
    slopeSumFilter(void) {}
private:
    volatile int slope_sum = 0;
    ringBuffer sampleBuffer; // filtered samples
public:
    int step(const int x) {
        // implemented with a ring buffer for efficiency
        // calculation is equivalence to adding all of the positive slopes over the interval
        int old_slope = sampleBuffer[1] - sampleBuffer[0];

        if (old_slope > 0) {
            slope_sum -= old_slope;
        }

        int new_slope = x - sampleBuffer[-1];

        if (new_slope > 0) {
            slope_sum += new_slope;
        }

        sampleBuffer.addSample(x);
        return slope_sum;
    }
};

class peakDetect {
public:
    peakDetect() {}
private:
    ringBuffer sb; // ssf sample buffer
    ringBuffer pb; // peak buffer

    volatile bool rising_edge = true;
    volatile int rp_counter = 0;

    volatile int peakSum = 0; // sum of all of the peaks in the peak buffer
    volatile int peakThreshold = 0; // threshold value 
public:
    bool isPeak(const int x) {
        // peak detection state machine
        // progression is: --> peak detected --> refractory period --> rising_edge detected --> repeat

        sb.addSample(x);
        // reduce the effect of noise and jitter by adding two adjacent samples, spaced ROLLING_POINT_SPACING apart
        // lrs = left rolling sum rrs = right rolling sum
        // increasing ROLLING_POINT_SPACING lowers peak detection sensitivity, increases delay
        int lrs = sb[BUFFER_LEN - ROLLING_POINT_SPACING] + sb[BUFFER_LEN - ROLLING_POINT_SPACING -1];
        int rrs = sb[BUFFER_LEN - 1] + x;

        if (rising_edge && lrs > rrs && lrs > peakThreshold) {
            updatePeakThreshold(lrs);
            rising_edge = false;
            rp_counter = 0;
            return(true);
        }
        // enter rising_edge state if refractory period over, slope is trending upwards
        if (!rising_edge && rp_counter > REFRACTORY_PERIOD && lrs < rrs) {
            rising_edge = true;
        // reset the magnitude threshold if peaks are not being detected
        } else if (rp_counter > THRESHOLD_RESET_PERIOD) {
            rp_counter += 1;
            resetPeakThreshold();
        } else {
            rp_counter += 1;
        }
        return(false);
    }

    void updatePeakThreshold(const int newPeakVal) {
        peakSum += newPeakVal;
        peakSum -= pb[BUFFER_LEN - PEAK_BUFFER_LEN];
        pb.addSample(newPeakVal);
        peakThreshold = peakSum / THRESHOLD_SCALE;
    }

    void resetPeakThreshold() {
        peakSum = 0;
        peakThreshold = 0;
        for(int ii=BUFFER_LEN - PEAK_BUFFER_LEN; ii<BUFFER_LEN; ii++) {
            pb[ii] = 0;
        }
    }
};

#endif
