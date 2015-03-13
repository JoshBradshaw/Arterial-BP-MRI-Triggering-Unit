from __future__ import division

import os
import ui_trigger
import sys
import numpy
from PyQt4 import QtCore, QtGui, Qt
import PyQt4.Qwt5 as Qwt
import serial
import serial.tools.list_ports
import logging
import logging.handlers
from datetime import datetime

TRIGGER_PULSE_CODE = 100000
SIXTEEN_BIT_TO_COUNTS = 13107.2 # 2^16 counts / 5 V = 13107.2 counts / volt
SERIAL_BAUDRATE = 115200

# larger x-axis -> slower progression of the line accross the plot
speeds = {
    'Slowest': 40,
    'Slow': 20,
    'Medium': 10,
    'Fast': 4
}

## setup gui
app = QtGui.QApplication(sys.argv)
### SET-UP WINDOWS
win_plot = ui_trigger.QtGui.QMainWindow()
gui = ui_trigger.Ui_win_trigger()
gui.setupUi(win_plot)
gui.bpPlot.setAxisScale(0, 0, 5, 5)
gui.bpPlot.setAxisScale(1, 0, 4, 4)
gui.bpPlot.setAxisTitle(0, "BP Signal (V)")
gui.triggerPlot.setAxisScale(0, 0, 1, 1)
gui.triggerPlot.setAxisMaxMinor(0, 1)
gui.triggerPlot.setAxisTitle(0, "Logic Level")
gui.triggerPlot.setCanvasBackground(Qt.Qt.black)
# times the plot refresh
gui.timer = QtCore.QTimer()
# line on blood pressure graph
bp_curve=Qwt.QwtPlotCurve()  
bp_curve.attach(gui.bpPlot)
bp_curve.setPaintAttribute(Qwt.QwtPlotCurve.PaintFiltered, False)
bp_curve.setPaintAttribute(Qwt.QwtPlotCurve.ClipPolygons, True)
bp_curve.setPen(Qt.QPen(Qt.Qt.green))
# line on triggering graph
trigger_curve = Qwt.QwtPlotCurve()
trigger_curve.attach(gui.triggerPlot)
trigger_curve.setPaintAttribute(Qwt.QwtPlotCurve.PaintFiltered, False)
trigger_curve.setPaintAttribute(Qwt.QwtPlotCurve.ClipPolygons, True)
trigger_curve.setPen(Qt.QPen(Qt.Qt.green))

# if its a windows 7 machine clean up the blinkyness by running anti aliasing
# if its a windows xp or mac, do not run anti aliasing because it will lag
# if its a osx or linux box this call will not work
anti_alias = False
try:
    major_verion = sys.getwindowsversion().major
    if major_verion >= 6:
        anti_alias = True
except:
    pass

if anti_alias:
    trigger_curve.setRenderHint(Qwt.QwtPlotItem.RenderAntialiased) # prettier, but laggy on slow computers
    bp_curve.setRenderHint(Qwt.QwtPlotItem.RenderAntialiased) # prettier, but laggy on slow computers


log_dir = "logs"
if not os.path.exists(log_dir):
    os.makedirs(log_dir)

log_filename = os.path.join(log_dir, 'bp_triggering.log')
# Set up a specific logger with our desired output level
logger = logging.getLogger('triggeringLogger')
logger.setLevel(logging.INFO)

handler = logging.handlers.RotatingFileHandler(log_filename, backupCount=200)
logger.addHandler(handler)

def open_log_directory():
    os.startfile(log_dir) # only works on windows, but that's of little consequence

