#!/usr/bin/python

"""
library with all search functions

P.Donker ASTRON
"""
from numpy import ma, fft, power, arange, asarray, isscalar, NaN, Inf, zeros
from sys import exit
import logging

search_version = '1013f'

logger = logging.getLogger()

"""
search for all peaks (min & max) in spectra
"""
class cSearchPeak:
    global logger
    def __init__(self, data):
        self.valid_data = False
        if len(data.shape) == 1:
            self.valid_data = True
            self.data = data.copy()
            self.n_data = len(data)
            self.max_peaks = []
            self.min_peaks = []
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
                continue

            now = self.data[i]
            if now > maxval:
                maxval = now
                maxpos = x[i]
                
            if now < minval:
                minval = now
                minpos = x[i]
                            
            if lookformax:
                if now < (maxval - delta):
                    #for j in range(maxpos,0,-1):
                    #    if self.data[j] < (maxval - delta):
                    #        self.max_peaks.append((x[i] + x[j]) / 2)
                    #        break
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

        # if no peak found with the given delta, return maximum found
        if len(self.max_peaks) == 0:
            self.max_peaks.append(maxpos)
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
            if self.data[sb] < peakval:
                minnr = sb
            if self.data[sb] <= (peakval - delta):
                break
        for sb in range(nr,self.data.shape[0],1):
            if self.data[sb] < peakval:
                maxnr = sb
            if self.data[sb] <= (peakval - delta):
                break
        return (minnr, maxnr)
        
    # return value and subband nr    
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
    _data = data.mean(axis=1)
    down_info = list()
    shifted_info = list()
    start_sb = subband - 70
    stop_sb  = subband + 70
    delta    = 3
    
    peaks = cSearchPeak(ma.median(_data[:,start_sb:stop_sb], axis=0))
    if not peaks.valid_data:
        return (down_info, shifted_info)

    peaks.search(delta=delta)
    (median_max_val, median_max_sb) = peaks.getMaxPeak()
    bw_min, bw_max = peaks.getPeakWidth(median_max_sb, delta)
    median_max_sb += start_sb
    median_bw = bw_max - bw_min
    
    median_value = ma.median(_data[:,start_sb:stop_sb]) 
    logger.debug("reference peak in band %d .. %d : subband=%d, bw(3dB)=%d, median-value-band=%3.1fdB" %\
                (start_sb, stop_sb, (median_max_sb), median_bw, median_value))

    peaks_array    = zeros((data.shape[0]),'i')
    peaks_bw_array = zeros((data.shape[0]),'i')
    
    for rcu in range(data.shape[0]):
        peaks = cSearchPeak(_data[rcu,start_sb:stop_sb])
        if peaks.valid_data:
            peaks.search(delta=delta)
            (max_val, max_sb) = peaks.getMaxPeak()
            
            if max_sb > 0:
                bw_min, bw_max = peaks.getPeakWidth(max_sb, delta)
                peaks_bw_array[rcu] = bw_max - bw_min
                peaks_array[rcu] = start_sb + max_sb
            else:
                peaks_bw_array[rcu] = stop_sb - start_sb
                peaks_array[rcu] = subband
                
    for ant in range(data.shape[0]/2):
        x_rcu = ant * 2
        y_rcu = x_rcu + 1
        
        mean_val_trigger = False
        down_trigger     = False
        
        ant_x_mean_value = ma.mean(_data[x_rcu,start_sb:stop_sb])
        ant_y_mean_value = ma.mean(_data[y_rcu,start_sb:stop_sb])
            
        logger.debug("RCU=%d/%d: X-top, sb%d, bw=%d, mean-value-band=%3.1f  Y-top, sb%d, bw=%d, mean-value-band=%3.1f" %\
                    (x_rcu, y_rcu, peaks_array[x_rcu], peaks_bw_array[x_rcu], ant_x_mean_value, peaks_array[y_rcu], peaks_bw_array[y_rcu], ant_y_mean_value))
        
        if (((ant_x_mean_value < (median_value - 3.0)) and (ant_x_mean_value > 66)) and
            ((ant_y_mean_value < (median_value - 3.0)) and (ant_y_mean_value > 66))):
            logger.debug("RCU=%d/%d: mean signal in test band for X and Y lower than normal" %(x_rcu, y_rcu))
            mean_val_trigger = True
            
        if ((((abs(peaks_array[x_rcu] - median_max_sb) > 5) or (abs(peaks_bw_array[x_rcu] - median_bw) > 5)) and (peaks_bw_array[x_rcu] > 3)) or 
            (((abs(peaks_array[y_rcu] - median_max_sb) > 5) or (abs(peaks_bw_array[y_rcu] - median_bw) > 5)) and (peaks_bw_array[y_rcu] > 3))):
            logger.debug("RCU=%d/%d: antenna probable down" %(x_rcu, y_rcu))
            down_trigger = True
        
        if mean_val_trigger and down_trigger:    
            down_info.append((ant, peaks_array[x_rcu], peaks_array[y_rcu], median_max_sb))
        
        if not down_trigger:
            if ((peaks_bw_array[x_rcu] > 3) and (abs(peaks_array[x_rcu] - median_max_sb) > 5)):
                logger.debug("RCU=%d: X-top shifted normal=%d, now=%d" %(x_rcu, median_max_sb, peaks_array[x_rcu]))
                shifted_info.append((x_rcu, peaks_array[x_rcu], median_max_sb))
            
            if ((peaks_bw_array[y_rcu] > 3) and (abs(peaks_array[y_rcu] - median_max_sb) > 5)):
                logger.debug("RCU=%d: Y-top shifted normal=%d, now=%d" %(y_rcu, median_max_sb, peaks_array[y_rcu]))
                shifted_info.append((y_rcu, peaks_array[y_rcu], median_max_sb))
                
    return (down_info, shifted_info)
    

