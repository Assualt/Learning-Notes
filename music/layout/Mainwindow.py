#!/usr/bin/python
#! -*- encoding:utf-8 -*-

import sys
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.Qt import *

import json
sys.path.append('./tools')
import ConfMgr as conf

class Mainwindow(QWidget):

    def __init__(self,parent=None):
        super(Mainwindow,self).__init__()
        self.initData()#1.初始化数据
        self.initUI()#2.初始化界面
        self.initSlots()#3.初始化信号槽函数
        pass
    def initUI(self):
        print("Mainwindow initUI")
        #1.设置主窗口大小以及
        minHeight=self._kvMap['global'].get('minHeight')
        minWeight=self._kvMap['global'].get('minWeight')
        print(minHeight,minWeight)
        self.setMinimumSize(minWeight,minHeight)
        mainlayout = QVBoxLayout(self)
        mainlayout.setContentsMargins(0,0,0,0)
        #2.添加自定义layout
        self._createTitleHeader()
        mainlayout.addWidget(self.titleHeader,Qt.AlignTop)
        self._createBodyWidget()
        mainlayout.addWidget(self.bodywidget)
        self._createBottomWidget()
        mainlayout.addWidget(self.bottomwidget)

        self.setLayout(mainlayout)
        #3.自定义标题栏
        # self.setWindowFlags(Qt.CustomizeWindowHint)
        #4.定义标题
        self.setWindowTitle('自制版网易云音乐')
        self.setWindowIcon(QIcon('./res/app.ico'))

        pass
    def initSlots(self):
        print("Mainwindow initUI")

        pass

    def initData(self):
        print("Mainwindow initData")
        cf=conf.ConfMgr('./conf/global.cf')
        data={
            'minHeight':cf.getIntDft(0,'global','minHeight'),
            'minWeight':cf.getIntDft(0,'global','minWeight')
        }
        self._kvMap={}
        self._kvMap['global']=data
        self._playTimer=QTimer(self)
        self._playMusicName=''
        self._playMusicEndTime=180

        pass

    def _createTitleHeader(self):
        self.titleHeader=QWidget()
        self.titleHeader.setFixedHeight(49)
        self.titleHeader.setStyleSheet('background:rgb(34,34,37);')
        self.titleHeader.setContentsMargins(0,0,0,0)

        titleHeaderlayout=QGridLayout()
        label_logo = QLabel()
        label_logo.setFixedSize(30,30)
        logo_pixmap = QPixmap('./res/app.ico')
        logo_pixmap.scaled(label_logo.size(),Qt.KeepAspectRatioByExpanding)
        label_logo.setScaledContents(True)
        label_logo.setPixmap(logo_pixmap)
        #logo
        label_title=QLabel('网易云音乐',self)
        label_title.setStyleSheet("font:20px;font-weight:600;font-family:'微软雅黑';color:white;min-width")
        label_title.setCursor(QCursor(Qt.PointingHandCursor))
        # 左右翻页
        button_histroy_back = QPushButton("<")
        # button_histroy_back.setIcon(QIcon('./res/back.png'))
        button_histroy_right = QPushButton(">")
        # button_histroy_right.setIcon(QIcon('./res/right.png'))
        button_histroy_back.setFixedSize(30,20)
        button_histroy_right.setFixedSize(30,20)
        button_histroy_back.setStyleSheet("border-radius:5px;font-size:14px;padding:0;border:1px solid rgb(23,23,26);font-weight:600;color:white;")
        button_histroy_right.setStyleSheet("border-radius:5px;font-size:14px;padding:0;border:1px solid rgb(23,23,26);font-weight:600;color:white;")
        button_histroy_back.setCursor(QCursor(Qt.PointingHandCursor))
        button_histroy_right.setCursor(QCursor(Qt.PointingHandCursor))
        # 搜索框
        lineEdit=QLineEdit()
        lineEdit.setFixedSize(219,24)
        lineEdit.setPlaceholderText("搜索音乐,歌手,歌词,用户")
        lineEdit.setStyleSheet('border:1px solid rgb(32,32,35);border-radius:10px;padding:2px 6px;font-size:14px;color:white;background:rgb(23,23,25);font-weight:400')
        lineEditAction=QAction(self)
        lineEditAction.setIcon(QIcon('./res/search.png'))
        lineEdit.addAction(lineEditAction,QLineEdit.TrailingPosition)

        #person Infomation
        button_info_img=QPushButton()
        button_info_img.setIcon(QIcon('./res/logo.png'))
        button_info_img.setFixedSize(30,30)
        # button_info_img.setIconSize(button_info_img.size())
        button_info_img.setFlat(True)
        button_info_img.setStyleSheet('border-radius:12px;')
        button_info_img.setCursor(QCursor(Qt.PointingHandCursor))

        #info person
        label_info_name=QLabel("Sunshine_Morning")
        label_info_name.setMaximumWidth(100)
        label_info_name.setStyleSheet("font-family:'微软雅黑';color:white;font-size:14px;")
        label_info_name.setCursor(QCursor(Qt.PointingHandCursor))

        button_info_show=QPushButton()
        button_info_show.setIcon(QIcon('./res/tabxiala.png'))
        button_info_show.setFixedSize(30,30)
        button_info_show.setFlat(True)
        button_info_show.setCursor(QCursor(Qt.PointingHandCursor))
        button_info_show.setStyleSheet("border-radius:12px;")
        button_info_show.setCursor(QCursor(Qt.PointingHandCursor))

        # 皮肤
        button_change_pifu =QPushButton()
        button_change_pifu.setIcon(QIcon('./res/pifu.png'))
        button_change_pifu.setFixedSize(25,25)
        button_change_pifu.setFlat(True)
        button_change_pifu.setStyleSheet("border-radius:12px;")
        button_change_pifu.setCursor(QCursor(Qt.PointingHandCursor))

        #设置
        button_setting=QPushButton()
        button_setting.setIcon(QIcon('./res/shezhi.png'))
        button_setting.setFixedSize(25,25)
        button_setting.setFlat(True)
        button_setting.setStyleSheet("border-radius:12px;")
        button_setting.setCursor(QCursor(Qt.PointingHandCursor))

        # 邮箱
        button_mailbox = QPushButton()
        button_mailbox.setIcon(QIcon('./res/youxiang.png'))
        button_mailbox.setFixedSize(25, 25)
        button_mailbox.setFlat(True)
        button_mailbox.setStyleSheet("border-radius:12px;")
        button_mailbox.setCursor(QCursor(Qt.PointingHandCursor))

        # mini
        button_mini = QPushButton()
        button_mini.setIcon(QIcon('./res/mini.png'))
        button_mini.setFixedSize(25, 25)
        button_mini.setIconSize(button_mini.size())
        button_mini.setFlat(True)
        button_mini.setStyleSheet("border-radius:12px;")
        button_mini.setCursor(QCursor(Qt.PointingHandCursor))

        # min
        button_min = QPushButton()
        button_min.setIcon(QIcon('./res/min.png'))
        button_min.setFixedSize(25, 25)
        button_min.setIconSize(button_min.size())
        button_min.setFlat(True)
        button_min.setStyleSheet("border-radius:12px;")
        button_min.setCursor(QCursor(Qt.PointingHandCursor))

        #max
        button_max = QPushButton()
        button_max.setIcon(QIcon('./res/max.png'))
        button_max.setFixedSize(25, 25)
        button_max.setIconSize(button_max.size())
        button_max.setFlat(True)
        button_max.setStyleSheet("border-radius:12px;")
        button_max.setCursor(QCursor(Qt.PointingHandCursor))

        #close
        button_close = QPushButton()
        button_close.setIcon(QIcon('./res/close.png'))
        button_close.setFixedSize(25, 25)
        button_close.setIconSize(button_close.size())
        button_close.setFlat(True)
        button_close.setStyleSheet("border-radius:12px;")
        button_close.setCursor(QCursor(Qt.PointingHandCursor))

        #Label
        label_test=QLabel("|")
        label_test.setFixedSize(10,30)
        label_test.setStyleSheet("font-size:14px;font-weight:600;")
        titleHeaderlayout.addWidget(label_logo,0,0,1,1)
        titleHeaderlayout.addWidget(label_title,0,1,1,1)
        titleHeaderlayout.addWidget(button_histroy_back,0,2,1,1)
        titleHeaderlayout.addWidget(button_histroy_right,0,3,1,1)
        titleHeaderlayout.addWidget(lineEdit,0,4,1,2)
        titleHeaderlayout.addWidget(button_info_img,0,6,1,1)
        titleHeaderlayout.addWidget(label_info_name,0,7,1,1)
        titleHeaderlayout.addWidget(button_info_show,0,8,1,1)
        titleHeaderlayout.addWidget(button_change_pifu,0,10,1,1)
        titleHeaderlayout.addWidget(button_setting,0,11,1,1)
        titleHeaderlayout.addWidget(button_mailbox,0,12,1,1)

        titleHeaderlayout.addWidget(label_test,0,13,1,1)
        titleHeaderlayout.addWidget(button_mini,0,14,1,1)
        titleHeaderlayout.addWidget(button_min,0,15,1,1)
        titleHeaderlayout.addWidget(button_max,0,16,1,1)
        titleHeaderlayout.addWidget(button_close,0,17,1,1)

        #init slots
        #button_info_show.clicked.connect()

        self.titleHeader.setLayout(titleHeaderlayout)
        pass

    def _createBodyWidget(self):
        self.bodywidget=QWidget()
        self.bodywidget.setContentsMargins(0,0,0,0)
        self.bodywidget.setStyleSheet("border:1px solid red;")
        bodylayout=QVBoxLayout()
        button=QPushButton("helloworld")

        bodylayout.addWidget(button)
        self.bodywidget.setLayout(bodylayout)

        pass

    def _createBottomWidget(self):
        self.bottomwidget=QWidget()
        self.bottomwidget.setStyleSheet("background:rgb(35,35,38);")
        self.bottomwidget.setFixedHeight(51)
        bottomlayout=QGridLayout()
        #播放上一首
        button_prev=QPushButton()
        button_prev.setFixedSize(50,30)
        button_prev.setIcon(QIcon('./res/prev.png'))
        button_prev.setIconSize(QSize(30,30))
        button_prev.setStyleSheet("border-radius:12px;")
        button_prev.setCursor(QCursor(Qt.PointingHandCursor))
        #播放
        self._bplaymusic=False
        button_play=QPushButton()
        button_play.setFixedSize(50,35)
        button_play.setIcon(QIcon('./res/play.png'))
        button_play.setIconSize(QSize(35,35))
        button_play.setStyleSheet("border-radius:12px;")
        button_play.setCursor(QCursor(Qt.PointingHandCursor))
        #播放下一首
        button_next=QPushButton()
        button_next.setFixedSize(50,30)
        button_next.setIcon(QIcon('./res/next.png'))
        button_next.setIconSize(QSize(30,30))
        button_next.setStyleSheet("border-radius:12px;")
        button_next.setCursor(QCursor(Qt.PointingHandCursor))
        #label start
        label_start=QLabel("00:00")
        label_start.setAlignment(Qt.AlignCenter)
        label_start.setFixedSize(40,20)
        label_start.setStyleSheet("color:white;font-size:14px;font-family:'微软雅黑';")
        #play progress bar
        play_progress=QProgressBar()
        play_progress.setFixedHeight(5)
        play_progress.setRange(0,100)
        play_progress.setValue(10)
        play_progress.setTextVisible(False)
        #label end
        label_end=QLabel("00:00")
        label_end.setAlignment(Qt.AlignCenter)
        label_end.setFixedSize(40, 20)
        label_end.setStyleSheet("color:white;font-size:12px;font-family:'微软雅黑'")
        #voice
        button_voice=QPushButton()
        button_voice.setIcon(QIcon('./res/voice.png'))
        button_voice.setFixedSize(40,20)
        button_voice.setIconSize(button_voice.size())
        button_voice.setCursor(QCursor(Qt.PointingHandCursor))
        button_voice.setStyleSheet("border-radius:12px;")
        #voice progressbar
        voice_progress=QProgressBar()
        voice_progress.setFixedHeight(5)
        voice_progress.setRange(0, 100)
        voice_progress.setValue(0)

        #play ways
        button_play_ways=QPushButton()
        button_play_ways.setFixedSize(50,20)
        button_play_ways.setIcon(QIcon('./res/shunxubofang.png'))
        button_play_ways.setIconSize(QSize(30,20))
        button_play_ways.setStyleSheet("border-radius:12px;")
        #play geci
        button_gc=QPushButton()
        button_gc.setFixedSize(30,30)
        button_gc.setIcon(QIcon('./res/lrc.png'))
        button_gc.setIconSize(QSize(30,20))
        button_gc.setStyleSheet("border-radius:12px;")
        #play list
        button_play_list=QPushButton()
        button_play_list.setFixedSize(50,20)
        button_play_list.setIcon(QIcon('./res/bofangjilu.png'))
        button_play_list.setIconSize(QSize(30,20))
        button_play_list.setStyleSheet("border-radius:12px;")
        #label
        label_test=QLabel("h")

        bottomlayout.addWidget(button_prev,0,1,1,1)
        bottomlayout.addWidget(button_play,0,2,1,1)
        bottomlayout.addWidget(button_next,0,3,1,1)
        bottomlayout.addWidget(label_start,0,4,1,1)
        bottomlayout.addWidget(play_progress,0,5,1,4)
        bottomlayout.addWidget(label_end,0,9,1,1)
        bottomlayout.addWidget(button_voice,0,10,1,1)
        bottomlayout.addWidget(voice_progress,0,11,1,2)

        bottomlayout.addWidget(button_play_ways,0,13,1,1)
        bottomlayout.addWidget(button_gc,0,14,1,1)
        bottomlayout.addWidget(button_play_list,0,15,1,1)

        button_play.clicked.connect(lambda :self.onPlay(button_play,self._bplaymusic))
        self._playTimer.timeout.connect(lambda :self.onUpdateTime(button_play,label_start,play_progress))
        self.bottomwidget.setLayout(bottomlayout)

    @pyqtSlot()
    def onPlay(self,obj,bplay):
        if bplay== False:
            obj.setIcon(QIcon('./res/pasue.png'))
            self._startPlayMusic()
            self._playTimer.start(50)
        else:
            obj.setIcon(QIcon('./res/play.png'))
            self._playTimer.stop()
        self._bplaymusic= not bplay
        pass
    @pyqtSlot()
    def onUpdateTime(self,button_play,label_start,play_progress):
        temp=label_start.text().split(':')
        startTime=0
        for index,t in enumerate(temp):
            startTime+= int(t) * pow(60,len(temp)-index-1)
        print("starTime:",startTime,temp)
        if startTime >= self._playMusicEndTime:
            self._playTimer.stop()
            QMessageBox.information(self, "163音乐提醒", "音乐播放完毕")
            button_play.setIcon(QIcon('./res/play.png'))
            play_progress.setValue(0)
            label_start.setText('00:00')
            print("歌曲已经播放完毕")
        else:
            startTime+=1
            starttemp="%02d:%02d"%(startTime//60,startTime%60)
            label_start.setText(starttemp)
            play_progress.setValue(int(100* startTime/self._playMusicEndTime))
        pass
    def _startPlayMusic(self):
        print("_startPlayMusic")

        pass


if __name__ == '__main__':
    m = Mainwindow()