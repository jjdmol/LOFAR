#!/usr/bin/python
import numpy
import math
from copy import copy
from numpy.polynomial.polynomial import polyval
from scipy.stats import chi2

from src.connectionMonet import MonetConnection
from src.connectionPostgres import PgConnection

class Spectra(object):
    """
    Calculating spectral indices from fluxes.
    """
    def __init__(self, conn):
        """
        """
        self.conn = conn
        self.args = []
        self.freq = []
        self.flux = []
        self.flux_err = []
        self.need_update = False

    def get_sql_data(self, runcatid):
        """
        Get spectral information for a given source.
        """
        cur = self.conn.get_cursor("""
select case when last_update_date > last_spectra_update_date
              or last_spectra_update_date is null then 1
       else 0 end as need_update, spectral_power,
       spectral_index_0, spectral_index_1, spectral_index_2,
       spectral_index_3, spectral_index_4
  from runningcatalog
 where runcatid = {0};""".format(runcatid))
        result = cur.fetchone()
        cur.close()
        print result
        self.need_update = result[0] == 1
        if not self.need_update:
            for f in xrange(int(result[1])):
                self.args.append(result[f+2])
        else:
            self.fit_spectra(runcatid)
        return self.args

    def get_approx(self, x):
        return polyval(x, self.args)

    def get_chi(self):
        tmp = 0
        for ind, y in enumerate(self.flux):
            tmp_v = self.get_approx(self.freq[ind])
            tmp = tmp + numpy.power((y - tmp_v), 2) #/fabs(tmp_v)
        return tmp

    def one_fit(self, power):
        self.args = numpy.polyfit(self.freq, self.flux, power)[::-1]
        return self.get_chi()

    def fit_spectra(self, runcat_id):
        """
        Fit spectra for a given object.
        """
        self.freq = []
        self.flux = []
        self.flux_err = []
        cursor = self.conn.get_cursor("""
select log(f.freq_central), log(rf.wm_f_int), rf.avg_weight_f_int
  from frequencybands f,
       runningcatalog_fluxes rf
 where f.freqbandid = rf.band
   and rf.runcat_id = %s
   and rf.stokes = 'I'""" % runcat_id)
        for xdata in iter(cursor.fetchone, None):
            self.freq.append(xdata[0])
            self.flux.append(xdata[1])
            self.flux_err.append(xdata[2])
        print self.freq
        print self.flux

        #for ind, y in enumerate(self.flux):
        #    print self.freq[ind], y
        cursor.close()
        sp_power = 0
        old_ratio = -2
        chi = None
        ratio = -1
        while sp_power < 5 and sp_power < len(self.freq) and old_ratio <= ratio:
            old_ratio = ratio
            old_chi = chi
            chi = self.one_fit(sp_power)
            if not old_chi:
                print sp_power, self.args, chi
                sp_power = sp_power + 1
                continue
            saved_args = copy(self.args)
            print sp_power, self.args, chi, old_chi, old_chi/chi
            ratio = numpy.abs(old_chi/chi)
            sp_power = sp_power + 1
        self.args = saved_args
        print self.args
        sp_power = sp_power - 1
        sp_update = ','.join(map(lambda x: 'spectral_index_%s = %s' % (x, self.args[x]), range(sp_power)))
        self.conn.execute("""
update runningcatalog
   set spectral_power = %s,
       %s,
       last_update_date = current_timestamp,
       last_spectra_update_date = current_timestamp
 where runcatid = %s;""" % (sp_power, sp_update, runcat_id))

