#!/usr/bin/python
import monetdb.sql as db
from src.errors import SourceException
from src.gsmconnectionmanager import GSMConnectionManager
from src.gsmlogger import get_gsm_logger
from src.sqllist import get_sql
from src.grouper import Grouper
from src.updater import run_update
from src.pipeline import GSMPipeline
import logging
import math

class Reprocessor(GSMPipeline):
    """
    Reprocessing pipeline.
    """

    def remove_image(self, image_id):
        """
        Remove all data from runningcatalog/runningcatalog_fluxes.
        """
        self.log.info('removing data from image %s' % image_id)
        self.conn.start()
        for sql in ['deduct runningcatalog',
                    'deduct runningcatalog non-zero',
                    'deduct runningcatalog extended',
                    'deduct runningcatalog extended non-zero',
                    'deduct runningcatalog_fluxes',
                    'deduct runningcatalog_fluxes non-zero']:
            run_update(self.conn, sql, image_id)
        self.conn.execute(get_sql('deduct cleanup', image_id))
        self.conn.execute(get_sql('update runningcatalog XYZ', image_id))
        self.conn.execute("""
update images
   set status = 2,
       process_date = current_timestamp
 where imageid = %s""" % image_id)
        self.conn.commit()

    def reprocess_image(self, image_id):
        """
        Remove old and insert new data.
        """
        self.remove_image(image_id)
        self.process_image(image_id, sources_loaded=True)
        self.conn.execute("""
update images
   set reprocessing = reprocessing + 1
 where imageid = %s""" % image_id)
        self.conn.commit()
