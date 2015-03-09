# Signal Monitoring Tool

The blood pressure triggering device needs to have an interface so that the researcher operating it can tell whether it is functioning properly. The interface must provide the operator with near real-time feedback on the signal level of the pressure sensor, and the timing of the trigger pulses. The interface should provide the researcher with an easy way to export the pressure data for further analysis, and should be sufficiently simple to install on a new computer that it can be quickly transferred via USB key or network drive to a different computer.

## Starting Point

I looked at a few different open source real time plotting solutions in python. The first one I tried involved using MatPlotlib. It worked, but wouldn't run at the frame rate I wanted (>24fps) without time consuming optimization. Next I found [this excellent project](http://www.swharden.com/blog/2013-05-08-realtime-data-plotting-in-python/) which gives a simple method for incorporating real time plotting, and a easy to use GUI tool. This was exactly what I needed.
