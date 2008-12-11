#! /usr/bin/env python

import numarray, sys

# This module classifies an incoming "sbstats" array as one of the following:
#
# NOMINAL
# ZEROES
# NO_SIGNAL
# LOW_SIGNAL
# ABNORMAL

# SKY
# DUBIOUS
# FLAT
#

def classify(spectrum):

    sorted_spectrum = spectrum.copy()
    sorted_spectrum.sort()

    wings1 = numarray.array(spectrum[16:64])
    wings2 = numarray.array(spectrum[511-64:511-16])
    sorted_wings1 = wings1.copy()
    sorted_wings1.sort()
    med_wings1 = sorted_wings1[16]
    sorted_wings2 = wings2.copy()
    sorted_wings2.sort()
    med_wings2 = sorted_wings2[16]

    band = numarray.array(spectrum[128:384])
    sorted_band = band.copy()
    sorted_band.sort()
    med_band = sorted_band[128]
    peak_band = sorted_band[255]
    
    idx = numarray.argsort(spectrum)
    med_chan = idx[255]
    peak_chan = idx[511]
    med_power = sorted_spectrum[255]
    peak_power = sorted_spectrum[511]

    # check for "ZEROS" conditions. This happens if the Antenna Processor
    # is FFT'ing a constant (DC) signal. Since the AP inputs from the RCU
    # are registered, the DC offset isn't necessarily zero, but all other
    # spectral components are.

    classification = []

    # The AP is not getting a sane digitized signal.
    if numarray.sometrue(spectrum==0):
        classification.append("ZEROES")

        # check if we're at least within reasonable bounds

    # classify bandpass shape
    ratio = med_band/(med_wings1 + med_wings2)/2

    if (med_band < 2e6):
        classification.append("FLAT")
    elif (med_band > 4e6):
        classification.append("SKY")
    else:
        classification.append("DUBIOUS")
    
    return (classification, med_power, peak_power, med_chan)

#    #elif (mean_power<8e5) or (mean_power>1e7) or (peak_power<1e6) or (peak_power>1e13):
#
#
#        # We're out-of-bounds
#        classification = "ABNORMAL"
#        #classification = "HIGH_SIGNAL" # KLUDGE
#    else:
#
#        peak = peak_power / mean_power
#
#        if (peak < 1.5):
#            classification = "NO_SIGNAL"
#        elif (peak < 10):
#            classification = "LOW_SIGNAL"
#        elif (peak < 50):
#            classification = "MEDIUM_SIGNAL"
#        else:
#            classification = "HIGH_SIGNAL"
#
#    # done classifying
#
#
