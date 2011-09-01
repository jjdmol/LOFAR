#!/usr/bin/python

#
# Program to determine optimum AP delays RSP boards
# M.J. Norden
# Version 0.8                1-sep-2011

# 04 feb: creation script
# 1 apr: modifications
# 4 apr: Find the optimum delay from the test results
# 11 apr: Make table with optimum delay
# 14 apr: Write optimum values to config file
# 26 apr: New name config file PPSDelays.conf 
# 13 may: new format and directory delay config file
# 15 jul: removed C from name. Date in config file
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
from numpy import zeros,ones

################################################################################
# Init

# Variables
debug=0
clkoffset=1

# Variables Menno
checks=1
loops =0

lijst=[]
#evenref=[]
#oddref=[]

TestLogPath=('/localhome/data/')	# Logging remote (on Kis001)
HistLogPath=('/localhome/data/')	# Logging local (on station)


tm=strftime("%a, %d %b %Y %H:%M:%S", localtime())	# Determine system time
#tme=strftime("_%b_%d_%Y_%H.%M", localtime())		# Time for fileheader History log file
tme=strftime("%d-%b-%Y-%H%M", localtime())	# Time for fileheader History log file	
StIDlist = os.popen3('hostname -s')[1].readlines()	# Name of the station
#StID = str(StIDlist[0].strip('\n'))
StID = StIDlist[0][0:5]
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
op.add_option('-r', type='int', dest='rsp_nr',
  help='Provide number of rsp boards that will be used in this test',default=None)

opts, args = op.parse_args()


# - Option checks and/or reformatting
if opts.rsp_nr==None:
  op.error('Option -r must specify the number of rsp boards')
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
#logName = '/opt/stationtest/data/SUBR-%05d-%05d.dat' % (opts.rsp_nr, opts.tbb_nr)
#cli.command('rm -f /opt/stationtest/data/SUBR-%05d-%05d.dat', appLev) 
#logName = '/localhome/data/PPS-OPT-%s-%05s.dat' % (StID,tme)
logName = '/localhome/data/PPS-OPT-%s-%s.dat' % (StID,tme)
configName = '/opt/lofar/etc/%s-CHECK-PPSdelays.conf' % (StID)
#logName = '/localhome/data/SUBR-%05d.dat' % (opts.rsp_nr)
cli.command('rm -f /localhome/data/SUBR-%05d.dat', appLev) 

sr = testlog.Testlog(vlev, testId, logName)

sr.setResult('PASSED')

sr.setId('Station - ')
sr.appendLog(11,'')
sr.appendLog(1,' Station AP delay test %s' % logName)
sr.appendLog(11,'')


# Define config file

st_log = file(configName, 'w')
st_log.write('#\n')
st_log.write('# APdelay.conf for %s\n' % StID)
st_log.write('#\n')
st_log.write('# %s\n' % tme)
st_log.write('#\n')


################################################################################
# Initialise the variables
###

cnt = 0
max0 = 0
max1 = 0
max2 = 0
	
maxl0 = 0
maxl1 = 0
maxl2 = 0
	
index0 = 0
index1 = 0
index2 = 0

indexl0 = 0
indexl1 = 0
indexl2 = 0	
 

################################################################################
# Function Check clock speed 160MHz or 200MHz
###
def CheckClkSpeed():
	
	res = os.popen3('rspctl --clock')[1].readlines()
	b = res[0].lstrip('Sample frequency: clock=')
	if "200MHz" in b:
	   clock = 200
	else:
	   clock = 160
	
	return clock
	
################################################################################
# Reset PPS input delay to default and capture on rising edge
###
def DelayResetRise():
	
	res = os.popen3('python verify.py --brd %s --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge r --pps_delay 0' %(RspBrd,))
	time.sleep(1)	
	return 
		
################################################################################
# Reset PPS input delay to default and capture on rising edge
###
def DelayRise():
	
	res = os.popen3('python verify.py --brd %s --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge r --pps_delay 1' %(RspBrd,))
	time.sleep(1)	
	return 
	
################################################################################
# Reset PPS input delay to default and capture on faling edge
###
def DelayResetFall():
	
	res = os.popen3('python verify.py --brd %s --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge f --pps_delay 0' %(RspBrd,))
	time.sleep(1)	
	return 
		
################################################################################
# Reset PPS input delay to default and capture on faling edge
###
def DelayFall():
	
	res = os.popen3('python verify.py --brd %s --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge f --pps_delay 1' %(RspBrd,))
	time.sleep(1)	
	return 

