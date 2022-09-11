from configparser import ConfigParser
import requests
import json
import os
import logging

Request_Headers = {
    "User-Agent": "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.1 (XHTML, like Gecko) Chrome/22.0.1207.1 "
                  "Safari/537.1 ",
}
logging.basicConfig(
    # 日志级别
    level=logging.DEBUG,
    # 日志格式
    # 时间、代码所在文件名、代码行号、日志级别名字、日志信息
    format='T:%(thread)s %(asctime)s %(filename)s %(levelname)s %(message)s',
    # 打印日志的时间
    datefmt='[%Y-%m-%d %H:%M:%S]',
    # 日志文件存放的目录（目录必须存在）及日志文件名
    # filename = 'd:/report.log',
    # 打开日志文件的方式
    # filemode = 'w'
)
logger = logging.getLogger("main")

class AppConfig:
    def __init__(self, config_file):
        self.config = ConfigParser()
        self.config.read(config_file)
        self.stations = []
        self._init_basic_setting()

    def _init_basic_setting(self):
        self.init_station(self.config.get('server', 'StationCodeFilePath'), self.config.get('server', 'StationCodeUrl'))

        pass

    def get_station_name(self):
        result = []
        for item in self.stations:
            result.append(item.get('name') + '(' + item.get('code') + ')')
        return result

    def init_station(self, file_path, download_link):
        if os.path.exists(file_path):
            logger.info(f"begin to load stations from {file_path}")
            with open(file_path, 'r') as f:
                for line in f.readlines():
                    line = line.strip()
                    if not line:
                        continue
                    station = json.loads(line)
                    self.stations.append(station)
            logger.debug(f"successful insert {len(self.stations)} stations")
        else:
            logger.debug("begin to download station from link: ", download_link)
            try:
                result = requests.get(download_link, headers=Request_Headers)
                if result.status_code == 200:
                    data = result.text
                    i = 0
                    fromIndex = 0
                    self.stations = []
                    while True:
                        station = {}
                        toIndex = data.find(str(i), fromIndex)
                        if toIndex != -1:
                            per_station = data[fromIndex:toIndex]
                            temp = per_station[per_station.find('@') + 1:].split('|')
                            station["py"] = temp[0]
                            station["name"] = temp[1]
                            station["code"] = temp[2]
                            station["fullpy"] = temp[3]
                            self.stations.append(station)
                            fromIndex = toIndex
                            i += 1
                        else:
                            break
                    logger.info(f"successful insert {i} stations")
                    with open(file_path, 'w', encoding='utf8') as f:
                        for station in self.stations:
                            f.write(json.dumps(station) + "\n")
                else:
                    logger.error(f"request {download_link} error with status code:{result.status_code}")

            except Exception as e:
                logger.exception(f"request {download_link} error with exception message:{e}")
                pass


config = AppConfig("resource/text.ini")
