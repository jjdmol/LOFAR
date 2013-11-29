#!/usr/bin/python

#
# Sheck state of the RSP by polling with rsuctl3
# H. Meulman
# Version 0.1                17 aug 2012


# 17 aug 2012: Start


# todo:

#import sys
#import thread
#import threading
#import array
#import os
import time
#import commands
#import operator
#import math
#import numpy
import subprocess as sp

# Variables
debug=1
RSPs=['00','01','02','03','04','05','06','07','08','09','0a','0b']
#RSPs=['04']
RSPlog=['RSPflt:']

################################################################################
# Function RSPrunning
# Checks if RSP is running
# flt = fault code:
# 0 = RSP is running in Factory image = OK
# 1 = RSP is running in User image
# 2 = RSP is running but AP not running
# 3 = RSP FATAL protocol error
# 4 = RSP not running

def isRSPrunning(board):
	
	global RSPlog
	macAdr=('10:fa:00:00:%s:00' % board)
	proc = sp.Popen(['sudo','rsuctl3','-q','-m',macAdr,'-V'], shell=False, stdout=sp.PIPE, stderr=sp.PIPE)
	print ('RSP %s' % board),
	
	timeout = 6
	while proc.poll() == None and timeout > 0:
		time.sleep(1.0)    
		timeout -= 1    
		if debug >= 2: print "busy"
	
	if timeout > 0:  
		output = proc.communicate()[1]	# rsuctl3 sends back returncode via stderr, so use [1] here!
		flt=5
		if debug >= 2:print "output:" + output
		#if debug >= 1:print "RSP is running"
		if 'Factory' in output: flt=0
		if 'User image' in output: flt=1
		if 'AP[' not in output: flt=2
		if 'FATAL' in output: flt=3
		if flt==0:
			if debug >= 1:print(' is running in Factory Image')
		if flt==1:
			if debug >= 1:print(' is running in User Image!')
		if flt==2:
			if debug >= 1:print ' ,AP\'s not running!'
		if flt==3:
			if debug >= 1:print ' FATAL protocol error'
		if flt==5:
			if debug >= 1:print ' misc error'
	else:
		flt=4
		PrID = proc.pid
		if debug >= 2:print 'ID=', PrID
#		proc.terminate()
		res = sp.Popen(["sudo kill %s" % PrID], shell=True, stdout=sp.PIPE)
		if debug >= 1:print "RSP not running!"
		if debug >= 2:print "Process terminated"
	RSPlog=RSPlog + [str(flt)]
#	time.sleep(0.5)
	return

################################################################################
# Main program
for RSP in RSPs:
	isRSPrunning(RSP)
print RSPlog
