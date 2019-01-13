#£¡/usr/bin/env bash
#! -*- encoding: utf-8 -*-
import xml.dom.minidom as xmlread

xmlconfig = xmlread.parse('./config.xml')

def getxmlValue(rootnode, keyname, attr=None):  # such as getXmlValue(root,"string/path")
    root = rootnode.documentElement
    params = keyname.split('/')
    if len(params) == 1:
        if len(root.getElementsByTagName(params[0])[0].childNodes) == 0:
            return ''
        else:
            return root.getElementsByTagName(params[0])[0].firstChild.data
        pass
    for i, param in enumerate(params):
        root = root.getElementsByTagName(param)
        if len(root) == 0:
            return ''
        else:
            bFound = False
            for item in root:
                searchIndex = i
                if i+1 < len(params):
                    searchIndex += 1
                node = item.getElementsByTagName(params[searchIndex])
                if len(node) != 0:
                    root = item
                    bFound = True
                    break
            if bFound == False:
                break
        pass
    if len(root[0].childNodes) == 0:
        return ''
    else:
        if attr != None:
            return root[0].getAttribute(attr)
        return root[0].firstChild.data
    pass


if __name__ == '__main__':
    print(getxmlValue(xmlconfig, 'user/tag', 'vxalue'))
    print(getxmlValue(xmlconfig, 'name'))
    print(getxmlValue(xmlconfig, 'pss/tag', 'value'))