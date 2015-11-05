#!/usr/bin/python
"""
General query generator for GSM.
"""
import math


def _get_distance(runcat_alias, extract_alias):
    """
    Create a query part for association distance.

    @param runcat_alias: alias for runningcatalog;
    @param extract_alias: alias for extractedsources;
    """
    return """\
3600*DEGREES(2 * ASIN(SQRT(({0}.x - {1}.x) * ({0}.x - {1}.x)
                         + ({0}.y - {1}.y) * ({0}.y - {1}.y)
                         + ({0}.z - {1}.z) * ({0}.z - {1}.z)*0.5)))
""".format(runcat_alias, extract_alias)


def _get_assoc_r(runcat_alias, extract_alias):
    """
    Create a query part for association distance.

    @param runcat_alias: alias for runningcatalog;
    @param extract_alias: alias for extractedsources;
    """
    return """\
3600*sqrt((({0}.wm_ra * cos(RADIANS({0}.wm_decl)) - {1}.ra * cos(RADIANS({1}.decl)))
          *({0}.wm_ra * cos(RADIANS({0}.wm_decl)) - {1}.ra * cos(RADIANS({1}.decl)))
          )/ ({0}.wm_ra_err * {0}.wm_ra_err + {1}.ra_err * {1}.ra_err)
        + (({0}.wm_decl - {1}.decl) * ({0}.wm_decl - {1}.decl))
         / ({0}.wm_decl_err * {0}.wm_decl_err + {1}.decl_err * {1}.decl_err))
""".format(runcat_alias, extract_alias)


def _get_column_update(column_alias, new_value, new_weight):
    return """wm_{0} = (avg_w{0} + {2})/(avg_weight_{0} + {1}),
wm_{0}_err = sqrt(1.0/(avg_weight_{0} + {1})),
avg_w{0} = avg_w{0} + {2},
avg_weight_{0} = avg_weight_{0} + {1}
    """.format(column_alias, new_value, new_weight)


def get_field(ra, decl, radius, band, min_flux=None):
    """
    Create a query to get sources for a given fov in a given band.
    """
    decl = math.radians(decl)
    ra = math.radians(ra)
    x = math.cos(decl) * math.cos(ra)
    y = math.cos(decl) * math.sin(ra)
    z = math.sin(decl)
    r = math.cos(math.radians(radius))
    sql = """select r.wm_ra as ra, r.wm_decl as decl, f.avg_f_peak
  from runningcatalog r,
       runningcatalog_fluxes f
 where r.x * {0} + r.y * {1} + r.z * {2} > {3}
   and r.x between {0} - {3} and {0} + {3}
   and r.y between {1} - {3} and {1} + {3}
   and r.z between {2} - {3} and {2} + {3}
   and f.runcat_id = r.runcatid
   and f.stokes = "I"
   and f.band = {4}""".format(x, y, z, r, band)
    if min_flux:
        sql = "%s\n and f.avg_f_peak > %s" % (sql, min_flux)
    return sql


def get_insert_image(parset_id, frequency):
    """
    Insert image and return image_id.
    """
    return ["""
insert into images (ds_id, tau, band, imagename, centr_ra, centr_decl)
select 0, 1, freqbandid, \'{0}\' as imagename, 0.0, 0.0
  from frequencybands
 where freq_low < {1} and freq_high > {1};""".format(parset_id, frequency),
"""select max(imageid) from images;"""]


def get_insert_temprunningcatalog(image_id, deRuiter_r):
    """Select matched sources

    Here we select the extractedsources that have a positional match
    with the sources in the running catalogue table (runningcatalog)
    and those who have will be inserted into the temporary running
    catalogue table (temprunningcatalog).
    """
    sql = """\
INSERT INTO temp_associations (xtrsrc_id, runcat_id,
                               distance_arcsec,
                               lr_method, r, group_head_id)
SELECT x0.xtrsrcid, rc.runcatid,"""\
+ _get_distance('rc', 'x0') + """ AS assoc_distance_arcsec, 1, """\
+ _get_assoc_r('rc', 'x0') + """ as assoc_r,
       rc.group_head_id
  FROM runningcatalog rc
      ,extractedsources x0
      ,images im0
 WHERE x0.image_id = {0}
   AND x0.image_id = im0.imageid
   AND rc.decl_zone BETWEEN CAST(FLOOR(x0.decl - {1}) as INTEGER)
                        AND CAST(FLOOR(x0.decl + {1}) as INTEGER)
   AND rc.wm_decl BETWEEN x0.decl - {1}
                      AND x0.decl + {1}
   AND rc.wm_ra BETWEEN x0.ra - alpha({1},x0.decl)
                    AND x0.ra + alpha({1},x0.decl)
 AND """ + _get_assoc_r('rc', 'x0') + """ < {2}"""
    return sql.format(image_id, 0.025, deRuiter_r)


def get_inserts_new_sources(image_id):
    return """
insert into runningcatalog(first_xtrsrc_id, datapoints, decl_zone,
                           wm_ra, wm_decl, wm_ra_err, wm_decl_err,
                           x, y, z,
                           avg_wra, avg_wdecl, avg_weight_ra, avg_weight_decl)
select e.xtrsrcid, 1, zone, ra, decl, ra_err, decl_err, x, y, z,
       ra/(ra_err*ra_err), decl/(decl_err*decl_err), 1/(ra_err*ra_err), 1/(decl_err*decl_err)
  from extractedsources e
 where image_id = {0}
   and not exists (select x.xtrsrc_id
                     from temp_associations x
                    where x.xtrsrc_id = e.xtrsrcid);

insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method,
                            lr, r)
select r.first_xtrsrc_id, r.runcatid, 0.0, 0, 0.0, 0.0
  from runningcatalog r,
       extractedsources e
 where e.image_id = {0}
   and e.xtrsrcid = r.first_xtrsrc_id
   and not exists (select x.xtrsrc_id
                     from temp_associations x
                    where x.xtrsrc_id = e.xtrsrcid);

insert into runningcatalog_fluxes(runcat_id, band, datapoints, avg_f_peak, avg_weight_f_peak)
select r.runcatid, i.band, 1, e.f_peak, 1/(e.f_peak_err*e.f_peak_err)
  from extractedsources e,
       images i,
       runningcatalog r
 where e.image_id = {0}
   and i.imageid = e.image_id
   and r.first_xtrsrc_id = e.xtrsrcid
   and not exists (select x.xtrsrc_id
                     from temp_associations x
                    where x.xtrsrc_id = e.xtrsrcid);""".format(image_id)
