#!/usr/bin/env python
import monetdb.sql as db
from src.errors import SourceException, ImageStateError
from src.gsmconnectionmanager import GSMConnectionManager
from src.gsmlogger import get_gsm_logger
from src.sqllist import get_sql, get_svn_version
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

    def reopen_connection(self):
        """
        Reopen connection in case it was closed.
        """
        if not self.conn or not self.conn.established():
            try:
                self.conn = self.conn_manager.get_connection(**params)
                self.log.info('Pipeline connection reopened.')
            except db.Error as exc:
                self.log.error("Failed to connect: %s" % exc)
                raise exc

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

    def process_image(self, image_id, sources_loaded=False):
        """
        Process single image.
        @sources_loaded: True if there are records in the extractedsources
        already.
        """
        self.conn.start()
        status, band, stokes = self.conn.exec_return("""
        select status, band, stokes
          from images
         where imageid = %s;""" % image_id, single_column=False)
        if status == 1:
            raise ImageStateError('Image %s in state 1 (Ok). Cannot process' %
                                  image_id)
        self.conn.execute("delete from temp_associations;")
        if not sources_loaded:
            self.conn.execute(get_sql('insert_extractedsources', image_id))
            self.conn.execute(get_sql('insert dummysources', image_id))
        self.conn.execute(get_sql('Associate point',
                                      image_id, math.sin(0.025), 1.0))
        self.conn.execute_set(get_sql('Associate extended',
                                      image_id, math.sin(0.025), 0.5))
        self.conn.call_procedure("fill_temp_assoc_kind();")
        # Process one-to-one associations;
        self.conn.execute(get_sql('add 1 to 1'))
        #process one-to-many associations;
        self.conn.execute(get_sql('add 1 to N'))
        self.conn.execute_set(get_sql('update flux_fraction'))
        #process many-to-one associations;
        self.conn.execute_set(get_sql('add N to 1', band))
        #Process many-to-many;
        self.run_grouper()

        #updating runningcatalog
        run_update(self.conn, 'update runningcatalog', image_id)
        run_update(self.conn, 'update runningcatalog extended', image_id)
        self.conn.execute(get_sql('update runningcatalog XYZ', image_id))
        #First update, then insert new (!!!)
        run_update(self.conn, 'update runningcatalog_fluxes',
                             image_id)
        self.conn.execute(get_sql('insert new bands for point sources',
                                  image_id, band))
        #inserting new sources
        self.conn.execute_set(get_sql('Insert new sources', image_id))
        self.conn.execute_set(get_sql('Join extended', image_id))
        #update image status and save current svn verion.
        self.conn.execute("""
update images
   set status = 1,
       process_date = current_timestamp,
       svn_version = %s
 where imageid = %s""" % (get_svn_version(), image_id))
        self.conn.commit()