# find oscillation
def search_oscillation(data, delta, start_sb, stop_sb):
    global logger
    info = list()
    
    _data = data[:,:,start_sb:stop_sb]
    median_spectra = ma.median(ma.mean(_data,axis=1),axis=0)
    peaks = cSearchPeak(median_spectra)
    if not peaks.valid_data:
        return (info)
    peaks.search(delta=delta)
    median_max_peak  = peaks.getMaxPeak()[0]
    median_n_peaks   = peaks.nMaxPeaks()
    median_sum_peaks = peaks.getSumPeaks()
    median_low       = _data.min(axis=1).mean()
    info.append((-1, median_sum_peaks, median_n_peaks, median_low))
    
    logger.debug("ref.data: used-delta=%3.1f dB, max_peak=%3.1fdB, n_peaks=%d, sum_all_peaks=%5.3f, median_low_value=%3.1fdB" %\
                (delta, median_max_peak, median_n_peaks, median_sum_peaks, median_low)) 
    
    for rcu in range(_data.shape[0]):
        max_peak_val  = 0
        max_n_peaks   = 0
        max_sum_peaks = 0
        for sec in range(_data.shape[1]):
            peaks = cSearchPeak(_data[rcu,sec,:])
            if peaks.valid_data:
                peaks.search(delta=delta)
                max_peak_val  = max(max_peak_val, peaks.getMaxPeak()[0])
                max_n_peaks   = max(max_n_peaks, peaks.nMaxPeaks())
                max_sum_peaks = max(max_sum_peaks, peaks.getSumPeaks())
            
        #if max_n_peaks > (median_n_peaks + 5):
        if max_sum_peaks > (median_sum_peaks * 2.0):
            rcu_low = _data[rcu,:,:].min(axis=0).mean()
            logger.debug("RCU=%d: number-of-peaks=%d  max_value=%3.1f  peaks_sum=%5.3f low_value=%3.1f" %\
                        (rcu, max_n_peaks, max_peak_val, max_sum_peaks, rcu_low))        
            if rcu_low > (median_low + 1.5): #peaks.getSumPeaks() > (median_sum_peaks * 2.0):
                info.append((rcu, max_sum_peaks, max_n_peaks, rcu_low))
        
        if max_peak_val > 150.0: # only one high peek
            info.append((rcu, max_sum_peaks, max_n_peaks, rcu_low))
            
    return (info) #(sorted(info,reverse=True))

