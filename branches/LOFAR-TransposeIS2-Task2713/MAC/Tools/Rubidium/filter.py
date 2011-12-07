#!/usr/bin/python
import sys
from pylab import plot, show, figure, title, ylabel,xlabel, semilogy

ttList = []

def main():
    global ttList
    output = None
    args = sys.argv[1:]
    if len(args) == 2:
        inputF = args[0]
        outputF = args[1]
    elif len(args) == 1:
        inputF = args[0]
    else:
        print "need one or two files, one input, one output file (optional)"
        sys.exit()
        
    fpI = open(inputF, 'r')
    if output != None:
        fpO = open(outputF, 'w')

    distDict = {}
    emptyLine = False
    pllNoise = []
    pllPower = []
    piList = []
    sfList = []
    stList = []
    stDict1 = {}
    stDict2 = {}
    stDict3 = {}
    stDict4 = {}
    stDict5 = {}
    stDict6 = {}
    dicts = {'1': stDict1, '2': stDict2,'3': stDict3,'4': stDict4,'5': stDict5,'6': stDict6}
    pllResetList = []
    count = 0
    defNew = 0
    while(not emptyLine):
        line = fpI.readline()
        count += 1
#        if  count > 10000:
#            emptyLine = True
        defOld = defNew
        defNew = int(count / 10000)
        if defNew != defOld:
            print count
#        print line
        if line != "" and line.find("; ; ; ;") == -1 and len(line)> 10:
            parts = line.split(';')
            line = line.strip()
            timeAndStatus = parts.pop(0)
            time = None
            status = None
            if timeAndStatus.find(' ')!= -1:
                subParts = timeAndStatus.split(' ')
                time = subParts[0]
                status = subParts[1]
            elif len(parts) == 8: 
                time = parts.pop(0)
                status = parts.pop(0)
            FC = parts.pop(0).strip()
            DS = parts.pop(0).strip()
            MR = parts.pop(0).strip()
            TT = parts.pop(0).strip()
            PI = parts.pop(0).strip()
            SF = None
            if len(parts)>0:
                SF = parts.pop(0).strip()
                
#            print len(parts)
            #Time and status
            newTTag = False
            if TT != "-1":
                newTTag = True

            if newTTag:
                value = 0
                try:
                    value = int(TT)
                except:
                    print "Error while converting the timetag of: ", line
                try:
                    curVal = distDict[value]
                    distDict[value] = curVal +1
                except:
                    distDict[value] = 1
                if value > 500000000 :
                    value = value - 1000000000
                ttList.append(value)
                if output != None:
                    fpO.write(line)
                
                pllError = DS
                pllE = pllError.split(',')
                pllNoise.append(pllE[0])
                pllPower.append(pllE[1])
                piList.append(int(PI))
                if SF != None and SF!= "":
                    sfList.append(int(SF))
                if status != None:
                    statP = status.split(',')
                    val1 = int(statP[0].strip())
                    val2 = int(statP[1].strip())
                    val3 = int(statP[2].strip())
                    val4 = int(statP[3].strip())
                    val5 = int(statP[4].strip())
                    val6 = int(statP[5].strip())
                    stList.append(val5)
                    rstBit = 0
                    if val5& 32>0:
                        rstBit = 1
                    pllResetList.append(rstBit)
                    count5 = 1
                    if stDict5.has_key(val5):
                        count5 = stDict5[val5]
                        count5 += 1
                    stDict5[val5] = count5
                    count1 = 1
                    if stDict1.has_key(val1):
                        count1 = stDict1[val1]
                        count1 += 1
                    stDict1[val1] = count1
                    count2 = 1
                    if stDict2.has_key(val2):
                        count2 = stDict2[val2]
                        count2 += 1
                    stDict2[val2] = count2
                    count3 = 1
                    if stDict3.has_key(val3):
                        count3 = stDict3[val3]
                        count3 += 1
                    stDict3[val3] = count3
                    count4 = 1
                    if stDict4.has_key(val4):
                        count4 = stDict4[val4]
                        count4 += 1
                    stDict4[val4] = count4
                    count6 = 1
                    if stDict6.has_key(val6):
                        count6 = stDict6[val6]
                        count6 += 1
                    stDict6[val6] = count6

#            emptyLine = True
        else:
            if line == "":
                emptyLine = True

    fpI.close()
    if output != None:
        fpO.close()
 
    plot(piList, '+')
    title('Integrator history')
    xlabel('Sample number')
    ylabel('Integrator value')
    if len(sfList)>10:
        figure()
        plot(sfList, '+')
        title('Frequency setting history')
        xlabel('Sample number')
        ylabel('Setting')
   
#    xList = distDict.keys()
#    yList = distDict.values()
#    plot(xList, yList)

    ttList = ttList[250:-1]
#    ttList = ttList[36000:-1]
    figure()
    plot(ttList, '+')
    title('Time tag history')
    xlabel('Sample number')
    ylabel('Time tag')

    if False:
        figure()
        plot(pllNoise, '+')
        title('Pll noise history')
        xlabel('Sample number')
        ylabel('Pll noise, mV')
        
        figure()
        plot(pllPower, '+')
        title('Pll power history')
        xlabel('Sample number')
        ylabel('Pll power, mV')

#    figure()
#    plot( stList, '+')
#    title('ST5 history')
#    xlabel('Sample number')
#    ylabel('ST 5')

    figure()
    plot(pllResetList, '+')
    xlabel('Sample number')
    ylabel('Pll reset')

    for dict in dicts.keys():
        curDict = dicts[dict]
        keys = curDict.keys()
        vals = curDict.values()
        print 'St ', dict,' status:'
        for k in keys:
            print 'Value: ', k, ' Occured: ', curDict[k]


#    distList = []
#    for i in range(1000000):
#        distList.append(ttList.count(i))

#    plot(distList)
    show()
#    print len(ttList)
#    plot(ttList)
#    show()
    
    

main()
