from pylab import *

def normalized_gaussian(x, sigma):
    return exp(-0.5*(x/sigma)**2)/(sigma*sqrt(2*pi))

def dispersion_delay_s(freq_hz, DM_pc_cm):
    freq_mhz = freq_hz/1e6
    D_mhz_s = 4148.808
    return D_mhz_s*DM_pc_cm/(freq_mhz**2)

def subband_differrential_dispersion_s(freq_hz, DM_pc_cm, clock_mhz=200):
    clock_hz = clock_mhz*1e6
    subband_width_hz = clock_hz / 1024.0
    return (dispersion_delay_s(freq_hz - (subband_width_hz*0.5), DM_pc_cm) -
            dispersion_delay_s(freq_hz + (subband_width_hz*0.5), DM_pc_cm))


def dispersed_pulse(t_s, t_0_s, width_s, freq_hz, DM_pc_cm):
    return normalized_gaussian(t_s - t_0_s - dispersion_delay_s(freq_hz, DM_pc_cm), width_s)


def fractional_shift(x_123, fraction):
    r'''
    Fraction is the fraction of an element that the signal has to
    shift towards a smaller.
    '''
    x_minus, x_0, x_plus = x_123

    shift_factor = exp(2j*pi*fraction/3.0)
    ff           = exp(2j*pi/3)
    y_plus  = (x_plus/ff + x_0 +x_minus*ff)*shift_factor/3.0
    y_0     = (x_plus + x_0 + x_minus)/3.0
    y_minus = ((x_plus*ff + x_0 + x_minus/ff)/shift_factor)/3.0
    
    z_plus = (y_plus*ff + y_0 +y_minus/ff)
    z_0    = y_plus + y_0 + y_minus
    z_minus = (y_plus/ff + y_0 +y_minus*ff)
    
    return array([z_minus, z_0, z_plus])

