#!/usr/bin/env python2

from pyrap.tables   import table
from pyrap.measures import measures
from pyrap.quanta   import quantity
from numpy import array, pi, arccos, inner, concatenate, unique
from numpy.linalg import norm


import sys


def test_ant_columns(msname):
    r'''
    Test if ANTENNA1 <= ANTENNA2.
    '''
    tab  = table(msname)
    ant1 = tab.getcol('ANTENNA1')
    ant2 = tab.getcol('ANTENNA2')
    
    ok = ant1 <= ant2
    if not all(ok):
        raise ValueError('ANTENNA1 is not always  <= ANTENNA2')
    return ok





def test_uvw_casacore(msname):
    r'''
    Test if UVW = UVW_FROM_ITRF(XYZ_ANTENNA2 - XYZ_ANTENNA1)
    '''
    
    dm = measures()
    
    tab = table(msname)
    ant1 = tab.getcol('ANTENNA1')
    ant2 = tab.getcol('ANTENNA2')
    uvw  = tab.getcol('UVW')
    time = tab.getcol('TIME')
    time_epochs = [dm.epoch('UTC',quantity(mjds, 's'))
                   for mjds in concatenate([time[:3000], time[-3000:]], axis=0)]

    field_table         = table(tab.getkeyword('FIELD'))
    phase_dir           = field_table.getcol('PHASE_DIR')[0,0,:]
    phase_dir_meas_info = field_table.getcolkeyword('PHASE_DIR', 'MEASINFO')
    phase_dir_units     = field_table.getcolkeyword('PHASE_DIR', 'QuantumUnits')
    phase_dir_quantities = ['%r%s' % (angle, unit)
                            for angle, unit in zip(phase_dir, phase_dir_units)]

    
    lofar = dm.position('ITRF', '3826577.462m', '461022.624m', '5064892.526m')
    dm.do_frame(lofar)
    dm.do_frame(dm.direction(phase_dir_meas_info['Ref'],
                             phase_dir_quantities[0], phase_dir_quantities[1]))

    ant_table = table(tab.getkeyword('ANTENNA'))
    xyz       = ant_table.getcol('POSITION')
    xyz_1 = array([xyz[ant,:] for ant in ant1])
    xyz_2 = array([xyz[ant,:] for ant in ant2])
    delta_xyz = (xyz_2 - xyz_1)
    
    uvw_computed = []
    for dxyz, epoch  in zip(delta_xyz[:2000], time_epochs):
        dm.do_frame(epoch)
        bl_quant = [quantity(value, 'm') for value in dxyz]
        #print bl_quant
        bl = dm.baseline('ITRF', bl_quant[0], bl_quant[1], bl_quant[2])
        #print bl
        uvw_computed.append(dm.to_uvw(bl))
    
    for a1, a2, uvw_ms, uvw_comp in zip(ant1, ant2, uvw, uvw_computed):
        uvw_comp = array(uvw_comp['xyz'].get_value('m'))
        if norm(uvw_ms) > 0.0 :
            arg = inner(uvw_ms, uvw_comp)/(norm(uvw_ms)**2)
            if arg > 1.0:
                arg = 1.0
            if arg < -1.0:
                arg = -1.0
            angle_mas = arccos(arg)*180*3600*1000./pi
            if angle_mas > 50.0:
                fmt = 'Angle UVW computed / MS %03d--%03d = %.3f mas (%.3f deg)'
                raise ValueError(fmt %
                                 (a1, a2, angle_mas, angle_mas/1000./3600.0))
        diff = uvw_ms - uvw_comp
        if norm(diff) > 1e-3:
            raise ValueError('UVW computed - UVW MS %03d--%03d = %.3f mm' %
                             (a1, a2, norm(diff)*1e3))

    return True
    


