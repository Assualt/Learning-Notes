import time
import sys
import requests
import math,random,re,json
import urllib3
agents = ["Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.1 (KHTML, like Gecko) Chrome/22.0.1207.1 Safari/537.1",
"Mozilla/5.0 (X11; CrOS i686 2268.111.0) AppleWebKit/536.11 (KHTML, like Gecko) Chrome/20.0.1132.57 Safari/536.11",
"Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.6 (KHTML, like Gecko) Chrome/20.0.1092.0 Safari/536.6",
"Mozilla/5.0 (Windows NT 6.2) AppleWebKit/536.6 (KHTML, like Gecko) Chrome/20.0.1090.0 Safari/536.6",
"Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.1 (KHTML, like Gecko) Chrome/19.77.34.5 Safari/537.1",
"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.9 Safari/536.5",
"Mozilla/5.0 (Windows NT 6.0) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.36 Safari/536.5",
"Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1063.0 Safari/536.3",
"Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1063.0 Safari/536.3",
"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_0) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1063.0 Safari/536.3",
"Mozilla/5.0 (Windows NT 6.2) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1062.0 Safari/536.3",
"Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1062.0 Safari/536.3",
"Mozilla/5.0 (Windows NT 6.2) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1061.1 Safari/536.3",
"Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1061.1 Safari/536.3",
"Mozilla/5.0 (Windows NT 6.1) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1061.1 Safari/536.3",
"Mozilla/5.0 (Windows NT 6.2) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1061.0 Safari/536.3",
"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/535.24 (KHTML, like Gecko) Chrome/19.0.1055.1 Safari/535.24",
"Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/535.24 (KHTML, like Gecko) Chrome/19.0.1055.1 Safari/535.24",
]
def getRandomAgent():
    a = random.randint(0,len(agents)-1)
    return agents[a]

def getResult(strFrom,strDest,strDate):
    urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
    head={}
    head['User-Agent']=getRandomAgent()
    if strFrom == "" or strDest == "":
        print("输入的起点或者终点为空，查询失败")
        return False
    elif strDate == "":
        strDate = time.strftime("%Y-%m-%d",time.localtime(time.time()+3600*24))
    url = ('https://kyfw.12306.cn/otn/leftTicket/query?'
           'leftTicketDTO.train_date={}&'
           'leftTicketDTO.from_station={}&'
           'leftTicketDTO.to_station={}&'
           'purpose_codes=ADULT').format(strDate, strFrom, strDest)
    print(url)
    try:
        if url=="":
            print("unhandle error")
        else:
            res=requests.get(url,verify=False)

            if res.status_code/100 !=2:
                return False
            data=res.json()["data"]["result"]
            valList=[]
            for it in data:
                val = {}
                items=it.split('|')

                # print(json.dumps(ret))
                for i,item in enumerate(items):
                    if i == 2:
                        val['train_type']=item
                    elif i == 3:
                        val['train_num'] = item
                    elif i == 4:
                        val['train_src']= item
                    elif i == 5:
                        val['train_dest']=item
                    elif i == 6:
                        val['train_travel_src'] = item
                    elif i == 7:
                        val['train_travel_dest'] = item
                    elif i == 8:
                        val['train_start_time'] = item
                    elif i == 9:
                        val['train_end_time'] = item
                    elif i == 10:
                        val['train_spend_time'] = item
                    elif i == 11:
                        val['train_is_sold'] = item
                    elif i == 13:
                        val['train_travel_date'] = item
                    elif i == 15:
                        val['train_is_k_or_dg'] = item
                    elif i == 23:
                        val['train_soft_sleep'] = item
                    elif i == 26:
                        val['train_hard_sleep'] = item
                    elif i == 27:
                        val['train_soft_seat'] = item
                    elif i == 28:
                        val['train_hard_seat'] = item
                    elif i == 29:
                        val['train_null_seat'] = item
                    elif i == 30:
                        val['train_second_seat'] = item
                    elif i == 31:
                        val['train_first_seat'] = item
                    elif i == 32:
                        val['train_td_seat'] = item
                    elif i == 33:
                        val['train_move_sleep']=item

                ret = query_ticket_price(items[2], items[16], items[17], items[35], strDate)
                print(ret)
                if ret != False and len(ret)!=0:
                    # 商务座
                    val['train_td_seat'] += " "+ret.get('A9','')
                    # 特等座
                    val['train_td_seat'] += "/" + ret.get('P','')
                    # 一等座
                    val['train_first_seat'] += " " + ret.get('M','')
                    # 二等座
                    val['train_second_seat'] += " " + ret.get('O','')
                    # 高级软卧
                    # val['train_advance_soft_sleep'] += " " + ret.get('A6','')
                    # 软卧
                    val['train_soft_sleep'] += " " + ret.get('A4','')
                    # 硬卧
                    val['train_hard_sleep'] += " " + ret.get('A3','')
                    # 软座
                    val['train_soft_seat'] += " " + ret.get('A2','')
                    # 硬座
                    val['train_hard_seat'] += " " + ret.get('A1','')
                    # 无座
                    val['train_null_seat'] += " " + ret.get('WZ','')
                    # print(val)
                valList.append(val)
            return valList
    except requests.exceptions.ConnectionError:
        print('ConnectionError -- please wait 3 seconds')
        time.sleep(3)
    except requests.exceptions.ChunkedEncodingError:
        print('ChunkedEncodingError -- please wait 3 seconds')
        time.sleep(3)

    # print(url)

def query_ticket_price(train_no, from_station_no, to_station_no, seat_types, train_date):
    urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
    url = "https://kyfw.12306.cn/otn/leftTicket/queryTicketPrice?"
    # train_no=630000K1920Q&from_station_no=14&to_station_no=16&seat_types=1413&train_date=2018-11-18
    #print (train_no, from_station_no, to_station_no, seat_types, train_date)
    payload = {
        'train_no': train_no,
        'from_station_no': from_station_no,
        'to_station_no': to_station_no,
        'seat_types': seat_types,
        'train_date': train_date,
    }
    try:
        header={}
        header['User-Agent']=getRandomAgent()
        requrl=url+'train_no=%s&from_station_no=%s&to_station_no=%s&seat_types=%s&train_date=%s'\
               %(train_no,from_station_no,to_station_no,seat_types,train_date)
        print("requrl :%s"%requrl)
        res = requests.get(requrl,headers=header,verify=False)
        if res.status_code == 200:
            if res.text.find('!DOCTYPE html')!=-1:
                print("爬取数据失败")
                return False
            ret=res.json()['data']
            return ret
        else:
            return False
    except requests.exceptions.ConnectionError:
        print('ConnectionError -- please wait 3 seconds')
        time.sleep(3)
    except requests.exceptions.ChunkedEncodingError:
        print('ChunkedEncodingError -- please wait 3 seconds')
        time.sleep(3)

if __name__ == '__main__':
    getResult("GZQ", "QNW", "2018-11-23")

