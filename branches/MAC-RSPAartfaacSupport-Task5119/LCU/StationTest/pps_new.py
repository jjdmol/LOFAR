#!/usr/bin/python

#
# Program to determine optimum AP delays RSP boards
# M.J. Norden
# Version 0.7                07-feb-2012

# 04 feb: creation script
# 1 apr: modifications
# 4 apr: Find the optimum delay from the test results
# 11 apr: Make table with optimum delay
# 14 apr: Write optimum values to config file
# 26 apr: New name config file PPSDelays.conf 
# 13 may: new format and directory delay config file
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
StID = str(StIDlist[0].rstrip('C\n'))
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
configName = '/opt/lofar/etc/%s-PPSdelays.conf' % (StID)
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
st_log.write('# PPSDelays.conf for %s\n' % StID)
st_log.write('#\n')
st_log.write('# %s\n' % (tme))
st_log.write('#\n')

################################################################################
# Initialise the variables
###

cnt = 0
max0 = 0
max1 = 0
max2 = 0
max3 = 0
max4 = 0
max5 = 0
max6 = 0
max7 = 0
max8 = 0
max9 = 0
max10 = 0
max11 = 0
	
maxl0 = 0
maxl1 = 0
maxl2 = 0
maxl3 = 0
maxl4 = 0
maxl5 = 0
maxl6 = 0
maxl7 = 0
maxl8 = 0
maxl9 = 0
maxl10 = 0
maxl11 = 0
	
index0 = 0
index1 = 0
index2 = 0
index3 = 0
index4 = 0
index5 = 0
index6 = 0
index7 = 0
index8 = 0
index9 = 0
index10 = 0
index11 = 0

indexl0 = 0
indexl1 = 0
indexl2 = 0
indexl3 = 0
indexl4 = 0
indexl5 = 0
indexl6 = 0
indexl7 = 0
indexl8 = 0
indexl9 = 0
indexl10 = 0
indexl11 = 0

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
	
	global cnt,max0,max1,max2,max3,max4,max5,max6,max7,max8,max9,max10,max11,index0,index1,index2,index3,index4,index5,index6,index7,index8,index9,index10,index11
	global maxl0,maxl1,maxl2,maxl3,maxl4,maxl5,maxl6,maxl7,maxl8,maxl9,maxl10,maxl11,indexl0,indexl1,indexl2,indexl3,indexl4,indexl5,indexl6,indexl7,indexl8,indexl9,indexl10,indexl11
	
	
	rsp0 = meas[0:3]
  	rsp1 = meas[4:7]
  	rsp2 = meas[8:11]
	rsp3 = meas[12:15]
  	rsp4 = meas[16:19]
  	rsp5 = meas[20:23]
	rsp6 = meas[24:27]
  	rsp7 = meas[28:31]
  	rsp8 = meas[32:35]
	rsp9 = meas[36:39]
  	rsp10 = meas[40:43]
	rsp11 = meas[44:47]
  			
	# rsp 0 
	if sum(rsp0) == 0:
	   rsp0 = [0]
	   maxl0 +=1
	   if maxl0 == 1:
	      indexl0 = cnt	     
	else:
	   rsp0 = [1]
	   maxl0 = 0
	   indexl0 = 0	
	if maxl0 > max0:
           max0 = maxl0		
	   index0 = indexl0
	   
	# rsp 1	    
	if sum(rsp1) == 0:
	   rsp1 = [0] 
	   maxl1 +=1
	   if maxl1 == 1:
	      indexl1 = cnt	
	else:
	   rsp1 = [1]	
	   maxl1 = 0
	   indexl1 = 0
	if maxl1 > max1:
           max1 = maxl1		
	   index1 = indexl1
	   	  
	# rps 2 	  
	if sum(rsp2) == 0:
	   rsp2 = [0] 
	   maxl2 +=1
	   if maxl2 == 1:
	      indexl2 = cnt	
	else:
	   rsp2 = [1]		
	   maxl2 = 0
	   indexl2 = 0	
	if maxl2 > max2:
           max2 = maxl2		
	   index2 = indexl2
	   
	# rsp 3 
	if sum(rsp3) == 0:
	   rsp3 = [0]
	   maxl3 +=1
	   if maxl3 == 1:
	      indexl3 = cnt	     
	else:
	   rsp3 = [1]
	   maxl3 = 0
	   indexl3 = 0	
	if maxl3 > max3:
           max3 = maxl3		
	   index3 = indexl3
	      
	# rsp 4 
	if sum(rsp4) == 0:
	   rsp4 = [0]
	   maxl4 +=1
	   if maxl4 == 1:
	      indexl4 = cnt	     
	else:
	   rsp4 = [1]
	   maxl4 = 0
	   indexl4 = 0	
	if maxl4 > max4:
           max4 = maxl4		
	   index4 = indexl4   
	   
	# rsp 5 
	if sum(rsp5) == 0:
	   rsp5 = [0]
	   maxl5 +=1
	   if maxl5 == 1:
	      indexl5 = cnt	     
	else:
	   rsp5 = [1] 
	   maxl5 = 0
	   indexl5 = 0	
	if maxl5 > max5:
           max5 = maxl5		
	   index5 = indexl5      
	   
	# rsp 6 
	if sum(rsp6) == 0:
	   rsp6 = [0]
	   maxl6 +=1
	   if maxl6 == 1:
	      indexl6 = cnt	     
	else:
	   rsp6 = [1] 
	   maxl6 = 0
	   indexl6 = 0	
	if maxl6 > max6:
           max6 = maxl6		
	   index6 = indexl6      
	      
	# rsp 7 
	if sum(rsp7) == 0:
	   rsp7 = [0]
	   maxl7 +=1
	   if maxl7 == 1:
	      indexl7 = cnt	     
	else:
	   rsp7 = [1] 
	   maxl7 = 0
	   indexl7 = 0	
	if maxl7 > max7:
           max7 = maxl7		
	   index7 = indexl7            
	   
	# rsp 8 
	if sum(rsp8) == 0:
	   rsp8 = [0]
	   maxl8 +=1
	   if maxl8 == 1:
	      indexl8 = cnt	     
	else:
	   rsp8 = [1] 
	   maxl8 = 0
	   indexl8 = 0	
	if maxl8 > max8:
           max8 = maxl8		
	   index8 = indexl8 
	      
	# rsp 9 
	if sum(rsp9) == 0:
	   rsp9 = [0]
	   maxl9 +=1
	   if maxl9 == 1:
	      indexl9 = cnt	     
	else:
	   rsp9 = [1] 
	   maxl9 = 0
	   indexl9 = 0	
	if maxl9 > max9:
           max9 = maxl9		
	   index9 = indexl9       
	   
	# rsp 10
	if sum(rsp10) == 0:
	   rsp10 = [0]
	   maxl10 +=1
	   if maxl10 == 1:
	      indexl10 = cnt	     
	else:
	   rsp10 = [1] 
	   maxl10 = 0
	   indexl10 = 0	
	if maxl10 > max10:
           max10 = maxl10		
	   index10 = indexl10    
	   
	# rsp 11
	if sum(rsp11) == 0:
	   rsp11 = [0]
	   maxl11 +=1
	   if maxl11 == 1:
	      indexl11 = cnt	     
	else:
	   rsp11 = [1] 
	   maxl11 = 0
	   indexl11 = 0	
	if maxl11 > max11:
           max11 = maxl11		
	   index11 = indexl11       
	   
  	sr.appendLog(11,'%2d %s %s %s %s %s %s %s %s %s %s %s %s ' % (cnt,rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11))
	return
		
