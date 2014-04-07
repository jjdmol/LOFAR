#!/usr/bin/python
import numpy


class Spectra(object):
    """
    Calculating spectral indices from fluxes.
    """
    def __init__(self, conn):
        """
        """
        self.conn = conn

    def fit_spectra(self, runcat_id):
        """
        Fit spectra for a given object.
        """
        freq = []
        flux = []
        #flux_err = []
        cursor = self.conn.get_cursor("""
select log(f.freq_central), log(rf.avg_f_int), rf.avg_weight_f_int
  from frequencybands f,
       runningcatalog_fluxes rf
 where f.freqbandid = rf.band
   and rf.runcat_id = %s
   and rf.stokes = 'I'""" % runcat_id)
        for xdata in iter(cursor.fetchone, None):
            freq.append(xdata[0])
            flux.append(xdata[1])
            #flux_err.append(xdata[2])
        z = numpy.polyfit(freq, flux, 1)
        cursor.execute("""
update runningcatalog
   set spectral_power = 1,
       spectral_index_0 = %s,
       spectral_index_1 = %s,
       last_update_date = current_timestamp
 where runcatid = %s;""" % (z[0], z[1], runcat_id))
