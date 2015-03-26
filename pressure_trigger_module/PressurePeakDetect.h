#ifndef __PRESSUREPEAKDETECTH__
#define __PRESSUREPEAKDETECTH__

const int BUFFER_LEN = 15; // determines how many samples will be stored at a time

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


class slopesum {
    // taken from MIT, this filter excentuates the first rising edge
    // of the signal, and removes the secondary knee
public:
    slopesum(void) {}
private:
    int slope_sum = 0;
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
    volatile int refractory_period = 80; // 40 samples at 250Hz = 160ms which gives 375BPM maximum heart rate
    volatile int rp_counter = 0;
    volatile int peak_threshold = 0;
    volatile int peak_threshold_sum = 0;

public:
    void updatePeakThreshold(int newPeakVal) {
        // peak threshold averaged over the last BUFFER_LEN peaks
        // averaging over more peaks makes reduces sensitivity to amplitude spikes
        // thresholds are set at ~1/3 of the average amplitude, because testing showed
        // that they don't need to be very high
        peak_threshold_sum -= pb[0];
        peak_threshold_sum += newPeakVal;
        pb.addSample(newPeakVal);

        peak_threshold = peak_threshold_sum / (BUFFER_LEN * 2);
    }

    void updateMovingAverages() {
        // update right moving average
        right_moving_sum += sb[-1];
        right_moving_sum -= sb[-4];
        // update left moving average
        left_moving_sum += sb[-7];
        left_moving_sum -= sb[-10];
    }

    bool isPeak(const int x) {
        sb.addSample(x);
        updateMovingAverages();

        if (rising && left_moving_sum > right_moving_sum) {
            updatePeakThreshold(left_moving_sum);
            rising = false;
            rp_counter = 0;
            return(true);
        }

        if (!rising && rp_counter >= refractory_period
                && left_moving_sum > peak_threshold
                && left_moving_sum < right_moving_sum) {
            rising = true;
        } else {
            rp_counter += 1;
        }
        return(false);
    }
};

#endif
