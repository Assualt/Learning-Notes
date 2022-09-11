import sys
from PyQt5 import QtWidgets
from PyQt5.QtCore import QTimer, pyqtSlot
from demo.ui.mainWidget import Ui_Widget
from PyQt5.QtWidgets import QCompleter
from demo.config import config, logger
import time

class ResourceManager(object):
    def __init__(self):
        pass


class MyWindow(QtWidgets.QWidget, Ui_Widget):
    def __init__(self):
        super(MyWindow, self).__init__()
        self.setupUi(self)

        # Event
        self.lineEditDest.setCompleter(QCompleter(config.get_station_name()))
        self.lineEditOrigin.setCompleter(QCompleter(config.get_station_name()))

        # Slots
        self.init_slots()

    def init_slots(self):
        self.startTimers(1000, self.format_time_label)
        self.ButtonSearch.clicked.connect(self.button_search_slots)

    def startTimers(self, timeout=1000, func=None):
        timer = QTimer(self)  # 初始化一个定时器
        timer.timeout.connect(func)
        timer.start(timeout)

    @pyqtSlot()
    def format_time_label(self):
        self.label.setText(time.strftime("Date: %Y/%m/%d %H:%M:%S", time.localtime(time.time())))

    @pyqtSlot()
    def button_search_slots(self):
        """
        Button Search clicked
        :return:
        """
        logger.info("button clicked")


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    window = MyWindow()
    window.show()
    sys.exit(app.exec())
