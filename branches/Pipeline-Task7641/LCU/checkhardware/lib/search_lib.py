#!/usr/bin/python

"""
library with all search functions

P.Donker ASTRON
"""
from numpy import ma, fft, power, arange, asarray, isscalar, NaN, Inf, zeros
from sys import exit
import logging
from time import sleep

search_version = '0415'

logger = logging.getLogger()

"""
search for all peaks (min & max) in spectra
"""
class cSearchPeak:
    def __init__(self, data):
        self.valid_data = False
        if len(data.shape) == 1:
            self.valid_data = True
            self.data = data.copy()
            self.n_data = len(data)
            self.max_peaks = []
            self.min_peaks = []
        return

    def search(self, delta, max_width=100, skip_list=[]):
        self.max_peaks = []
        self.min_peaks = []

        x = arange(0,len(self.data),1)

        if not isscalar(delta):
            exit('argument delta must be a scalar')

        if delta <= 0:
            exit('argument delta must be positive')

        maxval, minval = self.data[1], self.data[1]
        maxpos, minpos  = 1, 1

        lookformax = True

        # add subband to skiplist
        skiplist = []
        if len(skip_list) > 0:
            for max_pos, min_sb, max_sb in skip_list:
                for sb in range(min_sb,max_sb+1,1):
                    skiplist.append(sb)

        # skip subband 0 (always high signal)
        for i in range(1,self.n_data,1):
            #sleep(0.001)
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
                    if maxpos not in skiplist:
                        peakwidth, min_sb, max_sb = self.getPeakWidth(maxpos, delta)
                        #logger.debug("maxpos=%d, width=%d" %(maxpos, peakwidth))
                        if (peakwidth < max_width):
                            self.max_peaks.append([maxpos, min_sb, max_sb])
                    minval = now
                    minpos = x[i]
                    lookformax = False
            else:
                if now > (minval + delta):
                    if minpos not in skiplist:
                        self.min_peaks.append(minpos)
                    maxval = now
                    maxpos = x[i]
                    lookformax = True

        # if no peak found with the given delta, return maximum found
        if len(self.max_peaks) == 0:
            self.max_peaks.append([maxpos, maxpos, maxpos])
        return

    # return data[nr]
    def getPeakValue(self, nr):
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

        return (maxnr-minnr, minnr, maxnr)

    # return value and subband nr
    def getMaxPeak(self):
        maxval = 0.0
        minsb  = 0.0
        maxsb  = 0.0
        binnr = -1

        for peak, min_sb, max_sb in self.max_peaks:
            if self.data[peak] > maxval:
                maxval = self.data[peak]
                binnr  = peak
                minsb  = min_sb
                maxsb  = max_sb
        return (maxval, binnr, minsb, maxsb)

    def getSumPeaks(self):
        peaksum = 0.0
        for peak, min_sb, max_sb in self.max_peaks:
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

def searchFlat(data):
    _data = data.getAll().mean(axis=1)
    flat_info = list()
    for rcu in data.active_rcus:
        mean_signal = ma.mean(_data[rcu,:])
        if mean_signal > 61.0 and mean_signal < 64.5:
            logger.info("rcu=%d: cable probable off" %(rcu))
            flat_info.append((rcu, mean_signal))
    return(flat_info)

def searchShort(data):
    _data = data.getAll().mean(axis=1)
    short_info = list()
    for rcu in data.active_rcus:
        mean_signal = ma.mean(_data[rcu,:])
        if mean_signal > 55.0 and mean_signal < 61.0:
            logger.info("rcu=%d: cable shorted" %(rcu))
            short_info.append((rcu, mean_signal))
    return(short_info)
    
