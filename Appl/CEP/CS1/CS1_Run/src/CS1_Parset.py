import time
import LOFAR_Parset
import math

class CS1_Parset(LOFAR_Parset.Parset):

    def __init__(self):
        LOFAR_Parset.Parset.__init__(self)
        self.inputFromMemory = False
        self.inputFromRaw = False
        self.stationList = list()

    def setClock(self, clock):
        self.clock = clock
        if self.clock == '160MHz':
            self['Observation.NSubbandSamples'] = 155648
            self['Observation.SampleRate'] = 156250
        elif self.clock == '200MHz':
            self['Observation.NSubbandSamples'] = 196608
            self['Observation.SampleRate'] = 195312.5
        self.updateSBValues()
        
    def getClockString(self):
        return self.clock

    def setSubbands(self, first, number):        
        self.firstSB = int(1e6 * float((first.split('MHz')[0])))
        self.nSubbands = number
        self.updateSBValues()

    def setStations(self, stationList):
        self.stationList = stationList
        self['Observation.NStations'] = len(stationList)
        self['Input.NRSPBoards'] = len(stationList)
        self['Storage.StorageStationNames'] = [s.getName() for s in stationList]
        positionlist = list()
        phaseslist = list()
        for s in stationList:
            (x,y,z) = s.getPosition()
            (xp, yp, zp) = s.getPhaseCentre()
            positionlist.append(x)
            positionlist.append(y)
            positionlist.append(z)
            phaseslist.append(xp)
            phaseslist.append(yp)
            phaseslist.append(zp)
        self['Observation.StationPositions'] = positionlist
        self['Observation.PhaseCentres'] = phaseslist

        for i in range(0, len(stationList)):
            station = stationList[i]
            # todo: this doesn't work yet with multiple rspboards per station
            THKey = 'Input.Transport.Station%d.Rsp0' % i
            if self.inputFromMemory == True:
                self[THKey + '.th'] = 'NULL'
            elif self.inputFromRaw:
                self[THKey + '.th'] = 'ETHERNET'
            else:
                self[THKey + '.th'] = 'UDP'
            self[THKey + '.interface'] = 'eth1'
            self[THKey + '.sourceMac'] = station.getSrcMAC()
            self[THKey + '.destinationMac'] = station.getDstMAC()
            self[THKey + '.port'] = station.getDstPort()
            self[THKey + '.host'] = station.getSrcIP()
            

    def setInputToMem(self):
        self.inputFromMemory = True
        for i in range(0, len(self.stationList)):
            station = self.stationList[i]
            # todo: this doesn't work yet with multiple rspboards per station
            THKey = 'Input.Transport.Station%d.Rsp0' % i
            self[THKey + '.th'] = 'NULL'

    def getInputNodes(self):
        return [s.getInputNode() for s in self.stationList]

    def getNStations(self):
        return int(self['Observation.NStations'])
        
    def getFirstSubband(self):
        return self.firstSB

    def setBeamdir(self):
	beamdir = self.getStringVector('Observation.BeamDirections')
	tmp = beamdir[0].split('[')
	ra = tmp[1]
	
	tmp1 = beamdir[1].split(']')
	dec = tmp1[0]
	
	#tests = beamdir.split(",")
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
	
	self['Observation.BeamDirections'] = [radeg*torad, decdeg*torad]	
	
    def setInterval(self, start, duration):
        self['Observation.StartTime'] = start
        self['Observation.StopTime'] = start + duration

    def setIntegrationTime(self, integrationTime):
        self['Storage.IntegrationTime'] = integrationTime

    def setMSName(self, msName):
        self['Storage.MSNames'] = msName

    def getMSName(self):
        return self['Storage.MSNames']

    def getNCells(self):
        subbands = self.getInt32('Observation.NSubbands')
        psetspercell = self.getInt32('BGLProc.PsetsPerCell')
        subbandsperpset = self.getInt32('General.SubbandsPerPset')
        return subbands / (psetspercell * subbandsperpset)

    def updateSBValues(self):
        if self.clock == '160MHz':
            subbandwidth = 156250
        elif self.clock == '200MHz':
            subbandwidth = 195312.5
        
        if not self.__dict__.has_key('nSubbands'):
            return
	    
	subbandIDs = self.getInt32Vector('Observation.SubbandIDs')
	if len(subbandIDs) != self.nSubbands:
	    raise Exception('nSubbands(%d) !=  SubbandIDs(%d).' % (self.nSubbands, len(subbandIDs)))

	sbs = list()
        for sb in range(0, len(subbandIDs)):
            sbs.append(subbandIDs[sb] * subbandwidth)

        # create the frequencies for all subbands
        self['Observation.RefFreqs'] = '[' + ', '.join(str(sb) for sb in sbs) + ']'
        self['Observation.NSubbands'] = self.nSubbands
        
        #the number of subbands should be dividable by the number of subbands per pset
        if not self.nSubbands % self.getInt32('General.SubbandsPerPset') == 0:
            raise Exception('Number of subbands(%d) in not dividable by the number of subbands per pset (%d).' % self.nSubbands, self['General.SubbandsPerPset'])
            
