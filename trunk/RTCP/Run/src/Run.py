#!/usr/bin/env python

import math
import time
import datetime
import os
import sys
import shutil
from optparse import OptionParser

#check hostname
for i in range(0,1):
    if os.environ.get('HOSTNAME', 'default') == 'listfen':
        hostname = 'listfen'
        break
    elif os.environ.get('HOSTNAME', 'default') == 'bgfen0':
        hostname = 'bgfen0'
        break
    else:    
        print 'Restart "Run.py" run-script on: listfen/bgfen0'
        sys.exit(1)

from Host_Names import *
from Parset import Parset
from Sections import *
from Stations import *

def doObservation(obsID, parset):
    if False:
        rsp = RSPCtl(CS10LCU)
        rsp.initBoards(parset.getClockString())
        rsp.selectSubbands(parset.getFirstSubband())
        rsp.selectRCUs([0, 1, 2, 8, 9])
        rsp.setWG([2], 60e6)

    mapTable = dict({'gels': gels, 'lofarsys': lofarsys, 'romein': romein, 'broekema': broekema, 'nieuwpoo': nieuwpoo})
    logname = os.environ.get("LOGNAME", os.environ.get("USERNAME"))
    if mapTable.has_key(logname):
        userId = mapTable.get(logname)
    else:
        print 'Invalid userId: ' + logname
	sys.exit(1)
    
    parsetfile = workingDir + 'RTCP.parset'
    
    sectionTable = dict({\
        'IONProcSection': IONProcSection(parset, userId.getHost(), options.partition, workingDir, parsetfile),
	'BGLProcSection': BGLProcSection(parset, userId.getHost(), options.partition, workingDir, parsetfile),
	'StorageSection': StorageSection(parset, listfen, workingDir, parsetfile)
        })

    sectionList = sectionTable.keys()
    sectionList.sort()
    
    if not sectionTable.has_key('IONProcSection') or not sectionTable.has_key('BGLProcSection'):
	parset['OLAP.OLAP_Conn.IONProc_BGLProc_Transport'] = 'NULL'
    
    if sectionTable.has_key('BGLProcSection') and sectionTable.has_key('StorageSection'):
	parset['OLAP.OLAP_Conn.BGLProc_Storage_Transport'] = 'TCP'
    else:
	parset['OLAP.OLAP_Conn.BGLProc_Storage_Transport'] = 'NULL'

    nSubbandSamples = float(parset['OLAP.BGLProc.integrationSteps']) * float(parset['Observation.channelsPerSubband'])
    stepTime = nSubbandSamples / (parset['Observation.sampleClock'] * 1000000.0 / 1024)

    startTime = parset['Observation.startTime']
    stopTime = parset['Observation.stopTime']
 
    sz = int(math.ceil((time.mktime(stopTime.timetuple()) - time.mktime(startTime.timetuple())) / stepTime))

    logdir = '/log/'
    if hostname == listfen.name:
	if not os.access(logdir, os.W_OK):
            logdir = './'
        os.mkdir(logdir + obsID)
    else:
         listfen.executeAsync('mkdir ' + logdir + obsID).waitForDone()
    
    parset.writeToFile(workingDir + obsID + '.parset')
    if hostname == listfen.name:
        shutil.move(workingDir + obsID + '.parset',  logdir + obsID + '/' + obsID + '.parset')
    else:
        listfen.sput(workingDir + obsID + '.parset', logdir + obsID + '/' + obsID + '.parset')
        os.remove(workingDir + obsID + '.parset')
    
    # write parset files
    parset.writeToFile(parsetfile)
    shutil.copy('OLAP.parset', workingDir)
	
    try:
        for section in sectionList:
            print ('Starting ' + sectionTable.get(section).package)
            runlog = logdir + obsID + '/' + sectionTable.get(section).getName() + '.' + options.partition +'.runlog'
            
            # todo 27-10-2006 this is a temporary hack because storage doesn't close neatly.
            # This way all sections run longer than needed and storage stops before the rest does

	    timeOut = ((sz+63)&~63) + 64

            if isinstance(sectionTable.get(section), StorageSection):
		s = ':'
		for slave in listfen.getSlaves():
		    machineNr = int(slave.getIntName()[len(slave.getIntName())-1])-1
		    if len(listfen.getSlaves())-1 != listfen.getSlaves().index(slave):
			s = s + '%d' % int(machineNr) + ','
	            else:
		        s = s + '%d' % int(machineNr)
			
		timeOut = (sz+63)&~63
	        commandstr ='cexec ' + s + ' mkdir /data/' + obsID
		listfen.executeAsync(commandstr).waitForDone()
		
 	    sectionTable.get(section).run(runlog, timeOut)
 
        for section in sectionList:
            print ('Waiting for ' + sectionTable.get(section).package + ' to finish ...')
            ret = sectionTable.get(section).isRunSuccess()
            print (ret)
            print
    except KeyboardInterrupt:
        for s in sectionList:
            print ('Aborting ' + sectionTable.get(s).package)
            sectionTable.get(s).abortRun()

