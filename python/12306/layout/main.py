import sys
from PyQt5 import QtWidgets
from mainwindow import MainWindow


def main_exec():
    app = QtWidgets.QApplication(sys.argv)

    main = MainWindow()
    main.show()
    sys.exit(app.exec())
    pass


if __name__ == '__main__':
    main_exec()
