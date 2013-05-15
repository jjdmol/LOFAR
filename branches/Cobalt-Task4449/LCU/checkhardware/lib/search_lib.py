#!/usr/bin/python

"""
library with all search functions

P.Donker ASTRON
"""
from numpy import ma, fft, power, arange, asarray, isscalar, NaN, Inf, zeros
from sys import exit
import logging

logger = logging.getLogger()    


"""
search for all peaks (min & max) in spectra
"""
class cSearchPeak:
    global logger
    def __init__(self, data):
        if len(data.shape) != 1:
           exit('wrong data format, must be 1 dimensional') 
        self.data = data.copy()
        self.n_data = len(data)
        self.max_peaks = []
        self.min_peaks = []
        #self.search()
        return
    
    def search(self, delta):
        self.max_peaks = []
        self.min_peaks = []
        
        x = arange(len(self.data))
        
        if not isscalar(delta):
            exit('argument delta must be a scalar')
        
        if delta <= 0:
            exit('argument delta must be positive')
        
        maxval, minval = self.data[0], self.data[0]
        maxpos, minpos  = 0, 0
        
        lookformax = True
        
        for i in range(self.n_data):
            if ma.count_masked(self.data) > 1 and self.data.mask[i] == True:
                lookformax = True
                maxval = 0.0
            else:
                now = self.data[i]
                if now > maxval:
                    maxval = now
                    maxpos = x[i]
                if now < minval:
                    minval = now
                    minpos = x[i]
                
                if lookformax:
                    if now < (maxval - delta):
                        self.max_peaks.append(maxpos)
                        minval = now
                        minpos = x[i]
                        lookformax = False
                else:
                    if now > (minval + delta):
                        self.min_peaks.append(minpos)
                        maxval = now
                        maxpos = x[i]
                        lookformax = True
        return
    
    # return data[nr]
    def getPeak(self, nr):
        try:
            return (self.data[nr])
        except:
            return (NaN)
    
    def getPeakWidth(self, nr, delta):
        peakval = self.data[nr]
        minnr   = nr
        maxnr   = nr
        for sb in range(nr,0,-1):
            if self.data[sb] < (peakval - delta):
                minnr = sb
                break
        for sb in range(nr,self.data.shape[0],1):
            if self.data[sb] < (peakval - delta):
                maxnr = sb
                break
        return (minnr, maxnr)
        
    # return value and sbband nr    
    def getMaxPeak(self):
        maxval = 0.0
        nr_bin = -1
        for peak in self.max_peaks:
            if self.data[peak] > maxval:
                maxval = self.data[peak]
                nr_bin = peak
        return (maxval, nr_bin)
    
    def getSumPeaks(self):
        peaksum = 0.0
        for peak in self.max_peaks:
            peaksum += self.data[peak]
        return (peaksum)
            
    # return value and sbband nr    
    def getMinPeak(self):
        minval = Inf
        nr_bin = -1
        for peak in self.min_peaks:
            if self.data[peak] < minval:
                minval = self.data[peak]
                nr_bin = peak
        return (minval, nr_bin)
    
    def nMaxPeaks(self):
        return (len(self.max_peaks))

# return psd off data and freq bins
def PSD(data, sampletime):
    if data.ndim != 1:
        return ([],[])
    fft_data = fft.fft(data)
    n  = fft_data.size    
    psd_freq = fft.fftfreq(n, sampletime)
    psd = power(abs(fft_data),2)/n
    return (psd[:n/2], psd_freq[:n/2])


def searchDown(data):
    global logger
    _data = data.max(axis=1)
    info = list()
    start_sb = 301 - 50
    stop_sb  = 301 + 50
    delta    = 3
    peaks_array = zeros((data.shape[0]),'i')
    for ant in range(data.shape[0]):
        peaks = cSearchPeak(_data[ant,start_sb:stop_sb])
        peaks.search(delta=delta)
        peaks_array[ant] = peaks.getMaxPeak()[1]+start_sb
    mean_max = peaks_array.mean()
    for i in range(data.shape[0]):
        if abs(peaks_array[i] - mean_max) > 7:
            info.append((i, peaks_array[i], mean_max))
            #logger.info("ant %d down, sb=%d" %(i, peaks_array[i]))
    return (info)
    

