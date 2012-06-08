#!/usr/bin/python
"""
General query generator for GSM.
"""
import math


def get_distance(runcat_alias, extract_alias):
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


def get_assoc_r(runcat_alias, extract_alias):
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


def get_column_insert(column_alias):
    return """
wm_{0}, wm_{0}_err, avg_w{0}, avg_weight_{0}""".format(column_alias)


def get_column_insert_values(column_alias):
    return """
{0}, {0}_err, {0}/({0}_err*{0}_err), 1/({0}_err*{0}_err)""".format(column_alias)


def get_column_insert_list(vlist):
    return ','.join(get_column_insert(col) for col in vlist)

def get_column_insert_values_list(vlist):
    return ','.join(get_column_insert_values(col) for col in vlist)


def get_column_update(column_alias, new_value, new_weight):
    """
    Updater for error-columns for single item ipdate.
    """
    return """wm_{0} = (avg_w{0} + {2})/(avg_weight_{0} + {1}),
wm_{0}_err = sqrt(1.0/(avg_weight_{0} + {1})),
avg_w{0} = avg_w{0} + {2},
avg_weight_{0} = avg_weight_{0} + {1}""".format(column_alias,
                                                new_value, new_weight)


def get_column_update_pg(column_alias, new_value, new_weight):
    """
    Updater for error columns for Postgres Update-from queries.
    """
    return """
wm_{0} = (avg_w{0} + {1}/({2}*{2}))/(avg_weight_{0} + 1/({2}*{2})),
wm_{0}_err = sqrt(1.0/(avg_weight_{0} + 1/({2}*{2}))),
avg_w{0} = avg_w{0} + {1}/({2}*{2}),
avg_weight_{0} = avg_weight_{0} + 1/({2}*{2})""".format(column_alias,
                                                        new_value, new_weight)


def get_field(ra, decl, radius, band, min_flux=None):
    """
    Create a query to get sources for a given fov in a given band.
    """
    decl = math.radians(decl)
    ra = math.radians(ra)
    x = math.cos(decl) * math.cos(ra)
    y = math.cos(decl) * math.sin(ra)
    z = math.sin(decl)
    r = math.sin(math.radians(radius))
    sql = """select r.wm_ra as ra, r.wm_decl as decl, f.wm_f_peak
  from runningcatalog r,
       runningcatalog_fluxes f
 where r.x * {0} + r.y * {1} + r.z * {2} > {3}
   and r.x between {0} - {3} and {0} + {3}
   and r.y between {1} - {3} and {1} + {3}
   and r.z between {2} - {3} and {2} + {3}
   and f.runcat_id = r.runcatid
   and f.stokes = 'I'
   and f.band = {4}""".format(x, y, z, r, band)
    if min_flux:
        sql = "%s\n and f.wm_f_peak > %s" % (sql, min_flux)
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
SELECT e.xtrsrcid, rc.runcatid,"""\
+ get_distance('rc', 'e') + """ AS assoc_distance_arcsec, 1, """\
+ get_assoc_r('rc', 'e') + """ as assoc_r,
       rc.group_head_id
  FROM runningcatalog rc
      ,extractedsources e
      ,images im0
 WHERE e.image_id = {0}
   AND e.image_id = im0.imageid
   and rc.x between e.x - {1} and e.x + {1}
   and rc.y between e.y - {1} and e.y + {1}
   and rc.z between e.z - {1} and e.z + {1}
   and e.source_kind = 0
   and rc.source_kind = 0
 AND """ + get_assoc_r('rc', 'e') + """ < {2}"""
    return sql.format(image_id, math.sin(0.025), deRuiter_r)

def get_insert_temprunningcatalog_extended(image_id, deRuiter_r):
    """
    So far UNUSED!.
    """
    sql = """\
INSERT INTO temp_associations (xtrsrc_id, runcat_id,
                               distance_arcsec,
                               lr_method, r, group_head_id)
SELECT e.xtrsrcid, rc.runcatid,"""\
+ get_distance('rc', 'e') + """ AS assoc_distance_arcsec, 2, """\
+ get_assoc_r('rc', 'e') + """ as assoc_r,
       rc.group_head_id
  FROM runningcatalog rc
      ,extractedsources e
      ,images im0
 WHERE e.image_id = {0}
   AND e.image_id = im0.imageid
   and rc.x between e.x - {1} and e.x + {1}
   and rc.y between e.y - {1} and e.y + {1}
   and rc.z between e.z - {1} and e.z + {1}
   and e.source_kind = 1
   and rc.source_kind = 1
   and rc.band = im0.band
   and rc.stokes = im0.stokes
 AND """ + get_assoc_r('rc', 'e') + """ < {2};

"""
    return sql.format(image_id, math.sin(0.025), deRuiter_r)
