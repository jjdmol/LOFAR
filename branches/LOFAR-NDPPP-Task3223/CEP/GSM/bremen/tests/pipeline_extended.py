#!/usr/bin/python
import unittest
from src.bbsfilesource import GSMBBSFileSource
from src.gsmparset import GSMParset
from src.pipeline import GSMPipeline
from src.gsmconnectionmanager import GSMConnectionManager
from tests.testlib import cleanup_db, get_frequency
from tests.tempparset import TempParset
from tests.switchable import SwitchableTest


class PipelineExtendedTest(SwitchableTest):
    def setUp(self):
        super(PipelineExtendedTest, self).setUp()
        cleanup_db(self.cm.get_connection(database='test'))
        self.pipeline = GSMPipeline(custom_cm=self.cm, database='test')

    def tearDown(self):
        self.check_datapoints()
        self.pipeline.conn.commit()

    def run_series_part(self, x, band='150000000'):
        parset = TempParset('data_extended/series%s.dat' % x, band)
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 1)

    def check_counts(self, fluxes, base, bands, points=None):
        self.pipeline.conn.start()
        res = self.pipeline.conn.exec_return("select count(*) from runningcatalog_fluxes;")
        self.assertEquals(res, fluxes, "Fluxes %s != %s" % (res, fluxes))
        res = self.pipeline.conn.exec_return("select count(*) from runningcatalog where band is null and source_kind = 1 and not deleted;")
        self.assertEquals(res, base, "Cross-band %s != %s" % (res, base))
        res = self.pipeline.conn.exec_return("select count(*) from runningcatalog where band is not null;")
        self.assertEquals(res, bands, "Per-band  %s != %s" % (res, bands))
        if points:
            res = self.pipeline.conn.exec_return("select count(*) from runningcatalog where source_kind = 0;")
            self.assertEquals(res, points, "Points")
        self.pipeline.conn.commit()

    def check_datapoints(self):
        self.pipeline.conn.start()
        cur = self.pipeline.conn.get_cursor("""
select parent_id,
       max(datapoints),
       count(distinct xtrsrc_id),
       sum(per_band_datapoints),
       sum(flux_datapoints)
  from v_catalog_info
group by parent_id""")
        for items in cur.fetchall():
            self.assertEquals(items[1], items[2], "Datapoints - objects")
            self.assertEquals(items[3], items[4], "Per-band datapoints - flux datapoints")
        cur.close()
        self.pipeline.conn.commit()

    def test_simple(self):
        parset = TempParset('data_extended/field_ext.dat', '150000000')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 2)
        self.check_counts(2, 2, 2)

    def test_update(self):
        parset = TempParset('data_extended/field_ext.dat', '150000000')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 2)
        parset = TempParset('data_extended/image0.dat', '170000000')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 3)
        self.check_counts(5, 3, 5)
        parset = TempParset('data_extended/image1.dat', '170000000')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 4)
        self.check_counts(6, 3, 5, 1)

    def test_1_to_N_classic(self):
        parset = TempParset('data_extended/field_ext_mult.dat', '150000000')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 2)
        parset = TempParset('data_extended/image0_mult.dat', '150000000')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 1)
        self.check_counts(3, 3, 3)
        parset = TempParset('data_extended/image1_mult.dat', '170000000')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 1)
        self.check_counts(4, 3, 4)
        self.check_datapoints()
        parset = TempParset('data_extended/image1_mult.dat', '170000000')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 1)
        self.check_counts(4, 3, 4)

    def test_series(self):
        for x in xrange(1,8):
            self.run_series_part(x, get_frequency(x))
        self.check_counts(7, 1, 7)

    def test_series2(self):
        self.run_series_part(1)
        self.run_series_part(7)
        self.check_counts(2, 2, 2)

    def test_join(self):
        self.run_series_part(2)
        self.run_series_part(3)
        self.check_datapoints()
        self.run_series_part(6, '160000000')
        self.run_series_part(4, '170000000')
        self.check_counts(3, 1, 3)

    def test_N_to_1(self):
        parset = TempParset('data_extended/field_ext.dat', '150000000')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 2)
        parset = TempParset('data_extended/image4.dat', '150000000')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 4)
        self.check_counts(4, 2, 4)

    def test_N_to_1_cross_band(self):
        parset = TempParset('data_extended/field_ext.dat', '150000000')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 2)
        parset = TempParset('data_extended/image4.dat', '160000000')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 4)
        self.check_counts(6, 2, 6)