def test_uvw_new_casacore(msname):
    r'''
    Test if UVW = UVW_FROM_ITRF(XYZ_ANTENNA2 - XYZ_ANTENNA1)
    '''
    
    dm = measures()
    
    tab = table(msname)
    ant1 = tab.getcol('ANTENNA1')
    ant2 = tab.getcol('ANTENNA2')
    uvw  = tab.getcol('UVW')
    time = tab.getcol('TIME')
    time_epochs = [dm.epoch('UTC',quantity(mjds, 's'))
                   for mjds in concatenate([time[:3000], time[-3000:]], axis=0)]

    field_table         = table(tab.getkeyword('FIELD'))
    phase_dir           = field_table.getcol('PHASE_DIR')[0,0,:]
    phase_dir_meas_info = field_table.getcolkeyword('PHASE_DIR', 'MEASINFO')
    phase_dir_units     = field_table.getcolkeyword('PHASE_DIR', 'QuantumUnits')
    phase_dir_quantities = ['%r%s' % (angle, unit)
                            for angle, unit in zip(phase_dir, phase_dir_units)]

    
    lofar = dm.position('ITRF', '3826577.462m', '461022.624m', '5064892.526m')
    dm.do_frame(lofar)
    dm.do_frame(dm.direction(phase_dir_meas_info['Ref'],
                             phase_dir_quantities[0], phase_dir_quantities[1]))

    ant_table = table(tab.getkeyword('ANTENNA'))
    xyz       = ant_table.getcol('POSITION')
    xyz_1 = array([xyz[ant,:] for ant in ant1])
    xyz_2 = array([xyz[ant,:] for ant in ant2])
    delta_xyz = (xyz_2 - xyz_1)
    
    uvw_computed = []
    for ant1_xyz, ant2_xyz, epoch  in zip(xyz_1[:2000], xyz_2[:2000], time_epochs):
        dm.do_frame(epoch)
        
        ant_1_quant = [quantity(p, 'm') for p in ant1_xyz]
        ant_2_quant = [quantity(p, 'm') for p in ant2_xyz]
        ant_pos = dm.position('ITRF', map(list, zip(ant_1_quant, ant_2_quant)))

        bl_quant = [quantity(value, 'm') for value in dxyz]
        #print bl_quant
        bl = dm.baseline('ITRF', bl_quant[0], bl_quant[1], bl_quant[2])
        #print bl
        uvw_computed.append(dm.to_uvw(bl))
    
    for a1, a2, uvw_ms, uvw_comp in zip(ant1, ant2, uvw, uvw_computed):
        uvw_comp = array(uvw_comp['xyz'].get_value('m'))
        if norm(uvw_ms) > 0.0 :
            arg = inner(uvw_ms, uvw_comp)/(norm(uvw_ms)**2)
            if arg > 1.0:
                arg = 1.0
            if arg < -1.0:
                arg = -1.0
            angle_mas = arccos(arg)*180*3600*1000./pi
            if angle_mas > 50.0:
                fmt = 'Angle UVW computed / MS %03d--%03d = %.3f mas (%.3f deg)'
                raise ValueError(fmt %
                                 (a1, a2, angle_mas, angle_mas/1000./3600.0))
        diff = uvw_ms - uvw_comp
        if norm(diff) > 1e-3:
            raise ValueError('UVW computed - UVW MS %03d--%03d = %.3f mm' %
                             (a1, a2, norm(diff)*1e3))

    return True



def test_frequency_labels(msname):
    tab = table(msname)
    spw_tab = table(tab.getkeyword('SPECTRAL_WINDOW'))
    chan_freq = spw_tab.getcol('CHAN_FREQ')
    ref_freq  = spw_tab.getcol('REF_FREQUENCY')
    num_chan  = spw_tab.getcol('NUM_CHAN')
    sb_bandwidth = spw_tab.getcol('TOTAL_BANDWIDTH')
    
    clock_mhz = None
    if sb_bandwidth[0] == 100e6/512.:
        clock_mhz = 200
    if sb_bandwidth[0] == 80e6/512.:
        clock_mhz = 160
    if not clock_mhz in [160, 200]:
        raise ValueError('Strange sub band width(s) %r' % sb_bandwidth)
    if len(unique(sb_bandwidth)) != 1:
        raise ValueError('Multiple different sub band widths: %r' % sb_bandwidth)
    
    for i, (freqs, ref) in enumerate(zip(chan_freq, ref_freq)):
        if len(freqs) != num_chan[i]:
            raise ValueError('SB %d: # channel frequencies (%r) != NUM_CHAN (%r)' %
                             (i, len(freqs), num_chan[i]))
        if freqs[num_chan[i]/2] != ref:
            raise ValueError('SB %d: chan_freq[%d](%r) != ref(%r)' %
                             (i, num_chan[i]/2, freqs[num_chan[i]/2], ref))
        chan_width = unique(freqs[1:]-freqs[:-1])
        if len(chan_width) != 1:
            raise ValueError('SB %d: Channel frequencies are not equidistant' % i)
        if chan_width <= 0.0:
            raise ValueError('SB %d: Channel frequencies are in reversed order' % i)
        if abs(chan_width[0] - sb_bandwidth[i]/num_chan[i]) > 1e-3:
            fmt = 'SB %d: Channel frequency spacing != sub band width / num_chan'
            raise ValueError(fmt % i)


    obs = table(tab.getkeyword('OBSERVATION'))
    if clock_mhz != int(obs[0]['LOFAR_CLOCK_FREQUENCY']):
        raise ValueError('clock_mhz(%r) != OBSERVATION.LOFAR_CLOCK_FREQUENCY(%r)' %
                         (clock_mhz, int(obs[0]['LOFAR_CLOCK_FREQUENCY'])))
    filter_name = obs[0]['LOFAR_FILTER_SELECTION']
    if filter_name in 'LBA_10_90 LBA_30_90':
        if ref_freq.max() >= 100e6:
            raise ValueError('%r MHz not in %s' % (ref_freq.max()/1e6, filter_name))
    elif filter_name == 'HBA_110_190':
        if ref_freq.min() < 100e6 or ref_freq.max() > 200e6:
            raise ValueError('%r or %r MHz not in %s' %
                             (ref_freq.moin()/1e6, ref_freq.max()/1e6, filter_name))
    elif filter_name == 'HBA_210_250':
        if ref_freq.min() < 200e6 or ref_freq.max() > 300e6:
            raise ValueError('%r or %r MHz not in %s' %
                             (ref_freq.moin()/1e6, ref_freq.max()/1e6, filter_name))
    else:
        raise ValueError('I don\'t know what to do with filter %s' %
                         filter_name)        
    return True
        



