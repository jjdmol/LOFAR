#!/usr/bin/env python

#
# Run the tests to test a LOFAR station
# H. Meulman
# Version 0.14                17-feb-2012	SVN*****

# 24 sep: local log directory aangepast
# 27 sept: 	- Toevoeging delay voor tbbdriver polling
#		- Aanzetten van LBA's m.b.v rspctl --aweights=8000,0
# 26 nov: Check op 160 MHZ en 200 MHZ clock.
# 18 jan 2011: Check op !="?" vervangen door =='LOCKED' in 160 en 200 MHz clock test 
# 18 jan 2011: Local Log directory aangepast
# 19 jan 2011: clocktest in 160 en 200 MHz clock test over een 10 itteraties!
# 19 jan 2011: Diff test AP nummer word nu ook gelogged. 'sync' verwijderd
# 19 jan 2011: TBB versie aangepast naar 2.32
# 21 jan 2011: TBB Version foutmelding aangepast (AP/BP Error moet TP/BP Error zijn)
# 02 feb 2011: LBA down toe gevoegd!
# 18 mrt 2011: Automatiesche detectie Core- remote- en International station toegevoegd.
# 18 mrt 2011: Volgende testen aangepast zodat ze ook internationale stations kunnen testen:
#		CheckRSPVersion, CheckTDSStatus160 en 200, CheckRSPStatus, CheckTBBVersion
#		LBAtest(), HBAModemTest(), HBAtest() De laatste test werkt pas als transmitters geinstalleerd zijn.
# 18 mrt 2011: Als alle LBA's niet werken, wordt error gelogd. (average < 4000000)
# 30 mrt 2011: TBBversion_int.gold aangepast voor internationale stations.
# 7 sep 2011: Bug removed. On the remote stations LBA mode 1 will now also be tested.
# oct 2011: added CS028 and CS031
# 22 nov 2011: TBB versie aanpassen naar 2.39
# 22 nov 2011: Changed filename to not overwrite testdata subrack test
# 12 jan 2012: Reject LBA antennas when signal differs more than 10dB up. These antennas shoeld not contribute to the average
# 26 jan 2012: Reject LBA antennas when signal differs less than 3dB down. These antennas shoeld not contribute to the average
# 27 jan 2012: Store logfiles in /localhome/stationtest/data in "local mode"
# 17 feb 2012: Added detection of oscillating tiles.
# 9 mar 2012: Devide by 0 error solved in HBAtest
# 13 sept 2012: Added for user0..9 sys.path.append("/opt/stationtest/modules")

# todo:
# - Als meer dan 10 elementen geen rf signaal hebben, keur dan hele tile af
# - als beamserver weer goed werkt deze weer toevoegen aan LBA test
# - =='LOCKED' in 160 en 200 MHz clock test over een aantal keren!
# Loggen absolute waarden van alle antennes (LBH LBL eb HBA)
# BIST toevoegen RSP boards
# Cabinet temperatuur monitoren



import sys
sys.path.append("/opt/stationtest/modules")
from optparse import OptionParser
import cli
import testlog
from time import localtime, strftime
import array
import os
import time
import commands
import operator
import math
import numpy

################################################################################
# Init

# Variables
debug=0
clkoffset=1

factor = 30	# station statistics fault window: Antenna average + and - factor = 100 +/- 30

InternationalStations = ('DE601C','DE602C','DE603C','DE604C','DE605C','FR606C','SE607C','UK608C')
RemoteStations = ('CS302C','RS106C','RS205C','RS208C','RS306C','RS307C','RS406C','RS503C')
CoreStations = ('CS001C','CS002C','CS003C','CS004C','CS005C','CS006C','CS007C','CS011C','CS013C','CS017C','CS021C','CS024C','CS026C','CS028C','CS030C','CS031','CS032C','CS101C','CS103C','CS201C','CS301C','CS401C','CS501C')
NoHBAelementtestPossible = ('DE601C','DE602C','DE603C','DE605C','FR606C','SE607C','UK608C')
NoHBANaStestPossible = ('')
HBASubband = dict( 	DE601C=155,\
			DE602C=155,\
			DE603C=284,\
			DE604C=474,\
			DE605C=479,\
			FR606C=155,\
			SE607C=155,\
			UK608C=155)

# Do not change:
Severity=0	# Severity (0='' 1=feature 2=minor 3=major 4=block 5=crash
Priority=0	# Priority (0=no 1=low 2=normal 3=high 4=urgent 5=immediate
SeverityLevel=('--     ','feature','minor  ','Major  ','BLOCK  ','CRASH  ')
PriorityLevel=('--       ','low      ','normal   ','High     ','URGENT   ','IMMEDIATE')
#print (SeverityLevel[Severity])
#print (PriorityLevel[Priority])

# Time
tm=strftime("%a, %d %b %Y %H:%M:%S", localtime())		# Determine system time
tme=strftime("_%b_%d_%Y_%H.%M", localtime())			# Time for fileheader History log file
	
# Determine station ID and station type
StationType = 0
Core = 1
Remote = 2
International = 3
StIDlist = os.popen3('hostname -s')[1].readlines()		# Name of the station
StID = str(StIDlist[0].strip('\n'))
if debug: print ('StationID = %s' % StID)
if StID in InternationalStations: StationType = International	# International station
if StID in RemoteStations: StationType = Remote			# Remote Station
if StID in CoreStations: StationType = Core			# Core Station
if debug: print ('StationType = %d' % StationType)
if StationType == 0: print ('Error: StationType = %d (Unknown station)' % StationType)

# Path
if os.path.exists('/globalhome'): 
	print('ILT mode')
	if StationType == International: 
		RSPgoldfile=('/misc/home/etc/stationtest/gold/rsp_version_int.gold')
		TBBgoldfile=('/misc/home/etc/stationtest/gold/tbb_version_int.gold')
		TDS=[0,4,8,12,16,20]
	else: 
		RSPgoldfile=('/misc/home/etc/stationtest/gold/rsp_version.gold')
		TBBgoldfile=('/misc/home/etc/stationtest/gold/tbb_version.gold')
		TDS=[0,4,8]
	TBBmgoldfile=('/misc/home/etc/stationtest/gold/tbb_memory.gold')
	#LogPath=('/misc/home/log/')
	TestLogPath=('/misc/home/log/')	# Logging remote (on Kis001)
	#TestLogPath=('/opt/stationtest/data/')	# Logging local (on station)

else: 
	print('Local mode')
	if StationType == International: 
		RSPgoldfile=('/opt/stationtest/gold/rsp_version_int.gold')
		TBBgoldfile=('/opt/stationtest/gold/tbb_version_int.gold')
		TDS=[0,4,8,12,16,20]
	else: 
		RSPgoldfile=('/opt/stationtest/gold/rsp_version.gold')
		TBBgoldfile=('/opt/stationtest/gold/tbb_version.gold')
		TDS=[0,4,8]
	TBBmgoldfile=('/opt/stationtest/gold/tbb_memory.gold')
	#LogPath=('/misc/home/log/')
	#TestLogPath=('/misc/home/log/')	# Logging remote (on Kis001)
	TestLogPath=('/opt/stationtest/data/')	# Logging local (on station)
	
#HistLogPath=('/opt/stationtest/data/')	# Logging local (on station)
HistLogPath=('/localhome/stationtest/data/')	# Logging local (on station)

TestlogName = ('%sstationtest_%s.tmp' % (TestLogPath, StID))
TestlogNameFinalized = ('%sstationtest_%s.log' % (TestLogPath, StID))
HistlogName = ('%sstationtest_%s%s.log' % (HistLogPath, StID, tme))

# Array om bij te houden welke Tiles niet RF getest hoeven worden omdat de modems niet werken.
if len(sys.argv) < 3 :
	if StationType == International:
        	num_rcu=192
	else:
		num_rcu=96
else :
	num_rcu = int(sys.argv[2])
ModemFail=[0 for i in range (num_rcu/2)]
if debug: print ModemFail

#print (TestlogName)
#print (TestlogNameFinalized)

# Parse command line for station ID
#
# -v  0 : only PASSED or FAILED
# -v  1 : overall title
# -v 11 : result per test
# -v 21 : title per test

op = OptionParser(usage='usage: python %prog [options]', version='%prog 0.1')

op.add_option('-v', type='int', dest='verbosity',
  help='Verbosity level',default=11)

#op.add_option('-r', type='int', dest='rsp_nr',
#  help='Provide number of rsp boards that will be used in this test',default=None)
#op.add_option('-t', type='int', dest='tbb_nr',
#  help='Provide number of tbb boards that will be used in this test',default=None)

opts, args = op.parse_args()
opts.rsp_nr=12		# fixed number
opts.tbb_nr=6		# Fixed number
if (StationType == Core or StationType == Remote):		# NL station doe have 12 rsp's and 6 TBB's
	opts.rsp_nr=12		# fixed number
	opts.tbb_nr=6		# Fixed number
	noTBB=6
if StationType == International:				# INT station doe have 24 rsp's and 12 TBB's
	opts.rsp_nr=24		# fixed number
	opts.tbb_nr=12		# Fixed number
	noTBB=12
if debug: print ('RSPs = %d' % opts.rsp_nr)
if debug: print ('TBBs = %d' % opts.tbb_nr)

# - Option checks and/or reformatting
if opts.rsp_nr==None:
  op.error('Option -r must specify the number of rsp boards')
if opts.tbb_nr==None:
  op.error('Option -t must specify the number of tbb boards')
if opts.rsp_nr == 4:
        RspBrd = 'rsp0,rsp1,rsp2,rsp3'
        SubBrd = 'rsp0'
        SubRck = 'sub0'      
if opts.rsp_nr == 12:
        RspBrd = 'rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11'
        SubBrd = 'rsp0,rsp4,rsp8'
        SubRck = 'sub0,sub1,sub2'  
if opts.rsp_nr == 24:
        RspBrd = 'rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11,rsp12,rsp13,rsp14,rsp15,rsp16,rsp17,rsp18,rsp19,rsp20,rsp21,rsp22,rsp23'
        SubBrd = 'rsp0,rsp4,rsp8,rsp12,rsp16,rsp20'
        SubRck = 'sub0,sub1,sub2,sub3,sub4,sub5'
if debug: print ('RspBrd = %s' % RspBrd)

# Define subrack testlog class for pass/fail and logging
vlev = opts.verbosity
testId = ''
appLev = False
logName = '/opt/stationtest/data/STAT-%05d-%05d.dat' % (opts.rsp_nr, opts.tbb_nr)
cli.command('rm -f /opt/stationtest/data/STAT-%05d-%05d.dat', appLev) 
sr = testlog.Testlog(vlev, testId, logName)

sr.setResult('PASSED')

sr.setId('Station - ')
sr.appendLog(11,'')
sr.appendLog(1,'Station production test %s' % logName)
sr.appendLog(11,'')

# Define station testlog
st_log = file(TestlogName, 'w')
st_log.write('StID  >: %s\n' % StID)
st_log.write('Lgfl  >: %s\n' % TestlogNameFinalized)
st_log.write('Time  >: %s\n' % tm)
#cli.command('rm -f %s' % logName, appLev)
#cli.command('rm -f /opt/stationtest/data/stationtest.log', appLev)
#sr.setId('')
#sr.appendLog(11,'')
#sr.appendLog(1,'Lgfl  >: %s' % logNameFinalized)
#sr.appendLog(1,'StID  >: %s' % StID)
#sr.appendLog(1,'Time  >: %s' % tm)

#time.sleep(20)

################################################################################
# Function CheckTBB : CHeck if TBB's are running. The returned string 
# "V 0.3  V 4.7  V 2.4  V 2.9" Shouls have 4 times 'V'

def CheckTBB():
    SeverityOfThisTest=3
    PriorityOfThisTest=3

    global Severity
    global Priority
    print 'Checking TBB!!!'
    print 'wait 60 sec'
    time.sleep(60)
    if debug: print int(len(os.popen3('tbbctl --version')[1].readlines()))
    sr.setId('TBB   >: ')
    n=0 # Maximum itteration
    while len(os.popen3('tbbctl --version')[1].readlines()) < 4:
        print ('-'),
