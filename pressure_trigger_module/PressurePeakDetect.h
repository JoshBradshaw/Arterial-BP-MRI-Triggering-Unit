#ifndef __PRESSUREPEAKDETECTH__
#define __PRESSUREPEAKDETECTH__

const int BUFFER_LEN = 15; // determines how many samples will be stored at a time
const int PEAK_BUFFER_LEN = 5;
const int THRESHOLD_RESET_PERIOD = 1000; // reset magnitude thresholds after 2.4 seconds without heartbeat
const int ROLLING_POINT_SPACING = 2;
const int REFRACTORY_PERIOD = 45; // 40 samples at 250Hz = 160ms which gives 375BPM maximum heart rate

class ringBuffer {
    // old items overwrite new items
    // item 0 is the oldest, item BUFFER_LEN -1 is the newest
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

//Low pass chebyshev filter order=1 alpha1=0.2
class lowPassFilter {
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
    // taken from MIT, this filter excentuates the first rising edge
    // of the signal, and removes the secondary knee
public:
    slopeSumFilter(void) {}
private:
    volatile int slope_sum = 0;
    ringBuffer sampleBuffer; // filtered samples
public:
    int step(const int x) {
        // the result of this recursive calculation is equivilant to adding all of the positive
        // slopes. we just subtract the contribution of the one we're taking away, and add
        // the contribution of the one we're adding
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
    ringBuffer sb; // ssf samples
    ringBuffer pb; // peak values

    volatile bool rising = true;
    volatile int rp_counter = 0;
    
    volatile int peakSum = 0;
    volatile int peakThreshold = 0;
public:
     bool isPeak(const int x) {
        sb.addSample(x);
        int lrs = sb[BUFFER_LEN - ROLLING_POINT_SPACING] + sb[BUFFER_LEN - ROLLING_POINT_SPACING -1];
        int rrs = sb[BUFFER_LEN - 1] + x;

        if (rising && lrs > rrs) {
            updatePeakThreshold(lrs);
            rising = false;
            if(rp_counter > REFRACTORY_PERIOD){
              rp_counter = 0;
              return(true);
            }
        }
        // enter rising state if refractory period over, slope is trending upwards, and above peak threshold
        if (!rising && lrs > peakThreshold && lrs < rrs) {
            rising = true;
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
        peakThreshold = peakSum / (3 * PEAK_BUFFER_LEN / 2);
    }

    void resetPeakThreshold() {
        peakSum = 0;
        peakThreshold = 0;
        for(int ii=BUFFER_LEN - PEAK_BUFFER_LEN; ii<PEAK_BUFFER_LEN; ii++){
            pb[ii] = 0;
        }
    }
};

#endif
