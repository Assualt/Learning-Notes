import json
filesdir='./res/location.txt'

try:
    with open(filesdir,'r',encoding='utf-8') as f:
        stres = f.read()
        i = 0
        fromIndex = 0
        toIndex = 0
        stations=[]
        while True:
            station={}
            toIndex = stres.find(str(i),fromIndex)
            if toIndex != -1:
                perstation = stres[fromIndex:toIndex]
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
    with open('./res/location.json','w',encoding='utf-8') as f:
        f.write(json.dumps(stations))
except IOError:
    print("can't read")

