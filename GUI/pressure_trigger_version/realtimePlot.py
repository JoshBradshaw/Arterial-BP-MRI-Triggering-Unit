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

ser = serial.Serial()
ser.baudrate = 115200
ser.port = 'COM14'
ser.timeout = 1
ser.open()


def setup_scale():
    global xs
    global ys
    global trigger
    global LAST_POINT
    global ts

    xs=numpy.arange(0, 4, 0.016)
    numPoints = len(xs)
    LAST_POINT = numPoints-1
    ys=numpy.zeros(numPoints)
    ts=numpy.zeros(numPoints)
    trigger = False

def serial_port_chosen():
    print gui.serialPortSelector.currentText()
    return

def speed_selected():
    print gui.speedSelect.currentText()
    return

def start_pressed():
    print gui.startBtn.isChecked()
    return

def log_data_pressed():
    print gui.logDataButton.isChecked()
    return


def plot_bp_and_trigger():
    global ys
    global ts
    global trigger

    try:
        val = int(ser.readline())
    except ValueError:
        print "failed to parse serial input"
        return

    if val == TRIGGER_PULSE_CODE:
        trigger = True
        return
    else:
        ys=numpy.roll(ys, -1)
        ts=numpy.roll(ts, -1)  

        ys[LAST_POINT] = val / SIXTEEN_BIT_TO_COUNTS

        if trigger:
            ts[LAST_POINT] = 1
            trigger = False
        else:
            ts[LAST_POINT] = 0
            

        bp_curve.setData(xs, ys)
        gui.bpPlot.replot() 
        trigger_curve.setData(xs, ts)
        gui.triggerPlot.replot()  

    

if __name__ == "__main__":
    app = QtGui.QApplication(sys.argv)

    ### SET-UP WINDOWS
    ## get all serial ports


    # WINDOW plot
    setup_scale()
    win_plot = ui_trigger.QtGui.QMainWindow()
    gui = ui_trigger.Ui_win_trigger()
    gui.setupUi(win_plot)
    gui.bpPlot.setAxisScale(0, 0, 5, 5)
    gui.bpPlot.setAxisTitle(0, "BP Signal (V)")
    gui.bpPlot.setAxisTitle(1, "Time (seconds)")
    gui.triggerPlot.setAxisScale(0, 0, 1, 1)
    gui.triggerPlot.setAxisMaxMinor(0, 1)
    gui.triggerPlot.setAxisTitle(0, "Logic Level")
    gui.triggerPlot.setAxisTitle(1, "Time (seconds)")
    gui.triggerPlot.setCanvasBackground(Qt.Qt.black)

    port_option = {}
    for port, description, details in serial.tools.list_ports.comports():
        port_option[port] = description
        gui.serialPortSelector.addItem(port_option[port])
    
    gui.timer = QtCore.QTimer()
    gui.timer.start(100.0)
    gui.timer.setInterval(1.0)

    bp_curve=Qwt.QwtPlotCurve()  
    bp_curve.attach(gui.bpPlot)
    bp_curve.setPen(Qt.QPen(Qt.Qt.green))

    trigger_curve = Qwt.QwtPlotCurve()
    trigger_curve.attach(gui.triggerPlot)
    trigger_curve.setPen(Qt.QPen(Qt.Qt.green))

    win_plot.connect(gui.timer, QtCore.SIGNAL('timeout()'), plot_bp_and_trigger) 
    win_plot.connect(gui.serialPortSelector, QtCore.SIGNAL('activated(QString)'), serial_port_chosen)
    win_plot.connect(gui.speedSelect, QtCore.SIGNAL('activated(QString)'), speed_selected)
    win_plot.connect(gui.startBtn, QtCore.SIGNAL('released()'), start_pressed)
    win_plot.connect(gui.logDataButton, QtCore.SIGNAL('released()'), log_data_pressed)

    ### DISPLAY WINDOWS
    win_plot.show()

    #WAIT UNTIL QT RETURNS EXIT CODE
    sys.exit(app.exec_())