################################################################################
# Determine the maximum (max) number of good delays (index) for each subrack (sub)
###
def PrintMeas():
	
	global cnt,max0,max1,max2,index0,index1,index2
	global maxl0,maxl1,maxl2,indexl0,indexl1,indexl2
	
	
	sub0 = meas[0:15]
  	sub1 = meas[16:31]
  	sub2 = meas[32:47]
		
		
		
	# local maximum maxl.
	# local index indexl.
	# global maximum max.
	# global index index.
	
	# subrack 0 
	if sum(sub0) == 0:
	   sub0 = [0]
	   maxl0 +=1
	   if maxl0 == 1:
	      indexl0 = cnt	     
	else:
           print sub0		
	   sub0 = [1]
	   maxl0 = 0
	   indexl0 = 0	
	if maxl0 > max0:
           max0 = maxl0		
	   index0 = indexl0
	   
	# subrack 1	    
	if sum(sub1) == 0:
	   sub1 = [0] 
	   maxl1 +=1
	   if maxl1 == 1:
	      indexl1 = cnt	
	else:
           print sub1
	   sub1 = [1]	
	   maxl1 = 0
	   indexl1 = 0
	if maxl1 > max1:
           max1 = maxl1		
	   index1 = indexl1
	   	  
	# subrack 2 	  
	if sum(sub2) == 0:
	   sub2 = [0] 
	   maxl2 +=1
	   if maxl2 == 1:
	      indexl2 = cnt	
	else:
	   print sub2	
	   sub2 = [1]		
	   maxl2 = 0
	   indexl2 = 0	
	if maxl2 > max2:
           max2 = maxl2		
	   index2 = indexl2
  	sr.appendLog(11,'%2d %s %s %s ' ' %2d '  ' %2d '  ' %2d '  ' %2d '  ' %2d '  ' %2d' % (cnt,sub0,sub1,sub2,max0,max1,max2,index0,index1,index2))
	#print meas
	return
		
################################################################################
# Function make odd and even reference list
###
		
def PrintConfig():
	
	i = 1
	st_log.write('48 [ \n')
	while i < 49:
	  if i < 17:
	    st_log.write('%d ' % (index0+(max0/2)))
	    if i == 16: st_log.write('\n')
	  elif i<33:
	    st_log.write('%d ' % (index1+(max1/2)))
	    if i == 32: st_log.write('\n')
	  else:
	    st_log.write('%d ' % (index2+(max2/2))) 	  
	  i +=1
	st_log.write('\n]' ) 	  
	return	
################################################################################
# Function make odd and even reference list
###
def OddEvenReference(lijst):
	
	global evenref,oddref
	# make reference list for odd/even second
	evenref=[]
	oddref=[]
	lijst=[]
			
        a = CheckRSPStatus(lijst) 
	if a:
           evenref=lijst
           lijst=[]
           time.sleep(2)
           CheckRSPStatus(lijst)
           oddref=lijst
        else:
           oddref=lijst
           lijst=[]
           time.sleep(2)
           CheckRSPStatus(lijst)
           evenref=lijst
	
	return (evenref,oddref)
################################################################################
# Check difference between current status and reference
###
def CheckDiff(lijst):
	
	global meas
        # make empty list for measurement results
	meas = zeros(len(evenref))
        #meas =["0" for i in range (len(evenref))]
        i=0
	while i < 10:
           lijst=[]
           time.sleep(2)
           a = CheckRSPStatus(lijst) # a is odd or even
	   if a:
              cnt=0
	      while cnt < len(evenref):
                 if lijst[cnt] != evenref[cnt]:
		   meas[cnt] = 1
		 cnt+=1
           else:
              cnt=0 
	      while cnt < len(oddref):
                 if lijst[cnt] != oddref[cnt]:
		   meas[cnt] = 1
		 cnt+=1
           i +=1   

#############################################################################
# Function Check RSP status bytes
#
# returns False (Odd) or True (Even) in CheckRSPStatus and list with DIFF values
def CheckRSPStatus(lijst):
	
	time.sleep(1)
	res = os.popen3('rspctl --status')[1].readlines()
	
	linecount=0
	if len(res) > 0:
		for line in res:
			sync=line.find('RSP[ 0] Sync')
			if sync==0: break
			linecount+=1
	# finds start line of DIFF table 
	for rsp in range(opts.rsp_nr):
		x = res[linecount+rsp].split( )
		for sync in range(1, 5):
			diff = res[linecount+rsp*5+sync].lstrip('RSP').strip('[').strip(':').split()
			lijst.append(diff[2])
			if diff[5] == '195312' or '156250': 
			   even = True
			elif diff[5] == '195313' or '156250':
		 	   even = False	 
			else:
			   print "fout"
			
	return even

################################################################################
# Main program
if __name__ == '__main__':
	
  OddEvenReference(lijst)
  print 'dit is de even referentie', evenref	
  print 'dit is de oneven referentie', oddref		
  
  sr.appendLog(11,' test rising edge delay')
  sr.appendLog(11,'')
  a = CheckClkSpeed()
  if a == 200:
	sr.appendLog(11,' Clock speed is 200 MHz')  
  else:
  	sr.appendLog(11,' Clock speed is 160 MHz')  
  sr.appendLog(11,'')
  
  sr.appendLog(11,' i s0  s1  s2   m0  m1  m2  i0  i1  i2')

  # find optimum value delay AP for rising edge 
  while cnt < 100:
    CheckDiff(lijst)
    PrintMeas()
    #DelayRise()
    cnt +=1
  PrintConfig()  
  st_log.close()
  sr.appendLog(11,'')
  sr.appendLog(11,' d1 d2 d3')
  sr.appendLog(11,' %2d %2d %2d' % (index0+(max0/2),index1+(max1/2),index2+(max2/2)))
  
################################################################################
# End of the subrack test

  sr.setId('Subrack - ')
  dt = sr.getRunTime()
  sr.appendLog(2,'Duration: %02dm:%02ds' % (dt/60 % 60, dt % 60))
  sr.appendLog(0,sr.getResult())
  sr.closeLog()

################################################################################