def searchDown(data, subband):
    _data = data.getAll().mean(axis=1)
    down_info = list()
    shifted_info = list()
    start_sb = subband - 70
    stop_sb  = subband + 70
    delta    = 3

    peaks = cSearchPeak(ma.median(_data[:,start_sb:stop_sb], axis=0))
    if not peaks.valid_data:
        return (down_info, shifted_info)

    peaks.search(delta=delta)
    (median_max_val, median_max_sb, min_sb, max_sb) = peaks.getMaxPeak()
    median_bandwidth = max_sb - min_sb
    #peakwidth, min_sb, max_sb = peaks.getPeakWidth(median_max_sb, delta)
    median_max_sb += start_sb

    median_value = ma.median(_data[:,start_sb:stop_sb])
    logger.debug("reference peak in band %d .. %d : subband=%d, bw(3dB)=%d, median-value-band=%3.1fdB" %\
                (start_sb, stop_sb, (median_max_sb), median_bandwidth, median_value))

    peaks_array    = zeros((_data.shape[0]),'i')
    peaks_bw_array = zeros((_data.shape[0]),'i')

    for rcu in data.active_rcus:
        peaks = cSearchPeak(_data[rcu,start_sb:stop_sb])
        if peaks.valid_data:
            peaks.search(delta=delta)
            (maxpeak_val, maxpeak_sb, min_sb, max_sb) = peaks.getMaxPeak()

            if maxpeak_sb > 0:
                #peakwidth, min_sb, max_sb = peaks.getPeakWidth(maxpeak_sb, delta)
                peaks_bw_array[rcu] = max_sb - min_sb
                peaks_array[rcu] = start_sb + maxpeak_sb
            else:
                peaks_bw_array[rcu] = stop_sb - start_sb
                peaks_array[rcu] = subband

    for ant in range(_data.shape[0]/2):
        x_rcu = ant * 2
        y_rcu = x_rcu + 1

        mean_val_trigger = False
        down_trigger     = False

        ant_x_mean_value = ma.mean(_data[x_rcu,start_sb:stop_sb])
        ant_y_mean_value = ma.mean(_data[y_rcu,start_sb:stop_sb])

        logger.debug("rcu=%d/%d: X-top, sb%d, bw=%d, mean-value-band=%3.1f  Y-top, sb%d, bw=%d, mean-value-band=%3.1f" %\
                    (x_rcu, y_rcu, peaks_array[x_rcu], peaks_bw_array[x_rcu], ant_x_mean_value, peaks_array[y_rcu], peaks_bw_array[y_rcu], ant_y_mean_value))

        if (((ant_x_mean_value < (median_value - 3.0)) and (ant_x_mean_value > 66)) or
            ((ant_y_mean_value < (median_value - 3.0)) and (ant_y_mean_value > 66))):
            logger.debug("rcu_bin=%d/%d: mean signal in test band for X and Y lower than normal" %(x_rcu, y_rcu))
            mean_val_trigger = True

        if ((((abs(peaks_array[x_rcu] - median_max_sb) > 10) or (abs(median_bandwidth - peaks_bw_array[x_rcu]) > 10)) and (peaks_bw_array[x_rcu] > 3)) or
            (((abs(peaks_array[y_rcu] - median_max_sb) > 10) or (abs(median_bandwidth - peaks_bw_array[y_rcu]) > 10)) and (peaks_bw_array[y_rcu] > 3))):
            logger.debug("rcu=%d/%d: antenna probable down" %(x_rcu, y_rcu))
            down_trigger = True

        if mean_val_trigger and down_trigger:
            down_info.append((ant, peaks_array[x_rcu], peaks_array[y_rcu], median_max_sb))

        if not down_trigger:
            if ((peaks_bw_array[x_rcu] > 20) and (abs(peaks_array[x_rcu] - median_max_sb) > 10)):
                logger.debug("rcu=%d: X-top shifted normal=%d, now=%d" %(x_rcu, median_max_sb, peaks_array[x_rcu]))
                shifted_info.append((x_rcu, peaks_array[x_rcu], median_max_sb))

            if ((peaks_bw_array[y_rcu] > 20) and (abs(peaks_array[y_rcu] - median_max_sb) > 10)):
                logger.debug("rcu=%d: Y-top shifted normal=%d, now=%d" %(y_rcu, median_max_sb, peaks_array[y_rcu]))
                shifted_info.append((y_rcu, peaks_array[y_rcu], median_max_sb))
    
    # if more than half de antennes down or shifted, skip test
    if len(down_info) > (_data.shape[0] / 2):
        down_info = list()
    if len(shifted_info) > (_data.shape[0] / 2):
        shifted_info = list()    
    return (down_info, shifted_info)


