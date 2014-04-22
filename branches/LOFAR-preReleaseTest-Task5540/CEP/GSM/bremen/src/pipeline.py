#!/usr/bin/env python
import logging
import math
import os

import monetdb.sql as db

from src.errors import SourceException, ImageStateError
from src.gsmconnectionmanager import GSMConnectionManager
from src.gsmlogger import get_gsm_logger
from src.sqllist import get_sql, get_svn_version, GLOBALS
from src.grouper import Grouper
from src.updater import run_update
from src.utils import get_pixels, load_parameters
from src.matcher import MatcherF90, MatcherSQL
from src.resolveFlux import FluxResolver
from src.resolveQuad import QuadResolver
from src.resolveSimple import SimpleResolver


class GSMPipeline(object):
    """
    General pipeline class.
    """
    def __init__(self, custom_cm=None, use_monet=None,
                 profile=False,
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
            self.conn.commit()
        except db.Error as exc:
            self.log.error("Failed to connect: %s" % exc)
            raise exc
        self.options = load_parameters('%s/settings.ini' % 
                                       os.path.dirname(__file__))
        self.log.debug('Pipeline parameters: %s' % self.options)
        self.log.info('Pipeline started.')

    def reopen_connection(self, **params):
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
        self.conn.start()
        parset.process(self.conn)
        self.parset = parset
        self.process_image(parset.image_id, parset.run_id)
        self.log.info('Parset %s done.' % parset.filename)
        return parset.image_id

    def run_grouper(self):
        """
        Detect/update and store groups of sources for later processing.
        """
        #Update groups by merging overlapping patches.
        cursor = self.conn.get_cursor(get_sql("GroupFinder"))
        grouper = Grouper(cursor.fetchall())
        while grouper.is_completed():
            grouper.one_cycle()
            self.conn.execute_set(get_sql("GroupUpdate",
                                      grouper.group,
                                      ",".join(map(str, grouper.runcatset))))
            grouper.cleanup()
        for resolver in [SimpleResolver]:
            self.run_resolver(resolver)
        self.conn.execute(get_sql("GroupFill"))

    def run_resolver(self, resolve_class):
        #Running resolver
        resolver = resolve_class(self.conn)
        for group_id in self.conn.get_cursor(get_sql("GroupCycle")):
            if not resolver.run_resolve(group_id[0]):
                #Failed to resolve
                self.log.debug("Group id %s not resolved by %s." % 
                                   (group_id[0], resolver.__class__.__name__))
                self.conn.log.debug("Group id %s not resolved." % group_id[0])
                self.conn.execute_set(get_sql("GroupUpdate runcat",
                                      group_id[0]))
            else:
                self.log.debug("Group id %s resolved by %s."  % 
                                   (group_id[0], resolver.__class__.__name__))
                self.conn.log.debug("Group id %s resolved." % group_id[0])

    def update_image_pointing(self, image_id):
        """
        Update image pointing to average ra/decl of all sources.
        """
        avg_x, avg_y, avg_z, count = self.conn.exec_return(
                            get_sql('Image properties selector', image_id),
                                     single_column=False)
        avg_x, avg_y, avg_z = avg_x / count, avg_y / count, avg_z / count
        decl = math.asin(avg_z)
        ra = math.atan2(avg_x, avg_y)
        self.conn.execute(get_sql('Image properties updater',
                                  ra, decl, image_id))

    def process_image(self, image_id, run_id=None, sources_loaded=False):
        """
        Process single image.
        @sources_loaded: True if there are records in the extractedsources
        already.
        """
        self.conn.start()
        status, band, stokes, fov_radius, \
        centr_ra, centr_decl, run_loaded, bmaj = \
        self.conn.exec_return("""
        select status, band, stokes, fov_radius, 
               centr_ra, centr_decl, run_id, bmaj
          from images
         where imageid = %s;""" % image_id, single_column=False)
        if not run_id:
            run_id = run_loaded
        if status == 1:
            raise ImageStateError('Image %s in state 1 (Ok). Cannot process' %
                                  image_id)
        GLOBALS.update({'i': image_id, 'r': run_id,
                        'b': band, 's': stokes})
        if not sources_loaded:
            self.conn.execute(get_sql('insert_extractedsources'))
            self.conn.execute(get_sql('insert dummysources'))
        if bmaj:
            max_assoc = float(bmaj)
        else:
            max_assoc = float(self.options.get('maximum_association_distance'))
        self.log.debug('Using options: %s' % self.options)
        self.log.debug('Final max_assoc_dist %s' % max_assoc)
        
        #Now do the matching!
        if self.options.get('matcher') == 'F90':
            matcher_class = MatcherF90
        else:
            matcher_class = MatcherSQL
        matcher = matcher_class(self.conn, max_assoc, 
                  self.options.get('match_distance'),
                  self.options.get('match_distance_extended'),
                  get_pixels(centr_ra, centr_decl, fov_radius + 0.5))
        matcher.match(image_id)

        self.conn.call_procedure("fill_temp_assoc_kind(%s);" % image_id)
        #Process many-to-many;
        self.run_grouper()

        # Process one-to-one associations;
        self.conn.execute(get_sql('add 1 to 1'))
        #process one-to-many associations;
        self.conn.execute(get_sql('add 1 to N'))
        self.conn.execute_set(get_sql('update flux_fraction'))
        #process many-to-one associations;
        self.conn.execute_set(get_sql('add N to 1'))
        #updating runningcatalog
        run_update(self.conn, 'update runningcatalog')
        run_update(self.conn, 'update runningcatalog extended')
        self.conn.execute(get_sql('update runningcatalog XYZ'))
        #First update, then insert new (!!!)
        run_update(self.conn, 'update runningcatalog_fluxes')
        self.conn.execute(get_sql('insert new bands for point sources'))
        #inserting new sources
        self.conn.execute_set(get_sql('Insert new sources'))
        self.conn.execute_set(get_sql('Join extended'))
        #update image status and save current svn verion.
        self.conn.execute_set(get_sql('Cleanup', get_svn_version()))
        if self.parset.recalculate_pointing:
            self.update_image_pointing(image_id)
        self.conn.commit()
