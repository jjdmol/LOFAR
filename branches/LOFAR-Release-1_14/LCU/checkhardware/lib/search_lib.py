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
                        for j in range(maxpos,0,-1):
                            minpos = x[j]
                            if self.data[j] < maxval - delta:
                                break
                                                    
                        self.max_peaks.append((x[i]+minpos)/2)
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


def searchDown(data, subband):
    global logger
    _data = data.max(axis=1)
    down_info = list()
    shifted_info = list()
    start_sb = subband - 70
    stop_sb  = subband + 70
    delta    = 3
    
    peaks = cSearchPeak(ma.mean(_data[:,start_sb:stop_sb], axis=0))
    peaks.search(delta=1)
    for peak in peaks.max_peaks:
        min_sb, max_sb = peaks.getPeakWidth(peak, 1)
        logger.debug("peak min_sb=%d, max_sb=%d" %(start_sb+min_sb, start_sb+max_sb))
        if (max_sb - min_sb) < 25:
            logger.debug("mask subbands %d..%d" %(start_sb+min_sb-1,start_sb+max_sb+1))
            for i in range(start_sb+min_sb-1, start_sb+max_sb+2, 1):        
                _data[:,i] = ma.masked
    
    median_value = ma.median(_data[:,start_sb:stop_sb]) 
    peaks_array = zeros((data.shape[0]),'i')
    logger.debug("median val sb%d..sb%d = %3.1fdB" %(start_sb, stop_sb, median_value))
    
    for rcu in range(data.shape[0]):
        peaks = cSearchPeak(_data[rcu,start_sb:stop_sb])
        peaks.search(delta=delta)
        peaks_array[rcu] = peaks.getMaxPeak()[1]+start_sb
    mean_max = peaks_array.mean()
    logger.debug("mean top sb%d..sb%d on sb%d" %(start_sb, stop_sb, mean_max))
    
    for ant in range(data.shape[0]/2):
        x_rcu = ant * 2
        y_rcu = x_rcu + 1
        ant_x_mean_value = ma.mean(_data[x_rcu,start_sb:stop_sb])
        ant_y_mean_value = ma.mean(_data[y_rcu,start_sb:stop_sb])
            
        logger.debug("ant %d X top on sb%d mean-val=%3.1f X top on sb%d mean-val=%3.1f" %\
                    (ant, peaks_array[x_rcu], ant_x_mean_value, peaks_array[y_rcu], ant_y_mean_value))
        
        if ant_x_mean_value <= (median_value - 1.5) or ant_y_mean_value <= (median_value - 1.5):
            logger.info("Ant %d mean signal in test band lower than normal Xmean=%3.1f Ymean=%3.1f" %(ant, ant_x_mean_value, ant_y_mean_value))
            
        if abs(peaks_array[x_rcu] - mean_max) > 5 and abs(peaks_array[y_rcu] - mean_max) > 5:
            if abs(peaks_array[x_rcu] - peaks_array[y_rcu]) > 7:
                if ant_x_mean_value < (median_value - 1.5) or ant_y_mean_value < (median_value - 1.5):
                    down_info.append((ant, peaks_array[x_rcu], peaks_array[y_rcu], mean_max))
                    
        elif abs(peaks_array[x_rcu] - mean_max) > 4:
            shifted_info.append((x_rcu, peaks_array[x_rcu], mean_max))
        
        elif abs(peaks_array[y_rcu] - mean_max) > 4:
            shifted_info.append((y_rcu, peaks_array[y_rcu], mean_max))
    return (down_info, shifted_info)
    

# find oscillation
def search_oscillation(data, delta, start_sb, stop_sb):
    global logger
    info = list()
    
    max_sec = data.shape[1]
    min_sec = max(max_sec-4, 0)
    _data = data[:,min_sec:max_sec,start_sb:stop_sb]
    logger.debug("seconds to analyse=%d" %(_data.shape[1]))
    mean_spectra = ma.mean(ma.mean(_data,axis=1),axis=0)
    peaks = cSearchPeak(mean_spectra)
    peaks.search(delta=delta)
    mean_n_peaks   = peaks.nMaxPeaks()
    mean_sum_peaks = peaks.getSumPeaks()
    mean_low = _data.min(axis=1).mean()
    
    logger.debug("mean n_peaks=%d, sum_peaks=%5.3f, low=%3.1f" %(mean_n_peaks, mean_sum_peaks, mean_low)) 
    
    for rcu in range(_data.shape[0]):
        max_peak_val  = 0
        max_n_peaks   = 0
        max_sum_peaks = 0
        for sec in range(_data.shape[1]):
            peaks = cSearchPeak(_data[rcu,sec,:])
            peaks.search(delta=delta)
            max_peak_val  = max(max_peak_val, peaks.getMaxPeak()[0])
            max_n_peaks   = max(max_n_peaks, peaks.nMaxPeaks())
            max_sum_peaks = max(max_sum_peaks, peaks.getSumPeaks())
            
        if max_n_peaks > (mean_n_peaks + 5):
            rcu_low = _data[rcu,:,:].min(axis=0).mean()
            logger.debug("RCU %d sb=%d..%d number-of-peaks=%d (delta=%3.1fdB) maxvalue=%3.1f sum=%5.3f low=%3.1f" %\
                        (rcu, start_sb, stop_sb, max_n_peaks, delta, max_peak_val, max_sum_peaks, rcu_low))        
            if rcu_low > (mean_low + 1.0): #peaks.getSumPeaks() > (mean_sum_peaks * 2.0):
                info.append((max_sum_peaks, max_n_peaks, rcu))                                
    return (sorted(info,reverse=True))


