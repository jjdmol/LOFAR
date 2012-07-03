#!/usr/bin/python
import unittest
from src.bbsfilesource import GSMBBSFileSource
from src.gsmparset import GSMParset
from src.pipeline import GSMPipeline
from src.gsmconnectionmanager import GSMConnectionManager
from tests.testlib import cleanup_db
from tests.tempparset import TempParset
from tests.switchable import SwitchableTest


class PipelineTest(SwitchableTest):
    def setUp(self):
        super(PipelineTest, self).setUp()
        cleanup_db(self.cm.get_connection(database='test'))

    def tearDown(self):
        self.check_datapoints()

    def check_datapoints(self):
        pipeline = GSMPipeline(custom_cm=self.cm, database='test')
        pipeline.conn.start()
        cur = pipeline.conn.get_cursor("""
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
        pipeline.conn.commit()

    def test_simple(self):
        pipeline = GSMPipeline(custom_cm=self.cm, database='test')
        parset = GSMParset('tests/pipeline1.parset')
        pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 5)
        res = pipeline.conn.exec_return("select count(*) from runningcatalog_fluxes;")
        self.assertEquals(res, 5)
        parset = GSMParset('tests/image1.parset')
        pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 2)
        res = pipeline.conn.exec_return("select count(*) from runningcatalog_fluxes;")
        self.assertEquals(res, 7)
        res = pipeline.conn.exec_return("select count(*) from runningcatalog where datapoints = 2;")
        self.assertEquals(res, 2)
        parset = GSMParset('tests/image2.parset')
        pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 2)
        res = pipeline.conn.exec_return("select count(*) from runningcatalog where datapoints = 1;")
        self.assertEquals(res, 5)

    def test_1_to_N(self):
        pipeline = GSMPipeline(custom_cm=self.cm, database='test')
        parset = GSMParset('tests/pipeline1.parset')
        pipeline.run_parset(parset)
        parset = GSMParset('tests/image3.parset')
        pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 1)
        res = pipeline.conn.exec_return("select count(*) from runningcatalog where datapoints = 1;")
        self.assertEquals(res, 4)
        res = pipeline.conn.exec_return("select count(*) from runningcatalog where datapoints = 2;")
        self.assertEquals(res, 1)


    def test_N_to_1(self):
        pipeline = GSMPipeline(custom_cm=self.cm, database='test')
        parset = GSMParset('tests/pipeline1.parset')
        pipeline.run_parset(parset)
        parset = GSMParset('tests/image4.parset')
        pipeline.run_parset(parset)
        self.assertEquals(parset.source_count, 2)
        res = pipeline.conn.exec_return("""
select count(*)
  from assocxtrsources a, extractedsources e
 where e.image_id = %s and a.xtrsrc_id = e.xtrsrcid;""" % parset.image_id)
        self.assertEquals(res, 2)
        res = pipeline.conn.exec_return(
            "select count(*)  from runningcatalog;")
        self.assertEquals(res, 6)


    def test_N_to_N(self):
        pipeline = GSMPipeline(custom_cm=self.cm, database='test')
        parset = TempParset('data/field_multy.dat', '150000000')
        pipeline.run_parset(parset)
        parset1 = TempParset('data/image4.dat', '160000000')
        pipeline.run_parset(parset1)
        parset1 = TempParset('data/image5.dat', '160000000')
        pipeline.run_parset(parset1)

    def test_N_to_N_more(self):
        pipeline = GSMPipeline(custom_cm=self.cm, database='test')
        parset = TempParset('data/field_multy2.dat', '150000000')
        pipeline.run_parset(parset)
        parset1 = TempParset('data/image4.dat', '160000000')
        pipeline.run_parset(parset1)
        parset2 = TempParset('data/image6.dat', '160000000')
        pipeline.run_parset(parset2)


    #def tearDown(self):
    #    cleanup_db(GSMConnectionManager(database='test').get_connection())
