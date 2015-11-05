#!/usr/bin/python
from src.sqllist import get_sql
from src.updater import run_update
from src.pipeline import GSMPipeline


class Reprocessor(GSMPipeline):
    """
    Reprocessing pipeline.
    """

    def remove_image(self, image_id, delete_observations=False):
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
        if delete_observations:
            self.conn.execute(get_sql('deduct remove extractedsources',
                                      image_id))
            image_status = 99
        else:
            image_status = 2
        self.conn.execute("""
update images
   set status = %s,
       process_date = current_timestamp
 where imageid = %s""" % (image_status, image_id))
        self.conn.commit()

    def reprocess_image(self, image_id):
        """
        Remove old and insert new data.
        Do not reload the data and do not touch extractedsources.
        """
        self.remove_image(image_id)
        self.process_image(image_id, sources_loaded=True)
        self.conn.execute("""
update images
   set reprocessing = reprocessing + 1
 where imageid = %s""" % image_id)
        self.conn.commit()

    def full_reprocess_image(self, image_id, new_parset):
        """
        Remove old and insert new data.
        Reload the data to the extractedsources.
        New image_id will be created with the new parset,
        with the old image switched to status=99.
        """
        self.remove_image(image_id, delete_observations=True)
        self.run_parset(new_parset)
        self.conn.execute("""
update images
   set reprocessing = (select reprocessing
                         from images
                        where imageid = %s) + 1
 where imageid = %s""" % (image_id, new_parset.image_id))
        return new_parset.image_id