# find summator noise
def search_summator_noise(data, start_sb, stop_sb):
    global logger
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
        logger.debug("RCU %d sb=%d..%d number-of-peaks=%d (delta=1.5dB) maxvalue=%3.1f" %(rcu, start_sb, stop_sb, n_peaks, peaks.getMaxPeak()[0]))
        if n_peaks > ref_peaks:
            if peaks.getMaxPeak()[0] < 120:
                meanval = psd[10:].mean()
                maxval = psd[85:100].max()
                if maxval > (meanval+1.5):
                    info.append((rcu, maxval, len(peaks.max_peaks)))                                
    return (info)
    
# find noise
# noise looks like the noise floor is going up and down
#
# kijk ook naar op en neer gaande gemiddelden
def search_noise(data, low_deviation, high_deviation, max_diff):
    global logger
    high_info   = list()
    low_info    = list()
    jitter_info = list()
    
    spec_median  = ma.median(data, axis=2)
    spec_max     = spec_median.max(axis=1)
    spec_min     = spec_median.min(axis=1)
    ref_value    = ma.median(data)
    ref_diff     = ma.median(spec_max) - ma.median(spec_min)
    ref_std      = ma.std(spec_median)
    
    limit = ref_value + min(max((ref_std * 3.0),0.75), high_deviation)
    
    n_secs = data.shape[1]
    logger.info("median-signal=%5.3fdB, median-fluctuation=%5.3fdB, std=%5.3f, high-limit=%5.3fdB" %(ref_value, ref_diff, ref_std, limit))
    for rcu in range(data.shape[0]):
        peaks = cSearchPeak(data[rcu,0,:])
        peaks.search(delta=10.0)
        if peaks.nMaxPeaks() >= 30:
            logger.debug("rcu %d found %d peaks, skip noise test" %(rcu, peaks.nMaxPeaks()))
        else:
            n_bad_high_secs    = 0
            n_bad_low_secs     = 0
            n_bad_jitter_secs  = 0
            
            rcu_max_diff = spec_max[rcu] - spec_min[rcu]
            if rcu_max_diff > (ref_diff + max_diff):
                check_high_value = ref_value + (ref_diff / 2.0)
                check_low_value  = ref_value - (ref_diff / 2.0)
                for val in spec_median[rcu,:]:
                    if val > check_high_value or val < check_low_value:
                        n_bad_jitter_secs += 1
                jitter_info.append((rcu, rcu_max_diff, ref_diff, n_bad_jitter_secs))
                logger.debug("rcu %d max spectrum fluctuation %5.3f dB" %(rcu, rcu_max_diff)) 
            
            
            for val in spec_median[rcu,:]:
                #logger.debug("rcu %d high-noise value=%5.3fdB  max-ref-value=%5.3fdB" %(rcu, val, ref_val)) 
                if ((val > limit) and (rcu_max_diff > 1.0)) or (val > (ref_value + high_deviation)):
                    n_bad_high_secs += 1
                
                if ((val < (ref_value + low_deviation)) and (rcu_max_diff > 1.0))  or (val < (ref_value + low_deviation)):
                    n_bad_low_secs += 1
            
            if n_bad_high_secs > 0:    
                high_info.append((rcu, spec_max[rcu], n_bad_high_secs, limit, rcu_max_diff))
                logger.debug("rcu %d max-noise=%5.3f  %d of %d seconds bad" %(rcu, spec_max[rcu], n_bad_high_secs, n_secs)) 

            if n_bad_low_secs > 0:    
                low_info.append((rcu, spec_min[rcu], n_bad_low_secs , (ref_value+low_deviation), rcu_max_diff)) 
                logger.debug("rcu %d min-noise=%5.3f %d of %d seconds bad" %(rcu, spec_min[rcu], n_bad_low_secs, n_secs)) 

    return (low_info, high_info, jitter_info)



# find spurious around normal signals
# 
def search_spurious(data, delta):
    _data = data.copy()
    max_data = _data.max(axis=1) #ma.mean(_data, axis=1)
    median_spec = ma.median(max_data, axis=0)
    peaks = cSearchPeak(median_spec)
    peaks.search(delta=delta) #deta=20 for HBA 
    for peak in peaks.max_peaks:
        min_sb, max_sb = peaks.getPeakWidth(peak, delta)        
        for i in range(min_sb-2, max_sb+2, 1):        
            max_data[:,i] = ma.masked
    
    info = list()
    for rcu in range(_data.shape[0]):
        peaks = cSearchPeak(max_data[rcu,:])
        peaks.search(delta=delta)
        if peaks.nMaxPeaks() > 10:
            #print rcu, peaks.nMaxPeaks()                    
            info.append(rcu)
    return(info)
    