# find oscillation
def search_oscillation(data, delta, start_sb, stop_sb):
    info = list()
    if data.ndim > 2:
        _data = data.max(axis=1)
    
    for rcu in range(_data.shape[0]):
        peaks = cSearchPeak(_data[rcu,start_sb:stop_sb])
        peaks.search(delta=delta) #deta=20 for HBA
        n_peaks = peaks.nMaxPeaks()        
        if n_peaks > 10:
            if peaks.getMaxPeak()[0] > 100:#100 for HBA
                info.append((peaks.getSumPeaks(), len(peaks.max_peaks), rcu))                                
    return (sorted(info,reverse=True))


# find summator noise
def search_summator_noise(data, start_sb, stop_sb):
    info = list()
    if data.ndim > 2:
        _data = data.max(axis=1)
    
    peaks = cSearchPeak(_data.mean(axis=0)[start_sb:stop_sb])
    peaks.search(delta=1.5)
    ref_peaks = peaks.nMaxPeaks() + 5
    
    for rcu in range(_data.shape[0]):
        psd, psd_freq = PSD(_data[rcu,start_sb:stop_sb], ((1.0/200e6)*1024))
        peaks = cSearchPeak(_data[rcu,start_sb:stop_sb])
        peaks.search(delta=1.5)
        n_peaks = peaks.nMaxPeaks()
        #print rcu, n_peaks, peaks.getMaxPeak()
        if n_peaks > ref_peaks:
            if peaks.getMaxPeak()[0] < 95:
                meanval = psd[10:].mean()
                maxval = psd[85:100].max()
                if maxval > (meanval+1.5):
                    info.append((rcu, maxval, len(peaks.max_peaks)))                                
    return (info)
    
# find noise
# noise looks like the noise floor is going up and down
#
# kijk ook naar op en neer gaande gemiddelden
def search_noise(data, min_deviation, max_deviation, max_diff):
    global logger
    high_info   = list()
    low_info    = list()
    jitter_info = list()
    
    spec_median  = ma.median(data, axis=2)
    spec_max     = spec_median.max(axis=1)
    spec_min     = spec_median.min(axis=1)
    ref_value    = ma.median(data)
    ref_diff     = ma.median(spec_max) - ma.median(spec_min)

    n_secs = data.shape[1]
    logger.info("ref-signal = %5.3f dB, ref-diff = %5.3f dB" %(ref_value, ref_diff))
    for rcu in range(data.shape[0]):
        peaks = cSearchPeak(data[rcu,0,:])
        peaks.search(delta=10.0)
        logger.debug("rcu %d maxpeaks %d" %(rcu, peaks.nMaxPeaks()))
        if peaks.nMaxPeaks() < 15:
            proc_max  = 0.0
            proc_min  = 0.0
            spec_diff = 0.0
            if (spec_max[rcu] - spec_min[rcu]) > max_diff:
                spec_diff = spec_max[rcu] - spec_min[rcu]
                jitter_info.append((rcu, spec_diff, ref_diff))
                logger.debug("rcu %d max spectrum difference %5.3f dB" %(rcu, spec_diff)) 
                
            for val in spec_median[rcu,:]:
                if val > (ref_value + max_deviation):
                    proc_max += 100.0 / n_secs
                if val < (ref_value + min_deviation):
                    proc_min += 100.0 / n_secs
            
            if proc_max > 0.0:    
                high_info.append((rcu, spec_max[rcu], proc_max, (ref_value+max_deviation), (spec_max[rcu] - spec_min[rcu])))
                logger.debug("rcu %d max noise %5.3f %% of time" %(rcu, proc_max)) 

            if proc_min > 0.0:    
                low_info.append((rcu, spec_min[rcu], proc_min, (ref_value+min_deviation), (spec_max[rcu] - spec_min[rcu]))) 
                logger.debug("rcu %d min noise %5.3f %% of time" %(rcu, proc_min)) 

    return (low_info, high_info, jitter_info)



# find spurious around normal signals
# 
def search_spurious(data, delta):
    _data = data.copy()
    mean_data = ma.mean(_data, axis=1)
    mean_spec = ma.mean(mean_data, axis=0)
    peaks = cSearchPeak(mean_spec)
    peaks.search(delta=delta) #deta=20 for HBA 
    for peak in peaks.max_peaks:
        min_sb, max_sb = peaks.getPeakWidth(peak, delta)        
        for i in range(min_sb-1, max_sb+2, 1):        
            mean_data[:,i] = ma.masked
    
    info = list()
    for rcu in range(_data.shape[0]):
        peaks = cSearchPeak(mean_data[rcu,:])
        peaks.search(delta=delta)
        if peaks.nMaxPeaks() > 10:
            #print rcu, peaks.nMaxPeaks()                    
            info.append(rcu)
    return(info)
    
