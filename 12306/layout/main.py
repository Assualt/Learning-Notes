import sys
from PyQt5 import QtWidgets,QtCore

from mainwindow import MainWindow

def mainExec():
    app=QtWidgets.QApplication(sys.argv)

    main = MainWindow()
    main.show()
    sys.exit(app.exec())
    pass


if __name__ == '__main__':
    mainExec()
