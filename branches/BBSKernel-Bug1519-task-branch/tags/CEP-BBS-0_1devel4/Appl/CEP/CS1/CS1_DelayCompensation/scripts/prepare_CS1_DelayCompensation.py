#!/usr/bin/env python

import math
import time
import datetime
import os
import sys
import copy

class CS1_Parset(object):

    def __init__(self):
        self.stationList = list()
	self.parameters = dict()

    def readFromFile(self, fileName):
        lastline = ''
        for line in open(fileName, 'r').readlines():
            lastline = lastline + line.split('#')[0]
            lastline = lastline.rstrip()
            if len(lastline) > 0 and lastline[-1] == '\\':
                lastline = lastline[:-1]
            elif '=' in lastline:
                key, value = lastline.split('=')
                self.parameters[key.strip()] = value.strip()
                lastline = ''

    def writeToFile(self, fileName):
        outf = open(fileName, 'w')
        for key, value in sorted(self.parameters.iteritems()):
            outf.write(key + ' = ' + str(value) + '\n')
        outf.close()

    def __contains__(self, key):
        return key in self.parameters
        
    def __setitem__(self, key, value):
        self.parameters[key] = value

    def __getitem__(self, key):
        return self.parameters[key]

    def getInt32Vector(self, key):
        ln = self.parameters[key]
	ln_tmp = ln.split('[')
        line = 	ln_tmp[1].split(']')
        return [int(lp) for lp in line[0].split(',')]

    def getInt32(self, key):
        return int(self.parameters[key])

    def getStringVector(self, key):
        line = self.parameters[key]
        line.strip('[').strip(']')
        return line.split(',')

    def getString(self, key):
        return self.parameters[key]

    def getFloat(self, key):
        return float(self.parameters[key])

	
if __name__ == '__main__':

    # create the parset
    parset = CS1_Parset() 
    stationList = list()
    
    if os.path.exists("../share/DelayCompensation.parset"):
       
        #read keys from parset file: Transpose.parset
        parset.readFromFile('../share/DelayCompensation.parset')

        '''
        if parset.getString('OLAP.OLAP_Conn.station_Input_Transport') == 'NULL':
            # Read from memory!
            parset['Observation.startTime'] = datetime.datetime.fromtimestamp(1)
        else:
            start=int(time.time() + 90)
            parset['Observation.startTime'] = datetime.datetime.fromtimestamp(start)
	
	duration = 300

        parset['Observation.stopTime'] = datetime.datetime.fromtimestamp(start + duration) 
        '''

	if parset.getString('OLAP.OLAP_Conn.input_DelayComp_Transport') == 'Null':
	    parset['OLAP.OLAP_Conn.input_DelayComp_Transport']	 = 'NULL'
	    
	if parset.getString('OLAP.OLAP_Conn.input_BGLProc_Transport') == 'Null':
	    parset['OLAP.OLAP_Conn.input_BGLProc_Transport']	 = 'NULL'
	    
	if parset.getString('OLAP.OLAP_Conn.station_Input_Transport') == 'Null':
	    parset['OLAP.OLAP_Conn.station_Input_Transport']	 = 'NULL'

	if parset.getString('OLAP.OLAP_Conn.BGLProc_Storage_Transport') == 'Null':
	    parset['OLAP.OLAP_Conn.BGLProc_Storage_Transport']	 = 'NULL'
	
	#get the stations
	stationList = parset.getStringVector('OLAP.storageStationNames')
	parset['OLAP.nrRSPboards'] = len(stationList)
	
        if parset.getInt32('Observation.sampleClock') == 160:
	    parset['OLAP.BGLProc.integrationSteps'] = 608
        elif parset.getInt32('Observation.sampleClock') == 200:
	    parset['OLAP.BGLProc.integrationSteps'] = 768
	
	parset.writeToFile('../share/DelayCompensation.parset')
	    
    else:
        print '../share/DelayCompensation.parset does not exist!'
        sys.exit(0) 	
	
