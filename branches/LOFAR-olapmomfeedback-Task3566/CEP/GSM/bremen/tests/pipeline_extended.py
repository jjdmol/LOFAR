#!/usr/bin/python
import unittest
from src.bbsfilesource import GSMBBSFileSource
from src.gsmparset import GSMParset
from src.pipeline import GSMPipeline
from src.gsmconnectionmanager import GSMConnectionManager
from tests.testlib import cleanup_db, get_frequency
from tests.tempparset import TempParset
from tests.pipelinegeneral import PipelineGeneralTest

class PipelineExtendedTest(PipelineGeneralTest):
    PARSET_EXTRAS = {'pointing_ra': 0.0,
                     'pointing_decl': 0.0,
                     'beam_size': 1.0 }
    def run_series_part(self, x, band='150000000'):
        parset = TempParset('data_extended/series%s.dat' % x, band, **self.PARSET_EXTRAS)
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 1)

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

