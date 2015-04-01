#ifndef __PRESSUREPEAKDETECTH__
#define __PRESSUREPEAKDETECTH__

const int BUFFER_LEN = 12; // determines how many samples will be stored at a time
const int THRESHOLD_RESET_PERIOD = 1000; // reset magnitude thresholds after 2.4 seconds without heartbeat

class circularBuffer {
    // old items overwrite new items
    // item 0 is the oldest, item BUFFER_LEN -1 is the newest
public:
    circularBuffer() {
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
        if (nIndex > -1)  {
            return buff[(first + nIndex)%BUFFER_LEN];
        }
        else {
            return buff[(first + validItems + nIndex)%BUFFER_LEN];
        }
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
class filter
{
private:
    volatile int x_n_1 = 0;
    volatile int y_n_1 = 0;
public:
    int step(const int x_n)
    {
        int y_n = (x_n/20) + (x_n_1/20) + 9*y_n_1/10;
        x_n_1 = x_n;
        y_n_1 = y_n;
        return(y_n);
    }
};

class slopesum {
    // taken from MIT, this filter excentuates the first rising edge
    // of the signal, and removes the secondary knee
public:
    slopesum(void) {}
private:
    volatile int slope_sum = 0;
    circularBuffer sampleBuffer; // filtered samples
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
// peak detection state machine
// state 0: rising
// state 1: refractory period

public:
    peakDetect() {}
private:
    circularBuffer sb; // ssf samples
    circularBuffer pb; // peak values

    volatile bool rising = true;
    volatile int left_moving_sum = 0;
    volatile int right_moving_sum = 0;
    volatile int refractory_period = 30; // 40 samples at 250Hz = 160ms which gives 375BPM maximum heart rate
    volatile int rp_counter = 0;
    volatile int peak_threshold = 0;
    volatile int peak_threshold_sum = 0;

    volatile int peak1 = 0;
    volatile int peak2 = 0;
    volatile int peak3 = 0;
    volatile int peak4 = 0;
    volatile int peak5 = 0;
    volatile int peak6 = 0;

public:
    void updatePeakThreshold(const int newPeakVal) {
        // peak threshold averaged over the last BUFFER_LEN peaks
        // averaging over more peaks makes reduces sensitivity to amplitude spikes
        // thresholds are set at ~1/3 of the average amplitude, because testing showed
        // that they don't need to be very high
        peak_threshold_sum -= peak1;
        peak_threshold_sum += newPeakVal;

        peak1 = peak2;
        peak2 = peak3;
        peak3 = peak4;
        peak4 = peak5;
        peak5 = peak6;
        peak6 = newPeakVal;
        // set the threshold to about 1/2
        peak_threshold = peak_threshold_sum / 14;
    }

    void resetPeakThreshold() {
        // if threshold becomes too high, it must timeout and reset
        peak_threshold_sum = 0;
        peak_threshold = 0;
        peak1 = 0;
        peak2 = 0;
        peak3 = 0;
        peak4 = 0;
        peak5 = 0;
        peak6 = 0;
    }

    void updateMovingAverages() {
        // update right moving average
        right_moving_sum += sb[-1];
        right_moving_sum -= sb[-2];
        // update left moving average
        left_moving_sum += sb[-5];
        left_moving_sum -= sb[-6];
    }

    bool isPeak(const int x) {
        sb.addSample(x);
        updateMovingAverages();
        
        bool positive_slope = left_moving_sum < right_moving_sum;
        bool above_threshold = right_moving_sum > peak_threshold;
        bool refractory_period_over = rp_counter > refractory_period;
        
        if (rising && !positive_slope) {
            updatePeakThreshold(left_moving_sum);
            rising = false;
            if(refractory_period_over) {
                rp_counter = 0;
                return(true);
            }
        }

        // enter rising state if refractory period over, slope is trending upwards, and above peak threshold
        if (!rising && positive_slope && above_threshold) {
            rising = true;
        } 
        // after a prolonged period of not detecting any peaks, reset the threshold
        else if (rp_counter > THRESHOLD_RESET_PERIOD) {
            rp_counter += 1;
            resetPeakThreshold();            
        } else {
            rp_counter += 1;
        }
        return(false);
    }
};

#endif