class Teensy(object):
    """Teensy is the microcontroller that drives the BP triggering unit. While
    it runs it it sends its current sensor levels out over serial. This serial
    communication is strictly one-way.

    The teensy has two codes it sends continuously:
        Trigger sent -> 100000
        Sensor Value -> [0, 65536] value read by the 16 bit ADC every 16 ms
    """
    def __init__(self):
        self.port_options = {}
        # populate the dropdown menu of serial ports on the form
        for port, description, details in serial.tools.list_ports.comports():
            port = str(port)
            self.port_options[description] = port
            gui.serialPortSelector.addItem(description)
        self.ser = serial.Serial()

    def start(self):
        """begin Serial communications with the teensy"""
        if self.ser.isOpen():
            self.ser.close()
        # must be the same baudrate as the one used in Serial.begin in the microcontroller program
        self.ser.baudrate = SERIAL_BAUDRATE
        self.ser.timeout = 1 # 1 second just in case readline() ever hangs
        self.ser.port = self.get_serial_port()
        self.ser.open()

    def stop(self):
        """end Serial communications with the teensy, freeing the serial port for other uses like reprogramming"""
        if self.ser.isOpen():
            self.ser.close()

    def get_sensor_val(self):
        """read one line of data over serial and parse it"""
        try:
            return int(self.ser.readline())
        except ValueError:
            print "Failed to parse serial input"
            return
    
    def get_serial_port(self):
        """get the port which is currently selected in the form"""
        return self.port_options[str(gui.serialPortSelector.currentText())]


class plotData(object):
    """Updates the plot animations with the most recent sensor data, and 
    rescales the axis as required.
    """
    def __init__(self):
        self.select_speed()
        self.logging = gui.logDataButton.isChecked()
        self.teensy = Teensy()

    def plot_bp_and_trigger(self):
        """shifts the lines on the chart animation by one points, and adds the new point to the rightmost edge"""
        val = self.teensy.get_sensor_val()
        # trigger pulses are not marked immediately, instead they are marked when the next sensor
        # value is recieved for the sake of staying in perfect synch
        if val == TRIGGER_PULSE_CODE:
            self.trigger = True
            return
        # shift the curves one point
        self.ys=numpy.roll(self.ys, -1)
        self.ts=numpy.roll(self.ts, -1)  
        # 16 bit ADC value range 0-65536, want to reduce to 0-5V for human readability
        self.ys[self.last_point] = val / SIXTEEN_BIT_TO_COUNTS
        # mark trigger pulse
        if self.trigger:
            self.ts[self.last_point] = 1
            self.trigger = False
        else:
            self.ts[self.last_point] = 0
        # log the sample
        logger.info("{}: {} {}".format(datetime.now().strftime('%Y-%m-%d-%H-%M-%f'), self.ys[self.last_point], self.ts[self.last_point]))
        # redraw the lines (note this is really inefficient, redrawing a dirty rectangle only would be much faster)
        bp_curve.setData(self.xs, self.ys)
        gui.bpPlot.replot() 
        trigger_curve.setData(self.xs, self.ts)
        gui.triggerPlot.replot()

    def select_speed(self):
        """get the speed selected on the dropdown and setup the axis scales accordingly"""
        self.speed = speeds[str(gui.speedSelect.currentText())]
        self.xs = numpy.arange(0, self.speed, 0.016)
        self.numPoints = len(self.xs)
        self.last_point = self.numPoints-1
        self.ys = numpy.zeros(self.numPoints)
        self.ts = numpy.zeros(self.numPoints)
        self.trigger = False

    def start_stop(self):
        """start and stop the animation and logging (this has no effect on the actual triggering unit)"""
        if gui.startBtn.isChecked():
            handler.doRollover()
            self.teensy.start()
            gui.timer.start(1.0) # poll the serial port every 1ms, 1 byte is expected every 4ms
            win_plot.connect(gui.timer, QtCore.SIGNAL('timeout()'), self.plot_bp_and_trigger) 
        else:
            gui.timer.stop()
            self.teensy.stop()
        

if __name__ == '__main__':
    plots = plotData()
    # connect the buttons and dropdowns to there handler functions
    win_plot.connect(gui.speedSelect, QtCore.SIGNAL('activated(QString)'), plots.select_speed)
    win_plot.connect(gui.startBtn, QtCore.SIGNAL('released()'), plots.start_stop)
    win_plot.connect(gui.logDataButton, QtCore.SIGNAL('released()'), open_log_directory)

    ### DISPLAY WINDOWS
    win_plot.show()

    #WAIT UNTIL QT RETURNS EXIT CODE
    sys.exit(app.exec_())