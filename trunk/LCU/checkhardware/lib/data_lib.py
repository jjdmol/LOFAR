#!/usr/bin/python
# data lib

from general_lib import *
from lofar_lib import *
from search_lib import *
import os
import numpy as np
import logging
from time import sleep

test_version = '0515'

logger = None
def init_data_lib():
    global logger
    logger = logging.getLogger()
    logger.debug("init logger data_lib")

# get and return recorded data in various ways
class cRCUdata:
    global logger
    def __init__(self, n_rcus, minvalue=1):
        self.n_rcus   = n_rcus
        self.frames   = 0
        self.clock    = 200.0
        self.minvalue = minvalue
        self.active_rcus = range(self.n_rcus)
        self.mask = [] # mask selects subbands to block
        self.reset()

    def reset(self):
        self.ssData = np.ones((self.n_rcus, 1, 512), np.float64)
        self.testSignal_X   = -1.0
        self.testSubband_X  = 0
        self.testSignal_Y   = -1.0
        self.testSubband_Y  = 0

    def setActiveRcus(self, rcus):
        self.active_rcus = rcus

    def setInActiveRcus(self, rcus):
        self.active_rcus = range(self.n_rcus)
        for i in rcus:
            self.active_rcus.remove(i)

    def getActiveRcus(self, pol='XY'):
        rcus = []
        for i in self.active_rcus:
            if pol in ('X', 'x') and i % 2 == 0:
                rcus.append(i)
            if pol in ('Y', 'y') and i % 2 == 1:
                rcus.append(i)
            if pol in ('XY', 'xy'):
                rcus.append(i)        
        return (rcus)

    def setMask(self, blocked_subbands):
        self.mask = blocked_subbands

    def getMask(self):
        return (self.mask)
    
    def record(self, rec_time=2, read=True, slow=False):
        removeAllDataFiles()
        self.reset()
        
        if slow == True:
            x_list = []
            y_list = []
            for i in sorted(self.active_rcus):
                if i % 2 == 0:
                    x_list.append(i)
                else:
                    y_list.append(i)    
            rcus = selectStr(x_list)
            logger.debug("Wait %d seconds while recording X data" %(rec_time))
            rspctl('--statistics --duration=%d --integration=1 --directory=%s --select=%s' %(rec_time, dataDir(), rcus), wait=0.0)
            
            rcus = selectStr(y_list)
            logger.debug("Wait %d seconds while recording Y data" %(rec_time))
            rspctl('--statistics --duration=%d --integration=1 --directory=%s --select=%s' %(rec_time, dataDir(), rcus), wait=0.0)
        else:    
            rcus = selectStr(self.active_rcus)
            logger.debug("Wait %d seconds while recording XY data" %(rec_time))
            rspctl('--statistics --duration=%d --integration=1 --directory=%s --select=%s' %(rec_time, dataDir(), rcus), wait=0.0)
            
        if read == True:
            self.readFiles()
    
    def readFile(self, full_filename):
        sleep(0.02)
        data = np.fromfile(full_filename, dtype=np.float64)
        n_samples = len(data)
        if (n_samples % 512) > 0:
            logger.warn("data error: number of samples (%d) not multiple of 512 in '%f'" %(n_samples, full_filename))
        self.frames = n_samples / 512
        data = data.reshape(self.frames,512)
        #logger.info("recorded data shape %s" %(str(data.shape)))
        return (data)
        
    def readFiles(self):
        files_in_dir = sorted(os.listdir(dataDir()))
        data_shape = self.readFile(os.path.join(dataDir(),files_in_dir[0])).shape
        ssdata = np.zeros((96,data_shape[0],data_shape[1]), dtype=np.float64)
        for file_name in files_in_dir:
            #path, filename = os.split(file_name)
            rcu = int(file_name.split('.')[0][-3:])
            ssdata[rcu,:,:] = self.readFile(os.path.join(dataDir(),file_name))
            #logger.debug("%s  rcu=%d" %(file_name, rcu))
        # mask zero values and convert to dBm
        self.ssData = np.log10(np.ma.masked_less(ssdata, self.minvalue)) * 10.0
        # do not use subband 0
        self.ssData[:,:,0] = np.ma.masked
    
    # subbands is list to mask
    def getMaskedData(self):
        data = self.ssData.copy()
        for sb in self.mask:
            data[:,:,sb] = np.ma.masked
        return (data)

    def getMeanSpectra(self, pol='XY'):
        if pol in ('XY', 'xy'):
            return (self.ssData[:,:,:].mean(axis=1).mean(axis=0))
        if pol in (0, 'X', 'x'):
            return (self.ssData[0::2,:,:].mean(axis=1).mean(axis=0))
        if pol in (1, 'Y', 'y'):
            return (self.ssData[1::2,:,:].mean(axis=1).mean(axis=0))
    
    def getMedianSpectra(self, pol='XY'):
        if pol in ('XY', 'xy'):
            return (np.median(np.mean(self.ssData, axis=1), axis=0))
        if pol in (0, 'X', 'x'):
            return (np.median(np.mean(self.ssData[0::2,:,:], axis=1), axis=0))
        if pol in (1, 'Y', 'y'):
            return (np.median(np.mean(self.ssData[1::2,:,:], axis=1), axis=0))    
    
    def getSpectra(self, rcu):
        return(np.mean(self.ssData[rcu,:,:], axis=0))
        
    def getSubbands(self, rcu):
        return (self.ssData[int(rcu),:,:].mean(axis=0))
    
    def getSubbandX(self):
        return (self.ssData[0::2,:,self.testSubband_Y].mean(axis=1))
    
    def getSubbandY(self):
        return (self.ssData[1::2,:,self.testSubband_Y].mean(axis=1))
                       
    def getAll(self, pol='XY'):
        if pol in ('XY', 'xy'):
            return (self.getMaskedData())
        if pol in ('x','X',0):
            return (self.getMaskedData()[0::2,:,:])
        if pol in ('y','Y',1):
            return (self.getMaskedData()[1::2,:,:])        
    
    def getAllX(self):
        return (self.getMaskedData()[0::2,:,:])
    
    def getAllY(self):
        return (self.getMaskedData()[1::2,:,:])
           
    def getMedianRcu(self, rcu):
        return(np.ma.median(self.ssData[int(rcu),:,:].mean(axis=0)))

    def searchTestSignal(self, subband=-1, minsignal=75.0, maxsignal=100.0):
        # ss = median for all band over all rcu's
        # forget subband 0    
        ssX = np.ma.median(self.ssData[::2,:,:].mean(axis=1),axis=0)
        ssY = np.ma.median(self.ssData[1::2,:,:].mean(axis=1),axis=0)
        
        if subband != -1:
            if ssX[subband] > minsignal and ssY[subband] > minsignal:
                self.testSignal_X = ssX[subband]
                self.testSubband_X = subband
                self.testSignal_Y = ssY[subband]
                self.testSubband_Y = subband
                return                
            else:
                logger.debug("Test signal on subband %d not strong enough X=%3.1fdB Y=%3.1fdB" %(subband, ssX[subband], ssY[subband]))
                
        # no subband given or not in requested range, look for better
        for i in range(0,ssX.shape[0],1):
            if ssX[i] > minsignal  and ssX[i] < maxsignal and ssX[i] > self.testSignal_X:
                self.testSignal_X = ssX[i]
                self.testSubband_X = i
            if ssY[i] > minsignal  and ssY[i] < maxsignal and ssY[i] > self.testSignal_Y:
                self.testSignal_Y = ssY[i]
                self.testSubband_Y = i
        return
#### end of cRCUdata class ####
