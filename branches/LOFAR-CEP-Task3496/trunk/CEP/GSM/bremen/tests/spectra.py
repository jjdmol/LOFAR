#!/usr/bin/python
import unittest
from math import log10, pow
from numpy.polynomial.polynomial import polyval
from numpy.testing import assert_array_almost_equal
from stress.generator import FREQUENCY
from src.gsmconnectionmanager import GSMConnectionManager
from tests.testlib import cleanup_db
from src.spectra import Spectra

class SpectraTest(unittest.TestCase):
    def setUp(self):
        self.conn = GSMConnectionManager(use_console=False,
                                         use_monet=False).get_connection(database='test')
        cleanup_db(self.conn)
        self.sp = Spectra(self.conn)

    def tearDown(self):
        self.conn.commit()

    def insert_data(self, params, bands=8):
        self.conn.execute("""
insert into runningcatalog (runcatid, first_xtrsrc_id, datapoints,
wm_ra, wm_ra_err, wm_decl, wm_decl_err, x, y, z)
values (100, 1, 1, 1, 0.1, 1, 0.1, 1, 1, 1);""")
        for band in xrange(1, bands+1):
            flux = pow(10, polyval(log10(FREQUENCY[band]), params))
            self.conn.execute("""
insert into runningcatalog_fluxes(runcat_id, band, datapoints, wm_f_int, avg_weight_f_int)
values(100, %s, 1, %s, 1)""" % (band, flux))

    def test_spectra(self):
        args = [1.0, 0.01, 0.001, 0.0001]
        self.insert_data(args)
        fit = self.sp.get_sql_data(100)
        assert_array_almost_equal(fit, args)

    def test_spectra2(self):
        args = [1.0, 0.01, 0.001]
        self.insert_data(args)
        fit = self.sp.get_sql_data(100)
        assert_array_almost_equal(fit, args)
