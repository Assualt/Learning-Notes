#!/usr/bin/python
# -*- encoding : utf-8 -*-

from functools import wraps
import time

class ConfMgr(object):

    def __init__(self, ConfName):
        if type(ConfName) == str:  # type judge (must be str)
            self._cfname = ConfName
        self._CFMAP = {}
        self._UpdateMap = {}
        self._initFile()
        pass
    def _initFile(self):
        try:
            with open(self._cfname, 'r', encoding='utf-8') as f:
                lines = f.readlines()
                i = 0
                while True:
                    if i + 1 == len(lines):
                        break
                    line = lines[i]
                    line = line.strip()
                    if line.startswith('#') or line == "":
                        i += 1
                        continue
                    elif line.startswith('['):  # begin to read the section
                        startIndex = line.find('[')
                        endIndex = line.find(']')
                        desIndex = line.find('#', endIndex)
                        strSectionKey = ''
                        if startIndex != -1 and endIndex != -1 and endIndex > startIndex:
                            if desIndex != -1 and desIndex > endIndex:  # for example [xx1]####
                                strSectionKey = line[startIndex + 1:endIndex]
                            elif desIndex == -1:  # for example [xx1]
                                strSectionKey = line[startIndex + 1:endIndex]

                            # begin to scan key:value
                            # begin to Enter Next Cnt
                            kvmap = {}
                            self._CFMAP[strSectionKey] = kvmap
                            while True:
                                if i + 1 == len(lines):
                                    break
                                i += 1
                                line = lines[i].strip()
                                startIndex = line.find('[')
                                endIndex = line.find(']')
                                desIndex = line.find('#', endIndex)
                                if line.startswith('#') or line == "":  # that line is description and ignore it
                                    continue
                                elif startIndex != -1 and endIndex != -1 and endIndex > startIndex:  ## found thenext key
                                    if desIndex != -1 and desIndex > endIndex:  # forExample [xx1
                                        break
                                    elif desIndex == -1:
                                        break
                                else:  # append key:value in section map
                                    strEquals = line.find('=')
                                    if strEquals != -1:
                                        strKey = line[:strEquals]
                                        desIndex = line.find('#', strEquals)
                                        strValue = line[strEquals + 1:desIndex] if desIndex != -1 else line[
                                                                                                       strEquals + 1:]
                                        strValue = strValue[1:] if strValue.startswith('"') else strValue
                                        strValue = strValue[:-1] if strValue.endswith('"') else strValue
                                        kvmap[strKey] = strValue

        except IOError:
            print("Catch IOException: can't open such file %s" % self._cfname)
        pass

    def checkType(self, strdiff, stype):  # judge the type of strdiff and the whether the strdiff is empty or not
        return type(strdiff) == stype and len(strdiff) != 0

    def _getSection(self, strSectionName):
        if self.checkType(strSectionName, str):
            if strSectionName in self._CFMAP.keys():  # found section
                return self._CFMAP[strSectionName]
        return {}
        pass

    def getStringDft(self, strDefaultValue, strSection, strKey):
        sectionMap = self._getSection(strSection)
        if self.checkType(strKey, str):
            if strKey in sectionMap.keys():
                return str(sectionMap[strKey])
            else:
                return strDefaultValue
        else:
            return None
        pass

    def getString(self, strSection, strKey):
        sectionMap = self._getSection(strSection)
        if self.checkType(strKey, str):
            if strKey in sectionMap.keys():
                return str(sectionMap[strKey])
        pass

    def getBoolDft(self, strDefaultValue, strSection, strKey):
        sectionMap = self._getSection(strSection)
        if self.checkType(strKey, str):
            if strKey in sectionMap.keys():
                strValue = sectionMap[strKey]
                try:
                    strValue = bool(strValue)
                except ValueError:
                    print("Catch Value Error Exception: can't convert %s to bool" % strValue)
                    return None
                return strValue
            else:
                return strDefaultValue
        else:
            return None
        pass

    def getBool(self, strSection, strKey):
        sectionMap = self._getSection(strSection)
        if self.checkType(strKey, str):
            if strKey in sectionMap.keys():
                strValue = sectionMap[strKey]
                try:
                    strValue = bool(strValue)
                except ValueError:
                    print("Catch Value Error Exception: can't convert %s to bool" % strValue)
                    return None
                return strValue
        pass

    def getIntDft(self, strDefaultValue, strSection, strKey):
        sectionMap = self._getSection(strSection)
        if self.checkType(strKey, str):
            if strKey in sectionMap.keys():
                strValue = sectionMap[strKey]
                try:
                    strValue = int(strValue)
                except ValueError:
                    print("Catch IOException: can't convert %s to int" % strValue)
                    return strDefaultValue
                return strValue
            else:
                return strDefaultValue
        else:
            return None
        pass

    def getInt(self, strSection, strKey):
        sectionMap = self._getSection(strSection)
        if self.checkType(strKey, str):
            if strKey in sectionMap.keys():
                strValue = sectionMap[strKey]
                try:
                    strValue = int(strValue)
                except ValueError:
                    print("Catch Value Error Exception: can't convert %s to Int" % strValue)
                    return None
                return strValue
        pass

    def getDoubleDft(self, strDefaultValue, strSection, strKey):
        sectionMap = self._getSection(strSection)
        if self.checkType(strKey, str):
            if strKey in sectionMap.keys():
                strValue = sectionMap[strKey]
                try:
                    strValue = float(strValue)
                except ValueError:
                    print("Catch Value Error Exception: can't convert %s to float" % strValue)
                    return strDefaultValue
                return strValue
            else:
                return strDefaultValue
        else:
            return None
        pass

    def getDouble(self, strSection, strKey):
        sectionMap = self._getSection(strSection)
        if self.checkType(strKey, str):
            if strKey in sectionMap.keys():
                strValue = sectionMap[strKey]
                try:
                    strValue = float(strValue)
                except ValueError:
                    print("Catch Value Error Exception: can't convert %s to float" % strValue)
                    return None
                return strValue
        pass

    def _updateMap(self, strSection, strKey, strVal):
        if strSection in self._UpdateMap.keys():
            self._UpdateMap[strSection][strKey] = strVal
        else:
            kvmap = {}
            kvmap[strKey] = strVal
            self._UpdateMap[strSection] = kvmap
        pass

    def updateKeyVal(self, strSection, strKey, strVal):
        if type(strVal) == str:
            strVal = '\"'+strVal+'\"'
        if self.checkType(strSection, str) and self.checkType(strKey, str):  # check section and key
            if strSection in self._CFMAP.keys():
                if strKey in self._CFMAP[strSection].keys():  # update the key val
                    self._CFMAP[strSection][strKey] = strVal

                    self._updateMap(strSection, strKey, strVal)
                else:  # insert it
                    self._CFMAP[strSection][strKey] = strVal
                    self._updateMap(strSection, strKey, strVal)
            else:  # new section
                kvmap = {}
                kvmap[strKey] = strVal
                self._CFMAP[strSection] = kvmap
                self._updateMap(strSection, strKey, strVal)

        self._flushtoFile()
        pass

    def _write2bak(self, strtime):
        try:
            with open(self._cfname + ".bak" + strtime, 'w', encoding='utf-8') as f1, open(self._cfname, 'r',
                                                                                          encoding='utf-8') as f2:
                for line in f2.readlines():
                    f1.write(line)
        except IOError:
            print("Catch IOException: can't open such file %s" % self._cfname + ".bak" + strtime)

    def _flushtoFile(self):
        strtime = time.strftime("%Y%m%d", time.localtime())
        self._write2bak(strtime)
        try:  # 1.先copy一份到bak文件中去
            with open(self._cfname + ".bak" + strtime, 'r', encoding='utf-8') as f1, open(self._cfname, 'w',
                                                                                          encoding='utf-8') as f2:
                lines = f1.readlines()
                i = 0
                while True:
                    if i + 1 == len(lines):
                        self._UpdateMap.clear()
                        break
                    line = lines[i]
                    linetemp = line.strip()
                    if linetemp.startswith('#') or len(linetemp) == 0:
                        f2.write(line)
                        i += 1
                        continue
                    elif linetemp.startswith('['):  # begin to read the section
                        startIndex = linetemp.find('[')
                        endIndex = linetemp.find(']')
                        desIndex = linetemp.find('#', endIndex)
                        strSectionKey = ''
                        if startIndex != -1 and endIndex != -1 and endIndex > startIndex:
                            if desIndex != -1 and desIndex > endIndex:  # for example [xx1]####
                                strSectionKey = line[startIndex + 1:endIndex]
                            elif desIndex == -1:  # for example [xx1]
                                strSectionKey = line[startIndex + 1:endIndex]

                            # begin to scan key:value
                            # begin to Enter Next Cnt
                            f2.write(line)  # write sectionName
                            bInsection = False
                            if strSectionKey in self._UpdateMap.keys():
                                bInsection = True
                            while True:
                                if i + 1 == len(lines):  #end
                                    if len(self._UpdateMap) != 0 :  # 说明为新加的段
                                        for kval in self._UpdateMap.keys():
                                            if kval not in self._CFMAP.keys():
                                                f2.write('\n[' + str(kval) + ']\n')  # 写段名称
                                                for key in self._UpdateMap[str(kval)]:  # 写属性值
                                                    f2.write(str(key) + "=" + str(self._UpdateMap[kval][key]) + '\n')
                                    self._UpdateMap.clear()
                                    break
                                i += 1
                                line = lines[i]
                                linetemp = line.strip()
                                startIndex = linetemp.find('[')
                                endIndex = linetemp.find(']')
                                desIndex = linetemp.find('#', endIndex)
                                if linetemp.startswith('#') or len(line) == 0:  # that line is description and ignore it
                                    f2.write(line)
                                    continue
                                elif startIndex != -1 and endIndex != -1 and endIndex > startIndex:  ## found thenext key
                                    if desIndex != -1 and desIndex > endIndex:  # forExample [xx1
                                        if not bInsection != True and len(self._UpdateMap[strSectionKey]) != 0:
                                            for kval in self._UpdateMap[strSectionKey]:
                                                f2.write(str(kval) + '=' + str(self._UpdateMap[strSectionKey][kval])+"\n")
                                        if strSectionKey in self._UpdateMap.keys():
                                            self._UpdateMap.pop(strSectionKey)
                                        break
                                    elif desIndex == -1:
                                        if not bInsection != True and len(self._UpdateMap[strSectionKey]) != 0:
                                            for kval in self._UpdateMap[strSectionKey]:
                                                f2.write(str(kval) + '=' + str(self._UpdateMap[strSectionKey][kval])+"\n")
                                        if strSectionKey in self._UpdateMap.keys():
                                            self._UpdateMap.pop(strSectionKey)
                                        break
                                else:  # append key:value in section map
                                    if not bInsection != False:
                                        f2.write(line)
                                    else:
                                        strEquals = linetemp.find('=')
                                        if strEquals != -1:
                                            strKey = linetemp[:strEquals]
                                            strdes = "" if linetemp.find('#', strEquals) == -1 else linetemp[linetemp.find('#', strEquals):]
                                            if strKey in self._UpdateMap[strSectionKey].keys():  # 如果匹配到这个属性
                                                f2.write(str(strKey) + "=" + str(
                                                    self._UpdateMap[strSectionKey][strKey]) + strdes + "\n")
                                                self._UpdateMap[strSectionKey].pop(strKey)
                                            else:
                                                f2.write(line)

                print("end of flush to file")
        except IOError:
            print("Catch IOException: can't open such file %s" % self._cfname + ".bak" + strtime)

        pass
if __name__ == '__main__':
    # InstanceTest()
    cf=ConfMgr('../conf/global.cf')
    print(cf.getString('global','minHeight'))
    cf.updateKeyVal('global','minHeight',41233)