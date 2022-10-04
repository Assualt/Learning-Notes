# -*- coding:utf-8 -*-
__author__ = 'xhou'

from PyQt5.QtWidgets import *
from PyQt5.QtCore import Qt, QDateTime, QTimer, pyqtSlot
import json
import time
from get_ticket_results import getResult


class MainWindow(QWidget):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.load_dict = None
        self.r_result = None
        self.station_list = None
        self.labelSearchCost = None
        self.initStations()
        self.initUi()
        self.startTimers()

    def initStations(self):
        try:
            self.station_list = []
            self.r_result = []
            with open('../res/location.json', 'r', encoding='utf-8') as f:
                self.load_dict = json.load(f)
                for val in self.load_dict:
                    self.station_list.append(val["name"] + "(" + val["code"] + ")")
        except IOError:
            print("can't open such file")
        pass

    def initUi(self):
        self.create_header_group_box()
        self.creatBodyGroupBox()
        mainLayout = QVBoxLayout()
        mainLayout.addWidget(self.headgroupbox)
        mainLayout.addWidget(self.bodyGroupBox)
        self.setLayout(mainLayout)
        pass

    def create_header_group_box(self):
        self.setWindowTitle('12306 查询器')
        # head widget
        self.headgroupbox = QGroupBox("head")
        self.headgroupbox.setAlignment(Qt.AlignLeading)
        headlayout = QGridLayout()
        # 组件
        labelSrc = QLabel("始发地")
        labelSrc.setAlignment(Qt.AlignCenter)
        self.labelSrcEdit = QLineEdit()
        self.labelSrcEdit.setPlaceholderText("例如:重庆 北京 上海 深圳 广州")

        labelDst = QLabel("目的地")
        labelDst.setAlignment(Qt.AlignCenter)
        self.labelDstEdit = QLineEdit()
        self.labelDstEdit.setPlaceholderText("例如:重庆 北京 上海 深圳 广州")

        labelDate = QLabel("出发日期")
        labelDate.setAlignment(Qt.AlignCenter)
        self.labelCalender = QDateTimeEdit(QDateTime.currentDateTime(), self.headgroupbox)
        self.labelCalender.setDisplayFormat('yyyy-MM-dd')
        self.labelCalender.setCalendarPopup(True)
        labelHide = QLabel()
        self.labelSearchCost = QLabel("搜索到结果,耗时:0s")
        ButtonSearch = QPushButton("搜索")
        ButtonSearch.clicked.connect(self.onSearch)

        headlayout.addWidget(labelSrc, 0, 0, 1, 2)
        headlayout.addWidget(self.labelSrcEdit, 0, 2, 1, 4)
        headlayout.addWidget(labelDst, 1, 0, 1, 2)
        headlayout.addWidget(self.labelDstEdit, 1, 2, 1, 4)
        headlayout.addWidget(labelDate, 2, 0, 1, 2)
        headlayout.addWidget(self.labelCalender, 2, 2, 1, 2)
        headlayout.addWidget(labelHide, 0, 7, 3, 1)
        headlayout.addWidget(self.labelSearchCost, 0, 9, 1, 1)
        headlayout.addWidget(ButtonSearch, 0, 9, 3, 2)

        # event
        self.labelSrcEdit.setCompleter(QCompleter(self.station_list))
        self.labelDstEdit.setCompleter(QCompleter(self.station_list))
        self.headgroupbox.setLayout(headlayout)
        pass

    def creatBodyGroupBox(self):
        self.bodyGroupBox = QGroupBox("body")
        bodylayout = QVBoxLayout()
        self.bodytablewdiget = QTableWidget(20, 20, self.bodyGroupBox)
        self.bodytablewdiget.setAlternatingRowColors(True)
        self.bodytablewdiget.setHorizontalHeaderLabels(
            ["车次", "始发站", "终点站", "出发站", "到达站", "出发时间",
             "到达时间", "历时", "商务座/特等座", "一等座", "二等座",
             "高级软卧", "软卧", "动卧", "硬卧", "软座", "硬座",
             "无座", "其他", "备注"])
        self.bodytablewdiget.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.bodytablewdiget.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        bodylayout.addWidget(self.bodytablewdiget, 1)

        bottomlayout = QHBoxLayout()
        self.labeltime = QLabel()
        bottomlayout.addWidget(self.labeltime, 1, Qt.AlignLeft)
        self.labelpageState = QLabel("总共0页, 共0条记录")
        self.labelpageCurrent = QLabel("当前第0页")
        bottomlayout.addWidget(self.labelpageState, 1, Qt.AlignCenter)
        bottomlayout.addWidget(self.labelpageCurrent, 1, Qt.AlignCenter)
        buttonPrev = QPushButton("上一页")
        buttonNext = QPushButton("下一页")
        bottomlayout.addWidget(buttonNext, 1, Qt.AlignRight)
        bottomlayout.addWidget(buttonPrev, 1, Qt.AlignRight)
        buttonPrev.clicked.connect(self.onpage_prev)
        buttonNext.clicked.connect(self.onpage_next)
        bodylayout.addLayout(bottomlayout)
        self.bodyGroupBox.setLayout(bodylayout)
        pass

    def startTimers(self):
        timer = QTimer(self)  # 初始化一个定时器
        timer.timeout.connect(self.onTimeUpdate)
        timer.start(1000)

    def getName(self, strCode):
        for item in self.station_list:
            if item.find(strCode) != -1:
                return item[:item.find(strCode) - 1]
        pass

    def getStationCode(self, strStation):
        for dic in self.load_dict:
            if dic['name'] == strStation:
                return dic['code']
        pass

    @pyqtSlot()
    def onpage_prev(self):
        if self.page_current == 1:
            QMessageBox.information(self, "12306查询器", "查询到头,无法进行操作")
        else:
            page_start = 0 if (self.page_current - 2) * 20 < 0 else (self.page_current - 2) * 20
            self.page_current -= 1
            page_end = int(page_start) + 20
            self.labelpageCurrent.setText("当前第%d页" % self.page_current)
            self.bodytablewdiget.clearContents()
            self.InsertTable(self.r_result[page_start:page_end])
        pass

    @pyqtSlot()
    def onpage_next(self):
        if self.page_current == self.page_num:
            QMessageBox.information(self, "12306查询器", "查询到底,无法进行操作")

        else:
            page_start = (self.page_current) * 20
            self.page_current += 1
            page_end = len(self.r_result) if (page_start + 20) > len(self.r_result) else (page_start + 20)
            self.labelpageCurrent.setText("当前第%d页" % self.page_current)
            self.bodytablewdiget.clearContents()
            self.InsertTable(self.r_result[page_start:page_end])
        pass

    @pyqtSlot()
    def onTimeUpdate(self):
        self.labeltime.setText(time.strftime("Date: %Y/%m/%d %H:%M:%S", time.localtime(time.time())))
        pass

    @pyqtSlot()
    def onSearch(self):
        strFrom = self.labelSrcEdit.text()
        if strFrom.find('(') != -1 and strFrom.rfind(')') != -1:
            strFrom = strFrom[strFrom.find('(') + 1:strFrom.rfind(')')]
        else:
            strFrom = self.getStationCode(strFrom.strip())
        strTo = self.labelDstEdit.text()
        if strTo.find('(') != -1 and strTo.find(')') != -1:
            strTo = strTo[strTo.find('(') + 1:strTo.rfind(')')]
        else:
            strTo = self.getStationCode(strTo.strip())
        strTime = self.labelCalender.text()
        if strFrom == None or strTo == None:
            QMessageBox.information(self, "12306查询器", "当前查询车站没有车站代码，无法进行本次查询")
            return
        start_time = time.time()
        result = getResult(strFrom, strTo, strTime)
        end_time = time.time()
        self.labelSearchCost.setText("搜索到结果,耗时:%d s" % int(end_time - start_time))
        if type(result) == bool and result == False:
            QMessageBox.information(self, "12306查询器", "12306查询异常")
        elif len(result) == 0:
            QMessageBox.information(self, "12306查询器", "当前查询的车站无效或者返回结果为空")
        else:
            self.r_result = result
            self.page_num = int(len(result) / 20) + 1
            self.page_current = 1
            self.labelpageState.setText("总共%d页, 共%d条记录" % (self.page_num, len(result)))
            self.labelpageCurrent.setText("当前第1页")
            # ["车次", "始发站", "终点站", "出发站", "到达站", "出发时间", "到达时间", "历时",
            # "商务座/特等座", "一等座", "二等座", "高级软卧", "软卧", "动卧","硬卧", "软座","硬座", "无座", "其他", "备注"]
            self.bodytablewdiget.clearContents()
            page_end = 20 if len(result) > 20 else len(result)
            self.InsertTable(result[:page_end])
        pass

    def InsertTable(self, result):
        for i, val in enumerate(result):  # 只显示第一页
            self.bodytablewdiget.setItem(i, 0, QTableWidgetItem(val['train_num']))
            self.bodytablewdiget.setItem(i, 1, QTableWidgetItem(self.getName(val['train_src'])))
            self.bodytablewdiget.setItem(i, 2, QTableWidgetItem(self.getName(val['train_dest'])))
            self.bodytablewdiget.setItem(i, 3, QTableWidgetItem(self.getName(val['train_travel_src'])))
            self.bodytablewdiget.setItem(i, 4, QTableWidgetItem(self.getName(val['train_travel_dest'])))
            self.bodytablewdiget.setItem(i, 5, QTableWidgetItem(val['train_start_time']))
            self.bodytablewdiget.setItem(i, 6, QTableWidgetItem(val['train_end_time']))
            self.bodytablewdiget.setItem(i, 7, QTableWidgetItem(val['train_spend_time']))
            self.bodytablewdiget.setItem(i, 8, QTableWidgetItem(val['train_td_seat']))
            self.bodytablewdiget.setItem(i, 9, QTableWidgetItem(val['train_first_seat']))
            self.bodytablewdiget.setItem(i, 10, QTableWidgetItem(val['train_second_seat']))
            self.bodytablewdiget.setItem(i, 11, QTableWidgetItem('-'))  # 高级软卧
            self.bodytablewdiget.setItem(i, 12, QTableWidgetItem(val['train_soft_sleep']))  # 软卧
            self.bodytablewdiget.setItem(i, 13, QTableWidgetItem(val['train_move_sleep']))
            self.bodytablewdiget.setItem(i, 14, QTableWidgetItem(val['train_hard_sleep']))
            self.bodytablewdiget.setItem(i, 15, QTableWidgetItem(val['train_soft_seat']))
            self.bodytablewdiget.setItem(i, 16, QTableWidgetItem(val['train_hard_seat']))
            self.bodytablewdiget.setItem(i, 17, QTableWidgetItem(val['train_null_seat']))
            self.bodytablewdiget.setItem(i, 18, QTableWidgetItem('-'))
            self.bodytablewdiget.setItem(i, 19, QTableWidgetItem('-'))
            for j in range(0, 20):
                self.bodytablewdiget.item(i, j).setTextAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
            pass
