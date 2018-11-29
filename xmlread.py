
import xml.dom.minidom as xmlread

xmlconfig = xmlread.parse('./config.xml')

def getxmlValue(rootnode,keyname): # such as getXmlValue(root,"string/path")
    root=rootnode.documentElement
    # print("begin:",root )
    params=keyname.split('/')
    if len(params)==1:
        if len(root.getElementsByTagName(params[0])[0].childNodes) == 0:
            return ''
        else:
            return root.getElementsByTagName(params[0])[0].firstChild.data
        pass
    for i,param in enumerate(params):
        root = root.getElementsByTagName(param)
        # print("for big",param,len(root),root,i)
        if len(root) == 0:
            # print("the keyname is Valid")
            return ''
        else:
            bFound=False
            for item in root:
                searchIndex=i
                if i+1 < len(params):
                    searchIndex+=1
                node=item.getElementsByTagName(params[searchIndex])
                # print("for small",node,params[i])
                if len(node)!=0:
                    root=item
                    bFound=True
                    break
            if bFound==False:
                # print("can't find such %s value"%params[searchIndex])
                # print("root:",root)
                break
        pass
    if len(root[0].childNodes) == 0:
        return ''
    else:
        return root[0].firstChild.data
    pass

if __name__ == '__main__':
    print(getxmlValue(xmlconfig,'user/tag'))
    print(getxmlValue(xmlconfig,'name'))

