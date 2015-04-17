## System Block Diagram

The blood pressure signal is collected by an optical pressure transducer, which is inserted into the fetal pig’s carotid artery. The optical blood pressure signal is converted into an analog electrical signal by the pressure measurement module (see Appendix A for details), which is then amplified by the variable gain amplifier and measured by the analog to digital converter. 

![alt text](/images/system-block-diagram.png "System block diagram")

The microcontroller uses a variety of digital signal processing algorithms to filter the signal and detect the pig’s pulse. Each time a heartbeat is detected, a triggering signal is sent to the MRI scanner, and a signal is sent to the monitoring software so that the experimenter can verify that the triggering signal was sent at the correct instant.