#	if debug: 
	print ('Polling TBB Driver')
        time.sleep(5)
	n+=1
	if n > 12:
		sr.appendLog(11,'Error: TBB driver is not running or some TBBs not active')
		sr.setResult('FAILED')
# store station testlog			
		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('TBB   >: Sv=%s Pr=%s, Error: TBB driver is not running\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
		return

    n=0                                                   # Check till 4 V's per TBB
    while n < 12:                                               # maximum itterations
        res2 = os.popen3('tbbctl --version')[1].readlines()
        if debug:
	    for line in res2:
	        print ('%s' % line.rstrip('\n'))
            #print ('res2 is: %s' % res2)
            #print ('res2[9] is: %s' % res2[9])
            print ('Itteration %d' % n)
        else:
            print '*',
        cnt=0
        TBBrange=range(noTBB)
        for TBBnr in TBBrange:
            cnt += res2[9+TBBnr].count('V')         # count number of 'V's (Version)
        if cnt == noTBB*4:                                               # 4 per TBB
            print "TBB's OK"
            break
        n+=1
        time.sleep(5)
    else:
        for TBBnr in TBBrange:
            if res2[9+TBBnr].count('V') != 4:                           # Log Errors
                sr.appendLog(11,'Error: TBB :%s' % str(res2[9+TBBnr].strip('\n')))
		sr.setResult('FAILED')
# store station testlog			
		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('TBB   >: Sv=%s Pr=%s, Error: TBB : %s\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], str(res2[9+TBBnr].strip('\n'))))
                #print ('number of Vs is ', res2[9+TBBnr].count('V')),
                #print (' Error in TBB : %s' % res2[9+TBBnr])
    if debug:
        print 'stopped Checking TBB'
        print ("number of V's is %d" % cnt)
    return

################################################################################
# Function Goto Swlevel 2

def GotoSwlevel2():
	res = os.popen3('swlevel 1')[1].readlines()
	if debug:
		print('System is Going to swlevel 1')
#		for line in res:
#			print ('To swlevel 1 ', line)	# werkt niet!!???
	time.sleep(5)

# Set swlevel 2 if not running
#
	res = os.popen3('swlevel')[1].readlines()
#print res[1]
	if len(res) > 0:
		for line in res:
			if debug: print ('%s' % line.rstrip('\n'))
			if line == ('2 : RSPDriver                 DOWN\n') or line == ('2 : TBBDriver                 DOWN\n'):
				print 'System is Going to swlevel 2'

#            errorprg = os.system('swlevel 2')
 #           if len(err) > 0:
 #           else:
#            print fromprg.readlines()

				res2 = os.popen3('swlevel 2')[1].readlines()
#            print  errorprg
				print 'wait 120 sec'
				if debug:
					for line in res2:
						print ('%s' % line.rstrip('\n'))
				time.sleep(120)
				res = os.popen3('rspctl --datastream=0')[1].readlines()
				print res
				#time.sleep(90)  # Tijdelijk toe gevoegd voor nieuwe tbbdriver. Deze loopt vast tijdens pollen
#				CheckTBB()	# Tijdelijk weg gelaten voor nieuwe tbbdriver. Deze loopt vast tijdens pollen
#fromprg.close()
				break
	return
#res.close()

################################################################################
# Check ntpd time demon
#
def CheckNtpd():
	SeverityOfThisTest=3
	PriorityOfThisTest=3
	
	global Severity
	global Priority
	print ('Check of the Ntpd!')
	sr.setId('Clock   - ')
	res = os.popen3('/usr/sbin/ntpq -p')[1].readlines()
	#res = os.popen3('/opt/stationtest/test/timing/ntpd.sh')[1].readlines()
	if debug:
		for line in res:
	        	print ('-%s' % line.rstrip('\n'))
	#print ('res : %s' % res)

	if len(res) > 0:
#		print (res[3])
		offset=0
		for line in res:
			if debug: print('line= %s' % line)
			locallock=line.find('*LOCAL(0)')
			if locallock==0: break
			gpslock=line.find('*GPS_ONCORE(0)')
			if gpslock==0:
				offset=float((line.split())[8])
				break
		if debug:
#			print ('res[3] is: %s' % res[3])
#			print ('res[4] is: %s' % res[4])
			print ('gpslock is %s' % gpslock)
			print ('locallock is %s' % locallock)
			print ('offset is %.3f' % offset)

		if gpslock > -1:
	        	if debug: print('GPS in Lock. OK')
		else:
			if locallock > -1:
				sr.appendLog(11,'Clock locked on Local Clock!!')
				if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
				if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
				st_log.write('Clock >: Sv=%s Pr=%s, Clock locked on Local Clock!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
				sr.setResult('FAILED')
			else: 
				sr.appendLog(11,'Clock out of sync!!')
				if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
				if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
				st_log.write('Clock >: Sv=%s Pr=%s, Clock out of sync!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
				sr.setResult('FAILED')
		if offset < -clkoffset or offset > clkoffset:
			if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
			if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
			st_log.write('Clock >: Sv=%s Pr=%s, Clock Offset to large : %.3f\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], offset))
			sr.appendLog(11,'Clock Offset to large : %.3f' % offset)
			sr.setResult('FAILED')
	else:
		sr.appendLog(11,'no answer from ntpq!')
		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('Clock >: Sv=%s Pr=%s, no answer from ntpq!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
		sr.setResult('FAILED')
	return

################################################################################
# Function Check RSP status bytes
#
def CheckRSPStatus():
#	debug = 1
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	
	global Severity
	global Priority
	
	sr.setId('RSPst >: ')
	print ('Check RSP Status')
	OutputClock,PLL160MHz,PLL200MHz=gettdstatus() # td-status
	time.sleep(1)
	res = os.popen3('rspctl --status')[1].readlines()
	#print res[1]
	linecount=0
	if len(res) > 0:
		for line in res:
			sync=line.find('RSP[ 0] Sync')
			if sync==0: break
			linecount+=1
	#print 'sync = ' + str(sync) + ' and linecount = ' + str(linecount)
	for rsp in range(opts.rsp_nr):
	#	print res[linecount+rsp]
	#	x = res[linecount+rsp].split( )
	#	print res[linecount+rsp*5].lstrip('RSP').strip('[').split()
		if debug: 
			print '\n',
			print res[linecount+rsp*5],
		for sync in range(1, 5):
			dif = res[linecount+rsp*5+sync].lstrip('RSP').strip('[').strip(':').split()
			if debug:
				print ('Dif = %s' % dif)
				#print str(linecount+rsp*5+sync),
				#print dif[2]
			if dif[2] not in ('0', '512'):		# was ('0', '1', '512', '513'):
				#if debug: print ('RSP : %d status error:  sync = %d, diff = %d' % (int(rsp), int(sync), int(dif[2])))
				sr.appendLog(11,'RSP : %d status error:  sync = %d diff = %d' % (int(rsp), int(sync), int(dif[2])))
				sr.setResult('FAILED')
				
				if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
				if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
				st_log.write('RSPst >: Sv=%s Pr=%s, RSP : %d AP%d status error at %s MHz:  diff = %d\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], int(rsp), int(dif[1].strip(':')), OutputClock, int(dif[2])))
				sr.setResult('FAILED')
			
	#time.sleep(3)
#	debug = 0
	return
				
################################################################################
# Function check if clock 160 MHz is locked
#
def CheckTDSStatus160():
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	
	global Severity
	global Priority

	sr.setId('TDSst >: ')
#	TDS=[0,4,8]
	if debug: print('TDS = ',TDS)
	
	if StationType == International:
		LockCount160=[0 for i in range (21)]
	else:
		LockCount160=[0 for i in range (9)]
	if debug: print('LockCount160 = ',LockCount160)

	PLL160MHz = '?'
	PLL200MHz = '?'
	res = os.popen3('rspctl --clock=160')[1].readlines()
	print ('Clock set to 160MHz')
	time.sleep(1)
	n=0                                                   # Wait till clock set
    	while n < 15:                                         # maximum itterations
		OutputClock,PLL160MHz,PLL200MHz=gettdstatus() # td-status
		if PLL160MHz=='LOCKED': 
			print ('Clock %s' %(PLL160MHz))
			break
#		print ('OutputClock = ',OutputClock)
#		print ('PLL160MHz = ',PLL160MHz)
#		print ('PLL200MHz = ',PLL200MHz)
		n+=1
		time.sleep(5)
		if n==15:
			print ('Clock never locked')
#			if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
#			if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
#			st_log.write('TDSst >: Sv=%s Pr=%s, TDS : all @ 160MHz never locked:  PLL200MHz = %s, PLL160MHz = %s, Output Clock = %s\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], PLL200MHz, PLL160MHz, OutputClock))
			sr.setResult('FAILED')
		
#	if n < 15:
	for TDSBrd in TDS:
#		print('TDSBrd = ',TDSBrd)
		LockCount160[TDSBrd]==0
		if debug: print('LockCount160[%s] = %s' % (TDSBrd,LockCount160[TDSBrd]))

	n=0						# Check if clock is LOCKED every 2 seconds for 10 times!
	while n < 10:
		n+=1
		for TDSBrd in TDS:
			valid=0
			PLL160MHz = '?'
			PLL200MHz = '?'
#			print('TDSBrd = ',TDSBrd)
			res = os.popen3('rspctl --tdstatus --sel=%s'%(TDSBrd))[1].readlines()
			if debug: print res[0]
			for line in res:
				if line[0] == 'R': 
					valid=1
					if debug: print ('valid tdstatus')
			#print res[0].split()
			if valid == 1: 
				for line in res:
					if line[0] == 'R':		# Check of regel geldig is!
						header=line.replace('|',' ').split()
						if debug: print ('header = ', header)
					else:		# Check of regel geldig is!
						status=line.replace('|',' ').replace('not locked','notlocked').split()
						if debug: 
							print ('status= ', status)
							print ('OutputClock = ',status[2])
							print ('PLL160MHz = ',status[4])
							print ('PLL200MHz = ',status[5])
						OutputClock = status[2]
						PLL160MHz = status[4]
						PLL200MHz = status[5]
				if PLL160MHz <> 'LOCKED':
					LockCount160[TDSBrd] += 1					# store station testlog		
#					print('LockCount160[TDSBrd] = ',LockCount160[TDSBrd])
					if LockCount160[TDSBrd] == 1:						# Store Error at the first time
						print ('Clock 160MHz not locked')
						if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
						if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
						st_log.write('TDSst >: Sv=%s Pr=%s, TDS : %s @ 160MHz not locked:  PLL200MHz = %s, PLL160MHz = %s, Output Clock = %s\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], TDSBrd, PLL200MHz, PLL160MHz, OutputClock))
						sr.setResult('FAILED')
				if (n==10 and LockCount160[TDSBrd]<>0):							# Store number of Errors only at the last time first time
					st_log.write('TDSlt >: Sv=%s Pr=%s, TDS : %s @ 160MHz Did go wrong %s out of 10 times\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], TDSBrd, LockCount160[TDSBrd]))
		time.sleep(1)			
	return
				
################################################################################
# Function check if clock 200 MHz is locked
#
def CheckTDSStatus200():
#	debug = 1
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	
	global Severity
	global Priority

	sr.setId('TDSst >: ')
	if debug: print('TDS = ',TDS)
	if StationType == International:
		LockCount200=[0 for i in range (21)]
	else:
		LockCount200=[0 for i in range (9)]
	if debug: print('LockCount200 = ',LockCount200)

	PLL160MHz = '?'
	PLL200MHz = '?'
	res = os.popen3('rspctl --clock=200')[1].readlines()
	print ('Clock set to 200MHz')
	time.sleep(1)
	n=0                                                   # Wait till clock set
    	while n < 15:                                         # maximum itterations
		OutputClock,PLL160MHz,PLL200MHz=gettdstatus() # td-status
		if PLL200MHz=='LOCKED': 
			print ('Clock %s' %(PLL200MHz))
			break
#		print ('OutputClock = ',OutputClock)
#		print ('PLL160MHz = ',PLL160MHz)
#		print ('PLL200MHz = ',PLL200MHz)
		n+=1
		time.sleep(5)
		if n==15:
			print ('Clock never locked')
#			if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
#			if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
#			st_log.write('TDSst >: Sv=%s Pr=%s, TDS : all @ 200MHz not locked:  PLL200MHz = %s, PLL160MHz = %s, Output Clock = %s\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], PLL200MHz, PLL160MHz, OutputClock))
			sr.setResult('FAILED')

#	if n < 15:	
	for TDSBrd in TDS:
#		print('TDSBrd = ',TDSBrd)
		LockCount200[TDSBrd]==0
#		print('LockCount200[TDSBrd] = ',LockCount200[TDSBrd])

	n=0						# Check if clock is LOCKED every 2 seconds for 10 times!
	while n < 10:
		n+=1
		for TDSBrd in TDS:
			valid=0
			PLL160MHz = '?'
			PLL200MHz = '?'
#			print('TDSBrd = ',TDSBrd)
			res = os.popen3('rspctl --tdstatus --sel=%s'%(TDSBrd))[1].readlines()
			if debug: print res[0]
			for line in res:
				if line[0] == 'R': 
					valid=1
					if debug: print ('valid tdstatus')
			#print res[0].split()
			if valid == 1: 
				for line in res:
					if line[0] == 'R':		# Check of regel geldig is!
						header=line.replace('|',' ').split()
						if debug: print ('header = ', header)
					else:		# Check of regel geldig is!
						status=line.replace('|',' ').replace('not locked','notlocked').split()
						if debug: 
							print ('status= ', status)
							print ('OutputClock = ',status[2])
							print ('PLL160MHz = ',status[4])
							print ('PLL200MHz = ',status[5])
						OutputClock = status[2]
						PLL160MHz = status[4]
						PLL200MHz = status[5]
				if PLL200MHz <> 'LOCKED':
					LockCount200[TDSBrd] += 1					# store station testlog		
#					print('LockCount200[TDSBrd] = ',LockCount200[TDSBrd])
					if LockCount200[TDSBrd] == 1:						# Store Error at the first time
						print ('Clock 200MHz not locked')
						if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
						if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
						st_log.write('TDSst >: Sv=%s Pr=%s, TDS : %s @ 200MHz not locked:  PLL200MHz = %s, PLL160MHz = %s, Output Clock = %s\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], TDSBrd, PLL200MHz, PLL160MHz, OutputClock))
						sr.setResult('FAILED')
				if (n==10 and LockCount200[TDSBrd]<>0):							# Store number of Errors only at the last time first time
					st_log.write('TDSlt >: Sv=%s Pr=%s, TDS : %s @ 200MHz Did go wrong %s out of 10 times\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], TDSBrd, LockCount200[TDSBrd]))
		time.sleep(1)	
#	debug = 0	
	return
				
################################################################################
# Function get the TD status
#
def gettdstatus():
	res = os.popen3('rspctl --tdstatus --sel=0')[1].readlines()
	for line in res:
		if line[0] == 'R':		# Check of regel geldig is!
			header=line.replace('|',' ').split()
			#print ('header = ', header)
		else:		# Check of regel geldig is!
			status=line.replace('|',' ').replace('not locked','notlocked').split()
			#print ('status= ', status)
			#print ('OutputClock = ',status[2])
			#print ('PLL160MHz = ',status[4])
			if debug: print ('PLL160MHz = %s, PLL200MHz = %s' % (status[4],status[5]))
			OutputClock = status[2]
			PLL160MHz = status[4]
			PLL200MHz = status[5]
	return OutputClock,PLL160MHz,PLL200MHz
				
################################################################################
# Function make RSP Version gold
#
def makeRSPVersionGold():
	res = os.popen3('rspctl --version')[1].readlines()
	time.sleep(3)
	if StationType == International:
		f_log = file('/misc/home/etc/stationtest/gold/rsp_version-int.gold', 'w')
	else:
		f_log = file('/misc/home/etc/stationtest/gold/rsp_version.gold', 'w')
	for line in res:
		print ('Res = ', line)
	        f_log.write(line)
	print ('RSP Version Gold file has been made!')
	return

################################################################################
# Function read RSP Version gold
#
def readRSPVersionGold():
	f=open(RSPgoldfile,'rb')
#	if debug: 
#		for line in f:
#			print ('Res = ', line)
	return f

################################################################################
# Function Check RSP Version
#
def CheckRSPVersion():
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	
	global Severity
	global Priority
	
	sr.setId('RSPver>: ')
	sr.appendLog(21,'')
	sr.appendLog(21,'### Verify LCU - RSP ethernet link by getting the RSP version info')
	sr.appendLog(21,'')	
	print ('Check RSP Version')
#	RSPgold=readRSPVersionGold()
	RSPgold = open(RSPgoldfile,'r').readlines()			# Read RSP Version gold
	RSPversion = os.popen3('rspctl --version')[1].readlines()	# Get RSP Versions
#	res = cli.command('./rsp_version.sh')
	if debug:
		for RSPnumber in range(len(RSPgold)):
			if RSPgold[RSPnumber] == RSPversion[RSPnumber]: print ('RSP OK = ', RSPnumber)
			else: print ('RSPNOK = ', RSPnumber)
# store subreck testlog			
	for RSPnumber in range(len(RSPgold)):
		if RSPgold[RSPnumber] != RSPversion[RSPnumber]: 
			sr.appendLog(11,'>>> RSP version test went wrong')
			#sr.appendLog(11,'CLI:')
			#sr.appendLog(11,res,1,1,1)
			sr.appendLog(11,'Result:')
			for line in RSPversion:
				#print ('%s' % line.rstrip('\n'))
				sr.appendLog(11,'%s' % line.rstrip('\n'))
			sr.appendLog(11,'Expected:')
			for line in RSPgold:
				#print ('%s' % line.rstrip('\n'))
				sr.appendLog(11,'%s' % line.rstrip('\n'))
			sr.setResult('FAILED')
			break
		
# store station testlog	
	for RSPnumber in range(len(RSPgold)):
		if RSPgold[RSPnumber] != RSPversion[RSPnumber]:
			if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
			if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
			st_log.write('RSPver>: Sv=%s Pr=%s, BP/AP Error! %s' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], RSPversion[RSPnumber]))
			sr.setResult('FAILED')
			if debug: print ('RSPNOK = ', RSPnumber)
	return

################################################################################
# Function make TBB Version gold
#
def makeTBBVersionGold():
	res = os.popen3('tbbctl --version')[1].readlines()
	time.sleep(3)
	f_log = file(TBBgoldfile, 'w')
	for line in res:
		print ('Res = ', line)
	        f_log.write(line)
	print ('TBB Version Gold file has been made!')
	return
	
################################################################################
# Function Check TBB Version
#
def CheckTBBVersion():
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	
	global Severity
	global Priority
	
	sr.setId('TBBver>: ')
	sr.appendLog(21,'')
	sr.appendLog(21,'### Verify LCU - TBB ethernet link by getting the TBB version info')
	sr.appendLog(21,'')	

	TBBgold = open(TBBgoldfile,'r').readlines()			# Read TBB Version gold
	TBBversion = os.popen3('tbbctl --version')[1].readlines()	# Get TBB Versions
	time.sleep(1)

	if len(TBBversion) < 4:
	# store station testlog			
		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('TBB   >: Sv=%s Pr=%s, Error: TBB driver is not running\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
		print ('Returned message from TBBversion: %s' % TBBversion)
		return
			
	if debug:
		print ('TBBgold: %s' % TBBgold)
		print ('TBBversion: %s' % TBBversion)
		for TBBnumber in range(len(TBBgold)):
			if TBBgold[TBBnumber] == TBBversion[TBBnumber]: print ('TBB OK = ', TBBnumber)
			else: print ('TBBNOK = ', TBBnumber)
# store subreck testlog			
	for TBBnumber in range(len(TBBgold)):
		if TBBgold[TBBnumber] != TBBversion[TBBnumber]: 
			sr.appendLog(11,'>>> TBB version test went wrong')
			#sr.appendLog(11,'CLI:')
			#sr.appendLog(11,res,1,1,1)
			sr.appendLog(11,'Result:')
			for line in TBBversion:
				#print ('%s' % line.rstrip('\n'))
				sr.appendLog(11,'%s' % line.rstrip('\n'))
			sr.appendLog(11,'Expected:')
			for line in TBBgold:
				#print ('%s' % line.rstrip('\n'))
				sr.appendLog(11,'%s' % line.rstrip('\n'))
			sr.setResult('FAILED')
			break
		
# store station testlog	
	for TBBnumber in range(len(TBBgold)):
		if TBBgold[TBBnumber] != TBBversion[TBBnumber]:
			if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
			if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
			st_log.write('TBBver>: Sv=%s Pr=%s, TP/MP Error! %s' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], TBBversion[TBBnumber]))
			sr.setResult('FAILED')
			if debug: print ('TBBNOK = ', TBBnumber)
	return

################################################################################
# Function Check TBB Version				Eventueel nog toevoegen!
#
#sr.setId('TBB version - ')
#sr.appendLog(21,'')
#sr.appendLog(21,'### Verify LCU - TBB ethernet link by getting the TBB version info')
#sr.appendLog(21,'')
#res = cli.command('./tbb_version.sh')
#if res.find('OK')==-1:
#  sr.appendLog(11,'>>> TBB version test went wrong')
#  sr.appendLog(11,'CLI:')
#  sr.appendLog(11,res,1,1,1)
#  sr.appendLog(11,'Result:')
#  sr.appendFile(11,'tbb_version.log')
#  sr.appendLog(11,'Expected:')
#  sr.appendFile(11,'gold/tbb_version.gold')
#  sr.setResult('FAILED')
#else:
#  sr.appendLog(11,'>>> TBB version test went OK')

################################################################################
# Function make TBB Memory gold
#
def makeTBBMemGold():
	res = os.popen3('./tbb_memory.sh')[1].readlines()
	time.sleep(3)
	f_log = file(TBBmgoldfile, 'w')
	for line in res:
		print ('Res = ', line)
	        f_log.write(line)
	print ('TBB Memory Gold file has been made!')
	return
	
################################################################################
# Function Check TBB Memory
#
def CheckTBBMemory():
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	
	global Severity
	global Priority
	
	sr.setId('TBBmem>: ')
	sr.appendLog(21,'')
	sr.appendLog(21,'### Verify TBB memory modules on the TBB')
	sr.appendLog(21,'')	
	
	print ('TBB Memory check')

	TBBmgold = open(TBBmgoldfile,'r').readlines()		# Read TBB Memory gold
	TBBmem = os.popen3('./tbb_memory.sh')[1].readlines()	# Start TBB memory test
#	res = cli.command('./tbb_version.sh')
	if debug:
		for TBBnumber in range(len(TBBmgold)):
			if TBBmgold[TBBnumber] == TBBmem[TBBnumber]: print ('TBB OK = ', TBBnumber)
			else: print ('TBBNOK = ', TBBnumber)
# store subreck testlog			
#	for TBBnumber in range(len(TBBgold)):
#		if TBBgold[TBBnumber] != TBBversion[TBBnumber]: 
#			sr.appendLog(11,'>>> TBB version test went wrong')
#			#sr.appendLog(11,'CLI:')
#			#sr.appendLog(11,res,1,1,1)
#			sr.appendLog(11,'Result:')
#			for line in TBBversion:
#				#print ('%s' % line.rstrip('\n'))
#				sr.appendLog(11,'%s' % line.rstrip('\n'))
#			sr.appendLog(11,'Expected:')
#			for line in TBBgold:
#				#print ('%s' % line.rstrip('\n'))
#				sr.appendLog(11,'%s' % line.rstrip('\n'))
#			sr.setResult('FAILED')
#			break
		
# store station testlog	
	for TBBnumber in range(len(TBBmgold)):
		if TBBmgold[TBBnumber] != TBBmem[TBBnumber]:
			if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
			if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
			st_log.write('TBBmem>: Sv=%s Pr=%s, BP/AP Error! %s' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], TBBmem[TBBnumber]))
			sr.setResult('FAILED')
			if debug: print ('TBBNOK = ', TBBnumber)
	return

################################################################################
# Function Check TBB Memory			Nog testen met defect TBB board!
#
def CheckTBBMemoryOrg():
	sr.setId('TBBmem>: ')
	sr.appendLog(21,'### Verify TBB memory modules on the TBB')
#	linecount=0
#	TBBmemory = os.popen3('./tbb_memory')[1].readlines()	# Get RSP Versions
#	if len(TBBmemory) > 0:
#		for line in TBBmemory:
#			lineContainingTBBmemory=line.find('TBB memory')
#			if lineContainingTBBmemory==0: break
#			linecount+=1
#			
#	if debug:
#		
#	for tbb in range(opts.tbb_nr):
#		print TBBmemory[linecount+tbb]
#
#	return
			
	res = cli.command('./tbb_memory.sh')
	if res.find('wrong')==-1:
		if debug: print(11,'>>> TBB memory test went OK')
	else:
		sr.appendLog(11,'>>> TBB memory test went wrong')
		sr.appendLog(11,'CLI:')
		sr.appendLog(11,res,1,1,1)
		sr.appendLog(11,'Result:')
		sr.appendFile(11,'tbb_memory.log')
		sr.appendLog(11,'Expected:')
		sr.appendFile(11,'gold/tbb_memory.gold')
		sr.setResult('FAILED')
	return

################################################################################
# Function Check TBB Size			Nog testen met defect TBB board!
#
def CheckTBBSizetmp():
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	
	global Severity
	global Priority
	
	TBBsgold = open(TBBsgoldfile,'r').readlines()		# Read TBB Memory gold
	TBBsze = os.popen3('./tbb_size.sh')[1].readlines()	# Start TBB memory test
#	res = cli.command('./tbb_version.sh')
	if debug:
		for TBBnumber in range(len(TBBsgold)):
			if TBBsgold[TBBnumber] == TBBsze[TBBnumber]: print ('TBB OK = ', TBBnumber)
			else: print ('TBBNOK = ', TBBnumber)
# store station testlog	
	for TBBnumber in range(len(TBBsgold)):
		if TBBsgold[TBBnumber] != TBBsze[TBBnumber]:
			if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
			if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
			st_log.write('TBBsze>: Sv=%s Pr=%s, TBBSize Error! %s' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], TBBsze[TBBnumber]))
			sr.setResult('FAILED')
			if debug: print ('TBBNOK = ', TBBnumber)
	return	
	
################################################################################
# Function Check TBB Size			Nog testen met defect TBB board!
#
def CheckTBBSize():
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	
	global Severity
	global Priority
	
	sr.setId('TBBsze>: ')
	sr.appendLog(21,'### Verify the size of the TBB memory modules')
	res = cli.command('./tbb_size.sh')
	if res.find('wrong')==-1:
		#sr.appendLog(11,'>>> TBB size test went OK')
		if debug: print(11,'>>> TBB size test went OK')
	else:
		sr.appendLog(11,'>>> TBB size test went wrong')
		sr.appendLog(11,'CLI:')
		sr.appendLog(11,res,1,1,1)
		sr.appendLog(11,'Result:')
		sr.appendFile(11,'tbb_size.log')
		sr.appendLog(11,'Expected:')
		sr.appendFile(11,'gold/tbb_size.gold')
		sr.setResult('FAILED')

# store station testlog			
		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('TBBsze>: Sv=%s Pr=%s, TBB size test went wrong!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
	return

################################################################################
# Function Pseudo Random TBB Test		Nog testen met defect TBB board!
#
def PseudoRandomTBBTest():
	sr.setId('PsRndT>: ')
	sr.appendLog(21,'### Verify the RCU - RSP - TBB LVDS interfaces by capturing pseudo random data on TBB')
	res = cli.command('./tbb_prbs_tester.sh')
	if res.find('wrong')==-1:
		#sr.appendLog(11,'>>> RCU - RSP - TBB LVDS interfaces test went OK')
		if debug: print(11,'>>> RCU - RSP - TBB LVDS interfaces test went OK')
	else:
		sr.appendLog(11,'>>> RCU - RSP - TBB LVDS interfaces went wrong')
		sr.appendLog(11,'CLI:')
		sr.appendLog(11,res,1,1,1)
		sr.setResult('FAILED')
	return

################################################################################
# Function CHeck SPU status			Nog testen met defect SPU board!
#
def CheckSPUStatus():
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	
	global Severity
	global Priority
	
	sr.setId('SPUst >: ')
	sr.setId('SPU status - ')
	sr.appendLog(21,'### Verify the RSP - SPU I2C interface by reading the SPU sensor data')
	res = cli.command('python i2c_spu.py --sub %s --rep 1 -v 11' %(SubRck,))
	res = cli.command('python i2c_spu.py --sub sub0,sub1,sub2')
	if res.find('FAILED')==-1:
		#sr.appendLog(11,'>>> RSP - SPU I2c interface test went OK')
		if debug: print(11,'>>> RSP - SPU I2c interface test went OK')
	else:
		sr.appendLog(11,'>>> RSP - SPU I2c interface test went wrong')
#		sr.appendLog(11,'CLI:')
#		sr.appendLog(11,res,1,1,1)
#		sr.appendLog(11,'Result:')
		sr.appendFile(11,'spustat.log')
		sr.setResult('FAILED')
		
# store station testlog			
		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('SPUst >: Sv=%s Pr=%s, RSP - SPU I2c interface test went wrong!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
	return


################################################################################
# Function CHeck RSP TD interface		Nog testen met defect interface!
#
def CheckRSPTdI2C():
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	
	global Severity
	global Priority
	
	sr.setId('RSPTD >: ')
	sr.appendLog(21,'### Verify the RSP - TD I2C interface by reading the TD sensor data')
	res = cli.command('python i2c_td.py --brd %s' %(SubBrd,))
	if debug: print('res = %s' % res)
	if res.find('FAILED')==-1:
		#sr.appendLog(11,'>>> RSP - TD I2c interface test went OK')
		if debug: print(11,'>>> RSP - TD I2c interface test went OK')
	else:
		sr.appendLog(11,'>>> RSP - TD I2c interface test went wrong')
#		sr.appendLog(11,'CLI:')
#		sr.appendLog(11,res,1,1,1)
#		sr.appendLog(11,'Result:')
		sr.appendFile(11,'tdstat.log')
		sr.setResult('FAILED')
		
# store station testlog			
		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('RSPTD >: Sv=%s Pr=%s, RSP - TD I2c interface test went wrong!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
	return

################################################################################
# Function Built in self test RSP		  Nog testen op een defecte RSP!
#
def Bist():
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	
	global Severity
	global Priority
	
	sr.setId('Bist  >: ')
	sr.appendLog(21,'### Build In Self Test (BIST)')
	res = cli.command('python verify.py --brd %s --rep 1 -v 21 --te tc/bist.py' %(RspBrd,))
	if debug: print('res = %s' % res)
	if res.find('wrong')==-1:
		#sr.appendLog(11,'>>> BIST went OK')
		if debug: print(11,'>>> BIST went OK')
		sr.appendLog(21,'tc/bist.log')
	else:
		sr.appendLog(11,'>>> BIST went wrong')
#		sr.appendLog(11,'CLI:')
#		sr.appendLog(11,res,1,1,1)
		sr.appendLog(11,'tc/bist.log')
		sr.appendLog('FAILED')
		
# store station testlog			
		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('Bist  >: Sv=%s Pr=%s, BIST went wrong!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
	return

################################################################################
# Function Pseudo Random RSP Test		  Nog testen op een defecte RSP!
#
def PseudoRandomRSPTest():
	sr.setId('PsRndR>: ')
	sr.appendLog(21,'### Verify the RCU -> RSP LVDS interfaces by capturing pseudo random data on RSP')
	res = cli.command('python verify.py --brd %s --fpga blp0,blp1,blp2,blp3 --pol x,y --rep 1 -v 11 --te tc/prsg.py' %(RspBrd,)) 
	if res.find('wrong')==-1:
		#sr.appendLog(11,'>>> RCU-RSP interface test went OK')
		if debug: print(11,'>>> RCU-RSP interface test went OK')
		sr.appendFile(21,'tc/prsg.log')
	else:
		sr.appendLog(11,'>>> RCU-RSP interface test went wrong')
#		sr.appendLog(11,'CLI:')
#		sr.appendLog(11,res,1,1,1)
		sr.appendFile(11,'tc/prsg.log')
		sr.setResult('FAILED')
	return

################################################################################
# Function RCU - HBA modem test                       Nog testen met defecte RCU
#
def RCUHBAModemTest():
	sr.setId('RCUHBm>: ')
	sr.appendLog(21,'### Verify the control modem on the RCU')
	res = cli.command('python verify.py --brd %s --fpga blp0,blp1,blp2,blp3 --rep 1 -v 11 --te tc/hba_client.py --client_access r --client_reg version --data 10' %(RspBrd,)) 
	if res.find('wrong')==-1:
		#sr.appendLog(11,'>>> RCU-HBA modem test went OK')
		if debug: print(11,'>>> RCU-HBA modem test went OK')
		sr.appendFile(21,'tc/hba_client.log')
	else:
		sr.appendLog(11,'>>> RCU-HBA modem test went wrong')
#		sr.appendLog(11,'CLI:')
#		sr.appendLog(11,res,1,1,1)
		sr.appendFile(11,'tc/hba_client.log')
		sr.setResult('FAILED')
		
# store station testlog	
		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('RCUHBm>: Sv=%s Pr=%s, RCU-HBA modem test went wrong!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
	return

################################################################################
# Function Serdes ring 'off' test	  Nog testen met losse infiniband kabel!
#
def SerdesRingTestOff():
	sr.setId('SerOff>: ')
	sr.appendLog(21,'### Verify the Serdes ring connection between the RSP boards with ring is off')
	cli.command('rspctl --splitter=0')
	res = cli.command('python verify.py --brd %s --rep 1 -v 21 --te tc/serdes.py --diag_sync 0' %(RspBrd,))
	if res.find('wrong')==-1:
		#sr.appendLog(11,'>>> Serdes ring off test went OK')
		if debug: print(11,'>>> Serdes ring off test went OK')
		sr.appendLog(21,'tc/serdes.log')
	else:
		sr.appendLog(11,'>>> Serdes ring off test went wrong')
#		sr.appendLog(11,'CLI:')
#		sr.appendLog(11,res,1,1,1)
		sr.appendLog(11,'tc/serdes.log')
		sr.appendLog('FAILED')
	return

################################################################################
# Function Serdes ring 'on' test	  Nog testen met losse infiniband kabel!
#
def SerdesRingTestOn():
	sr.setId('SerOn >: ')
	sr.appendLog(21,'### Verify the Serdes ring connection between the RSP boards with ring is on')
	cli.command('rspctl --splitter=1')
	res = cli.command('python verify.py --brd %s --rep 1 -v 21 --te tc/serdes.py --diag_sync 0' %(RspBrd,))
	if res.find('wrong')==-1:
		#sr.appendLog(11,'>>> Serdes ring on test went OK')
		if debug: print(11,'>>> Serdes ring on test went OK')
		sr.appendLog(21,'tc/serdes.log')
	else:
		sr.appendLog(11,'>>> Serdes ring on test went wrong')
#		sr.appendLog(11,'CLI:')
#		sr.appendLog(11,res,1,1,1)
		sr.appendLog(11,'tc/serdes.log')
		sr.appendLog('FAILED')
	return

################################################################################
# Function LBA test

# Read directory with the files to processs	
def open_dir(dirname) :		# Sub functions belonging to LBA test and HBA test
	files = filter(os.path.isfile, os.listdir('.'))
	#files.sort(key=lambda x: os.path.getmtime(x))
 	return files

def rm_files(dir_name,file) :
        cmdstr = 'rm -f ' + file
	os.popen(cmdstr)
	return

def rec_stat(dirname,num_rcu) :
	os.popen("rspctl --statistics --duration=1 --integration=1 --select=0:" + str(num_rcu-1) + " 2>/dev/null")
        return

# Open file for processsing
def open_file(files, file_nr) :
        # check if file is data file, no junk
	if files[file_nr][-3:] == 'dat':
		file_name = files[file_nr]
		fileinfo = os.stat(file_name)
		size = int(fileinfo.st_size)
		f=open(file_name,'rb')
		max_frames = size/(512*8)
		frames_to_process=max_frames
    		rcu_nr = int(files[file_nr][-7:-4])		# was [-6:-4]
		#print 'File nr ' + str(file_nr) + ' RCU nr ' + str(rcu_nr) + '  ' + files[file_nr][-6:-4]
	else :
		frames_to_process=0
		f=open(files[file_nr],'rb')
		rcu_nr = 0
	return f, frames_to_process, rcu_nr	

# Read single frame from file	
def read_frame(f):
	sst_data = array.array('d')	  	
	sst_data.fromfile(f,512)
	sst_data = sst_data.tolist()
	return sst_data


# LBA test
def LBAtest():
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	
	debug=0
	
	global Severity
	global Priority
	
	print ('LBA test')
	sr.setId('LBAmd1>: ')
	sub_time=[]
	sub_file=[]
	dir_name = './lbadatatest/' #Work directory will be cleaned
	if not (os.path.exists(dir_name)):
		os.mkdir(dir_name)
	rmfile = '*.log'
	ctrl_string='='
	# read in arguments
	if len(sys.argv) < 2 :
		subband_nr=301
	else :
		subband_nr = int(sys.argv[1])
	if len(sys.argv) < 3 :
		if StationType == International:
			num_rcu=192
		else:
			num_rcu=96
	else :
		num_rcu = int(sys.argv[2])
			
	if debug:
		print ' Dir name is ' + dir_name
		print ' Number of RCUs is ' + str(num_rcu)
	
	# init log file
	f_log = file('/opt/stationtest/test/hbatest/LBA_elements.log', 'w')
	f_log.write(' ************ \n \n LOG File for LBA element test \n \n *************** \n')
	f_logfac = file('/opt/stationtest/test/hbatest/LBA_factors.log', 'w')
	f_loglin = file('/opt/stationtest/test/hbatest/LBA_lin.log', 'w')
	f_logdown = file('/opt/stationtest/test/hbatest/LBA_down.log', 'w')	# log number that indicates if LBA antenna is falen over (down)
# initialize data arrays
	ref_data=range(0, num_rcu)
	meet_data=range(0, num_rcu)
	meet_data_left=range(0, num_rcu)
	meet_data_right=range(0, num_rcu)
	meet_data_down=range(0, num_rcu)
	os.chdir(dir_name)
	
        #---------------------------------------------
	# Set swlevel  and determine a beam
        rm_files(dir_name,'*')
        os.popen3("swlevel 2");
	
	if StationType == Core or StationType == Remote:		# Test LBA's in mode1 of NL stations only
		os.popen("rspctl --rcuenable=1")
	        time.sleep(5)
		res=os.popen3("rspctl --rcumode=1");
		if debug: print res
		time.sleep(1)
		res=os.popen3("rspctl --aweights=8000,0");
	#	time.sleep(5)
	#        res=os.popen3("beamctl --array=LBA_OUTER --rcus=0:95 --rcumode=1 --subbands=100:110 --beamlets=0:10 --direction=0,0,LOFAR_LMN&")
	#	if debug: print 'answer from beamclt = ' + res
	        time.sleep(1)
	
	#	To simulate a defect antenna:
		if debug:
			os.popen3("rspctl --rcu=0x10037880 --sel=50:53")
			time.sleep(1)
	
	        # get list of all files in dir_name
		files = open_dir(dir_name)
	 
	        #---------------------------------------
		# capture lba element data
	
	        #rm_files(dir_name,'*')
	        print 'Capture LBA data in mode 1.'
	        rec_stat(dir_name,num_rcu)
	        # get list of all files in dir_name
	 	files = open_dir(dir_name)
	
        	# start processing the element measurements
		averagesum=1
		Rejected_antennas=0
		for file_cnt in range(len(files)) :
			f, frames_to_process, rcu_nr  = open_file(files, file_cnt)
        	       	if frames_to_process > 0 : 
				sst_data = read_frame(f)
        	       		sst_subband = sst_data[subband_nr]
				meet_data[rcu_nr] = sst_subband
				if ((sst_subband>75000000) and (sst_subband<1500000000)): # average LCU is about 150.000.000. Reject antennes met grotere afwijking dan 10dB en kleiner dan 3dB
					averagesum=averagesum+sst_subband
				else:
					Rejected_antennas=Rejected_antennas+1
				if debug:
		                	if rcu_nr==0:
        	               			print ' waarde sst_subband 0 is ' + str(sst_subband)
        	        		if rcu_nr==2:
        	        			print ' waarde sst_subband 2 is ' + str(sst_subband)
					if rcu_nr==50:
						print ' waarde sst_subband 50 is ' + str(sst_subband)

			f.close
		if (num_rcu-Rejected_antennas) <> 0: average_lba=averagesum/(num_rcu-Rejected_antennas) # to avoid devide by zero when all antenna's are wrong!
		else: average_lba = 0
#		if debug: 
		print 'average = ' + str(average_lba)
		print 'Number of rejected antennas = ' + str(Rejected_antennas)
		f_loglin.write('Number of rejected antennas for mode 1 = ' + str(Rejected_antennas) + '\n')  
		if average_lba < 4000000:
			print ('LBA levels to low in mode 1!!!')
#			if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
#			if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
			st_log.write('LBAmd1>: Sv=%s Pr=%s, LBA levels to low!!!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
			return

		for rcuind in range(num_rcu) :			# Log lineair value of data
			print 'RCU: ' + str(rcuind) + ' factor: ' + str(meet_data[rcuind])
		        f_loglin.write(str(rcuind) + ' ' + str(meet_data[rcuind]) + '\n')  

	
		f_log.write('\nrcumode 1: \n')
		if average_lba <> 0:
			for rcuind in range(num_rcu) :
				if debug: print 'RCU: ' + str(rcuind) + ' factor: ' + str(round(meet_data[rcuind]*100/average_lba))
        		        f_logfac.write(str(rcuind) + ' ' + str(round(meet_data[rcuind]*100/average_lba)) + '\n')  
				if (round(meet_data[rcuind]*100/average_lba)) < 100-factor or (round((meet_data[rcuind]*100/average_lba))) > 100+factor:
					
					# Store in log file
        		                f_log.write('RCU: ' + str(rcuind)+ ' factor: ' + str(round(meet_data[rcuind]*100/average_lba)) + '\n')
					sr.appendLog(11,'LBL : subb. stat. RCU: ' + str(rcuind)+ ' factor: ' + str(round(meet_data[rcuind]*100/average_lba)))
				
					# store station testlog	
					st_log.write('LBAmd1>: Sv=%s Pr=%s, LBA Outer (LBL) defect: RCU: %s factor: %s\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], rcuind, str(round(meet_data[rcuind]*100/average_lba))))
#					if debug==0: print('RCU: ' + str(rcuind)+ ' factor: ' + str(round(meet_data[rcuind]*100/average_lba)))
					sr.setResult('FAILED')
		else:
			sr.appendLog(11,'No Beam set in mode 1!!')
			sr.setResult('FAILED')
			# store station testlog	
			st_log.write('LBAmd1>: Sv=%s Pr=%s, No Beam set in mode 1!!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))

# When LBA antenna resonance frequency has low level (<60 >2) and the resonance is shifted more than 10 subbands, the antenna is falen over!
		Highest_subband=0
		Previous_subband=0
		for file_cnt in range(len(files)) :
			f, frames_to_process, rcu_nr  = open_file(files, file_cnt)
               		if frames_to_process > 0 : 
				sst_data = read_frame(f)
               			sst_subband = sst_data[subband_nr]
				meet_data[rcu_nr] = sst_subband
				window = range(-40,40)
#				print window
				Highest_subband=0
				Previous_subband=0
				for scan in window:
#					print ' sst_data = ' + str(sst_data[subband_nr+scan])
					if sst_data[subband_nr+scan] > Previous_subband:
						Previous_subband = sst_data[subband_nr+scan]
						Highest_subband = scan
				print ' Highest_subband = ' + str(Highest_subband)
				meet_data_down[rcu_nr] = Highest_subband
				if (round(meet_data[rcu_nr]*100/average_lba)) < 60 and (round(meet_data[rcu_nr]*100/average_lba)) > 2:
					if (Highest_subband < -10 or Highest_subband > +10):
						st_log.write('LBAdn1>: Sv=%s Pr=%s, LBA Outer (LBL) down: RCU: %s factor: %s offset: %s\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], rcu_nr, str(round(meet_data[rcu_nr]*100/average_lba)), Highest_subband))
			f.close

		if average_lba <> 0:
			for rcuind in range(num_rcu) :
				print 'RCU: ' + str(rcuind) + ' factor: ' + str(round(meet_data_down[rcuind]))
		       	        f_logdown.write(str(rcuind) + ' ' + str(round(meet_data_down[rcuind])) + '\n')  


        	f_log.close
		f_logfac.close
		rm_files(dir_name,'*')
#		os.popen("killall beamctl")

	sr.setId('LBAmd3>: ')
#
	os.popen("rspctl --rcuenable=1")
        time.sleep(5)
	res=os.popen3("rspctl --rcumode=3");
	if debug: print res
	time.sleep(1)
	res=os.popen3("rspctl --aweights=8000,0")
#	time.sleep(5)
#        res = os.popen3("beamctl --array=LBA_INNER --rcus=0:95 --rcumode=3 --subbands=100:110 --beamlets=0:10 --direction=0,0,LOFAR_LMN&")
        time.sleep(1)

	# To simulate a defect antenna:
	if debug:
		print (res)
		os.popen("rspctl --rcu=0x10037880 --sel=54:55")
		time.sleep(1)

        # get list of all files in dir_name
	files = open_dir(dir_name)
 
        #---------------------------------------
	# capture lba element data

        #rm_files(dir_name,'*')
        print 'Capture LBA data in mode 3'
        rec_stat(dir_name,num_rcu)
        # get list of all files in dir_name
 	files = open_dir(dir_name)

        # start processing the element measurements
	averagesum=1
	Rejected_antennas=0
	for file_cnt in range(len(files)) :
		f, frames_to_process, rcu_nr  = open_file(files, file_cnt)
               	if frames_to_process > 0 : 
			sst_data = read_frame(f)
               		sst_subband = sst_data[subband_nr]
			meet_data[rcu_nr] = sst_subband
			if ((sst_subband>75000000) and (sst_subband<1500000000)): # average LCU is 150.000.000. Reject antennes met grotere afwijking dan 10dB en kleiner dan 3dB
				averagesum=averagesum+sst_subband
			else:
				Rejected_antennas=Rejected_antennas+1
			#averagesum=averagesum+sst_subband
			if debug:
	                	if rcu_nr==0:
                       			print ' waarde sst_subband 0 is ' + str(sst_subband)
                		if rcu_nr==2:
                			print ' waarde sst_subband 2 is ' + str(sst_subband)
				if rcu_nr==50:
					print ' waarde sst_subband 50 is ' + str(sst_subband)
		f.close
	if (num_rcu-Rejected_antennas) <> 0: average_lba=averagesum/(num_rcu-Rejected_antennas) # to avoid devide by zero when all antenna's are wrong!
	else: average_lba = 0
#	if debug: 
	print 'average = ' + str(average_lba)
	print 'Number of rejected antennas = ' + str(Rejected_antennas)
	f_loglin.write('Number of rejected antennas for mode 3 = ' + str(Rejected_antennas) + '\n')  
	if average_lba < 4000000:
		print ('LBA levels to low in mode 3!!!')
#		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
#		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('LBAmd3>: Sv=%s Pr=%s, LBA levels to low!!!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
		return
			
	for rcuind in range(num_rcu) :			# Log lineair value of data
		print 'RCU: ' + str(rcuind) + ' factor: ' + str(meet_data[rcuind])
	        f_loglin.write(str(rcuind) + ' ' + str(meet_data[rcuind]) + '\n')  
	
	f_log.write('\nrcumode 3: \n')
	if average_lba <> 0:
		for rcuind in range(num_rcu) :
			if debug: print 'RCU: ' + str(rcuind) + ' factor: ' + str(round(meet_data[rcuind]*100/average_lba))
        	        f_logfac.write(str(rcuind) + ' ' + str(round(meet_data[rcuind]*100/average_lba)) + '\n')  
			if (round(meet_data[rcuind]*100/average_lba)) < 100-factor or (round((meet_data[rcuind]*100/average_lba))) > 100+factor:
				
				# Store in log file
        	                f_log.write('RCU: ' + str(rcuind)+ ' factor: ' + str(round(meet_data[rcuind]*100/average_lba)) + '\n')
				sr.appendLog(11,'LBH : subb. stat. RCU: ' + str(rcuind)+ ' factor: ' + str(round(meet_data[rcuind]*100/average_lba)))
			
				# store station testlog	
				st_log.write('LBAmd3>: Sv=%s Pr=%s, LBA Inner (LBH) defect: RCU: %s factor: %s\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], rcuind, str(round(meet_data[rcuind]*100/average_lba))))
				
				sr.setResult('FAILED')
	else:
		sr.appendLog(11,'No Beam set in mode 3!!')
		sr.setResult('FAILED')
		# store station testlog	
		st_log.write('LBAmd3>: Sv=%s Pr=%s, No Beam set in mode 3!!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
		
# When LBA antenna resonance frequency has low level (<60% >2%) and the resonance is shifted more than 10 subbands, the antenna is falen over!
	Highest_subband=0
	Previous_subband=0
	for file_cnt in range(len(files)) :
		f, frames_to_process, rcu_nr  = open_file(files, file_cnt)
               	if frames_to_process > 0 : 
			sst_data = read_frame(f)
               		sst_subband = sst_data[subband_nr]
			meet_data[rcu_nr] = sst_subband
			window = range(-40,40)
#			print window
			Highest_subband=0
			Previous_subband=0
			for scan in window:
#				print ' sst_data = ' + str(sst_data[subband_nr+scan])
				if sst_data[subband_nr+scan] > Previous_subband:
					Previous_subband = sst_data[subband_nr+scan]
					Highest_subband = scan
#			print ' Highest_subband = ' + str(Highest_subband)
			meet_data_down[rcu_nr] = Highest_subband
			if (round(meet_data[rcu_nr]*100/average_lba)) < 60 and (round(meet_data[rcu_nr]*100/average_lba)) > 2:
				if (Highest_subband < -10 or Highest_subband > +10):
					st_log.write('LBAdn3>: Sv=%s Pr=%s, LBA Inner (LBH) down: RCU: %s factor: %s offset: %s\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], rcu_nr, str(round(meet_data[rcu_nr]*100/average_lba)), Highest_subband))
		f.close

	if debug:
		if average_lba <> 0:
			for rcuind in range(num_rcu) :
				print 'RCU: ' + str(rcuind) + ' factor: ' + str(round(meet_data_down[rcuind]))
				f_logdown.write(str(rcuind) + ' ' + str(round(meet_data_down[rcuind])) + '\n')
	
        f_log.close
	f_logfac.close
	f_loglin.close
	rm_files(dir_name,'*')
#	os.popen("killall beamctl")
	if debug:
		print ('Factor should be inbetween %d and %d. ' % (int(100-factor), int(100+factor)))
		print 'Factor 100 is average of all antennas.'
	return

################################################################################
# Function HBA Modem test
#

def isodd(n):
	return bool(n%2)

def HBAModemTest():
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	global Severity
	global Priority
	global ModemFail
	
	debug=0
	
	sr.setId('HBAmdt>: ')
	print ('HBA ModemTest')
	res = os.popen3('cd /opt/stationtest/test/hbatest/ ; rm hba_modem1.log')[1].readlines()
	#res = cli.command('./modemtest.sh')
	#res = os.popen3('cd /opt/stationtest/test/hbatest/ ". .bash_profile ; ./modemtest.sh" &')[1].readlines()
	res = os.popen3('cd /opt/stationtest/test/hbatest/ ; ./modemtest.sh')[1].readlines()
#	print res[1]
	time.sleep(1)

	try:
		f=open('/opt/stationtest/test/hbatest/hba_modem1.log','rb')
	except: 
		print ('Import error')
		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('HBAmdt>: Sv=%s Pr=%s, No modem-logfile found!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
		return
	time.sleep(1)

	for line in f:
		ModemReply=line
		ModemReplyGold=['HBA', '95', 'real', 'delays=', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16']
		if debug: print ('line = ',line[0])
		if line[0] == 'H':		# Check of regel geldig is!
			ModemReply=line.replace('[',' ').replace('].',' ').split()
			RCUNr=int(ModemReply[1])
			TileNr=RCUNr/2
			if debug:
				print ('line           = ',line)
				print ('ModemReply     = ',ModemReply)
				print ('ModemReplyGold = ',ModemReplyGold)
				print ('RCUNr          = ',RCUNr)
				print ('TileNr         = ',TileNr)
	
# Check if HBA modems work!
			count=0
			for ElementNumber in range(4, 20):
#				print ModemReplyGold[ElementNumber]
				if ModemReply[ElementNumber] != ModemReplyGold[ElementNumber]:
					count+=1
					ModemFail[TileNr]=1 # global variabele om in HBA element test de RF meting over te slaan.

#					
			if (count > 10 and isodd(RCUNr)): 	#Als er meer dan 10 fouten in zitten, keur dan hele tile af!
				print ('Tile %s - RCU %s; Broken. No modem communication' % (TileNr,RCUNr))
				
				# store station testlog	
				#if debug: print ('ModemFail      = ',ModemFail) 
				if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
				if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
				st_log.write('HBAmdt>: Sv=%s Pr=%s, Tile %s - RCU %s; Broken. No modem communication\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], TileNr, RCUNr))
				sr.setResult('FAILED')
				
			else:		#Anders keur elementen af als fout.
				for ElementNumber in range(4, 20):
					if (ModemReply[ElementNumber] != ModemReplyGold[ElementNumber] and isodd(RCUNr)):
						print ('Tile %s - RCU %s; Element %s; Broken. No modem communication : (%s, %s)' % (TileNr, RCUNr, ElementNumber-3, ModemReply[ElementNumber], ModemReplyGold[ElementNumber]))
						# store station testlog	
						if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
						if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
						st_log.write('HBAmdt>: Sv=%s Pr=%s, Tile %s - RCU %s; Element %s Broken. No modem communication : (%s, %s)\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], TileNr, RCUNr, ElementNumber-3, ModemReply[ElementNumber], ModemReplyGold[ElementNumber]))
						sr.setResult('FAILED')
#			print ('ModemFail      = ',ModemFail) 
	
	return


################################################################################
# Function HBA Noise and Spurious
#
# Failure modes to detect:
# - Large oscillations on a single tile
# - Spurious on a single tile
# - To high and to low noise levels on a single tile over wide range of subbands
# - Fluctuating noise levels on a single tile over wide range of subbands
#
# Ignore subbands:
# - Large signals on all tiles (Close-by radio transmitter)
# - Subbands outside frequencyband of 120 to 180 MHz (mode5)
#
# Detecting methods:
# - Large oscillations on one single tile
#   Fail when subband is not ignored and
#        when subband signal of one tile is larger then the average of all tiles by a factor of "HBAoscLim"
# - Spurious on a single tile
#   Fail when subband is not ignored and
#        when subband signal of one tile is larger then the average of all tiles by a factor of "HBAspurLim"
# - To high and to low noise levels on a single tile over wide range of subbands
#   Fail when subband is not ignored and
#	 when the average levels of a range of subbands is higher or lower than the average levels of a range of the subbands of all tiles by a factor of "HBAnoiseLim"
# - Fluctuating noise levels on a single tile over wide range of subbands
#   Fail when subband is not ignored and
#	 when maximun subband value minus the minimum subband value of the multiple captures differ by a factor of "HBAfluctLim"
#
# Determine subband average of multiple captures
#   Ignore when subband is ignored and
#	   when the subband of all captures is larger then "HBAnominal * IgnoreHBAsubbHiLim" or
#	   when the subband of all captures is smaller then "HBAnominal * IgnoreHBAsubbLoLim"
#
# Signal levels
# Inband noise = 9.2 E+6
# Ouband noise = 920 E+3
# P2000 (subband 155) = 1.5 E+12
#

def HBANaStest():
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	global Severity
	global Priority
	
#	Limmits:
	HBAoscLim = 10000			# To determine high signal levels due to oscillation 
	HBAspurLim = 3			# To determine increased signal levels due to Summator spurious
	HBAnoiseLim = 3			# To determine to high or to low noise levels du to bad connectivity or defect elements
	IgnoreHBAsubbHiLim = 10		# Ignore subbands that have a signal level of "HBAnominal" * this factor higher than this factor on all tiles (to determine average)
	IgnoreHBAsubbLoLim = 0.2	# Ignore subbands that have a signal level of this factor lower than this factor on all tiles (to determine average)
	HBAnominal = 9200000		# Nominal value of subband 150
	
	HBANaSdata = []			# 2D array with captured lineair data of all HBA tiles
	HBANaSarray = []		# 3D array with multiple captures of lineair data of all HBA tiles

	
	CaptureIterations = 1		# How many times the HBA spectrum will be captured!
	SubbStart = 98			# Ignore subbands below
	SubbStop = 420			# Ignore subbands above
#	SubbStart = 0
#	SubbStop = 512
	ctrlword = 253
	
	Ignore = 1
	
	HBANaSfile=('/opt/stationtest/data/HBANaS.csv')
	NaS_log = file(HBANaSfile, 'w')
		
	if StID in NoHBANaStestPossible: 
		print ('No HBA elementtest Possible!!!')
		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('HBAmd5>: Sv=%s Pr=%s, No HBA elementtest Possible!!!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
	else:
		debug=0
		
		print ('HBA Noise Spurious and Oscillation check')
		sr.setId('HBAosc>: ')	
		subband_nr=155
		if StationType == International: subband_nr = HBASubband[StID]
		if debug: print (' subband_nr of %s = %d %d' % (StID,subband_nr,HBASubband[StID]))

		sub_time=[]
		sub_file=[]
		dir_name = '/opt/stationtest/test/hbatest/hbadatatest/' #Work directory will be cleaned
		if not(os.path.exists(dir_name)):
			os.mkdir(dir_name)
		rmfile = '*.log'
		hba_elements=16
		sleeptime=10
		
		ctrl_string='='

		print ' Dir name is ' + dir_name
		os.chdir(dir_name)
		if len(sys.argv) < 3 :
			if StationType == International:
				num_rcu=192
			else:
				num_rcu=96
		else :
			num_rcu = int(sys.argv[2])
		print ' Number of RCUs is ' + str(num_rcu)
		## initialize data arrays
		ref_data=range(0, num_rcu)
		
		# Determine Subbands to be ignored: manualy part!
		IgnoreHBA = [0 for i in range(512)]	# 1 = ignore subband...
		for i in range(0,SubbStart): IgnoreHBA[i]=1
		for i in range(SubbStop,512): IgnoreHBA[i]=1
		#print ('IgnoreHBA: %s' % (IgnoreHBA))

		##os.popen("rspctl --clock=200")
		##print 'Clock is set to 200 MHz'
		##time.sleep(10)
		##---------------------------------------------
		## capture reference data (all HBA elements off)

		switchon_hba()
		##os.popen("rspctl --rcumode=5 2>/dev/null")
		##os.popen("rspctl --rcuenable=1 2>/dev/null")
		time.sleep(2)
	##	To simulate a defect antenna:
		#if debug==2:
			#os.popen3("rspctl --rcu=0x10037880 --sel=50:53")
			#time.sleep(1)
		for ind in range(hba_elements) :
			ctrl_string=ctrl_string + '253,'
		strlength=len(ctrl_string)
		ctrl_string=ctrl_string[0:strlength-1]
		print('rspctl --hbadelay' + ctrl_string + ' 2>/dev/null')
		cmd_str='rspctl --hbadelay' + ctrl_string + ' 2>/dev/null'
		os.popen(cmd_str)

		time.sleep(sleeptime)
		#res = os.popen3('rspctl --rcumode=0 --sel=52:53,66:67')[1].readlines()	# for test
		#time.sleep(sleeptime)
		#time.sleep(sleeptime)
		
		# T E S T ! ! !
#		print('rspctl --hbadelay=253,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>/dev/null')
#		cmd_str=('rspctl --hbadelay=253,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>/dev/null')
#		os.popen(cmd_str)
#		res = os.popen3('rspctl --rcumode=0 --sel=10,11,94,95')[1].readlines()
#		time.sleep(sleeptime)
		
		# Capture HBA data
		for i in range(0,CaptureIterations):
			rm_files(dir_name,'*')	
			HBANaSdata = [[0 for j in range(512)] for k in range(num_rcu)]
			print ('Capture HBA data nr %s of %s' % (i+1,CaptureIterations))
			rec_stat(dir_name,num_rcu)
			#rm_files(dir_name,rmfile)
			# get list of all files in dir_name
			files = open_dir(dir_name)
			print (files)
			# start processing the measurement
			for file_cnt in range(len(files)) :
				f, frames_to_process, rcu_nr  = open_file(files, file_cnt)
				if frames_to_process > 0 : 
					sst_data = read_frame(f)
					#print ('Number or RCUs processed: ' + str(rcu_nr))
					#sst_subband = sst_data[subband_nr]
					#ref_data[rcu_nr] = sst_subband
					#HBANaSdata.append(sst_data)
					for subnr in range(0, 512): HBANaSdata[rcu_nr][subnr] = sst_data[subnr]
				f.close
			#print('file_cnt = %s' % len(files))
			#print('HBANaSdata = %s' % HBANaSdata)
			#print('From RCU %s subband nr %s = %s' % (0,155,HBANeSdata[0][155]))
			#print('From RCU %s subband nr %s = %s' % (0,150,HBANeSdata[0][150]))
			HBANaSarray.append(HBANaSdata)
		print('Capture %s from RCU %s subband nr %s = %s' % (0,0,155,HBANaSarray[0][0][155]))
		print('Capture %s from RCU %s subband nr %s = %s' % (0,54,155,HBANaSarray[0][54][155]))
		print('Capture %s from RCU %s subband nr %s = %s' % (0,94,154,HBANaSarray[0][94][154]))
		print('Capture %s from RCU %s subband nr %s = %s' % (0,66,155,HBANaSarray[0][66][155]))
		
		##---------------------------------------------
		## compute hba data for all tiles
		#noRCU = 96
		#noEll = 16
		#HBAlist = [[0 for i in range(noEll)] for j in range(noRCU)]	# Array (list) with HBA antenna elements. 0=OK 1=defect

		# calculate average of multiple captures of all RCU's
		# Determine subband average of multiple captures
		#   Ignore when subband is ignored and
		#	   when the subband of all captures is larger then "HBAnominal * IgnoreHBAsubbHiLim" or
		#	   when the subband of all captures is smaller then "HBAnominal * IgnoreHBAsubbLoLim"
		HBAaverageSubb = [0 for i in range(512)]
		HBAfail = [0 for i in range(num_rcu)]
		HBAfact = [0 for i in range(num_rcu)]
		HBAoscFactor = [0 for i in range(512)]		# Subband with highest signal value = factor
		HBAoscRCU = [0 for i in range(512)]		# RCU with highest signal
		
		for Subnr in range(0,512):
			CountIgnore = 0
			NaS_log.write('SubbNr %s;' % (Subnr))
			# Ignore when the subband of all captures is larger then "HBAnominal * IgnoreHBAsubbHiLim"
			for RCUnr in range(0,num_rcu):
				# Get the average of the subband signals over multiple captures
				SubbValue = 0
				for Capt in range(0,CaptureIterations):
					SubbValue = SubbValue + HBANaSarray[Capt][RCUnr][Subnr]
				SubbValue  = SubbValue  / CaptureIterations
				NaS_log.write('%s;' % (SubbValue))
				if (SubbValue > (HBAnominal * IgnoreHBAsubbHiLim)): CountIgnore+=1	# Count to High
				elif (SubbValue < (HBAnominal * IgnoreHBAsubbLoLim)): CountIgnore+=1	# Count to Low
				else:HBAaverageSubb[Subnr] = HBAaverageSubb[Subnr] + SubbValue
			if CountIgnore > (num_rcu / 2): IgnoreHBA[Subnr]=1 # Ignore subband when the subband signal of more than half of the RCU's is to high
			if (num_rcu-CountIgnore) != 0: HBAaverageSubb[Subnr] = (HBAaverageSubb[Subnr] / (num_rcu-CountIgnore))
			else: HBAaverageSubb[Subnr] = HBAnominal
			NaS_log.write(';\n')
			#if IgnoreHBA[RCUnr] == 1: print ('RCUnr %s Subnr %s = %s' % (RCUnr,Subnr,HBAaverageSubb[Subnr]))
		#print(HBAaverageSubb)
		#print('HBAaverageSubb[] = %s' % HBAaverageSubb)
		#for i in range(512): 
			#if IgnoreHBA[i] == Ignore: 
			#print('IgnoreHBA[%s] = %s  HBAaverageSubb = %s' % (i,IgnoreHBA[i],HBAaverageSubb[i]))
		for i in range(CaptureIterations): 
			print('Capture %s from RCU %s subband nr %s = %s' % (i,0,150,HBANaSarray[i][0][150]))
		print('The average of all captures of All RCUs of subband nr %s = %s' % (150,HBAaverageSubb[150]))
		print('Capture %s from RCU %s subband nr %s = %s' % (0,66,338,HBANaSarray[0][66][338]))
		
		# - Large oscillations on one single tile
		#   Fail when subband is not ignored and
		#        when subband signal of one tile is larger then the average of all tiles by a factor of "HBAoscLim"
		
		# for test:
		#IgnoreHBA[155] = 0
		#HBAaverageSubb[155] = HBAnominal
		
		
		for RCUnr in range(0,num_rcu):
			for Subnr in range(0,512):
				if IgnoreHBA[Subnr] != Ignore: # Ignore when the subband of all captures is larger then "HBAnominal * IgnoreHBAsubbHiLim"
					# Get the average of the subband signals over multiple captures and test if to high
					SubbValue = 0
					for Capt in range(0,CaptureIterations):
						SubbValue = SubbValue + HBANaSarray[Capt][RCUnr][Subnr]
					SubbValue  = SubbValue  / CaptureIterations
					if (SubbValue/HBAnominal) > (HBAoscFactor[Subnr]):	# Remember highest osc factor
						HBAoscFactor[Subnr] = round(SubbValue/HBAnominal)
						HBAoscRCU[Subnr]=RCUnr				# Remember RCU number with highest osc factor
				
					#if (SubbValue > (HBAaverageSubb[Subnr] * HBAoscLim)): 			# Detect oscillations
					#if (SubbValue > (HBAnominal * HBAoscLim)): 			# Detect oscillations
					#	HBAfail[RCUnr] = 1
						#if (SubbValue/HBAaverageSubb[Subnr]) > (HBAoscFactor[RCUnr]):	# Remember highest osc factor
						#	HBAoscFactor[RCUnr] = round(SubbValue/HBAaverageSubb[Subnr])
					#	if (SubbValue/HBAnominal) > (HBAoscFactor[RCUnr]):	# Remember highest osc factor
					#		HBAoscFactor[RCUnr] = round(SubbValue/HBAnominal)
		
		for Subnr in range(0,512):
			#for RCUnr in range(0,num_rcu):
			
			if (HBAoscFactor[Subnr] > HBAoscLim):
				HBAfail[HBAoscRCU[Subnr]] = 1
				HBAfact[HBAoscRCU[Subnr]] = HBAoscFactor[Subnr]
				
		for Subnr in range(0,512): print('Osc factors Subnr %s = %s, of RCU %s (Fail=%s)' % (Subnr,HBAoscFactor[Subnr],HBAoscRCU[Subnr],HBAfail[HBAoscRCU[Subnr]]))
		
		# Save in log file
		for RCUnr in range(0,num_rcu):
			if HBAfail[RCUnr] == 1:
				if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
				if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
				st_log.write('HBAosc>: Sv=%s Pr=%s, Tile %s - RCU %s; Large oscillation (Factor = %s, CtrlWord = %s)\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], str(RCUnr/2), RCUnr, str(HBAfact[RCUnr]), ctrlword))
				sr.setResult('FAILED')
		print('HBAosc>: Sv=%s Pr=%s, Tile %s - RCU %s; Large oscillation (Factor = %s, CtrlWord = %s)\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], str(66/2), 66, str(HBAfact[66]), ctrlword))

#		for k in range(0,512):
#			for j in range(0,num_rcu):
#				NaS_log.write('%s;' % (k,j+1))
#				try: 
#					for i in range(0,100): hist_log.write('%s;' % (HBAlists[i][k][j]))
#				except: 
#					hist_log.write('\n')
		NaS_log.close
		
				
				
		##---------------------------------------------
		## capture hba element data for all elements
		#for temp_ctrl in ctrl_word:
			#print 'Capture data for control word: ' + str(temp_ctrl)
			## init log file
			#filename='/opt/stationtest/test/hbatest/HBA_elements_' + str(temp_ctrl)
			#f_log = file(filename, 'w')
			#writestring=' ************ \n \n LOG File for HBA element test (used ctrl word for active element:' + str(temp_ctrl) +' \n \n *************** \n \n'
			#f_log.write(writestring)
			#filename='/opt/stationtest/test/hbatest/HBA_factors_' + str(temp_ctrl)
			#f_logfac = file(filename, 'w')
	
			#for element in range(hba_elements) :
				#meet_data=capture_data(dir_name,num_rcu,hba_elements,temp_ctrl,sleeptime,subband_nr,element)
	
				##Find the factor
				#data_tmp=10*numpy.log10(meet_data)
				#data_tmp=numpy.sort(data_tmp)
				#median=data_tmp[len(data_tmp)/2]
				#factor=median/2
				#print 'Processing element ' + str(element) + ' using a limit of ' + str(round(factor,1)) + ' dB'
				##Write results to file
				#for rcuind in range(num_rcu) :
					#f_logfac.write(str(element+1) + ' ' + str(rcuind) + ' ' + str(round(meet_data[rcuind]/ref_data[rcuind])) + '\n')  
					#if meet_data[rcuind] < factor*ref_data[rcuind] :        
						#if rcuind == 0 :
							#tilenumb=0
						#else:
							#tilenumb=int(rcuind/2)
						#f_log.write('Element ' + str(element+1) + ', Tile ' + str(tilenumb) + ' in RCU: ' + str(rcuind)+ ' factor: ' + str(round(meet_data[rcuind]/ref_data[rcuind])) + '\n')
						
						## store station testlog	
						#if ModemFail[tilenumb] != 1:
							#if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
							#if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
							#st_log.write('HBAmd5>: Sv=%s Pr=%s, Tile %s - RCU %s; Element %s Broken. RF-signal to low : (Factor = %s, CtrlWord = %s)\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], str(tilenumb), rcuind, str(element+1), str(round(meet_data[rcuind]/ref_data[rcuind])), temp_ctrl))
							#sr.setResult('FAILED')
				
		#f_log.close
		#f_logfac.close
	return	


################################################################################
# Function HBA test
#

# functions belonging to HBA test:

def capture_data(dir_name,num_rcu,hba_elements,ctrl_word,sleeptime,subband_nr,element):
	meet_data=range(0, num_rcu)
        rm_files(dir_name,'*')
        ctrl_string='='
        for ind in range(hba_elements) :
               if ind == element:
			ctrl_string=ctrl_string + '128,'
	       else:	
			ctrl_string=ctrl_string + '2,'
	strlength=len(ctrl_string)
        ctrl_string=ctrl_string[0:strlength-1]
	cmd_str='rspctl --hbadelay' + ctrl_string + ' 2>/dev/null'
        os.popen(cmd_str)
	time.sleep(sleeptime)
        print 'Capture HBA element ' + str(element+1) + ' data'
        rec_stat(dir_name,num_rcu)
        # get list of all files in dir_name
 	files = open_dir(dir_name)
        
        # start processing the element measurements
	for file_cnt in range(len(files)) :
		f, frames_to_process, rcu_nr  = open_file(files, file_cnt)
                if frames_to_process > 0 : 
		   	sst_data = read_frame(f)
                   	sst_subband = sst_data[subband_nr]
		   	meet_data[rcu_nr] = sst_subband
		f.close
	return meet_data

def switchon_hba() :
	
	try:
           os.popen3("rspctl --rcumode=5 --sel=0:31")
           time.sleep(1)
	   os.popen3("rspctl --rcumode=5 --sel=32:63")
           time.sleep(1)
	   os.popen3("rspctl --rcumode=5 --sel=64:95")
           time.sleep(1)
	   os.popen3("rspctl --rcumode=5 --sel=96:127")
           time.sleep(1)
	   os.popen3("rspctl --rcumode=5 --sel=128:159")
           time.sleep(1)
	   os.popen3("rspctl --rcumode=5 --sel=160:191")
           time.sleep(1)
	except:
          print "This is a NL station"
        os.popen("rspctl --rcuenable=1")
        return 
	
# HBA test
def HBAtest():
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	global Severity
	global Priority
	
	if StID in NoHBAelementtestPossible: 
		print ('No HBA elementtest Possible!!!')
		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('HBAmd5>: Sv=%s Pr=%s, No HBA elementtest Possible!!!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
	else:
		debug=0
		
		print ('HBA element test')
		sr.setId('HBAmd5>: ')	
		subband_nr=155
		if StationType == International: subband_nr = HBASubband[StID]
		if debug: print (' subband_nr of %s = %d %d' % (StID,subband_nr,HBASubband[StID]))

		sub_time=[]
		sub_file=[]
		dir_name = '/opt/stationtest/test/hbatest/hbadatatest/' #Work directory will be cleaned
		if not(os.path.exists(dir_name)):
			os.mkdir(dir_name)
		rmfile = '*.log'
		hba_elements=16
		sleeptime=10
		ctrl_word=[128,253]
		ctrl_string='='
		# read in arguments
#		if len(sys.argv) < 2 :
#			subband_nr=155
#		else :
#			subband_nr = int(sys.argv[1])
		print ' Dir name is ' + dir_name
		if len(sys.argv) < 3 :
			if StationType == International:
				num_rcu=192
			else:
				num_rcu=96
		else :
			num_rcu = int(sys.argv[2])
		print ' Number of RCUs is ' + str(num_rcu)
		#print ' Number of the used Subband is ' + str(subband_nr)
		print (' Number of the used Subband of %s is  = %d' % (StID,subband_nr))
		# initialize data arrays
		ref_data=range(0, num_rcu)
		os.chdir(dir_name)
		#os.popen("rspctl --clock=200")
		#print 'Clock is set to 200 MHz'
		#time.sleep(10)
		#---------------------------------------------
		# capture reference data (all HBA elements off)
		rm_files(dir_name,'*')
		switchon_hba()
		#os.popen("rspctl --rcumode=5 2>/dev/null")
		#os.popen("rspctl --rcuenable=1 2>/dev/null")
		time.sleep(2)
	#	To simulate a defect antenna:
		if debug==2:
			os.popen3("rspctl --rcu=0x10037880 --sel=50:53")
			time.sleep(1)
		for ind in range(hba_elements) :
			ctrl_string=ctrl_string + '2,'
		strlength=len(ctrl_string)
		ctrl_string=ctrl_string[0:strlength-1]
		cmd_str='rspctl --hbadelay' + ctrl_string + ' 2>/dev/null'
		os.popen(cmd_str)
		time.sleep(sleeptime)
		print 'Capture reference data'
		rec_stat(dir_name,num_rcu)
		#rm_files(dir_name,rmfile)
		# get list of all files in dir_name
		files = open_dir(dir_name)
		# start processing the reference measurement
		for file_cnt in range(len(files)) :
			f, frames_to_process, rcu_nr  = open_file(files, file_cnt)
			if frames_to_process > 0 : 
				sst_data = read_frame(f)
				sst_subband = sst_data[subband_nr]
				ref_data[rcu_nr] = sst_subband
				#if rcu_nr==0:
				#	print ' waarde is ' + str(sst_subband)
			f.close
		#---------------------------------------------
		# capture hba element data for all elements
		for temp_ctrl in ctrl_word:
			print 'Capture data for control word: ' + str(temp_ctrl)
			# init log file
			filename='/opt/stationtest/test/hbatest/HBA_elements_' + str(temp_ctrl)
			f_log = file(filename, 'w')
			writestring=' ************ \n \n LOG File for HBA element test (used ctrl word for active element:' + str(temp_ctrl) +' \n \n *************** \n \n'
			f_log.write(writestring)
			filename='/opt/stationtest/test/hbatest/HBA_factors_' + str(temp_ctrl)
			f_logfac = file(filename, 'w')
	
			for element in range(hba_elements) :
				meet_data=capture_data(dir_name,num_rcu,hba_elements,temp_ctrl,sleeptime,subband_nr,element)
	
				#Find the factor
				data_tmp=10*numpy.log10(meet_data)
				data_tmp=numpy.sort(data_tmp)
				median=data_tmp[len(data_tmp)/2]
				factor=median/2
				print 'Processing element ' + str(element) + ' using a limit of ' + str(round(factor,1)) + ' dB'
				#Write results to file
				for rcuind in range(num_rcu) :
					#print ('ref_data = %d rcuind = %d' % (ref_data[rcuind],rcuind))
					if ref_data[rcuind] != 0: f_logfac.write(str(element+1) + ' ' + str(rcuind) + ' ' + str(round(meet_data[rcuind]/ref_data[rcuind])) + '\n')
					if meet_data[rcuind] < factor*ref_data[rcuind] :        
						if rcuind == 0 :
							tilenumb=0
						else:
							tilenumb=int(rcuind/2)
						f_log.write('Element ' + str(element+1) + ', Tile ' + str(tilenumb) + ' in RCU: ' + str(rcuind)+ ' factor: ' + str(round(meet_data[rcuind]/ref_data[rcuind])) + '\n')
						
						# store station testlog	
						if ModemFail[tilenumb] != 1:
							if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
							if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
							st_log.write('HBAmd5>: Sv=%s Pr=%s, Tile %s - RCU %s; Element %s Broken. RF-signal to low : (Factor = %s, CtrlWord = %s)\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], str(tilenumb), rcuind, str(element+1), str(round(meet_data[rcuind]/ref_data[rcuind])), temp_ctrl))
							sr.setResult('FAILED')
				
		f_log.close
		f_logfac.close
	return	




################################################################################
# Function WriteAll: To leave message on the station!
#
def WriteAll(msg):
	res = os.popen3('wall %s' % (msg))[1].readlines()
	return

################################################################################
# Main program
Message=('!!!     This station will be in use for a test! Please do not use the station!     !!!')

WriteAll(Message)
GotoSwlevel2()			# Set system in software level 2
CheckNtpd()			# Check the pps and GPS ST
##makeRSPVersionGold()		# make RSP Version gold ST
CheckRSPVersion()		# Check RSP Version ST
CheckTDSStatus160()		# Set clock to 200 MHz and check if locked
CheckRSPStatus()		# Check status bits form the RSP ST
CheckTDSStatus200()		# Set clock to 200 MHz and check if locked
CheckRSPStatus()		# Check status bits form the RSP ST
GotoSwlevel2()			# Set system in software level 2 again (via level 1). Switching the clock will hold the TBBdriver
#makeTBBVersionGold()		# make TBB Version ST
CheckTBBVersion()		# CHeck TBB Version ST
#makeTBBMemGold()		# make TBB Memory gold ST
#CheckTBBMemory()		# Verify TBB memory modules on the TBB ST
#CheckTBBSize()			# Verify the size of the TBB memory modules ST
#RCUHBAModemTest()		# Verify the control modem on the RCU ST (Gaat nog iets fout op CS003!!!!!
#PseudoRandomTBBTest()		# Verify the RCU - RSP - TBB LVDS interfaces by capturing pseudo random data on TBB
#CheckSPUStatus()		# Verify the RSP - SPU I2C interface by reading the SPU sensor data ST
#CheckRSPTdI2C() 		# Verify the RSP - TD I2C interface by reading the TD sensor data ST
#Bist()				# Build In Self Test for RSP (BIST) ST
#PseudoRandomRSPTest()		# Verify the RCU -> RSP LVDS interfaces by capturing pseudo random data on RSP
##RCUHBAModemTest()		# Verify the control modem on the RCU

#SerdesRingTestOff()		# Verify the Serdes ring connection between the RSP boards with ring is off
#SerdesRingTestOn()		# Verify the Serdes ring connection between the RSP boards with ring is on

res = os.popen3('rspctl --rcuprsg=0')[1].readlines()
LBAtest()			# Check LBH and LBL antenna's in mode 1 and 3 ST
HBAModemTest()			# Test of the HBA server modems
HBAtest()			# Check HBA tiles in mode 5
HBANaStest()			# HBA Noise and Spurious


Message=('!!!     The test is ready and the station can be used again!                       !!!')
WriteAll(Message)


################################################################################
# End of the subrack test

res = os.popen3('rspctl --rcuprsg=0')[1].readlines()
#cli.command('rspctl --rcuprsg=0') 
sr.setId('Subrack - ')
dt = sr.getRunTime()
sr.appendLog(2,'Duration: %02dm:%02ds' % (dt/60 % 60, dt % 60))
sr.appendLog(0,sr.getResult())
sr.closeLog()

################################################################################
# End of the station test

# Define station testlog
st_log.write('Status>: %s\n' % sr.getResult())
if Priority > 0 or Severity > 0:
	st_log.write('Sever >: %s\n' % SeverityLevel[Severity])
	st_log.write('Prio  >: %s\n' % PriorityLevel[Priority])
st_log.write('TestTm>: %02dm:%02ds\n' % (dt/60 % 60, dt % 60))
#st_log.flush
st_log.close()
time.sleep(1)
res = os.popen3('swlevel 1')[1].readlines()	# Put station in current saving mode.....

# Finaly move temporary logfile to final logfile
res = os.popen3("scp -rp %s %s" % (TestlogName , HistlogName))[1].readlines()
if debug: print res
time.sleep(1)
res = os.popen3("mv %s %s" % (TestlogName , TestlogNameFinalized))
if debug: print res
print ('TestlogName: ',TestlogName)
print ('HistlogName: ',HistlogName)
print ('TestlogNameFinalized: ',TestlogNameFinalized)


