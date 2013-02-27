#!/usr/bin/python
from math import sin
from src.sqllist import get_sql
from time import time

from f90_src.gsm_matcher import gsm_matcher as gm
import numpy as np


class Matcher(object):
    """General matching class."""
    def __init__(self, conn, max_assoc, max_dist, max_dist_ext, pix):
        """Store matching parameters."""
        self.conn = conn
        self.max_assoc = sin(max_assoc)
        self.max_dist = max_dist
        self.max_dist_ext = max_dist_ext
        self.pix = pix

    def match(self, image_id):
        """Do the matching (here do nothing)"""
        pass


class MatcherSQL(Matcher):
    """Match via SQL queries"""
    def match(self, image_id):
        self.conn.execute(get_sql('Associate point', self.max_assoc, 
                             self.max_dist, self.pix))
        self.conn.execute_set(get_sql('Associate extended', self.max_assoc, 
                              self.max_dist_ext, self.pix))

    
class MatcherF90(Matcher):
    """Match via external F90+OpenMP call"""
    DEBUG_TIME = False
    
    def load(self, cursor, iid):
        """Load data from the database to F90 module"""
        count = self.conn._get_lastcount(cursor)
        if count > 0:
            ids = np.empty(count, dtype=int)
            coords = np.empty([count, 8], dtype=float)
            types = np.empty(count, dtype=int)
            for i, raw in enumerate(cursor):
                ids[i] = raw[0]
                coords[i] = raw[1:9]
                types[i] = raw[9]
            gm.load_data(iid, np.array(ids), np.array(types), 
                         np.array(coords).transpose())
        cursor.close()
        return count
    
    def write(self, image_id, ids, count, dists, types):
        """Save matches in the database"""
        sql_insert = '''insert into temp_associations(
                        xtrsrc_id, xtrsrc_id2, runcat_id,
                        distance_arcsec, lr_method, r, image_id) values %s'''    
        j = 0
        sql = []
        for i in xrange(count):
            j = j + 1
            sql.append('(%s, %s, %s, %s, %s, %s, %s)' % (
                       ids[0][i], ids[0][i], ids[1][i], 
                       dists[1][i], types[i], dists[1][i], image_id))
            if j == 1000:
                self.conn.execute(sql_insert % ','.join(sql))
                sql = []
                j = 0
        if j > 0:
            self.conn.execute(sql_insert % ','.join(sql))
    
    
    def match(self, image_id):
        """Do the matching"""
        if self.DEBUG_TIME:
            t1 = time()
        sql1 = """select coalesce(xtrsrcid2, xtrsrcid), ra, decl, x, y, z,
                         ra_err, decl_err, g_major, source_kind
                    from extractedsources 
                   where image_id = %s
                     and (xtrsrcid2 is null or source_kind = 0);"""
        c1 = self.load(self.conn.get_cursor(sql1 % image_id), 1)
        if self.DEBUG_TIME:
            print 'LOAD 1', time() - t1
            t1 = time()
        sql1 = """select runcatid, wm_ra, wm_decl, x, y, z,
                         wm_ra_err, wm_decl_err, wm_g_major, 1
                    from runningcatalog rc
                   where rc.healpix_zone in (%s) 
                     and rc.source_kind = 1
                     and rc.band = (select i.band 
                                      from images i where i.imageid = %s)
                     and not rc.deleted
                union
                  select runcatid, wm_ra, wm_decl, x, y, z,
                         wm_ra_err, wm_decl_err, wm_g_major, source_kind*2
                    from runningcatalog rc
                   where rc.healpix_zone in (%s) 
                     and rc.band is null
                     and not rc.deleted;"""
        c2 = self.load(self.conn.get_cursor(
                       sql1 % (self.pix, image_id, self.pix)), 2)
        if self.DEBUG_TIME:
            print 'LOAD 2', time() - t1
            t1 = time()
        if (c1 > 0 and c2 > 0):
            gm.set_params(self.max_assoc, self.max_dist, self.max_dist_ext)
            #gm.MAX_MATCH_DISTANCE = self.max_assoc
            #gm.MAX_MATCH_DERUITER = self.max_dist
            #gm.MAX_MATCH_EXTENDED = self.max_dist_ext
            ids, count, dists, types = gm.do_match(1)
            if self.DEBUG_TIME:
                print 'match', time() - t1
                t1 = time()
            self.write(image_id, ids, count, dists, types)
            if self.DEBUG_TIME:
                print 'write', time() - t1
