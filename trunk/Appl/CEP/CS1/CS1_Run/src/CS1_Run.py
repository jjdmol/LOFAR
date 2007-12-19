#!/usr/bin/env python

import math
import time
import datetime
import os
import sys
from optparse import OptionParser

#check machine name
if  os.environ.get('HOSTNAME', 'default') != 'listfen':
    print 'Please restart CS1_Run script on hostname: listfen'
    sys.exit(1)

from CS1_Hosts import *
from CS1_Parset import CS1_Parset
from CS1_RSPCtl import RSPCtl
from CS1_Sections import *
from CS1_Stations import *

def doObservation(obsID, parset):
    if False:
        rsp = RSPCtl(CS10LCU)
        rsp.initBoards(parset.getClockString())
        rsp.selectSubbands(parset.getFirstSubband())
        rsp.selectRCUs([0, 1, 2, 8, 9])
        rsp.setWG([2], 60e6)

    mapTable = dict({'gels': gels, 'lofarsystem': lofarsystem, 'romein': romein, 'broekema': broekema})
    logname = os.environ.get("LOGNAME", os.environ.get("USERNAME"))
    if mapTable.has_key(logname):
        userId = mapTable.get(logname)
    else:
        print 'Invalid userId: ' + logname
	sys.exit(1)

    sectionTable = dict({\
        'DelayCompensationSection': DelayCompensationSection(parset, list003),
	'BGLProcSection': BGLProcSection(parset, userId.getHost(), options.partition),
	'StorageSection': StorageSection(parset, listfen)
	#Flagger(parset, listfen)
        })

    if sectionTable.has_key('DelayCompensationSection') and sectionTable.has_key('BGLProcSection'):
	parset['OLAP.OLAP_Conn.input_DelayComp_Transport'] = 'TCP'
    else:
	parset['OLAP.OLAP_Conn.input_DelayComp_Transport'] = 'NULL'
   
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
    if not os.access(logdir, os.W_OK):
        logdir = './'
    logdirCommand = 'mkdir ' + logdir + obsID
    if os.system(logdirCommand) != 0:
        print 'Failed to create directory: ' + logdirstr
    parset.writeToFile(logdir +'/' + obsID + '/' + obsID + '.parset')

    try:
        for section in sectionTable:
            print ('Starting ' + sectionTable.get(section).package)
            runlog = logdir + obsID + '/' + sectionTable.get(section).getName() + '.' + options.partition +'.runlog'

            # todo 27-10-2006 this is a temporary hack because storage doesn't close neatly.
            # This way all sections run longer than needed and storage stops before the rest does

	    noRuns = ((sz+15)&~15) + 16
	    
            if isinstance(sectionTable.get(section), StorageSection):
	        noRuns = (sz+15)&~15
	        commandstr ='cexec mkdir ' + '/data/' + obsID
	        if os.system(commandstr) != 0:
	            print 'Failed to create directory: /data/' + obsID
	    
	    sectionTable.get(section).run(runlog, noRuns)

        for section in sectionTable:
            print ('Waiting for ' + sectionTable.get(section).package + ' to finish ...')
            ret = sectionTable.get(section).isRunSuccess()
            print (ret)
            print
    except KeyboardInterrupt:
        for s in sectionTable:
            print ('Aborting ' + sectionTable.get(s).package)
            sectionTable.get(s).abortRun()

if __name__ == '__main__':

    parser = OptionParser()

    # do not use the callback actions of the OptionParser, because we must make sure we read the parset before adding any variables
    parser.add_option('--parset'         , dest='parset'         , default='CS1.parset', type='string', help='name of the parameterset [%default]')
    parser.add_option('--partition'      , dest='partition'      , default='R000_128_0T',type='string', help='name of the BGL partion [%default]')
    parser.add_option('--clock'          , dest='clock'          , default='160MHz'    , type='string', help='clock frequency (either 160MHz or 200MHz) [%default]')
    parser.add_option('--runtime'        , dest='runtime'        , default='600'       , type='int'   , help='length of measurement in seconds [%default]')
    parser.add_option('--starttime'      , dest='starttime', default=int(time.time() + 90), type='int', help='start of measurement in UTC seconds [now + 90s]')
    parser.add_option('--integrationtime', dest='integrationtime', default='60'        , type='int'   , help='length of integration interval in seconds [%default]')
    parser.add_option('--msname'         , dest='msname'                               , type='string', help='name of the measurement set')
    parser.add_option('--stationlist'    , dest='stationlist'	 , default='CS010_4dipoles0_4_8_12', type='string', help='name of the station or stationconfiguration (see CS1_Stations.py) [%default]')
    parser.add_option('--fakeinput'      , dest='fakeinput'      , action='count'                     , help='do not really read from the inputs, but from memory')
    # parse the options
    (options, args) = parser.parse_args()

    # create the parset
    parset = CS1_Parset() 

    parset.readFromFile(options.parset)

    parset.setClock(options.clock)
    parset.setIntegrationTime(options.integrationtime) 
    parset.setPartition(options.partition) 
    if options.msname:
        parset.setMSName(options.msname)

    # read the runtime (optional start in utc and the length of the measurement)
    parset.setInterval(options.starttime, options.runtime)

    # convert beamdirections from RA and Dec to Radians
    #parset.setBeamdir()
    
    # read the stations from CS1_Stations.py
    # todo: WARNING this is very dangerous, because there could be any code in the station string
    # the exec should probably be replaced by something safer, but this is only a temporary script
    # This way is convenient because a user can say CS10_4dipoles + CS01_4dipoles
    try:
        exec('stationList =' + options.stationlist)
    except:
        print 'Cannot parse station configuration: ' + str(options.stationlist)
        sys.exit(1)
    
    parset.setStations(stationList)

    # see if we are using fake input
    if options.fakeinput > 0:
        parset.setInterval(1, options.runtime+10)
        parset.setInputToMem()

    # if the msname wasn't given, read the next number from the file
    runningNumberFile = '/log/nextMSNumber'
    MSdatabaseFile = '/log/MSList'

    try:
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
    except:
	print 'caught exception'
	sys.exit(1)

    obsID = 'L' + dateStr[0] + '_' + '%05d' % measurementnumber
    
    # start the observation
    doObservation(obsID, parset)
