#! /usr/bin/env python

#import numarray, sys

# This module classifies an incoming "rspstatus" dict as one of the following:
#
# NOMINAL
#
# OFF_NOM_VOLTAGE
# OFF_NOM_TEMP
# OFF_NOM_OVERFLOW
# OFF_NOM_ADC

def classify_dbresult(rspstatus):
    """input:  Lofar.shm.db.DatabaseResult of rspstatus
       output: vector of classification strings"""

    if (rspstatus == None):
        return None

    classification = []

    classification.append(classify_adcs(rspstatus.ado_adc_offset_x,rspstatus.ado_adc_offset_y))
    classification.append(classify_overflows(rspstatus.rcu_num_overflow_x,rspstatus.rcu_num_overflow_y))
    classification.append(classify_temps(rspstatus.rsp_board_temps))
    classification.append(classify_volts(rspstatus.rsp_board_volts))

    #kludgey. remove null-strings = OKs
    for i in range(0, classification.count('')):
        classification.remove('')

    if (len(classification) == 0):
        classification.append('NOMINAL')

    return (classification)

def classify_temps(temps):
    #These limits come from Eric Kooistra

    assert (len(temps) == 6)

    classification = ''
    
    for temp in temps:
        if ((temp < 20) or (temp > 85)):
            classification = 'OFF_NOM_TEMP'
            break

    return classification

def classify_volts(volts):
    #These limits come from Eric Kooistra
    
    assert (len(volts) == 3)

    classification = ''
    
    if ((volts[0] < 1.15) or (volts[0] > 1.25) or 
        (volts[1] < 2.40) or (volts[1] > 2.60) or 
        (volts[2] < 3.18) or (volts[2] > 3.40)):
        classification = 'OFF_NOM_VOLT'
    
    return classification

def classify_adcs(adc_off_x, adc_off_y):
    #These limits come from Eric Kooistra
    
    assert ((len(adc_off_x) == 4) and (len(adc_off_y) == 4))
    
    classification = ''
    
    adcs = []
    adcs.extend(adc_off_x)
    adcs.extend(adc_off_y)
    for adc in adcs:
        if ((abs(adc) > 50)):
            classification = 'OFF_NOM_ADC'
            break

    return classification

def classify_overflows(overflow_x, overflow_y):
    #These limits come from Eric Kooistra
    
    assert ((len(overflow_x) == 4) and (len(overflow_y) == 4))
    
    classification = ''
    
    overflows = []
    overflows.extend(overflow_x)
    overflows.extend(overflow_y)
    for overflow in overflows:
        if (overflow != 0):
            classification = 'OFF_NOM_OVERFLOW'
            break

    return classification