# find summator noise
def search_summator_noise(data):
    global logger
    sn_info = list() # summator noise
    cr_info = list() # cable reflection
    _data = data[:,:,50:100].copy() 
    
    secs = _data.shape[1]
    for rcu in range(_data.shape[0]):
        max_valid = 0
        max_peaks = 0            
        for sec in range(secs):
            peaks = cSearchPeak(_data[rcu,sec,:])
            
            if peaks.valid_data:
                peaks.search(delta=0.7)
                n_peaks = peaks.nMaxPeaks()
    
                n_valid = 0
                last_p = 0
                for p in peaks.max_peaks:
                    p_diff = p - last_p
                    if p_diff in (3,4):
                        n_valid += 1
                    last_p = p
                max_valid = max_valid + n_valid
                max_peaks = max(max_peaks, n_peaks)

        if max_valid > (secs * 3.0):
            max_valid = max_valid / secs             
            info.append((rcu, max_valid, max_peaks))
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
    logger.debug("median-signal=%5.3fdB, median-fluctuation=%5.3fdB, std=%5.3f, high-limit=%5.3fdB" %(ref_value, ref_diff, ref_std, limit))
    for rcu in range(data.shape[0]):
        peaks = cSearchPeak(data[rcu,0,:])
        if not peaks.valid_data:
            return (low_info, high_info, jitter_info)
        peaks.search(delta=10.0)
        if peaks.nMaxPeaks() >= 30:
            logger.debug("RCU=%d: found %d peaks, skip noise test" %(rcu, peaks.nMaxPeaks()))
        else:
            n_bad_high_secs    = 0
            n_bad_low_secs     = 0
            n_bad_jitter_secs  = 0
            
            rcu_max_diff = spec_max[rcu] - spec_min[rcu]
            
            for val in spec_median[rcu,:]:
                #logger.debug("RCU=%d: high-noise value=%5.3fdB  max-ref-value=%5.3fdB" %(rcu, val, ref_val)) 
                if ((val > limit) and (rcu_max_diff > 1.0)) or (val > (ref_value + high_deviation)):
                    n_bad_high_secs += 1
                
                if ((val < (ref_value + low_deviation)) and (rcu_max_diff > 1.0))  or (val < (ref_value + low_deviation)):
                    n_bad_low_secs += 1
            
            if n_bad_high_secs > 0:    
                high_info.append((rcu, spec_max[rcu], n_bad_high_secs, limit, rcu_max_diff))
                logger.debug("RCU=%d: max-noise=%5.3f  %d of %d seconds bad" %(rcu, spec_max[rcu], n_bad_high_secs, n_secs)) 

            if n_bad_low_secs > 0:    
                low_info.append((rcu, spec_min[rcu], n_bad_low_secs , (ref_value+low_deviation), rcu_max_diff)) 
                logger.debug("RCU=%d: min-noise=%5.3f %d of %d seconds bad" %(rcu, spec_min[rcu], n_bad_low_secs, n_secs)) 
            
            if (n_bad_high_secs == 0) and (n_bad_low_secs == 0):
                if rcu_max_diff > (ref_diff + max_diff):
                    check_high_value = ref_value + (ref_diff / 2.0)
                    check_low_value  = ref_value - (ref_diff / 2.0)
                    for val in spec_median[rcu,:]:
                        if val > check_high_value or val < check_low_value:
                            n_bad_jitter_secs += 1
                    jitter_info.append((rcu, rcu_max_diff, ref_diff, n_bad_jitter_secs))
                    logger.debug("RCU=%d: max spectrum fluctuation %5.3f dB" %(rcu, rcu_max_diff)) 
                
    return (low_info, high_info, jitter_info)



# find spurious around normal signals
# 
def search_spurious(data, delta):
    global logger
    info = list()
    _data = data.copy()
    max_data  = ma.max(_data, axis=1)
    mean_data = ma.mean(_data, axis=1)
    median_spec = ma.mean(max_data, axis=0)
    peaks = cSearchPeak(median_spec)
    if not peaks.valid_data:
        return (info)
    
    # first mask peaks available in all data
    peaks.search(delta=(delta/2.0)) #deta=20 for HBA 
    for peak in peaks.max_peaks:
        min_sb, max_sb = peaks.getPeakWidth(peak, delta/2.0)
        if (max_sb - min_sb) > 8:
            continue
        min_sb = max(min_sb-1, 0)
        max_sb = min(max_sb+1, peaks.n_data-1)
        logger.debug("mask sb %d..%d" %(min_sb, max_sb))
        for i in range(min_sb, max_sb, 1):        
            mean_data[:,i] = ma.masked
    
    # search in all data for spurious 
    for rcu in range(_data.shape[0]):
        peaks = cSearchPeak(mean_data[rcu,:])
        if peaks.valid_data:
            peaks.search(delta=delta)
            for peak in peaks.max_peaks:
                min_sb, max_sb = peaks.getPeakWidth(peak, delta)
                if (max_sb - min_sb) > 10:
                    continue
                peak_val = peaks.getPeak(peak)
                if (max_sb - min_sb) < 100 and peak_val != NaN:
                    logger.debug("RCU=%d: spurious, subband=%d..%d, peak=%3.1fdB" %(rcu, min_sb, max_sb, peak_val))
            if peaks.nMaxPeaks() > 10:
                #print rcu, peaks.nMaxPeaks()                    
                info.append(rcu)
    return(info)
    
