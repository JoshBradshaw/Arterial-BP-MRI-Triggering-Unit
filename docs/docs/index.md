## System Overview

The Arterial Blood Pressure Triggering System is an open source tool developed at SickKids. The system converts a blood pressure signal into a TTL signal, as illustrated below.

![alt text](/images/bp_vs_ttl.jpg "illustration of how the triggering system transforms blood pressure signal into TTL signal")

This is useful for situations in which a triggering signal is required to correct for pulsatile flow or cardiac motion, but the standard clinical ECG or pulse oximeter based triggering systems can not be used. The original application of this system was to provide a retrospective gating signal during 4DFlow and phase contrast scans on prenatal Yorkshire pig fetus's hearts. During the experiments, the fetal pigs blood pressure was monitored using an invasive Samba flow probe in the carotid artery. TODO: INSERT SOME IMAGES

### Key Features

* Compatible with [Transonic](http://www.transonic.com/products/research/product/t402t403-multi-channel-research-consoles/) and Samba Sensors pressure measurement modules, and can easily be adapted to work with most other pressure measurement instruments
* Unit calibrates itself automatically every three seconds to correct for baseline drift, and large changes in maximum signal amplitude
* Performs well even on noisy and irregular blood pressure waveforms
* Plug-and-play operation with a one second boot time
* The triggering unit has voltage regulation circuitry complete with inline fuses, to ensure that the scanner will be protected in the event of an electrical fault in the computer or pressure measurement instrument
* Simple monitoring software shows real-time plots of the blood pressure and triggering waveforms, and logs that data so that it can be compared against the scan data in post processing

### Hardware

The triggering unit is shown below:

![alt text](/images/finished_build.JPG "triggering unit photo")

The connections on the front panel of the triggering unit are:

Samba Sensor Input (s-video)|Transonic Sensor Input (BNC) |External Trigger Out (RCA) 
----------------------------|-----------------------------|--------------------------
Input Selection Switch      |                             |Trigger Status LED

Finally, a USB cable runs out of the back corner of the unit. This cable is connected to a computer to power the unit, and optionally to transfer the blood pressure readings to the monitoring software.

For more information about the hardware design see:

1. Circuit design TODO: finish writing page
2. PCB Design TODO: finish writing page
3. Assembly
4. Triggering algorithm design

## Monitoring Software

![alt text](/images/monitoring_system.png "triggering unit photo")

The monitoring software is not required to use the triggering unit, but it is very useful. The top chart animation shows the input signal from the pressure catheter, and the bottom chart animation shows the triggering signal that's been sent to the scanner. These charts are updated in real-time with <20ms of delay, so you'll be aware of any irregular heartbeats, or problems with the pressure catheter, right as they happen. Every run of the triggering unit is logged with time and date stamps, so it can be compared with the MRI data in post processing if required.

For more information about the monitoring software see:
1. Monitoring software design

### Scanner compatibility

The SickKids research MRI scanner is the Siemens Magnetom Trio with a field strength of 3T, so the triggering unit's TTL output is calibrated to meet or exceed all of the parameters given in Siemen's timing diagram (taken from the latest version of the Magnetom Trio operating manual). The TTL pulses sent by the unit are 20ms in duration at 3.3V, which should be sufficient for most MRI scanners.

### How to aquire a unit

So far only two of these units have been built, and they were both hand assembled. If you're moderately competant with software and electronics you can build one yourself using the schematics and this tutuorial.

Feel free to email me at joshbradshaw11@gmail.com if you are interested in adapting this unit to work with a different blood pressure instrument, or want to commision me to build one for you.