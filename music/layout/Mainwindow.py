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
        mainlayout = QVBoxLayout()
        mainlayout.setSpacing(0)
        mainlayout.setAlignment(Qt.AlignTop)
        #2.添加自定义layout
        self._createTitleHeader()
        mainlayout.addWidget(self.titleHeader,Qt.AlignTop)
        self._createBodyWidget()
        mainlayout.addWidget(self.bodywidget,Qt.AlignCenter)
        self._createBottomWidget()
        mainlayout.addWidget(self.bottomwidget,Qt.AlignBottom)

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
        #当前播放音乐状态 False:未播放 True:播放
        self._bplaymusic = False
        #当前播放顺序 0:顺序播放 1:列表循环 2:单曲循环 3:随机播放
        self._playmusicWays=0
        #歌词打开
        self._bOpenlrc=False
        #播放列表打开
        self._bOpenPlayList=False
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
        self.bodywidget.setStyleSheet("border:1px solid red;background:rgb(25,27,31);")
        bodylayout=QGridLayout()
        bodylayout.setSpacing(0)
        #body left
        bodyleftWidget=QWidget()
        bodyleftWidget.setContentsMargins(0,0,0,0)
        bodyleftlayout=QVBoxLayout()
        bodyleftlayout.setAlignment(Qt.AlignTop)
        #推荐 我的音乐 我的歌单等等
        bodyLeftMidWidget=QWidget()
        bodyleftMidlayout=QVBoxLayout()
        bodyleftMidlayout.setAlignment(Qt.AlignTop)
        bodyLeftMidWidget.setStyleSheet("border: 1px solid black;")
        label_suggest=QLabel('推荐')
        label_suggest.setFixedHeight(20)
        label_suggest.setStyleSheet("color:rgb(124,124,124);font-family:'宋体';font-size:12px;padding-left:2px;")
        #发现音乐
        found_music_widget=QWidget()
        found_music_widget.setCursor(QCursor(Qt.PointingHandCursor))
        found_music_layout=QHBoxLayout()
        found_music_widget.setFixedHeight(35)
        label_found_music=QLabel()
        label_found_music.setFixedSize(20,20)
        pixmap=QPixmap("./res/yinle.png")
        pixmap.scaled(label_found_music.size(),Qt.KeepAspectRatioByExpanding)
        label_found_music.setPixmap(pixmap)
        label_found_music_1=QLabel('发现音乐')
        label_found_music_1.setFixedHeight(20)
        label_found_music_1.setStyleSheet("font-size:16px;font-family:'微软雅黑';color:rgb(220,221,227);font-weight:300")
        label_found_music.setScaledContents(True)
        found_music_layout.addWidget(label_found_music)
        found_music_layout.setSpacing(3)
        found_music_layout.addWidget(label_found_music_1)
        found_music_widget.setLayout(found_music_layout)

        #私人FM
        private_FM_widget = QWidget()
        private_FM_widget.setCursor(QCursor(Qt.PointingHandCursor))
        private_FM_widget.setFixedHeight(35)
        private_FM_layout=QHBoxLayout()
        label_private_FM = QLabel()
        label_private_FM.setFixedSize(20, 20)
        pixmap1 = QPixmap("./res/leida.png")
        pixmap1.scaled(label_private_FM.size(), Qt.KeepAspectRatioByExpanding)
        label_private_FM.setPixmap(pixmap1)
        label_private_FM_1 = QLabel('私人FM')
        label_private_FM_1.setFixedHeight(20)
        label_private_FM_1.setStyleSheet("font-size:16px;font-family:'微软雅黑';color:rgb(220,221,227);font-weight:300")
        label_private_FM.setScaledContents(True)
        private_FM_layout.addWidget(label_private_FM)
        private_FM_layout.addWidget(label_private_FM_1)
        private_FM_widget.setLayout(private_FM_layout)
        #MV
        mv_widget = QWidget()
        mv_widget.setCursor(QCursor(Qt.PointingHandCursor))
        mv_widget.setFixedHeight(35)
        mv_layout = QHBoxLayout()
        label_mv = QLabel()
        label_mv.setFixedSize(20, 20)
        pixmap2 = QPixmap("./res/shipin.png")
        pixmap2.scaled(label_mv.size(), Qt.KeepAspectRatioByExpanding)
        label_mv.setPixmap(pixmap2)
        label_mv_1 = QLabel('MV')
        label_mv_1.setFixedHeight(20)
        label_mv_1.setStyleSheet("font-size:16px;font-family:'微软雅黑';color:rgb(220,221,227);font-weight:300")
        label_mv.setScaledContents(True)
        mv_layout.addWidget(label_mv)
        mv_layout.addWidget(label_mv_1)
        mv_widget.setLayout(mv_layout)
        #pengyou
        friend_widget = QWidget()
        friend_widget.setCursor(QCursor(Qt.PointingHandCursor))
        friend_widget.setFixedHeight(35)
        friend_layout = QHBoxLayout()
        label_friend = QLabel()
        label_friend.setFixedSize(20, 20)
        pixmap3 = QPixmap("./res/pengyou.png")
        pixmap3.scaled(label_friend.size(), Qt.KeepAspectRatioByExpanding)
        label_friend.setPixmap(pixmap3)
        label_friend_1 = QLabel('朋友')
        label_friend_1.setFixedHeight(20)
        label_friend_1.setStyleSheet("font-size:16px;font-family:'微软雅黑';color:rgb(220,221,227);font-weight:300")
        label_friend.setScaledContents(True)
        friend_layout.addWidget(label_friend)
        friend_layout.addWidget(label_friend_1)
        friend_widget.setLayout(friend_layout)
        #我的音乐
        label_mymusic = QLabel('我的音乐')
        label_mymusic.setFixedHeight(20)
        label_mymusic.setStyleSheet("color:rgb(124,124,124);font-family:'宋体';font-size:12px;padding-left:2px;")
        #本地音乐
        localmusic_widget = QWidget()
        localmusic_widget.setCursor(QCursor(Qt.PointingHandCursor))
        localmusic_widget.setFixedHeight(35)
        localmusic_layout = QHBoxLayout()
        label_localmusic = QLabel()
        label_localmusic.setFixedSize(20, 20)
        pixmap4 = QPixmap("./res/bendiyinle.png")
        pixmap4.scaled(label_localmusic.size(), Qt.KeepAspectRatioByExpanding)
        label_localmusic.setPixmap(pixmap4)
        label_localmusic_1 = QLabel('本地音乐')
        label_localmusic_1.setFixedHeight(20)
        label_localmusic_1.setStyleSheet("font-size:16px;font-family:'微软雅黑';color:rgb(220,221,227);font-weight:300")
        label_localmusic.setScaledContents(True)
        localmusic_layout.addWidget(label_localmusic)
        localmusic_layout.addWidget(label_localmusic_1)
        localmusic_widget.setLayout(localmusic_layout)
        #下载管理
        downloadMgr_widget = QWidget()
        downloadMgr_widget.setCursor(QCursor(Qt.PointingHandCursor))
        downloadMgr_widget.setFixedHeight(35)
        downloadMgr_layout = QHBoxLayout()
        label_downloadMgr = QLabel()
        label_downloadMgr.setFixedSize(20, 20)
        pixmap5 = QPixmap("./res/xiazaiguanli.png")
        pixmap5.scaled(label_downloadMgr.size(), Qt.KeepAspectRatioByExpanding)
        label_downloadMgr.setPixmap(pixmap5)
        label_downloadMgr_1 = QLabel('下载管理')
        label_downloadMgr_1.setFixedHeight(20)
        label_downloadMgr_1.setStyleSheet("font-size:16px;font-family:'微软雅黑';color:rgb(220,221,227);font-weight:300")
        label_downloadMgr.setScaledContents(True)
        downloadMgr_layout.addWidget(label_downloadMgr)
        downloadMgr_layout.addWidget(label_downloadMgr_1)
        downloadMgr_widget.setLayout(downloadMgr_layout)
        #云盘
        yunpan_widget = QWidget()
        yunpan_widget.setCursor(QCursor(Qt.PointingHandCursor))
        yunpan_widget.setFixedHeight(35)
        yunpan_layout = QHBoxLayout()
        label_yunpan = QLabel()
        label_yunpan.setFixedSize(20, 20)
        pixmap6 = QPixmap("./res/yunpan.png")
        pixmap6.scaled(label_yunpan.size(), Qt.KeepAspectRatioByExpanding)
        label_yunpan.setPixmap(pixmap6)
        label_yunpan_1 = QLabel('我的音乐云盘')
        label_yunpan_1.setFixedHeight(20)
        label_yunpan_1.setStyleSheet("font-size:16px;font-family:'微软雅黑';color:rgb(220,221,227);font-weight:300")
        label_yunpan.setScaledContents(True)
        yunpan_layout.addWidget(label_yunpan)
        yunpan_layout.addWidget(label_yunpan_1)
        yunpan_widget.setLayout(yunpan_layout)
        #我的收藏
        soucang_widget = QWidget()
        soucang_widget.setCursor(QCursor(Qt.PointingHandCursor))
        soucang_widget.setFixedHeight(35)
        soucang_layout = QHBoxLayout()
        label_soucang = QLabel()
        label_soucang.setFixedSize(20, 20)
        pixmap7 = QPixmap("./res/wodeshoucang.png")
        pixmap7.scaled(label_soucang.size(), Qt.KeepAspectRatioByExpanding)
        label_soucang.setPixmap(pixmap7)
        label_soucang_1 = QLabel('我的收藏')
        label_soucang_1.setFixedHeight(20)
        label_soucang_1.setStyleSheet("font-size:16px;font-family:'微软雅黑';color:rgb(220,221,227);font-weight:300")
        label_soucang.setScaledContents(True)
        soucang_layout.addWidget(label_soucang)
        soucang_layout.addWidget(label_soucang_1)
        soucang_widget.setLayout(soucang_layout)
        # 创建的歌单
        create_music_list_widget = QWidget()
        create_music_list_layout = QHBoxLayout()
        #创建的歌单
        label_create_music_list=QLabel("创建的歌单")
        label_create_music_list.setFixedHeight(20)
        label_create_music_list.setStyleSheet("color:rgb(124,124,124);font-family:'宋体';font-size:12px;padding-left:2px;")
        btn_create_music_list = QPushButton()
        btn_create_music_list.setIcon(QIcon("./res/yuanxingjiahao.png"))
        btn_create_music_list.setFixedSize(20,20)
        btn_create_music_list.setStyleSheet("font-size:14px;font-weight:bold;color:")
        btn_expand_music_list = QPushButton()
        btn_expand_music_list.setIcon(QIcon("./res/right.png"))
        btn_expand_music_list.setFixedSize(20, 20)
        btn_expand_music_list.setStyleSheet("font-size:14px;font-weight:bold;")
        create_music_list_layout.addWidget(label_create_music_list)
        create_music_list_layout.addWidget(btn_create_music_list)
        create_music_list_layout.addWidget(btn_expand_music_list)
        create_music_list_widget.setLayout(create_music_list_layout)

        #收藏的歌单
        soucang_music_list_widget=QWidget()
        soucang_music_list_layout = QHBoxLayout()
        label_soucang_music_list = QLabel("收藏的歌单")
        label_soucang_music_list.setFixedHeight(20)
        label_soucang_music_list.setStyleSheet("color:rgb(124,124,124);font-family:'宋体';font-size:12px;padding-left:2px;")

        btn_soucang_music_list = QPushButton()
        btn_soucang_music_list.setIcon(QIcon("./res/right.png"))
        btn_soucang_music_list.setFixedSize(20, 20)
        btn_soucang_music_list.setStyleSheet("font-size:14px;font-weight:bolder;")
        soucang_music_list_layout.addWidget(label_soucang_music_list)
        soucang_music_list_layout.addWidget(btn_soucang_music_list)

        soucang_music_list_widget.setLayout(soucang_music_list_layout)

        bodyleftMidlayout.setStretch(0,0)
        bodyleftMidlayout.addWidget(label_suggest )
        bodyleftMidlayout.addWidget(found_music_widget )
        bodyleftMidlayout.addWidget(private_FM_widget )
        bodyleftMidlayout.addWidget(mv_widget )
        bodyleftMidlayout.addWidget(friend_widget )
        bodyleftMidlayout.addWidget(label_mymusic )
        bodyleftMidlayout.addWidget(localmusic_widget )
        bodyleftMidlayout.addWidget(downloadMgr_widget )
        bodyleftMidlayout.addWidget(yunpan_widget )
        bodyleftMidlayout.addWidget(soucang_widget )
        bodyleftMidlayout.addWidget(create_music_list_widget)
        bodyleftMidlayout.addWidget(soucang_music_list_widget)
        bodyLeftMidWidget.setLayout(bodyleftMidlayout)

        #播放信息窗口
        bodyleftBottomWidget=QWidget()
        bodyleftBottomWidget.setFixedHeight(54)
        bodyleftBottomWidget.setStyleSheet("border:1px solid green;padding:0;")
        bodyleftBottomlayout = QHBoxLayout()
        bodyleftBottomlayout.setAlignment(Qt.AlignLeft)
        # music logo
        label_music_info =QPushButton("music logo")
        label_music_info.setCursor(QCursor(Qt.PointingHandCursor))
        label_music_info.setFixedSize(40,40)

        music_info_widget = QWidget()
        music_info_widget.setFixedWidth(140)
        music_info_layout = QVBoxLayout()
        music_info_layout.setSpacing(0)
        music_info_layout.setContentsMargins(0,0,0,0)
        # label_music_name
        label_music_info_name = QLabel("music_name")
        label_music_info_name.setCursor(QCursor(Qt.PointingHandCursor))
        label_music_info_name.setStyleSheet("color:white;font-size:16px")
        label_music_info_author = QLabel("music_author")
        label_music_info_author.setCursor(QCursor(Qt.PointingHandCursor))
        label_music_info_name.setStyleSheet("color:#fffeee;font-size:16px")
        music_info_layout.addWidget(label_music_info_name)
        music_info_layout.addWidget(label_music_info_author)
        music_info_widget.setLayout(music_info_layout)

        # music_tools
        music_tools_widget = QWidget()
        music_tools_layout = QVBoxLayout()
        music_tools_layout.setSpacing(1)
        music_tools_layout.setContentsMargins(0,0,0,0)
        label_music_tools_love = QLabel()
        label_music_tools_love.setFixedSize(16,16)
        pixmap_tools_love = QPixmap("./res/xinaixin.png")
        pixmap_tools_love.scaled(label_music_tools_love.size(), Qt.KeepAspectRatioByExpanding)
        label_music_tools_love.setScaledContents(True)
        label_music_tools_love.setPixmap(pixmap_tools_love)
        label_music_tools_love.setCursor(QCursor(Qt.PointingHandCursor))
        label_music_tools_share = QLabel()
        label_music_tools_share.setFixedSize(16,16)
        pixmap_tools_share = QPixmap("./res/fenxiang.png")
        pixmap_tools_share.scaled(label_music_tools_share.size(), Qt.KeepAspectRatioByExpanding)
        label_music_tools_share.setScaledContents(True)
        label_music_tools_share.setPixmap(pixmap_tools_share)
        label_music_tools_share.setCursor(QCursor(Qt.PointingHandCursor))

        music_tools_layout.addWidget(label_music_tools_love)
        music_tools_layout.addWidget(label_music_tools_share)
        music_tools_widget.setLayout(music_tools_layout)

        bodyleftBottomlayout.addWidget(label_music_info)
        bodyleftBottomlayout.addWidget(music_info_widget)
        bodyleftBottomlayout.addWidget(music_tools_widget)
        bodyleftBottomWidget.setLayout(bodyleftBottomlayout)

        bodyleftlayout.addWidget(bodyLeftMidWidget,Qt.AlignCenter)
        bodyleftlayout.addWidget(bodyleftBottomWidget,Qt.AlignBottom)
        bodyleftWidget.setLayout(bodyleftlayout)

        #body right
        bodyrightWidget=QWidget(self.bodywidget)
        bodyrightlayout=QVBoxLayout()
        bodyrightlayout.addWidget(QPushButton('he'))
        bodyrightWidget.setLayout(bodyrightlayout)

        bodylayout.addWidget(bodyleftWidget,0,0,1,1)
        bodylayout.addWidget(bodyrightWidget,0,1,1,4)
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
        button_prev.setStyleSheet("border-radius:12px;color:white")
        button_prev.setCursor(QCursor(Qt.PointingHandCursor))
        button_prev.setToolTip('播放上一曲')
        #播放
        button_play=QPushButton()
        button_play.setFixedSize(50,35)
        button_play.setIcon(QIcon('./res/play.png'))
        button_play.setIconSize(QSize(35,35))
        button_play.setStyleSheet("border-radius:12px;color:white")
        button_play.setCursor(QCursor(Qt.PointingHandCursor))
        button_play.setToolTip('播放')
        #播放下一首
        button_next=QPushButton()
        button_next.setFixedSize(50,30)
        button_next.setIcon(QIcon('./res/next.png'))
        button_next.setIconSize(QSize(30,30))
        button_next.setStyleSheet("border-radius:12px;color:white")
        button_next.setCursor(QCursor(Qt.PointingHandCursor))
        button_next.setToolTip('播放下一曲')
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
        button_voice.setStyleSheet("border-radius:12px;color:white")
        button_voice.setToolTip('音量')
        #voice progressbar
        voice_progress=QProgressBar()
        voice_progress.setFixedHeight(5)
        voice_progress.setRange(0, 100)
        voice_progress.setValue(10)
        voice_progress.setTextVisible(False)
        #play ways
        button_play_ways=QPushButton()
        button_play_ways.setFixedSize(50,20)
        button_play_ways.setIcon(QIcon('./res/shunxubofang.png'))
        button_play_ways.setToolTip('顺序播放')
        button_play_ways.setIconSize(QSize(30,20))
        button_play_ways.setStyleSheet("border-radius:12px;color:white;")
        button_play_ways.setCursor(QCursor(Qt.PointingHandCursor))
        #play geci
        button_gc=QPushButton()
        button_gc.setFixedSize(30,30)
        button_gc.setIcon(QIcon('./res/lrc.png'))
        button_gc.setIconSize(QSize(30,20))
        button_gc.setStyleSheet("border-radius:12px;color:white;")
        button_gc.setCursor(QCursor(Qt.PointingHandCursor))
        button_gc.setToolTip('打开歌词')
        #play list
        button_play_list=QPushButton()
        button_play_list.setFixedSize(50,20)
        button_play_list.setIcon(QIcon('./res/bofangjilu.png'))
        button_play_list.setIconSize(QSize(30,20))
        button_play_list.setStyleSheet("border-radius:12px;color:white;")
        button_play_list.setCursor(QCursor(Qt.PointingHandCursor))
        button_play_list.setToolTip('打开播放列表')
        #label
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
        #播放按钮点击事件
        button_play.clicked.connect(lambda :self.onPlay(button_play,self._bplaymusic))
        #播放音乐事件 按秒定时更新播放进度
        self._playTimer.timeout.connect(lambda :self.onUpdateTime(button_play,label_start,play_progress))
        #播放方式按钮点击事件
        button_play_ways.clicked.connect(lambda :self.onPlayMusicWays(button_play_ways))
        #歌词打开
        button_gc.clicked.connect(lambda :self.onOpenlrc(button_gc))
        #播放列表打开
        button_play_list.clicked.connect(lambda:self.onOpenPlaylist(button_play_list))
        self.bottomwidget.setLayout(bottomlayout)

    @pyqtSlot()
    def onPlay(self,obj,bplay):
        if bplay== False:
            obj.setIcon(QIcon('./res/pasue.png'))
            obj.setToolTip('暂停')
            self._startPlayMusic()
            self._playTimer.start(50)
        else:
            obj.setIcon(QIcon('./res/play.png'))
            obj.setToolTip('播放')
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
    @pyqtSlot()
    def onOpenlrc(self,button_gc):
        self._bOpenlrc=not self._bOpenlrc
        if self._bOpenlrc == True:
            button_gc.setToolTip('关闭歌词')
            #打开歌词 dialog
        else:
            button_gc.setToolTip('打开歌词')
            #关闭歌词
    @pyqtSlot()
    def onOpenPlaylist(self,button_play_list):
        self._bOpenPlayList=not self._bOpenPlayList
        if self._bOpenPlayList==True:
            button_play_list.setToolTip('关闭播放列表')
            #打开播放列表
        else:
            button_play_list.setToolTip('打开播放列表')
            #关闭播放列表
        pass

    @pyqtSlot()
    def onPlayMusicWays(self,button_play_ways):
        self._playmusicWays=(self._playmusicWays+1)%4
        IconDir=['./res/shunxubofang.png','./res/liebiaoxunhuan.png','./res/danquxunhuan.png','./res/suijibofang.png']
        IconText=['顺序播放','列表循环','单曲循环','随机播放']
        button_play_ways.setIcon(QIcon(IconDir[self._playmusicWays]))
        button_play_ways.setToolTip(IconText[self._playmusicWays])


        pass
if __name__ == '__main__':
    m = Mainwindow()