# find oscillation
def search_oscillation(data, pol, delta):
    info = list()
    _data = data.getAll(pol=pol)
    mean_spectras = ma.mean(_data, axis=1)
    mean_spectra  = ma.mean(mean_spectras, axis=0)
    mean_low      = ma.mean(_data.min(axis=1))
    info.append((-1, 0, 0, 0))

    for rcu in data.getActiveRcus(pol):
        rcu_bin = rcu
        if pol not in ('XY', 'xy'):
            rcu_bin /= 2
        #logger.debug("rcu=%d  rcu_bin=%d" %(rcu, rcu_bin))
        #max_peak_val  = 0
        max_n_peaks   = 0
        max_sum_peaks = 0
        peaks = cSearchPeak(mean_spectras[rcu_bin,:] - mean_spectra)
        if peaks.valid_data:
            peaks.search(delta=delta, max_width=8)
            max_val       = mean_spectras[rcu_bin,:].max()
            max_n_peaks   = peaks.nMaxPeaks()
            max_sum_peaks = peaks.getSumPeaks()

        bin_low = _data[rcu_bin,:,:].min(axis=0).mean()
        if max_n_peaks > 5:
            logger.debug("rcu_bin=%d: number-of-peaks=%d  max_value=%3.1f  peaks_sum=%5.3f low_value=%3.1f" %\
                        (rcu_bin, max_n_peaks, max_val, max_sum_peaks, bin_low))
            if bin_low > (mean_low + 2.0): #peaks.getSumPeaks() > (median_sum_peaks * 2.0):
                info.append((rcu_bin, max_sum_peaks, max_n_peaks, bin_low))

        if max_val > 150.0: # only one high peek
            info.append((rcu_bin, max_sum_peaks, max_n_peaks, bin_low))

    return (info) #(sorted(info,reverse=True))

# find summator noise
def search_summator_noise(data, pol, min_peak):
    sn_info = list() # summator noise
    cr_info = list() # cable reflection
    _data = data.getAll(pol=pol)

    secs = _data.shape[1]
    for rcu in sorted(data.getActiveRcus(pol)):
        rcu_bin = rcu
        if pol not in ('XY', 'xy'):
            rcu_bin /= 2
        #logger.debug("rcu=%d  rcu_bin=%d" %(rcu, rcu_bin))
        sum_sn_peaks = 0
        sum_cr_peaks = 0
        max_peaks    = 0
        for sec in range(secs):
            peaks_ref = cSearchPeak(_data[:,sec,:].mean(axis=0))
            if peaks_ref.valid_data:
                peaks_ref.search(delta=min_peak)

            peaks = cSearchPeak(_data[rcu_bin,sec,:])
            if peaks.valid_data:
                peaks.search(delta=min_peak, skip_list=peaks_ref.max_peaks)
                n_peaks = peaks.nMaxPeaks()
                if n_peaks < 3:
                    continue
                sn_peaks   = 0
                cr_peaks   = 0
                last_sb, min_sb, max_sb = peaks.max_peaks[0]
                last_sb_val = peaks.getPeakValue(last_sb)
                for sb, min_sb, max_sb in peaks.max_peaks[1:]:
                    sb_val = peaks.getPeakValue(sb)

                    sb_diff     = sb - last_sb
                    sb_val_diff = sb_val - last_sb_val
                    if sb_diff in (3,4):
                        if abs(sb_val_diff) < 2.0:
                            sn_peaks += 1
                        elif sn_peaks < 6 and abs(sb_val_diff) > 3.0:
                            sn_peaks = 0
                    if sb_diff in (6,7):
                        if abs(sb_val_diff) < 2.0:
                            cr_peaks += 1
                        elif cr_peaks < 6 and  abs(sb_val_diff) > 3.0:
                            cr_peaks = 0
                    last_sb     = sb
                    last_sb_val = sb_val

                sum_sn_peaks += sn_peaks
                sum_cr_peaks += cr_peaks
                max_peaks = max(max_peaks, n_peaks)

        if (sum_sn_peaks > (secs * 3.0)):
            sn_peaks = sum_sn_peaks / secs
            sn_info.append((rcu_bin, sn_peaks, max_peaks))
        if sum_cr_peaks > (secs * 3.0):
            cr_peaks = sum_cr_peaks / secs
            cr_info.append((rcu_bin, cr_peaks, max_peaks))
    return (sn_info, cr_info)


