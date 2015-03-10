import ui_trigger

import sys

from PyQt4 import QtCore, QtGui

if __name__ == "__main__":
    app = QtGui.QApplication(sys.argv)

    ### SET-UP WINDOWS
    
    # WINDOW trigger
    win_trigger = ui_trigger.QtGui.QMainWindow()
    uitrigger = ui_trigger.Ui_win_trigger()
    uitrigger.setupUi(win_trigger)

    ### DISPLAY WINDOWS
    win_trigger.show()

    #WAIT UNTIL QT RETURNS EXIT CODE
    sys.exit(app.exec_())