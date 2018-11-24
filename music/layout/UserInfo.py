#!/usr/bin/python
#! -*- encoding:utf-8 -*-

import sys
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.Qt import *

import json



class UserInfoDialog(QDialog):

    def __init__(self,parent=None):
        super(UserInfoDialog,self).__init__(parent)

        self.initData()  # 1.初始化数据
        self.initUI()  # 2.初始化界面
        self.initSlots()  # 3.初始化信号槽函数

    def initData(self):
        pass

    def initUI(self):
        mainlayout=QVBoxLayout()

        self._createDialog()
        mainlayout.addWidget(self.bodywidget)
        self.setLayout(mainlayout)
        self.setGeometry(20,20,20,20)
        self.setFixedSize(276,407)
        self.setWindowFlags(Qt.CustomizeWindowHint)




        pass

    def initSlots(self):
        pass

    @pyqtSlot()
    def onExit(self):
        # self.hide()
        self.reject()
        pass

    def _createDialog(self):
        self.bodywidget=QWidget()
        bodylayout=QVBoxLayout()
        button = QPushButton("helowolrd")
        label = QLabel("helloworld")
        label.setFixedSize(80,80)

        bodylayout.addWidget(button)
        bodylayout.addWidget(label)

        self.bodywidget.setLayout(bodylayout)
        button.clicked.connect(self.onExit)
        pass

if __name__ == '__main__':
    app = QApplication(sys.argv)
    a=UserInfoDialog()
    a.show()
    sys.exit(app.exec())