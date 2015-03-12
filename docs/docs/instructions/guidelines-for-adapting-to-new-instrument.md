## Guidelines for Adapting the Pressure Trigger Module to Work with a new Sensor/Instrument

The Arterial BP Triggering System should be fairly easy to adapt to a new pressure measurement system. 

### Instruments with Analog Outputs

As it stands, the pressure trigger module has two analog inputs:

1. One 0-5V analog input for the Samba Sensors pressure measurement system. (connected to signal pin SB and ground pin SG on the circuit board)
2. One -5-5V analog input for the Transonic pressure measurement system. (connected to signal pin TC and ground pin TG on the circuit board)

If the pressure measurement system you wish to work with has an analog output less than or equal to either of these voltage ranges, then you're golden. Just cut the wires off of the connector. Do this carefully so that you can reuse the wires and avoid the painful process of de-soldering the wires from the PCB. 

This video will teach you everything you need to know about soldering: [Dave Jones Through Hole Soldering Tutorial](https://www.youtube.com/watch?v=fYz5nIHH0iY)

Examples: 

1. An instrument with an analog voltage output -2-2V could be connected to the Transonic input
2. An instrument with an analog voltage output of 0-3V could be connected to the Samba input.

If the instrument has an output voltage range greater than either of these inputs, then all is not lost. Just add a [voltage divider](http://en.wikipedia.org/wiki/Voltage_divider) between the instrument and the pressure trigger module's analog input. Choose resistors in the 10-100Kohm resistance range to avoid loading down the instrument and you'll be golden.

### Instruments with Digital Outputs

If the instrument only has a digital output available, then this problem will be more advanced. The internal circuitry and microcontroller code will need to be modified. Feel free to get in touch with me at joshbradshaw11@gmail.com with details about the instrument and I will help you.

If you wish to do the modifications to the circuit and PCB yourself, then everything you will need is in the eagle project files in the repository. Tons of useful information about interfacing the pressure trigger module's [Teensy3.1](https://www.pjrc.com/teensy/teensy31.html) microcontroller using various digital communications protocols is available on the [support forum](https://forum.pjrc.com/forum.php).

### Notes

1. If you need access to electronics test equipment beyond what is avaialable at SickKids, go to the mouse imaging cetner (MiCE) in the Toronto Center for Phenogenetics. Sharon Portney can get you access for the day. Bring your own parts 
2. Most connectors are available from the basement of the home hardware at College/Spadina or Active Surplus on Queen St. W. Failing that, order from digikey.com.