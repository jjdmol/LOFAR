#!/usr/bin/python
from src.sqllist import GLOBALS


class BasicResolver(object):
    """General resolver class"""
    def __init__(self, conn=None):
        self.conn = conn
        pass
    
    def resolve(self, detections, sources):
        """Template resolve function.
        Returns resolution status and an array of xtrsrcid-runcatid of
        resolved pairs (if possible)."""
        return False, []
    
    def load_detections(self, group_id):
        cursor = self.conn.get_cursor("""
select xtrsrcid, ra, ra_err, decl, decl_err, f_int, f_int_err
  from extractedsources e
 where e.image_id = %s
   and exists (select 1 from temp_associations ta
                where ta.xtrsrc_id2 = e.xtrsrcid
                  and ta.image_id = e.image_id
                  and ta.group_head_id = %s)""" % (GLOBALS['i'], group_id))
        detections = cursor.fetchall()
        cursor.close()
        return detections
    
    def load_sources(self, group_id):
        cursor = self.conn.get_cursor("""
select runcatid, wm_ra, wm_ra_err, wm_decl, wm_decl_err, wm_f_int, wm_f_int_err
  from runningcatalog r,
       runningcatalog_fluxes f,
       images i
 where i.imageid = %s
   and f.band = i.band
   and f.stokes = i.stokes
   and r.runcatid = f.runcat_id
   and exists (select 1 from temp_associations ta
                where ta.runcat_id = r.runcatid
                  and ta.image_id = i.imageid
                  and ta.group_head_id = %s)""" % (GLOBALS['i'], group_id))
        sources = cursor.fetchall()
        cursor.close()
        return sources

    def run_resolve(self, group_id):
        """Get data from Database,
        run resolver,
        saev results to temp_associations"""
        #--Run resolver--
        is_ok, solutions = self.resolve(self.load_detections(group_id),
                                        self.load_sources(group_id))
        if is_ok:
            #"delete" all associations from this group.
            self.conn.execute("""
update temp_associations
   set kind = -1
 where image_id = %s
   and group_head_id = %s;""" % (GLOBALS['i'], group_id))
            #"restore" associations that are "ok"
            for solution in solutions:
                self.conn.execute("""update temp_associations
   set kind = 1,
       group_head_id = null
 where image_id = %s
   and group_head_id = %s
   and xtrsrc_id2 = %s
   and runcat_id = %s;""" % (GLOBALS['i'], group_id,
                             solution[0], solution[1]))
        return is_ok
