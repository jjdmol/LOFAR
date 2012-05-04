#!/usr/bin/python
import unittest
import shutil
from tests.testlib import load_from_csv_file, cleanup_db
from src.pipeline import GSMPipeline

class MatchingTest(unittest.TestCase):
    """
    Testing the matching algorithm.
    """
    def setUp(self):
        self.pipeline = GSMPipeline()

    def sql_between(self, field):
        return "b.wm_%s + b.wm_%s_err > e.%s - e.%s_err " \
            "and b.wm_%s - b.wm_%s_err < e.%s + e.%s_err" % tuple(8*[field])

    def notest_match(self):
        #load_from_csv_file(self.pipeline.conn, 'data/gsm_base.dat', 'gsm_base')
        #load_from_csv_file(self.pipeline.conn, 'data/extracted.dat', 'extractedsources')
        cursor = self.pipeline.conn.cursor()
        cursor.execute("select count(*) as adata from extractedsources e " \
                       "join runningcatalog b on ( %s and %s);" %
                         (self.sql_between('ra'), self.sql_between('decl')))
        result = cursor.fetchone()
        cursor.close()
        self.assertEquals((13,),result)

    #def tearDown(self):
    #    cleanup_db(self.pipeline.conn)