def test_uvw_calc(msname):
    raise NotImplementedError()


def test_autocorr_real(msname):
    tab = table(msname)
    antenna_table = table(tab.getkeyword('ANTENNA'))
    field_names = antenna_table.getcol('NAME')
    num_ant = len(field_names)
    fractional_eps = 1e-9
    ants_with_nonzero_imags = []
    for ant in range(num_ant):
        selection = tab.query('ANTENNA1 == %d && ANTENNA2 == %d' % (ant, ant))
        data = selection.getcol('DATA')[:, 1:-1, 0::3]
        data = data.imag/data.real
        if abs(data).max() > fractional_eps:
            ants_with_nonzero_imags.append((field_names[ant], abs(data).max()))
    if len(ants_with_nonzero_imags) > 0:
        raise ValueError('Autocorrelations of %r has imaginary parts > %e*real!' % 
                         (ants_with_nonzero_imags, fractional_eps))

    


def number_switched_off_rcus(field_name, rcu_flags):
    if 'HBA0' in field_name:
        return rcu_flags[:24,:].sum()
    if 'HBA1' in field_name:
        return rcu_flags[24:,:].sum()
    return rcu_flags.sum()


def test_antenna_field(msname):
    tab = table(msname)
    antenna_field_info = table(tab.getkeyword('LOFAR_ANTENNA_FIELD'))
    if antenna_field_info.nrows() == 0:
        raise ValueError('LOFAR_ELEMENT_FAILURE empty')
    element_flags = [ row['ELEMENT_FLAG'] for row in antenna_field_info]
    antenna_table = table(tab.getkeyword('ANTENNA'))
    field_names = antenna_table.getcol('NAME')
    if len(element_flags) != antenna_table.nrows():
        raise ValueError('Length of LOFAR_ANTENNA_FIELD table not equal to ANTENNA')
    broken_rcus = array([number_switched_off_rcus(name, rcu_flags)
                         for name, rcu_flags in zip(field_names,
                                                    element_flags)])
    field_off_pairs = zip(field_names, broken_rcus)
    print('\n'.join(['%s: %d' % (name, off)
                     for name, off in field_off_pairs]))
    if any(broken_rcus % 2) == 1:
        raise ValueError('Only even numbers of broken RCUs expected')
    if broken_rcus.sum() == 0:
        raise ValueError('No broken RCUs detected?')
    no_broken = [off == 0 or ('RS' in field and 'HBA' in field and  off == 48)
                 for field, off in field_off_pairs]
    if all(no_broken):
        raise ValueError('Only 0 or 48 antenna\'s off? UNLIKELY!')
    return True




def run_tests(test_cases, msname):
    successful = []
    failed     = []
    for test in test_cases:
        try:
            print 'Running %s' % test.__name__
            result = test(msname)
            successful.append(test.__name__)
        except:
            print '--- FAILURE ---'
            message = ('%s: %s: %s' %
                       (test.__name__,
                        sys.exc_info()[0].__name__,
                        sys.exc_info()[1]))
            print message+'\n'
            failed.append(message)
    print '\nSuccessful\n----------\n  %s' % '\n  '.join(successful)
    if len(failed) > 0:
        print '\n'
        print 'FAILED\n======\n\n  %s' % '\n  '.join(failed)
    else:
        print 'OK'
    return len(failed) == 0




if __name__ == '__main__':
    msname = sys.argv[1]
    print 'Verifying %s' % msname
    sys.exit(run_tests([eval(name) for name in dir() if name[0:5] == 'test_'],
                       msname))
    
