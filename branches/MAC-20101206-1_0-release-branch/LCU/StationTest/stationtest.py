#!/usr/bin/python

#
# Run the tests to test a LOFAR station
# H. Meulman
# Version 0.3                27-sep-2010

# 24 sep: local log directory aangepast
# 27 sept: 	- Toevoeging delay voor tbbdirver polling
#		- Aanzetten van LBA's m.b.v rspctl --aweights=8000,0

# todo:
# - Als meer dan 10 elementen geen rf signaal hebben, keur dan hele tile af
# - als beamserver weer goed werkt deze weer toevoegen aan LBA test

import sys
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
noTBB=6
factor = 30	# station statistics fault window: Antenna average + and - factor = 100 +/- 30

# Do not change:
Severity=0	# Severity (0='' 1=feature 2=minor 3=major 4=block 5=crash
Priority=0	# Priority (0=no 1=low 2=normal 3=high 4=urgent 5=immediate
SeverityLevel=('--     ','feature','minor  ','Major  ','BLOCK  ','CRASH  ')
PriorityLevel=('--       ','low      ','normal   ','High     ','URGENT   ','IMMEDIATE')
#print (SeverityLevel[Severity])
#print (PriorityLevel[Priority])

# Path
RSPgoldfile=('/misc/home/etc/stationtest/gold/rsp_version.gold')
TBBgoldfile=('/misc/home/etc/stationtest/gold/tbb_version.gold')
TBBmgoldfile=('/misc/home/etc/stationtest/gold/tbb_memory.gold')
#LogPath=('/misc/home/log/')
TestLogPath=('/misc/home/log/')	# Logging remote (on Kis001)
#TestLogPath=('/opt/stationtest/data/')	# Logging local (on station)
#HistLogPath=('/opt/stationtest/data/')	# Logging local (on station)
HistLogPath=('/localhome/log/')	# Logging local (on station)


tm=strftime("%a, %d %b %Y %H:%M:%S", localtime())	# Determine system time
tme=strftime("_%b_%d_%Y_%H.%M", localtime())		# Time for fileheader History log file
StIDlist = os.popen3('hostname -s')[1].readlines()	# Name of the station
StID = str(StIDlist[0].strip('\n'))
if debug: print ('StationID = %s' % StID)

TestlogName = ('%sstationtest_%s.tmp' % (TestLogPath, StID))
TestlogNameFinalized = ('%sstationtest_%s.log' % (TestLogPath, StID))
HistlogName = ('%sstationtest_%s%s.log' % (HistLogPath, StID, tme))

# Array om bij te houden welke Tiles niet RF getest hoeven worden omdat de modems niet werken.
if len(sys.argv) < 3 :
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
opts.tbb_nr=6		# Fixed number (make autodetect later!!)

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

	
# Define subrack testlog class for pass/fail and logging
vlev = opts.verbosity
testId = ''
appLev = False
logName = '/opt/stationtest/data/SUBR-%05d-%05d.dat' % (opts.rsp_nr, opts.tbb_nr)
cli.command('rm -f /opt/stationtest/data/SUBR-%05d-%05d.dat', appLev) 
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
#    print 'wait 20 sec'
#    time.sleep(20)
    if debug: print int(len(os.popen3('tbbctl --version')[1].readlines()))
    sr.setId('TBB   >: ')
    n=0 # Maximum itteration
    while len(os.popen3('tbbctl --version')[1].readlines()) < 4:
        print ('-'),
	if debug: print ('Polling TBB Driver')
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
				print 'wait 30 sec'
				if debug:
					for line in res2:
						print ('%s' % line.rstrip('\n'))
				time.sleep(30)
				time.sleep(30)  # Tijdelijk toe gevoegd voor nieuwe tbbdriver. Deze loopt vast tijdens pollen
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
	sr.setId('Clock   - ')
	res = os.popen3('/usr/sbin/ntpq -p')[1].readlines()
	#res = os.popen3('/opt/stationtest/test/timing/ntpd.sh')[1].readlines()
	if debug:
		for line in res:
	        	print ('-%s' % line.rstrip('\n'))
	print ('res : %s' % res)

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
	SeverityOfThisTest=2
	PriorityOfThisTest=2
	
	global Severity
	global Priority
	
	sr.setId('RSPst >: ')
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
			dif = res[linecount+rsp*5+sync].lstrip('RSP').strip('[').split()
			if debug:
				print ('Dif = %s' % dif)
				#print str(linecount+rsp*5+sync),
				#print dif[2]
			if dif[2] not in ('0', '1', '511', '512'):
				#if debug: print ('RSP : %d status error:  sync = %d, diff = %d' % (int(rsp), int(sync), int(dif[2])))
				sr.appendLog(11,'RSP : %d status error:  sync = %d diff = %d' % (int(rsp), int(sync), int(dif[2])))
				sr.setResult('FAILED')
				
				if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
				if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
				st_log.write('RSPst >: Sv=%s Pr=%s, RSP : %d status error:  sync = %d diff = %d\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], int(rsp), int(sync), int(dif[2])))
				sr.setResult('FAILED')
			
	#time.sleep(3)
	return

################################################################################
# Function make RSP Version gold
#
def makeRSPVersionGold():
	res = os.popen3('rspctl --version')[1].readlines()
	time.sleep(3)
	f_log = file('/misc/home/etc/stationtest/gold/rsp_version.gold', 'w')
	for line in res:
		print ('Res = ', line)
	        f_log.write(line)
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
#	res = cli.command('./tbb_version.sh')
	if len(TBBversion) < 4:
	# store station testlog			
		if Severity<SeverityOfThisTest: Severity=SeverityOfThisTest
		if Priority<PriorityOfThisTest: Priority=PriorityOfThisTest
		st_log.write('TBB   >: Sv=%s Pr=%s, Error: TBB driver is not running\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
		return
			
	if debug:
		print ('TBBgold: %s' % TBBgold)
		print ('TBBgold: %s' % TBBversion)
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
			st_log.write('TBBver>: Sv=%s Pr=%s, BP/AP Error! %s' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest], TBBversion[TBBnumber]))
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
    		rcu_nr = int(files[file_nr][-6:-4])
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
	
