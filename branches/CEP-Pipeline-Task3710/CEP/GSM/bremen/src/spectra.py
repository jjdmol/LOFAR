#!/usr/bin/python
import numpy
from copy import copy


def _verify_versions(a, b):
    a = map(int, a.split('.'))
    b = map(int, b.split('.'))
    for i, val in enumerate(a):
        if b[i] < val:
            return False
    return True

if _verify_versions('1.4.0', numpy.__version__):
    print 'Using 1.4 version'
    from numpy.polynomial.polynomial import polyval
else:
    print 'Using substitute for 1.3 version'
    from numpy import polyval as polyval_numpy
    def polyval(x, args):
        if not isinstance(args, list):
            args = args.tolist()
        pargs = copy(args)
        pargs.reverse()
        return polyval_numpy(pargs, x)

#Unused:
#from scipy.stats import chi2


class Spectra(object):
    """
    Calculating spectral indices from fluxes.
    """
    def __init__(self, conn):
        """
        No data is loaded on init.
        @param conn: connection to the database. Has to be opened.
        """
        self.conn = conn
        self.args = []
        self.freq = []
        self.flux = []
        self.flux_err = []
        self.need_update = False

    def get_sql_data(self, runcatid):
        """
        Get spectral information for a given source (selected by ID).
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
        self.need_update = result[0] == 1
        if not self.need_update:
            for order in xrange(int(result[1])):
                self.args.append(result[order + 2])
        else:
            self.fit_spectra(runcatid)
        return self.args

    def _get_chi(self, args):
        """
        Gets chi-squared statistics for an approximation with given params.
        :param args: array of polynomia coefficients.
        """
        tmp = 0
        for ind, y in enumerate(self.flux):
            tmp_v = polyval(self.freq[ind], args)
            tmp = tmp + numpy.power((y - tmp_v) / self.flux_err[ind], 2)
        return tmp

    def _one_fit(self, power):
        """
        Fit the spectra with a polinom.
        :param power: polinom order.
        :returns: chi-squared value, chi-squared pdf value,
        polynom coefficients.
        """
        args = numpy.polyfit(self.freq, self.flux, power)[::-1]
        chi = self._get_chi(args)
        #so far this is not used
        #chi_pdf = chi2.logpdf(chi, len(self.freq) - power - 1)
        return chi, args

    def best_fit(self):
        """
        Find best fit to the data.
        Starts with polynom of order 0 (constant).
        On each iteration chi-square value and it's ratio to the
        previous value are calculated.
        Increases order of polynom until ratio is larger than 3,
        or until order = 5 or until order is higher than number of known
        points in the spectrum.
        :returns: coefficient list and order of polynom.
        """
        sp_power = 0
        ratio = None
        saved_args = []
        chi = None
        while sp_power < max(6, len(self.freq) - 1) and \
              (not ratio or ratio > 3):
            old_chi = chi
            old_args = copy(saved_args)
            chi, saved_args = self._one_fit(sp_power)
            if old_chi:
                ratio = numpy.abs(old_chi / chi)
            sp_power = sp_power + 1
        sp_power = sp_power - 1
        return old_args, sp_power

    def fit_spectra(self, runcat_id):
        """
        Fit spectra for a given object and save fit to the database.
        """
        self.freq = []
        self.flux = []
        self.flux_err = []
        if self.conn.is_monet():
            func = "log10("
        else:
            func = "log("
        cursor = self.conn.get_cursor("""
select %s f.freq_central), %s rf.wm_f_int), rf.avg_weight_f_int
  from frequencybands f,
       runningcatalog_fluxes rf
 where f.freqbandid = rf.band
   and rf.runcat_id = %s
   and rf.stokes = 'I'""" % (func, func, runcat_id))
        for xdata in iter(cursor.fetchone, None):
            self.freq.append(xdata[0])
            self.flux.append(xdata[1])
            self.flux_err.append(xdata[2])
        cursor.close()
        print self.freq, self.flux
        self.args, sp_power = self.best_fit()
        sp_update = ','.join(map(lambda x: 'spectral_index_%s = %s' %
                                       (x, self.args[x]), range(sp_power)))
        self.conn.execute("""
update runningcatalog
   set spectral_power = %s,
       %s,
       last_update_date = current_timestamp,
       last_spectra_update_date = current_timestamp
 where runcatid = %s;""" % (sp_power, sp_update, runcat_id))

