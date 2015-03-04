import bp_trigger_plot
import sys
import numpy
from PyQt4 import QtCore, QtGui
import PyQt4.Qwt5 as Qwt
import serial

ser = serial.Serial()
ser.baudrate = 115200
ser.port = 'COM14'
ser.timeout = 1
ser.open()

numPoints=100
xs=numpy.arange(numPoints)
ys=numpy.zeros(numPoints)

def plotSomething():
    global ys
    ys=numpy.roll(ys, -1)
    try:
        ys.flat[99] = int(ser.readline())
    except ValueError:
        print "caught valurerror"
        ys.flat[99] = ys.flat[98]
    #print "PLOTTING"
    c.setData(xs, ys)
    bpplot.qwtPlot.replot()   

if __name__ == "__main__":
    app = QtGui.QApplication(sys.argv)

    ### SET-UP WINDOWS
    
    # WINDOW plot
    win_plot = bp_trigger_plot.QtGui.QMainWindow()
    bpplot = bp_trigger_plot.Ui_win_trigger()
    bpplot.setupUi(win_plot)
    bpplot.btnA.clicked.connect(plotSomething)
    bpplot.btnB.clicked.connect(lambda: bpplot.timer.setInterval(100.0))
    bpplot.btnC.clicked.connect(lambda: bpplot.timer.setInterval(10.0))
    bpplot.btnD.clicked.connect(lambda: bpplot.timer.setInterval(1.0))
    c=Qwt.QwtPlotCurve()  
    c.attach(uiplot.qwtPlot)

    uiplot.timer = QtCore.QTimer()
    uiplot.timer.start(100.0)
    
    win_plot.connect(uiplot.timer, QtCore.SIGNAL('timeout()'), plotSomething) 
    

    ### DISPLAY WINDOWS
    win_plot.show()

    #WAIT UNTIL QT RETURNS EXIT CODE
    sys.exit(app.exec_())