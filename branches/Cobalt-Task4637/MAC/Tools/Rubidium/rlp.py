#!/usr/bin/python

VERSION = "1.0"

import sys
import os
from pylab import plot, show, figure, title, ylabel,xlabel, semilogy
from datetime import datetime
import calendar

helptext = \
"""\
Usage:
    rlp [<Rubidium log file>] <option>
        Parse the log file and plot it's contents. When the clockstats logfile
        of the same date is present, the sattelites tracked by gps are also 
        plotted. When "all" is put as option, the first parameter is treated as 
        a search string. All files which name contains the string are parsed.

    rlp -h, --help
        Show help (this text)

    rlp -v, --version
        Print the version number of PVML and exit

"""

versiontext = "Rubidium Log Plotter (RLP) v" + VERSION + "\n" 


offSet = 2208988800 #compensate, gps file use seconds since 1 jan 1900, python 1 jan 1970
intStart = 1
intStop = -1
ttList = []


def main():
    global ttList
    args = sys.argv[1:]
    multiFile = False
    all = None

    if len(args) == 2:
        inputF = args[0]
        all = args[1]
    elif len(args) == 1:
        if sys.argv[1] == '-h' or sys.argv[1] == '--help':
            print helptext
            sys.exit()
        if sys.argv[1] == '-v' or sys.argv[1] == '--version':
            print versiontext
            sys.exit()
        inputF = args[0]
    else:
        msg = "Incorrect parameters"
        print helptext
        print msg
        sys.exit(-1)

    if all != None and all == "all":
        multiFile = True
        

    fileList = []
    path = None
    path, base = os.path.split(inputF)
    if multiFile :
        set = os.listdir(path)
        for file in set:
            if file.find(base) != -1:
                fileList.append(os.path.join(path,file))
    else:
        fileList.append(inputF)

    if fileList[0].find('.') == -1:
        fileList.sort()
        first = fileList.pop(0)
        fileList.append(first)

    print "Files in List: ", fileList
    orgFileList = fileList[:]

    if len(fileList) > 0:
        inputFile = fileList.pop(0)  
    else:
        inputFile = inputF

    fpI = open(inputFile, 'r')

    emptyLine = False
    
    cmdResDict = {}
    count = 0
    defNew = 0
    firstTime = ""
    endTime = ""

    while(not emptyLine):
        line = fpI.readline()
        count += 1
        defOld = defNew
        defNew = int(count / 10000)
        if defNew != defOld:
            print "Line count: ", count

        if line != ""  and len(line) > 10:
            parts = line.split(';')
            line = line.strip()
            
            timeValue = parts.pop(0)
            timeValues = timeValue.split('.')
            try:
                structTime = datetime.strptime(timeValues[0], "%Y-%m-%dT%H:%M:%S")
            except Exception, e:
                print 'Error while converting: ', timeValues, ' Details: ', e.__str__()
            timeTest = structTime.timetuple()
            secValue = calendar.timegm(timeTest) + offSet
            try:
                timeList = cmdResDict['time']
                timeList.append(secValue)
                cmdResDict['time'] = timeList
                if len(timeList) == intStop or intStop == -1:
                    endTime = timeValue
                if len(timeList) == intStart:
                    firstTime = timeValue

            except:
                cmdResDict['time'] = [secValue]

            for p in parts:
                p = p.strip()
                resP =  p.split(' ')
                cmd = resP[0].strip()
                if len(resP)>1:
                    val = resP[1]
                    conVal = None
                    skip = False
                    try:
                        conVal = int(val)
                        if cmd == 'TT' and conVal > 500000000 :
                            conVal = conVal - 1000000000
                    except:
                        try:
                            conVal = float(val)
                        except Exception, e:
                            try:
                                parts = val.split(',')
                                lenParts = len(parts)
                                if lenParts > 1:
                                    skip = True
                                    for i in range(lenParts):
                                        conVal = int(parts[i])
                                        key = cmd + str(i+1)
                                        try:
                                            cmdList = cmdResDict[key]
                                            cmdList.append(conVal)
                                            cmdResDict[key] = cmdList
                                        except:
                                            cmdResDict[key] = [conVal]
                            except:
                                conVal = val
                    if not skip:        
                        try:
                            cmdList = cmdResDict[cmd]
                            cmdList.append(conVal)
                            cmdResDict[cmd] = cmdList
                        except:
                            cmdResDict[cmd] = [conVal]
        else:
            if line == "":
                if len(fileList)> 0:
                    inputFile = fileList.pop(0)
                    fpI = open(inputFile, 'r')
                else:
                    emptyLine = True

    fpI.close()

    cmdKeys = cmdResDict.keys()
    for k in cmdKeys:
        list = cmdResDict[k]
        if isinstance(list[0], int) or isinstance(list[0], float) or isinstance(list[0], long):
            oldValue = list[0]
            changed = False
            for item in list:
                if item != oldValue:
                    changed = True
            if changed:
                figure()
                plot(list, 'r+')
                titleText = 'History of: ' + k
                title(titleText)
                xlabel('Sample number')
                ylabel('Cmd value')
            else:
                print k, " cmd returned always: ", oldValue
        else:
            print "Trouble converting cmd: ", k,' Value:', list[0].__class__


    satCountList = []
    satCountListMax = []
    satTimeList = []
    clockList = []
    for item in orgFileList:
        clockFile = os.path.join(path, "clockstats")
        parts = item.split('.')
        if len(parts) > 1:
            timePart = parts[-1]
            clockFile += '.' + timePart        

        if os.path.exists(clockFile):
            clockList.append(clockFile)
        else:
            print "file does not exists:", clockFile

    skipGps = False
    if len(clockList)==0:
        skipGps = True
    
    for file in clockList:
        cFp = open( file)
        for line in cFp.readlines():
            startI = line.find('nsat')
            if startI != -1:
                stopI = line.find('traim')
                subLine = line[startI:stopI]
                parts = subLine.split()
                sats = parts[1].strip()
                satParts = sats.split(',')
                maxSat = int(satParts[0])
                curSat = int(satParts[1])
                satCountList.append(curSat)
                satCountListMax.append(maxSat)                
                
                lineParts = line.split(' ')
                gpsTime = lineParts[3]
                gpsTimes = gpsTime.split('.')
                satTimeList.append(int(gpsTimes[0]))
        cFp.close()

    startTime = timeList[intStart]
    stopTime = timeList[intStop]
    if not skipGps:
#        print satTimeList[0],satTimeList[-1], startTime, stopTime
        startIndex = 0
        try:
            startIndex = satTimeList.index(startTime)
        except Exception, e:
            try: 
                startIndex = satTimeList.index(startTime + 1)
            except Exception, e:
                print 'Trouble extracting the following starttime:', startTime, firstTime
                print 'first record: ', satTimeList[0]

            
        stopIndex = -1
        try:
            stopIndex = satTimeList.index(stopTime)
        except Exception, e:
            try: 
                stopIndex = satTimeList.index(stopTime-1)
            except Exception, e:
                print 'Trouble extracting the following stoptime:', stopTime, endTime
                print 'latest record: ', satTimeList[-1]
        figure()
        plot(satCountList[startIndex:stopIndex], 'r+')
        titleText = 'History of tracked gps sats' 
        title(titleText)
        xlabel('Sample number')
        ylabel('sats')

#        print startTime, stopTime, stopTime-startTime

        totalValue = 0.0
        for i in range(10):
            val1 = satCountList.count(i+1)
            totalValue += (i+1) * val1
            print 'gps nr of sats: ', i+1, ' counted: ', val1

        mean = totalValue/len(satCountList)
        print 'Gps sats mean:', mean


    show()
    
    

main()
