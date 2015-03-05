from __future__ import division

import ui_trigger
import sys
import numpy
from PyQt4 import QtCore, QtGui, Qt
import PyQt4.Qwt5 as Qwt
import serial
import serial.tools.list_ports

TRIGGER_PULSE_CODE = 100000
SIXTEEN_BIT_TO_COUNTS = 13107.2
SERIAL_BAUDRATE = 115200

speeds = {
    'Slow': 16,
    'Medium': 8,
    'Fast': 4
}

class Teensy(object):
    def __init__(self):
        self.port_options = {}
        for port, description, details in serial.tools.list_ports.comports():
            self.port_options[port] = description
            gui.serialPortSelector.addItem(self.port_options[port])

        self.port = gui.serialPortSelector.currentText()
        self.ser = serial.Serial()
        self.ser.baudrate = SERIAL_BAUDRATE
        self.ser.name = self.port
        self.ser.timeout = 1
    
    def start():
        self.ser.name = self.port # update port to match selection
        self.ser.open()

    def end():
        if self.ser.isOpen():
            self.ser.close()

    def select_serial_port():
        self.port = gui.serialPortSelector.currentText()


class plotData(object):
    def __init__(self):
        self.speed = gui.speedSelect.currentText()
        speed_selected(self.speed)
        self.logging = gui.logDataButton.isChecked()

    def plot_bp_and_trigger():
        try:
            val = int(ser.readline())
        except ValueError:
            print "failed to parse serial input"
            return

        if val == TRIGGER_PULSE_CODE:
            self.trigger = True
            return

        self.ys=numpy.roll(self.ys, -1)
        self.ts=numpy.roll(self.ts, -1)  

        self.ys[LAST_POINT] = val / SIXTEEN_BIT_TO_COUNTS

        if self.trigger:
            self.ts[LAST_POINT] = 1
            self.trigger = False
        else:
            self.ts[LAST_POINT] = 0
            
        bp_curve.setData(self.xs, self.ys)
        gui.bpPlot.replot() 
        trigger_curve.setData(self.xs, self.ts)
        gui.triggerPlot.replot()

    def select_speed():
        self.xs = numpy.arange(0, 4, 0.016)
        self.numPoints = len(self.xs)
        LAST_POINT = self.numPoints-1
        self.ys = numpy.zeros(self.numPoints)
        self.ts = numpy.zeros(self.numPoints)
        self.trigger = False

    def speed_selected():
        self.speed = gui.speedSelect.currentText()
        self.xs = numpy.arange(0, speeds[self.speed], 0.016)
        self.numPoints = len(self.xs)
        self.ys = numpy.zeros(self.numPoints)
        self.ts = numpy.zeros(self.numPoints)

    def start():
        if gui.startBtn.isChecked():
            gui.timer.start(2)
        else:
            gui.timer.stop()

    def toggle_logging():
        self.logging = gui.logDataButton.isChecked()
        

if __name__ == '__main__':
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

    gui.timer = QtCore.QTimer()

    bp_curve=Qwt.QwtPlotCurve()  
    bp_curve.attach(gui.bpPlot)
    bp_curve.setPaintAttribute(Qwt.QwtPlotCurve.PaintFiltered, False)
    bp_curve.setPaintAttribute(Qwt.QwtPlotCurve.ClipPolygons, True)
    bp_curve.setRenderHint(Qwt.QwtPlotItem.RenderAntialiased)
    bp_curve.setPen(Qt.QPen(Qt.Qt.green))

    trigger_curve = Qwt.QwtPlotCurve()
    trigger_curve.attach(gui.triggerPlot)
    trigger_curve.setPaintAttribute(Qwt.QwtPlotCurve.PaintFiltered, False)
    trigger_curve.setPaintAttribute(Qwt.QwtPlotCurve.ClipPolygons, True)
    trigger_curve.setRenderHint(Qwt.QwtPlotItem.RenderAntialiased)
    trigger_curve.setPen(Qt.QPen(Qt.Qt.green))

    teensy = Teensy()
    plots = plotData()

    win_plot.connect(gui.timer, QtCore.SIGNAL('timeout()'), plot_bp_and_trigger) 
    win_plot.connect(gui.serialPortSelector, QtCore.SIGNAL('activated(QString)'), teensy.select_serial_port())
    win_plot.connect(gui.speedSelect, QtCore.SIGNAL('activated(QString)'), plots.select_speed())
    win_plot.connect(gui.startBtn, QtCore.SIGNAL('released()'), plots.start())
    win_plot.connect(gui.logDataButton, QtCore.SIGNAL('released()'), plots.toggle_logging)

    ### DISPLAY WINDOWS
    win_plot.show()

    #WAIT UNTIL QT RETURNS EXIT CODE
    sys.exit(app.exec_())