################################################################################
# Function make odd and even reference list
###
		
def PrintConfig():
	
	
	i = 1
	st_log.write('48 [ \n')
	while i < 49:
	  if i == 17 or i == 33:st_log.write('\n')  	
	  if i < 5:
	      st_log.write('%d ' % (index0+(max0/2)))
	  elif i<9:
	      st_log.write('%d ' % (index1+(max1/2)))
	  elif i<13:
	      st_log.write('%d ' % (index2+(max2/2)))  
	  elif i<17:
	      st_log.write('%d ' % (index3+(max3/2)))  
	  elif i<21:
	      st_log.write('%d ' % (index4+(max4/2)))
	  elif i<25:
	      st_log.write('%d ' % (index5+(max5/2)))
	  elif i<29:
	      st_log.write('%d ' % (index6+(max6/2)))
	  elif i<33:
	      st_log.write('%d ' % (index7+(max7/2)))
	  elif i<37:
	      st_log.write('%d ' % (index8+(max8/2)))
	  elif i<41:
	      st_log.write('%d ' % (index9+(max9/2)))
	  elif i<45:
	      st_log.write('%d ' % (index10+(max10/2)))
	  else:
	      st_log.write('%d ' % (index11+(max11/2))) 	  
	  i +=1
	st_log.write('\n]' ) 	  
	return	
	    
################################################################################
# Function make odd and even reference list
###
		
def PrintConfig_new():
	
	
	cnt = 0
	st_log.write('48 [ \n')
	while cnt < 12:
	   if cnt == 4 or cnt == 8:st_log.write('\n')  	
	   b = 0	
	   while b < 4:
	      value = int('index%d' % (cnt))
	          
	      st_log.write('%d ' % (value))
	      b +=1
	   cnt+=1  
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
			if diff[5] == '195312': 
			   even = True
			elif diff[5] == '195313':
		 	   even = False	 
			else:
			   print "fout"
			
	return even

################################################################################
# Main program
if __name__ == '__main__':
  
  sr.appendLog(11,' test rising edge delay')
  sr.appendLog(11,'')
  sr.appendLog(11,' i r0  r1  r2  r3  r4  r5  r6  r7  r8  r9  r10 r11')

  # find optimum value delay AP for rising edge 
  while cnt < 64:
    OddEvenReference(lijst)
    #sr.appendLog(11,' %s' % evenref)
    #sr.appendLog(11,' %s' % oddref)
    CheckDiff(lijst)
    PrintMeas()
    DelayRise()
    cnt +=1
  PrintConfig()  
  st_log.close()
  sr.appendLog(11,'')
  sr.appendLog(11,' d0 d1 d2 d3 d4 d5 d6 d7 d8 d9 d10 d11')
  sr.appendLog(11,' %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d' % (index0+(max0/2),index1+(max1/2),index2+(max2/2),index3+(max3/2),index4+(max4/2),index5+(max5/2),index6+(max6/2),index7+(max7/2),index8+(max8/2),index9+(max9/2),index10+(max10/2),index11+(max11/2)))
  
################################################################################
# End of the subrack test

  sr.setId('Subrack - ')
  dt = sr.getRunTime()
  sr.appendLog(2,'Duration: %02dm:%02ds' % (dt/60 % 60, dt % 60))
  sr.appendLog(0,sr.getResult())
  sr.closeLog()

################################################################################

