#! /usr/bin/env python

import sys
import scipy
import numpy
import xcstats_classifier_max
import rcu_settings_masks
import re
import datetime
import sbstats_fitter as sbf

# This module classifies an incoming "sbstats" array as one or more of the following:
#  (the idea is that classification is a list of all applicable states)
#
# INPUT_DISABLE, ANTENNA_OFF, AMPLIFIER_OFF, ADC_OFF (some or all simultaneously)
# ZEROES 
# FLAT
# SHIFT_LEFT
# SHIFT_RIGHT
# INDETERMINATE_RCU_STATUS
# HBA_FILTERS_OFF
# NOMINAL, RCUMODE03 (or 4,5,6,7) (both)
# INVERTED, RCUMODE03 (or 4,5,6,7) (both)
# RCUSTATUS_WRONG_BAND, RCUMODE03, INVERTED (at least 1st&2nd)
# LOW_AMP
# OFF_NOMINAL

# RCUMODE truth table
# RCUMODE BANDSEL LB_FILTER HB_SEL_0 HB_SEL_1
#    3       1       0        x        x    
#    4       1       1        x        x
#    5       0       x        1        0
#    6       0       x        0        1
#    7       0       x        0        0

    

def classify(spectrum,rcu_byte):
    '''input: spectrum (512 bins), rcu_byte (16bits)
    classify SST discrete states'''

    classification = []

    rcu_status = rcu_settings_masks.rcu_status(rcu_byte)[0]

    #HERE I AM
    # add checks key HW switches, down the chain, quick exit because template fitting is expensive
    if not (rcu_status["INPUT_ENABLE"]):
        classification.append("INPUT_DISABLE")
    if not (rcu_status["LBL_EN"] or rcu_status["LBH_EN"] or rcu_status["HB_EN"]):
        classification.append("ANTENNA_OFF")
        # return [classification,0,0,0]
    if not (rcu_status["VL_EN"] or rcu_status["VH_EN"]):
        classification.append("AMPLIFIER_OFF")
        # return [classification,0,0,0]
    if not (rcu_status["VDIG_EN"]):
        classification.append("ADC_OFF")
        # return [classification,0,0,0]
    if (len(classification)>0):
        return [classification,0,0,0]
    
    spect = numpy.array(spectrum)
    # isn't there an easier way to reverse an array?
    ispect = spect[-1*numpy.arange(1,len(spect)+1)]

    # check for progressively less serious faults

    # check for "ZEROS" conditions. This happens if the Antenna Processor
    # is FFT'ing a constant (DC) signal. Since the AP inputs from the RCU
    # are registered, the DC offset isn't necessarily zero, but all other
    # spectral components are.

    if numpy.sometrue(spect == 0):
        classification.append("ZEROES")
        return [classification,0,0,0]

    # rcu_status = rcu_settings_masks.rcu_status(rcu_byte)[0]

    # classify passband shape

    # check for SHIFT condition
    # (one wing missing)

    # sorted_spect = spect.copy()
    # sorted_spect.sort()

    med_wing1 = numpy.median(spect[0:33])
    med_wing2 = numpy.median(spect[511-33:511])
    idx = numpy.argsort(spect[128:385])
    med_band = spect[128 + idx[128]]
    med_chan = 128 + idx[128]
    peak_band = spect[128+idx[-1]]
    peak_chan = idx[-1]

    # check for FLAT passband 
    # if (med_band < 2e6):
    if ( (abs(med_band - med_wing1) < 5e5) and
         (abs(med_band - med_wing2) < 5e5)):
        classification.append("FLAT")
        return [classification, med_band, peak_band, med_chan]

    # check for 'shifted' passband I've seen
    if (med_band/med_wing1 < 2.0):
        classification.append("SHIFT_LEFT")
        return [classification, med_band, peak_band, med_chan]
    elif (med_band/med_wing2 < 2.0):
        classification.append("SHIFT_RIGHT")
        return [classification, med_band, peak_band, med_chan]

    # compare passband shape to filter setting

    # get a list of rcumodes to which I can fit
    modefuncs = [sbf.rcumode03_fit(),sbf.rcumode04_fit(),sbf.rcumode05_fit(),sbf.rcumode06_fit(),sbf.rcumode07_fit()]
    # this below is flexible but probably quite slow:
    #modefuncs = []
    #modekeys = []
    #for k,v in sbf.__dict__.items():
    #    if re.match("rcumode.._fit",k):
    #        modekeys.append(k)
    #modekeys.sort()
    #for modekey in modekeys:
    #    modefuncs.append(sbf.__dict__[modekey]())
            
    modefuncsidx = -1
    ALL_OFF = False
    if (rcu_status['BANDSEL'] == 1):
        # LB modes
        if (rcu_status['LB_FILTER'] == 0):
            # RCUMODE03
            modefuncsidx = 0 
        else:
            # RCUMODE04
            modefuncsidx = 1
    else: 
        # BANDSEL=0 => HB modes
        if (rcu_status['HB_SEL_0'] == 1):
            if (rcu_status['HB_SEL_1'] == 1):
                # ALL OFF!
                ALL_OFF = True
            else:
                # RCUMODE05
                modefuncsidx = 2
        else:
            if (rcu_status['HB_SEL_1'] == 1):
                # RCUMODE06
                modefuncsidx = 3
            else:
                # RCUMODE07
                modefuncsidx = 4

    if ((modefuncsidx == -1) and (not ALL_OFF)):
        classification.append("INDETERMINATE_RCU_STATUS")
        return [classification,0,0,0]

    if (ALL_OFF):
        classification.append("HBA_FILTERS_OFF")
        return [classification,0,0,0]

    #fit alleged RCU mode and check goodness
    passband_fit = modefuncs[modefuncsidx]
    # (cov_x, infodict,mesg) = passband_fit.fit(spect)
    [gain, fitness] = passband_fit.fit(spect)
    # limits come from examples in shm db
    if passband_fit.inlimits():
        classification.append("NOMINAL")
        classification.append(passband_fit.name)
    else:
        # check freq inversion
        #(cov_x, infodict,mesg) = passband_fit.fit(ispect)
        [gain, fitness] = passband_fit.fit(ispect)
        if passband_fit.inlimits():
            classification.append("INVERTED")
            classification.append(passband_fit.name)

    # if fit wasn't good
    if (len(classification) == 0):

        #go through the other bands looking for a better fit
        S = set(modefuncs) - set([modefuncs[modefuncsidx]])
        W = {}
        G = {}
        for s in S:
            # trial_fit = s.fit()
            trial_fit = s
            # (cov_x, infodict, mesg) = trial_fit.fit(spect)
            [gain,fitness] = trial_fit.fit(spect)
            # debug
            print trial_fit.name, trial_fit.g
            if trial_fit.inlimits():
                W[trial_fit.name] = trial_fit.fitness
                G[trial_fit.name] = trial_fit.g
        
        if (len(W) >= 1):
            # seemingly the rcu status byte is wrong
            # take the fittest (lowest fitness) observing band
            K = W.keys()[0]
            v = W[K]
            for k in W.keys():
                if W[k] < v:
                    v = W[k]
                    K = k
            classification.append("RCUSTATUS_WRONG_BAND")
            classification.append(K)
            # return (classification, med_band, peak_band, med_chan)
            # return [classification, gain, fitness]
            return [classification, med_band, peak_band, med_chan, G[K], W[K]]

        else:
            # no fits found among other freq bands, check for spectral inversion
            for s in S:
                # trial_fit = s.fit()
                trial_fit = s
                # (cov_x, infodict, mesg) = trial_fit.fit(ispect)
                [gain,fitness] = trial_fit.fit(ispect)
                if trial_fit.inlimits():
                    W[trial_fit.name] = trial_fit.fitness
        
                if (len(W) >= 1):
                    # seemingly the rcu status byte is wrong
                    # take the fittest (lowest fitness) observing band
                    K = W.keys()[0]
                    v = W[K]
                    for k in W.keys():
                        if W[k] < v:
                            v = W[k]
                            K = k
                    classification.append("RCUSTATUS_WRONG_BAND")
                    classification.append(K)
                    classification.append("INVERTED")
                    # return (classification, med_band, peak_band, med_chan)
                    return [classification, med_band, peak_band, med_chan, gain, fitness]


        # Still here, look for low amplitudes.
        # FLATs are already out
        if (med_band < 2e6):
            classification.append("LOW_AMP")
            # return (classification, med_band, peak_band, med_chan)
            return [classification, med_band, peak_band, med_chan, gain, fitness]
            

        # default - if no rcumode fit to spect or inversion was good
        classification.append("OFF_NOMINAL")

    # return (classification, med_band, peak_band, med_chan)
    return [classification, med_band, peak_band, med_chan, gain, fitness]
