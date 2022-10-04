import json
import requests
locations_url ='https://kyfw.12306.cn/otn/resources/js/framework/station_name.js?station_version=1.9090'
try:
    res = requests.get(locations_url)
    data = res.text
    i = 0
    fromIndex = 0
    toIndex = 0
    stations=[]
    while True:
        station={}
        toIndex = data.find(str(i), fromIndex)
        if toIndex != -1:
            perstation = data[fromIndex:toIndex]
            temp = perstation[perstation.find('@')+1:].split('|')
            station["py"]=temp[0]
            station["name"]=temp[1]
            station["code"]=temp[2]
            station["fullpy"]=temp[3]
            stations.append(station)
            fromIndex = toIndex
            i+=1
        else:
            break
    with open('./res/location.json', 'w', encoding='utf-8') as f:
        f.write(json.dumps(stations))
except IOError:
    print("can't read")