# find noise
# noise looks like the noise floor is going up and down
#
# kijk ook naar op en neer gaande gemiddelden
def search_noise(data, pol, low_deviation, high_deviation, max_diff):
    _data = data.getAll(pol=pol)
    high_info   = list()
    low_info    = list()
    jitter_info = list()

    ref_value    = ma.median(_data)
    # loop over rcus
    for rcu in sorted(data.getActiveRcus(pol)):
        rcu_bin = rcu
        if pol not in ('XY', 'xy'):
            rcu_bin /= 2
        #logger.debug("rcu=%d  rcu_bin=%d" %(rcu, rcu_bin))
        rcu_bin_value = ma.median(_data[rcu_bin,:,:])
        if rcu_bin_value < (ref_value + low_deviation):
            logger.debug("rcu_bin=%d: masked, low signal, ref=%5.3f val=%5.3f" %(rcu_bin, ref_value, rcu_bin_value))
            low_info.append((rcu_bin, _data[rcu_bin,:,:].min(), -1 , (ref_value+low_deviation), (_data[rcu_bin,:,:].max() - _data[rcu_bin,:,:].min())))
            _data[rcu_bin,:,:] = ma.masked
    spec_median  = ma.median(_data, axis=2)
    spec_max     = spec_median.max(axis=1)
    spec_min     = spec_median.min(axis=1)
    ref_value    = ma.median(_data)
    ref_diff     = ma.median(spec_max) - ma.median(spec_min)
    ref_std      = ma.std(spec_median)
    #high_limit = ref_value + min(max((ref_std * 3.0),0.75), high_deviation)
    high_limit = ref_value + max((ref_std * 3.0), high_deviation)
    low_limit  = ref_value + min((ref_std * -3.0), low_deviation)
    n_secs = _data.shape[1]
    logger.debug("median-signal=%5.3fdB, median-fluctuation=%5.3fdB, std=%5.3f, high_limit=%5.3fdB low_limit=%5.3fdB" %\
                (ref_value, ref_diff, ref_std, high_limit, low_limit))
    # loop over rcus
    for rcu in sorted(data.getActiveRcus(pol)):
        rcu_bin = rcu
        if pol not in ('XY', 'xy'):
            rcu_bin /= 2
        #logger.debug("rcu=%d  rcu_bin=%d" %(rcu, rcu_bin))
        peaks = cSearchPeak(_data[rcu_bin,0,:])
        if not peaks.valid_data:
            return (low_info, high_info, jitter_info)
        peaks.search(delta=10.0)
        if peaks.nMaxPeaks() >= 30:
            logger.debug("rcu_bin=%d: found %d peaks, skip noise test" %(rcu_bin, peaks.nMaxPeaks()))
        else:
            n_bad_high_secs    = 0
            n_bad_low_secs     = 0
            if _data.shape[1] == 1:
                n_bad_high_secs = 1
                n_bad_low_secs  = 1
            n_bad_jitter_secs  = 0

            rcu_bin_max_diff = spec_max[rcu_bin] - spec_min[rcu_bin]
            #logger.debug("rcu_bin_max_diff %f" %(rcu_bin_max_diff))
            # loop over secs
            for val in spec_median[rcu_bin,:]:
                #logger.debug("rcu_bin=%d: high-noise value=%5.3fdB  max-ref-value=%5.3fdB" %(rcu_bin, val, ref_value))
                if (val > high_limit):
                    n_bad_high_secs += 1

                if (val < low_limit):
                    n_bad_low_secs += 1

            if n_bad_high_secs > 1:
                high_info.append((rcu_bin, spec_max[rcu_bin], n_bad_high_secs, high_limit, rcu_bin_max_diff))
                logger.debug("rcu_bin=%d: max-noise=%5.3f  %d of %d seconds bad" %(rcu_bin, spec_max[rcu_bin], n_bad_high_secs, n_secs))

            if n_bad_low_secs > 1:
                low_info.append((rcu_bin, spec_min[rcu_bin], n_bad_low_secs , low_limit, rcu_bin_max_diff))
                logger.debug("rcu_bin=%d: min-noise=%5.3f %d of %d seconds bad" %(rcu_bin, spec_min[rcu_bin], n_bad_low_secs, n_secs))

            if (n_bad_high_secs == 0) and (n_bad_low_secs == 0):
                max_cnt = 0
                min_cnt = 0
                if rcu_bin_max_diff > (ref_diff + max_diff):
                    check_high_value = ref_value + (ref_diff / 2.0)
                    check_low_value  = ref_value - (ref_diff / 2.0)
                    for val in spec_median[rcu_bin,:]:
                        if val > check_high_value:
                            max_cnt += 1
                        if val < check_low_value:
                            min_cnt += 1
                            
                    # minimal 20% of the values must be out of the check band
                    secs = _data.shape[1]
                    if max_cnt > (secs * 0.10) and min_cnt > (secs * 0.10):
                        n_bad_jitter_secs = max_cnt + min_cnt
                        jitter_info.append((rcu_bin, rcu_bin_max_diff, ref_diff, n_bad_jitter_secs))
                    logger.debug("rcu_bin=%d: max spectrum fluctuation %5.3f dB" %(rcu_bin, rcu_bin_max_diff))
    return (low_info, high_info, jitter_info)

