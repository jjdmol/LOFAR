#!/usr/bin/python
from tests.switchable import SwitchableTest
from tests.testlib import cleanup_db
from src.pipeline import GSMPipeline


class PipelineGeneralTest(SwitchableTest):
    """General test class for pipeline."""

    def setUp(self):
        super(PipelineGeneralTest, self).setUp()
        cleanup_db(self.cm.get_connection(database='test'))
        self.pipeline = GSMPipeline(custom_cm=self.cm, database='test')

    def tearDown(self):
        self.check_datapoints()
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
