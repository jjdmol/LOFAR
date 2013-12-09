#!/usr/bin/python
# data lib

from general_lib import *
from lofar_lib import *
from search_lib import *
import os
import numpy as np
import logging

test_version = '1113'

logger = None
def init_data_lib():
    global logger
    logger = logging.getLogger()
    logger.debug("init logger data_lib")

# get and return recorded data in various ways
class cRCUdata:
    global logger
    def __init__(self, n_rcus, minvalue=1):
        self.n_rcus = n_rcus
        self.frames = 0
        self.minvalue = minvalue
        self.ssData = np.ones((n_rcus, 1, 512), np.float64)
        self.testSignal_X = -1.0
        self.testSubband_X = 0
        self.testSignal_Y = -1.0
        self.testSubband_Y = 0
    
    def record(self, rec_time=2, read=True):
        removeAllDataFiles()
        logger.debug("Wait %d seconds while recording data" %(rec_time))
        rspctl('--statistics --duration=%d --integration=1 --directory=%s --select=0:%d' %(rec_time, dataDir(), self.n_rcus-1), wait=0.0)
        if read:
            self.readFiles()
    
    def readFile(self, full_filename):
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
        ssdata = np.array([self.readFile(os.path.join(dataDir(),file_name)) for file_name in files_in_dir])
        # mask zero values and convert to dBm
        self.ssData = np.log10(np.ma.masked_less(ssdata, self.minvalue)) * 10.0
        # do not use subband 0
        self.ssData[:,:,0] = np.ma.masked
    
    # sb_select list with selected subbands
    def setMask(self, sb_select):
        for sb in range(512):
            if sb in sb_select:
                # do not mask
                continue
            self.ssData[:,:,sb] = np.ma.masked
        return
    
    def getSubbands(self, rcu):
        return (self.ssData[int(rcu),:,:].mean(axis=0))
    
    def getSubbandX(self):
        return (self.ssData[0::2,:,self.testSubband_Y].mean(axis=1))
    
    def getSubbandY(self):
        return (self.ssData[1::2,:,self.testSubband_Y].mean(axis=1))
                       
    def getAll(self):
        return (self.ssData[:,:,:])
    
    def getAllX(self):
        return (self.ssData[0::2,:,:])
    
    def getAllY(self):
        return (self.ssData[1::2,:,:])
           
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
        for i in range(1,ssX.shape[0],1):
            if ssX[i] > minsignal  and ssX[i] < maxsignal and ssX[i] > self.testSignal_X:
                self.testSignal_X = ssX[i]
                self.testSubband_X = i
            if ssY[i] > minsignal  and ssY[i] < maxsignal and ssY[i] > self.testSignal_Y:
                self.testSignal_Y = ssY[i]
                self.testSubband_Y = i
        return
#### end of cRCUdata class ####
