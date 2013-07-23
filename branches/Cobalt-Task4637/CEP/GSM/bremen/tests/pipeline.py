#!/usr/bin/python
import unittest
from src.bbsfilesource import GSMBBSFileSource
from src.gsmparset import GSMParset
from src.gsmconnectionmanager import GSMConnectionManager
from tests.tempparset import TempParset
from tests.pipelinegeneral import PipelineGeneralTest


class PipelineTest(PipelineGeneralTest):
    def test_simple(self):
        parset = GSMParset('tests/pipeline1.parset')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 5)
        res = self.pipeline.conn.exec_return("select count(*) from runningcatalog_fluxes;")
        self.assertEquals(res, 5)
        parset = GSMParset('tests/image1.parset')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 2)
        res = self.pipeline.conn.exec_return("select count(*) from runningcatalog_fluxes;")
        self.assertEquals(res, 7)
        res = self.pipeline.conn.exec_return("select count(*) from runningcatalog where datapoints = 2;")
        self.assertEquals(res, 2)
        parset = GSMParset('tests/image2.parset')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 2)
        res = self.pipeline.conn.exec_return("select count(*) from runningcatalog where datapoints = 1;")
        self.assertEquals(res, 5)

    def test_1_to_N(self):
        parset = GSMParset('tests/pipeline1.parset')
        self.pipeline.run_parset(parset)
        parset = GSMParset('tests/image3.parset')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 1)
        res = self.pipeline.conn.exec_return("select count(*) from runningcatalog where datapoints = 1;")
        self.assertEquals(res, 4)
        res = self.pipeline.conn.exec_return("select count(*) from runningcatalog where datapoints = 2;")
        self.assertEquals(res, 1)


    def test_N_to_1(self):
        parset = GSMParset('tests/pipeline1.parset')
        self.pipeline.run_parset(parset)
        parset = GSMParset('tests/image4.parset')
        self.pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 2)
        res = self.pipeline.conn.exec_return("""
select count(*)
  from assocxtrsources a, extractedsources e
 where e.image_id = %s and a.xtrsrc_id = e.xtrsrcid;""" % parset.image_id)
        self.assertEquals(res, 2)
        res = self.pipeline.conn.exec_return(
            "select count(*)  from runningcatalog;")
        self.assertEquals(res, 6)


    def test_N_to_N(self):
        parset = TempParset('data/field_multy.dat', '160000000')
        self.pipeline.run_parset(parset)
        parset1 = TempParset('data/image4.dat', '160000000')
        self.pipeline.run_parset(parset1)
        parset1 = TempParset('data/image5.dat', '160000000')
        self.pipeline.run_parset(parset1)
        res = self.pipeline.conn.exec_return("""
select count(*)
  from runningcatalog
 where group_head_id > 0;""")
        self.assertEquals(res, 2)


    def test_N_to_N_more(self):
        parset = TempParset('data/field_multy2.dat', '150000000')
        self.pipeline.run_parset(parset)
        parset1 = TempParset('data/image4.dat', '160000000')
        self.pipeline.run_parset(parset1)
        parset2 = TempParset('data/image6.dat', '160000000')
        self.pipeline.run_parset(parset2)
