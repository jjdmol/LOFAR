import time
import datetime
import LOFAR_Parset
import math
import sys

class CS1_Parset(LOFAR_Parset.Parset):

    def __init__(self):
        LOFAR_Parset.Parset.__init__(self)
        self.inputFromMemory = False
        self.inputFromRaw = False
        self.stationList = list()

    def setClock(self, clock):
        self.clock = clock
        
        if self.clock == '160MHz':
	    self['Observation.sampleClock'] = 160
	    self['OLAP.BGLProc.integrationSteps'] = 608
        elif self.clock == '200MHz':
	    self['Observation.sampleClock'] = 200
	    self['OLAP.BGLProc.integrationSteps'] = 768
        self.updateSBValues()
        
    def getClockString(self):
        return self.clock

    def setSubbands(self, first, number):        
        self.firstSB = int(1e6 * float((first.split('MHz')[0])))
        self.nSubbands = number
        self.updateSBValues()

    def setStations(self, stationList):
        self.stationList = stationList
        self['OLAP.nrRSPboards'] = len(stationList)
        self['OLAP.storageStationNames'] = [s.getName() for s in stationList]
	
    def setInputToMem(self):
        self.inputFromMemory = True
	self['OLAP.OLAP_Conn.station_Input_Transport'] = 'NULL'

    def getInputNodes(self):
        inputNodelist = list()
	
	for s in self.stationList:
	    name = self.getString('PIC.Core.' + s.getName() + '.port')
	    name=name.split(":")
	    name=name[0].strip("lii")
	    inputNodelist.append(int(name))
    
        return inputNodelist
	
    def getNStations(self):
        return int(len(self.stationList))
        
    def getFirstSubband(self):
        return self.firstSB

    def setBeamdir(self):
	ra = self.getStringVector('Observation.Beam.angle1')[0]
	ra = ra.strip("[")
	ra = ra.strip("]")
	
	dec = self.getStringVector('Observation.Beam.angle2')[0]
	dec = dec.strip("[")
	dec = dec.strip("]")

	#ra = '18:32:45.213'
	#dec = '-01:47:26.1'
	
	torad = math.pi/180.0
	ras=ra.split(":")
	
	if int(ras[0]) not in range(25):
	    raise Exception('ras[0]=%d is not in the range 0 to 24' % int(ras[0]))
	
	if int(ras[1]) not in range(60):
	    raise Exception('ras[1]=%d is not in the range 0 to 59' % int(ras[1]))
	
	if float(ras[2]) < float(0.0) or float(ras[2]) > float(60.0):
	    raise Exception('ras[2]=%f is not in the range 0.0 to 60.0' % float(ras[2]))
	
	radeg = 15*((int(ras[0])+int(ras[1])/60.0) + (float(ras[2])/3600.0)) # 1hr=15deg
	if radeg < 0 or radeg > 360:
	    raise Exception('ERROR: RA not in the range 0 to 360 degrees')
	
	decs=dec.split(":")
	if int(decs[0]) < -38 or int(decs[0]) > 90:
	    raise Exception('decs[0]=%d is not in the range -38 to 90' % int(decs[0]))
	
	if int(decs[1]) not in range(60):
	    raise Exception('decs[1]=%d is not in the range 0 to 59' % int(decs[0]))

	if float(decs[2]) < float(0.0) or float(decs[2]) > float(60.0):
	    raise Exception('decs[2]=%f is not in the range 0.0 to 60.0' % float(decs[2]))
	
	if '-' in dec: # minus sign refers to the whole of the declination
	    decdeg = (int(decs[0])-int(decs[1])/60.0) - (float(decs[2])/3600.0)
	    if decdeg < -38:
	        raise Exception('ERROR: We cannot observe that far south')
	else:
	    decdeg = (int(decs[0])+int(decs[1])/60.0) + (float(decs[2])/3600.0)	
	    if decdeg > 90:
	        raise Exception('ERROR: There is nowhere north of 90degrees')
	
	self['Observation.Beam.angle1'] = [radeg*torad]
	self['Observation.Beam.angle2'] = [decdeg*torad]
	
    def setInterval(self, start, duration):
        self['Observation.startTime'] = datetime.datetime.fromtimestamp(start)
        self['Observation.stopTime'] = datetime.datetime.fromtimestamp(start + duration)

    def setIntegrationTime(self, integrationTime):
	if self.getBool('OLAP.IONProc.useGather'):
	  self['OLAP.IONProc.integrationSteps']     = integrationTime
	  self['OLAP.StorageProc.integrationSteps'] = 1
	else:
	  self['OLAP.IONProc.integrationSteps']     = 1
	  self['OLAP.StorageProc.integrationSteps'] = integrationTime

    def setMSName(self, msName):
        self['Observation.MSNameMask'] = msName

    def getMSName(self):
        return self['Observation.MSNameMask']

    def getNCells(self):
        subbands = len(self.getInt32Vector('Observation.subbandList'))
        psetspercell = self.getInt32('OLAP.BGLProc.psetsPerCell')
        subbandsperpset = self.getInt32('OLAP.subbandsPerPset')
        return subbands / (psetspercell * subbandsperpset)

    def updateSBValues(self):
        if self.clock == '160MHz':
            subbandwidth = 156250
        elif self.clock == '200MHz':
            subbandwidth = 195312.5
        
        if not self.__dict__.has_key('nSubbands'):
            return

	subbandIDs = self.getInt32Vector('Observation.subbandList')
	if len(subbandIDs) != self.nSubbands:
	    raise Exception('Number of subbandList(%d) in parset file is not equal with command line option: --subbands(%d).' % (len(subbandIDs), self.nSubbands))

        if self.getInt32('Observation.nyquistZone') not in range(1,4):
	    print 'Use nyquistZone 1, 2 or 3'
	    sys.exit(0)
	    
	sbs = list()
        for sb in range(0, len(subbandIDs)):
            sbs.append((self.getInt32('Observation.nyquistZone')-1)*(self.getInt32('Observation.sampleClock')*1000000/2)  + subbandIDs[sb] * subbandwidth)

        # create the frequencies for all subbands
        self['Observation.RefFreqs'] = '[' + ', '.join(str(sb) for sb in sbs) + ']'
        self['Observation.NSubbands'] = self.nSubbands
	
        #the number of subbands should be dividable by the number of subbands per pset
        if not self.nSubbands % self.getInt32('OLAP.subbandsPerPset') == 0:
            raise Exception('Number of subbands(%d) in not dividable by the number of subbands per pset (%d).' % self.nSubbands, self['OLAP.subbandsPerPset'])
            
