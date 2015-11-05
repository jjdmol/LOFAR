#!/usr/bin/env python
import monetdb.sql as db
from src.errors import SourceException
from src.gsmconnectionmanager import GSMConnectionManager
from src.gsmlogger import get_gsm_logger
from src.queries import get_insert_temprunningcatalog, get_inserts_new_sources
from src.sqllist import get_sql
from src.grouper import Grouper


class GSMPipeline(object):
    """
    General pipeline class.
    """
    def __init__(self, **params):
        self.conn_manager = GSMConnectionManager(**params)
        try:
            self.conn = self.conn_manager.get_connection()
        except db.Error as exc:
            print "Failed to connect: %s" % exc
        self.log = get_gsm_logger('pipeline', 'pipeline.log')
        self.log.info('Pipeline started.')

    def read_image(self, source):
        """
        Read image and detections from a given source.
        """
        if source:
            source.read_and_store_data(self.conn)
        else:
            raise SourceException('No source specified.')

    def run_parset(self, parset):
        """
        Process single parset file.
        """
        self.conn.execute("delete from detections;")
        parset.process(self.conn)
        self.process_image(parset.image_id)

    def _update_fluxes(self, runcat_id, band):
        """
        Update flux for a given source in a given band.
        """
        cursor2 = self.conn.get_cursor(get_sql('select flux for update',
                                               runcat_id, band))
        xdata = cursor2.fetchall()
        if xdata:
            xdata = xdata[0]
            self.conn.execute("""
update runningcatalog_fluxes
   set datapoints = {2},
       avg_f_peak = {3}/{2},
       avg_weight_f_peak = {4}/{2}
 where runcat_id = {0}
   and band = {1};""".format(runcat_id, band, xdata[0], xdata[1], xdata[2]))
        else:
            self.conn.execute("""
insert into runningcatalog_fluxes (runcat_id, band, datapoints,
                                   avg_f_peak, avg_weight_f_peak)
values ({0}, {1}, {2}, {3}, {4});""".format(runcat_id, band,
                                        xdata[0], xdata[1], xdata[2]))

    def run_grouper(self):
        """
        Detect/update and store groups of sources for later processing.
        """
        cursor = self.conn.get_cursor(get_sql("GroupFinder"))
        grouper = Grouper(cursor.fetchall())
        while grouper.is_completed():
            grouper.one_cycle()
            self.conn.execute_set(get_sql("GroupUpdate",
                                      grouper.group,
                                      ",".join(map(str, grouper.runcatset))))
            grouper.cleanup()
        self.conn.execute(get_sql("GroupFill"))

    def process_image(self, image_id):
        """
        Process single image.
        """
        self.conn.execute("delete from temp_associations;")
        self.conn.execute(get_sql('insert_extractedsources', image_id))
        self.conn.execute(get_insert_temprunningcatalog(image_id, 1.0))
        self.conn.call_procedure("fill_temp_assoc_kind();")
        # Process one-to-one associations;
        self.conn.execute(get_sql('add 1 to 1'))
        #process one-to-many associations;
        self.conn.execute(get_sql('add 1 to N'))
        #process one-to-many associations;
        self.conn.execute(get_sql('add N to 1'))
        #Process many-to-many;
        self.run_grouper()

        band = self.conn.exec_return("select band from images"\
                                     " where imageid = %s;" % image_id)
        cursor = self.conn.get_cursor(get_sql('update runningcat cursor',
                                              image_id))
        #updating runningcatalog
        for xdata in iter(cursor.fetchone, None):
            self.conn.execute(get_sql('update runningcatalog', xdata[0],
                                      xdata[1], xdata[2], xdata[3], xdata[4]))
            #inserting fluxes for new bands
            cursor2 = self.conn.get_cursor(get_sql('flux cursor',
                                                   xdata[0], band))
            for xsource in iter(cursor2.fetchone, None):
                if not xsource[1]:
                    self.conn.execute(get_sql('insert flux',
                                      band, xsource[0], image_id))
                else:
                    self.conn.execute(get_sql('update flux', xsource[0], band))
        #inserting new sources
        self.conn.execute_set(get_inserts_new_sources(image_id))
