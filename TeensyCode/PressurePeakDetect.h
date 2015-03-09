#ifndef __PRESSUREPEAKDETECTH__
#define __PRESSUREPEAKDETECTH__

const int BUFFER_LEN = 15; // determines how many samples will be stored at a time

class cicularBuffer {
    // old items overwrite new items
    // item 0 is the oldest, item BUFFER_LEN -1 is the newest
    public:
        cicularBuffer() {
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
        cicularBuffer sampleBuffer; // filtered samples
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
        cicularBuffer sb; // ssf samples
        volatile bool rising = true;
        volatile int left_moving_sum = 0;
        volatile int right_moving_sum = 0;
        volatile int refractory_period = 40;
        volatile int peak_threshold = 0;
        volatile int peak_threshold_sum = 0;
        volatile int rp_counter = 0;
        
        volatile int peak1 = 0;
        volatile int peak2 = 0;
        volatile int peak3 = 0;
        volatile int peak4 = 0;
        volatile int peak5 = 0;
        volatile int peak6 = 0;
        volatile int peak7 = 0;
        volatile int peak8 = 0;
        volatile int peak9 = 0;
        volatile int peak10 = 0;
    public:
        void updatePeakThreshold(int newPeakVal) {
            peak_threshold_sum -= peak1;
            peak_threshold_sum += newPeakVal;
            
            peak1 = peak2;
            peak2 = peak3;
            peak3 = peak4;
            peak4 = peak5;
            peak5 = peak6;
            peak6 = peak7;
            peak7 = peak8;
            peak8 = peak9;
            peak9 = peak10;
            peak10 = newPeakVal;
            
            peak_threshold = peak_threshold_sum / 25;
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
