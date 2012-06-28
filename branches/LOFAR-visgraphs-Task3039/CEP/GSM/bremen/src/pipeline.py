#!/usr/bin/env python
import monetdb.sql as db
from src.errors import SourceException
from src.gsmconnectionmanager import GSMConnectionManager
from src.gsmlogger import get_gsm_logger
from src.queries import get_insert_temprunningcatalog
from src.sqllist import get_sql
from src.grouper import Grouper
from src.updater import run_update
import logging
import math


class GSMPipeline(object):
    """
    General pipeline class.
    """
    def __init__(self, custom_cm=None, use_monet=None, profile=False,
                 **params):
        """
        @param custom_cm: allows to pass an object to be used as connection
        manager.
        """
        self.log = get_gsm_logger('pipeline', 'pipeline.log')
        self.use_monet = use_monet
        if not custom_cm:
            if use_monet != None:
                self.conn_manager = GSMConnectionManager(use_monet=use_monet)
            else:
                self.conn_manager = GSMConnectionManager()
        else:
            self.conn_manager = custom_cm
        try:
            self.conn = self.conn_manager.get_connection(**params)
            if profile:
                self.conn.profile = True
                self.conn.log.setLevel(logging.DEBUG)
        except db.Error as exc:
            self.log.error("Failed to connect: %s" % exc)
            raise exc
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
        self.log.info('Parset %s done.' % parset.filename)

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
        self.conn.start()
        self.conn.execute("delete from temp_associations;")
        self.conn.execute(get_sql('insert_extractedsources', image_id))
        self.conn.execute(get_insert_temprunningcatalog(image_id, 1.0))
        self.conn.execute_set(get_sql('Associate extended',
                                      image_id, math.sin(0.025), 1.0))
        self.conn.call_procedure("fill_temp_assoc_kind();")
        # Process one-to-one associations;
        self.conn.execute(get_sql('add 1 to 1'))
        #process one-to-many associations;
        self.conn.execute(get_sql('add 1 to N'))
        #process one-to-many associations;
        self.conn.execute_set(get_sql('add N to 1'))
        #Process many-to-many;
        self.run_grouper()

        band = self.conn.exec_return("select band from images"\
                                     " where imageid = %s;" % image_id)
        #updating runningcatalog
        run_update(self.conn, 'PG update runningcatalog', image_id)
        #First update, then insert new (!!!)
        run_update(self.conn, 'PG update runningcatalog_fluxes',
                                  image_id, band)
        self.conn.execute(get_sql('PG insert new bands', image_id, band))
        #inserting new sources
        self.conn.execute_set(get_sql('Insert new sources', image_id))
        self.conn.commit()