if __name__ == '__main__':

    parser = OptionParser()

    # do not use the callback actions of the OptionParser, because we must make sure we read the parset before adding any variables
    parser.add_option('--parset'         , dest='parset'         , default='RTCP.parset', type='string', help='name of the parameterset [%default]')
    parser.add_option('--partition'      , dest='partition'      , default='R000_256_1', type='string', help='name of the BGL partion [%default]')
    parser.add_option('--clock'          , dest='clock'          , default='200MHz'    , type='string', help='clock frequency (either 160MHz or 200MHz) [%default]')
    parser.add_option('--runtime'        , dest='runtime'        , default='600'       , type='int'   , help='length of measurement in seconds [%default]')
    parser.add_option('--starttime'      , dest='starttime', default=int(time.time() + 25), type='int', help='start of measurement in UTC seconds [now + 25]')
    parser.add_option('--integrationtime', dest='integrationtime', default='60'        , type='int'   , help='length of integration interval in seconds [%default]')
    parser.add_option('--msname'         , dest='msname'                               , type='string', help='name of the measurement set')
    parser.add_option('--stationlist'    , dest='stationlist'	 , default='CS010      ',type='string', help='name of the station or stationconfiguration (see Stations.py) [%default]')
    parser.add_option('--fakeinput'      , dest='fakeinput'      , action='count'                     , help='do not really read from the inputs, but from memory')
    # parse the options
    (options, args) = parser.parse_args()
    
    workingDir = os.getcwd()[:-len('LOFAR/RTCP/Run/src')]
    
    # create the parset
    parset = Parset() 

    parset.readFromFile(options.parset)
    parset.setClock(options.clock)
    parset.setIntegrationTime(options.integrationtime) 
    parset.setPartition(options.partition)
    parset.subbandsPerPset()
    parset.psetsPerStorage()
    
    parset.check()
    
    if options.msname:
        parset.setMSName(options.msname)

    # read the runtime (optional start in utc and the length of the measurement)
    parset.setInterval(options.starttime, options.runtime)

    # read the stations from Stations.py
    # todo: WARNING this is very dangerous, because there could be any code in the station string
    # the exec should probably be replaced by something safer, but this is only a temporary script
    # This way is convenient because a user can say CS10_4dipoles + CS01_4dipoles
    try:
        exec('stationList =' + options.stationlist)
    except:
        print 'Cannot parse station configuration: ' + str(options.stationlist)
        sys.exit(1)
    
    parset.setStations(stationList)
    parset.checkBeamformList()
    parset.setTiedArrayStations()
    parset.tabMapping()
    
    # see if we are using fake input
    if options.fakeinput > 0:
        parset.setInterval(1, options.runtime+10)
        parset.setInputToMem()

    # if the msname wasn't given, read the next number from the file
    if hostname != listfen.name:
        shutil.copy('/log/nextMSNumber', workingDir)
	shutil.copy('/log/MSList', workingDir)
        path = workingDir
    else:
        path = '/log/'
    
    runningNumberFile = path + 'nextMSNumber'
    MSdatabaseFile = path + 'MSList'
    
    try:
	print runningNumberFile
	inf = open(runningNumberFile, 'r')
	measurementnumber = int(inf.readline())
	print 'MS =', measurementnumber
	inf.close()
	parset['Observation.ObsID'] = measurementnumber
	outf = open(runningNumberFile, 'w')
	outf.write(str(measurementnumber + 1) + '\n')
	outf.close()
	dbfile = open(MSdatabaseFile, 'a')
	dateStr = time.strftime('%Y %0m %0d %H %M %S', time.gmtime()).split()
	MS = parset.getString('Observation.MSNameMask')
	MS = MS.replace('${YEAR}', dateStr[0])
	MS = MS.replace('${MONTH}', dateStr[1])
	MS = MS.replace('${DAY}', dateStr[2])
	MS = MS.replace('${HOURS}', dateStr[3])
	MS = MS.replace('${MINUTES}', dateStr[4])
	MS = MS.replace('${SECONDS}', dateStr[5])
	MS = MS.replace('${MSNUMBER}', '%05d' % parset['Observation.ObsID'])
	MS = MS.replace('${SUBBAND}', '*')
	
	dbfile.write(MS + '\t' + ' '.join(dateStr[0:3]) + '\n')
	dbfile.close()
	
	if hostname != listfen.name:
	    listfen.sput(runningNumberFile, '/app/log/')
	    listfen.sput(MSdatabaseFile, '/app/log/')
	    os.remove(runningNumberFile)
	    os.remove(MSdatabaseFile)
    except:
	print 'caught exception'
	sys.exit(1)

    obsID = 'L' + dateStr[0] + '_' + '%05d' % measurementnumber
    # start the observation
    doObservation(obsID, parset)