# find spurious around normal signals
#
def search_spurious(data, pol, delta):
    info = list()
    _data = data.getAll(pol=pol)
    max_data  = ma.max(_data, axis=1)
    mean_data = ma.mean(_data, axis=1)
    median_spec = ma.mean(max_data, axis=0)
    peaks = cSearchPeak(median_spec)
    if not peaks.valid_data:
        return (info)

    # first mask peaks available in all data
    peaks.search(delta=(delta/2.0)) #deta=20 for HBA
    for peak, min_sb, max_sb in peaks.max_peaks:
        peakwidth = max_sb - min_sb
        if peakwidth > 8:
            continue
        min_sb = max(min_sb-1, 0)
        max_sb = min(max_sb+1, peaks.n_data-1)
        logger.debug("mask sb %d..%d" %(min_sb, max_sb))
        for i in range(min_sb, max_sb, 1):
            mean_data[:,i] = ma.masked

    # search in all data for spurious
    for rcu in sorted(data.getActiveRcus(pol)):
        rcu_bin = rcu
        if pol not in ('XY', 'xy'):
            rcu_bin /= 2
        logger.debug("rcu=%d  rcu_bin=%d" %(rcu, rcu_bin))
        peaks = cSearchPeak(mean_data[rcu_bin,:])
        if peaks.valid_data:
            peaks.search(delta=delta)
            for peak, min_sb, max_sb in peaks.max_peaks:
                peakwidth = max_sb - min_sb
                if peakwidth > 10:
                    continue
                peak_val = peaks.getPeakValue(peak)
                if peakwidth < 100 and peak_val != NaN:
                    logger.debug("rcu_bin=%d: spurious, subband=%d..%d, peak=%3.1fdB" %(rcu_bin, min_sb, max_sb, peak_val))
            if peaks.nMaxPeaks() > 10:
                #print rcu_bin, peaks.nMaxPeaks()
                info.append(rcu_bin)
    return(info)

