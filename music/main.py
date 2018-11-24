import sys
from PyQt5 import QtWidgets,QtCore

sys.path.append("./layout")
import Mainwindow as main

def mainExec():
    app=QtWidgets.QApplication(sys.argv)
    m = main.Mainwindow()
    m.show()
    sys.exit(app.exec())
    pass

if __name__ == '__main__':
    mainExec()
    pass
