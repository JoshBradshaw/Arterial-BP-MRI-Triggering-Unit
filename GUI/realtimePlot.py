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

TRIGGER_PULSE_CODE = 100000
SIXTEEN_BIT_TO_COUNTS = 13107.2
SERIAL_BAUDRATE = 115200

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
bp_curve.setRenderHint(Qwt.QwtPlotItem.RenderAntialiased)
bp_curve.setPen(Qt.QPen(Qt.Qt.green))
# line on triggering graph
trigger_curve = Qwt.QwtPlotCurve()
trigger_curve.attach(gui.triggerPlot)
trigger_curve.setPaintAttribute(Qwt.QwtPlotCurve.PaintFiltered, False)
trigger_curve.setPaintAttribute(Qwt.QwtPlotCurve.ClipPolygons, True)
trigger_curve.setRenderHint(Qwt.QwtPlotItem.RenderAntialiased)
trigger_curve.setPen(Qt.QPen(Qt.Qt.green))

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
    os.startfile(log_dir)

class Teensy(object):
    """Teensy is the microcontroller that drives the BP triggering unit. While
    it runs it it sends its current sensor levels out over serial. This serial
    communication is strictly one-way.

    The teensy has two codes it sends continuously:
        Trigger sent: 100,000
        Sensor Value: [0, 65536] value read by the 16 bit ADC every 16 ms
    """
    def __init__(self):
        self.port_options = {}
        for port, description, details in serial.tools.list_ports.comports():
            port = str(port)
            self.port_options[description] = port
            gui.serialPortSelector.addItem(description)
        self.ser = serial.Serial()

    def start(self):
        if self.ser.isOpen():
            self.ser.close()

        self.ser.baudrate = SERIAL_BAUDRATE
        self.ser.timeout = 1
        self.ser.port = self.get_serial_port()
        self.ser.open()

    def stop(self):
        if self.ser.isOpen():
            self.ser.close()

    def get_sensor_val(self):
        assert(self.ser.isOpen(), "Serial communications have not started, can not get value")

        try:
            return int(self.ser.readline())
        except ValueError:
            print "Failed to parse serial input"
            return
    
    def get_serial_port(self):
        return self.port_options[str(gui.serialPortSelector.currentText())]


class plotData(object):
    def __init__(self):
        self.select_speed()
        self.logging = gui.logDataButton.isChecked()
        self.teensy = Teensy()

    def plot_bp_and_trigger(self):
        val = self.teensy.get_sensor_val()

        if val == TRIGGER_PULSE_CODE:
            self.trigger = True
            return

        self.ys=numpy.roll(self.ys, -1)
        self.ts=numpy.roll(self.ts, -1)  

        self.ys[self.last_point] = val / SIXTEEN_BIT_TO_COUNTS

        if self.trigger:
            self.ts[self.last_point] = 1
            self.trigger = False
        else:
            self.ts[self.last_point] = 0

        logger.info("{} {}".format(self.ys[self.last_point], self.ts[self.last_point]))
            
        bp_curve.setData(self.xs, self.ys)
        gui.bpPlot.replot() 
        trigger_curve.setData(self.xs, self.ts)
        gui.triggerPlot.replot()

    def select_speed(self):
        self.speed = speeds[str(gui.speedSelect.currentText())]
        self.xs = numpy.arange(0, self.speed, 0.016)
        self.numPoints = len(self.xs)
        self.last_point = self.numPoints-1
        self.ys = numpy.zeros(self.numPoints)
        self.ts = numpy.zeros(self.numPoints)
        self.trigger = False

    def start(self):
        if gui.startBtn.isChecked():
            handler.doRollover()
            self.teensy.start()
            gui.timer.start(2.0)
            win_plot.connect(gui.timer, QtCore.SIGNAL('timeout()'), self.plot_bp_and_trigger) 
        else:
            gui.timer.stop()
            self.teensy.stop()
        

if __name__ == '__main__':
    plots = plotData()
    win_plot.connect(gui.speedSelect, QtCore.SIGNAL('activated(QString)'), plots.select_speed)
    win_plot.connect(gui.startBtn, QtCore.SIGNAL('released()'), plots.start)
    win_plot.connect(gui.logDataButton, QtCore.SIGNAL('released()'), open_log_directory)

    ### DISPLAY WINDOWS
    win_plot.show()

    #WAIT UNTIL QT RETURNS EXIT CODE
    sys.exit(app.exec_())