#	debug=1
	
	global Severity
	global Priority
	
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
	# initialize data arrays
	ref_data=range(0, num_rcu)
	meet_data=range(0, num_rcu)
	os.chdir(dir_name)

        #---------------------------------------------
	# Set swlevel 3 and determine a beam
        rm_files(dir_name,'*')
        os.popen3("swlevel 2");
	os.popen("rspctl --rcuenable=1")
        time.sleep(5)
	res=os.popen3("rspctl --rcumode=1");
	print res
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
	averagesum=0
	for file_cnt in range(len(files)) :
		f, frames_to_process, rcu_nr  = open_file(files, file_cnt)
               	if frames_to_process > 0 : 
			sst_data = read_frame(f)
               		sst_subband = sst_data[subband_nr]
			meet_data[rcu_nr] = sst_subband
			averagesum=averagesum+sst_subband
			if debug:
	                	if rcu_nr==0:
                       			print ' waarde sst_subband 0 is ' + str(sst_subband)
                		if rcu_nr==2:
                			print ' waarde sst_subband 2 is ' + str(sst_subband)
				if rcu_nr==50:
					print ' waarde sst_subband 50 is ' + str(sst_subband)
		f.close
	average_lba=averagesum/num_rcu
#	if debug: 
	print 'average = ' + str(average_lba)
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
#				if debug==0: print('RCU: ' + str(rcuind)+ ' factor: ' + str(round(meet_data[rcuind]*100/average_lba)))
				sr.setResult('FAILED')
	else:
		sr.appendLog(11,'No Beam set in mode 1!!')
		sr.setResult('FAILED')
		# store station testlog	
		st_log.write('LBAmd1>: Sv=%s Pr=%s, No Beam set in mode 1!!\n' % (SeverityLevel[SeverityOfThisTest], PriorityLevel[PriorityOfThisTest]))
        f_log.close
	f_logfac.close
	rm_files(dir_name,'*')
#	os.popen("killall beamctl")

	sr.setId('LBAmd3>: ')
#
	res=os.popen3("rspctl --rcumode=3");
	print res
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
	averagesum=0
	for file_cnt in range(len(files)) :
		f, frames_to_process, rcu_nr  = open_file(files, file_cnt)
               	if frames_to_process > 0 : 
			sst_data = read_frame(f)
               		sst_subband = sst_data[subband_nr]
			meet_data[rcu_nr] = sst_subband
			averagesum=averagesum+sst_subband
			if debug:
	                	if rcu_nr==0:
                       			print ' waarde sst_subband 0 is ' + str(sst_subband)
                		if rcu_nr==2:
                			print ' waarde sst_subband 2 is ' + str(sst_subband)
				if rcu_nr==50:
					print ' waarde sst_subband 50 is ' + str(sst_subband)
		f.close
	average_lba=averagesum/num_rcu
#	if debug: 
	print 'average = ' + str(average_lba)
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
        f_log.close
	f_logfac.close
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
	
#	debug=1
	
	sr.setId('HBAmdt>: ')
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
	
	debug=0
	
	sr.setId('HBAmd5>: ')
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
        if len(sys.argv) < 2 :
	        subband_nr=155
        else :
		subband_nr = int(sys.argv[1])
        print ' Dir name is ' + dir_name
        if len(sys.argv) < 3 :
	        num_rcu=96
        else :
		num_rcu = int(sys.argv[2])
        print ' Number of RCUs is ' + str(num_rcu)
	print ' Number of Subband is ' + str(subband_nr)
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
                            f_logfac.write(str(element+1) + ' ' + str(rcuind) + ' ' + str(round(meet_data[rcuind]/ref_data[rcuind])) + '\n')  
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

################################################################################
# Main program
Message=('!!!     This station will be in use for a test! Please do not use the station!     !!!')

WriteAll(Message)
GotoSwlevel2()			# Set system in software level 2
CheckNtpd()			# Check the pps and GPS ST
##makeRSPVersionGold()		# make RSP Version gold ST
CheckRSPVersion()		# CHeck RSP Version ST
CheckRSPStatus()		# Check status bits form the RSP ST
##makeTBBVersionGold()		# make TBB Version ST
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
#cli.command('rspctl --rcuprsg=0') 
LBAtest()			# Check LBH and LBL antenna's in mode 1 and 3 ST
HBAModemTest()			# Test of the HBA server modems
HBAtest()			# Check HBA tiles in mode 5

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
res = os.popen3('swlevel 1')[1].readlines()

# Finaly move temporary logfile to final logfile
res = os.popen3("scp -rp %s %s" % (TestlogName , HistlogName))[1].readlines()
if debug: print res
time.sleep(1)
res = os.popen3("mv %s %s" % (TestlogName , TestlogNameFinalized))
if debug: print res
print ('TestlogName: ',TestlogName)
print ('HistlogName: ',HistlogName)
print ('TestlogNameFinalized: ',TestlogNameFinalized)


