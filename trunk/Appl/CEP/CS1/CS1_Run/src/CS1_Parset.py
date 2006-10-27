import time
import LOFAR_Parset

class CS1_Parset(LOFAR_Parset.Parset):

    def __init__(self):
        LOFAR_Parset.Parset.__init__(self)
        self.inputFromMemory = False
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
        for s in stationList:
            (x,y,z) = s.getPosition()
            positionlist.append(x)
            positionlist.append(y)
            positionlist.append(z)
        self['Observation.StationPositions'] = positionlist
        for i in range(0, len(stationList)):
            station = stationList[i]
            # todo: this doesn't work yet with multiple rspboards per station
            THKey = 'Input.Transport.Station%d.Rsp0' % i
            if self.inputFromMemory == True:
                self[THKey + '.th'] = 'NULL'
            else:
                self[THKey + '.th'] = 'ETHERNET'
            self[THKey + '.interface'] = 'eth1'
            self[THKey + '.sourceMac'] = s.getSrcMAC()
            self[THKey + '.destinationMac'] = s.getDstMAC()

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

    def setInterval(self, start, duration):
        self['Observation.StartTime'] = start
        self['Observation.StopTime'] = start + duration

    def setIntegrationTime(self, integrationTime):
        self['Storage.IntegrationTime'] = integrationTime

    def setMSName(self, msName):
        self['Storage.MSName'] = msName

    def getMSName(self):
        return self['Storage.MSName']

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
        sbs = list()
        for sb in range(0, self.nSubbands):
            sbs.append(self.firstSB + sb * subbandwidth)

        # create the frequencies for all subbands
        self['Observation.RefFreqs'] = '[' + ','.join(str(sb) for sb in sbs) + ']'
        self['Observation.NSubbands'] = self.nSubbands
        
        #the number of subbands should be dividable by the number of subbands per pset
        if not self.nSubbands % self.getInt32('General.SubbandsPerPset') == 0:
            raise Exception('Number of subbands(%d) in not dividable by the number of subbands per pset (%d).' % self.nSubbands, self['General.SubbandsPerPset'])
            
