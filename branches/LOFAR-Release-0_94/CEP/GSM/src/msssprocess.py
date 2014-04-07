# -*- coding: utf-8 -*-

#
# LOFAR Transients Key Project
#
# Bart Scheers, Evert Rol
#
# discovery@transientskp.org
#

import sys
import math
import logging
import monetdb.sql as db
from tkp.config import config
# To do: any way we can get rid of this dependency?
from tkp.sourcefinder.extract import Detection
from .database import ENGINE

AUTOCOMMIT = config['database']['autocommit']
DERUITER_R = config['source_association']['deruiter_radius']
BG_DENSITY = config['source_association']['bg-density']


def insert_dataset(conn, description):
    """Insert dataset with discription as given by argument.

    DB function insertDataset() sets the necessary default values.
    """

    newdsid = None
    try:
        cursor = conn.cursor()
        query = """\
        SELECT insertDataset(%s)
        """
        cursor.execute(query, (description,))
        newdsid = cursor.fetchone()[0]
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query: %s." % query)
        raise
    finally:
        conn.cursor().close()
    return newdsid


def insert_image(conn, dsid, freq_eff, freq_bw, taustart_ts, 
                       beam_bmaj, beam_bmin, beam_bpa,
                       centr_ra, centr_decl, url):
    """Insert an image for a given dataset with the column values
    given in the argument list.
    """

    newimgid = None
    try:
        cursor = conn.cursor()
        query = """\
        SELECT insertImage(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
        """
        cursor.execute(query, (dsid
                              ,freq_eff
                              ,freq_bw
                              ,taustart_ts
                              ,beam_bmaj
                              ,beam_bmin
                              ,beam_bpa
                              ,centr_ra
                              ,centr_decl
                              ,url
                              ))
        newimgid = cursor.fetchone()[0]
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query: %s." % query)
        raise
    finally:
        cursor.close()
    return newimgid


def load_LSM(conn, ira_min, ira_max, idecl_min, idecl_max, cat1="NVSS", cat2="VLSS", cat3="WENSS"):
    #raise NotImplementedError

    try:
        cursor = conn.cursor()
        query = """\
        CALL LoadLSM(%s, %s, %s, %s, %s, %s, %s)
        /*CALL LoadLSM(47, 59, 50, 58, 'NVSS', 'VLSS', 'WENSS')*/
        """
        cursor.execute(query, (ira_min,ira_max,idecl_min,idecl_max,cat1,cat2,cat3))
        #cursor.execute(query)
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed to insert lsm by procedure LoadLSM: %s" % e)
        raise
    finally:
        cursor.close()


# The following set of functions are private to the module
# these are called by insert_extracted_sources(), and should
# only be used that way
def _empty_detections(conn):
    """Empty the detections table

    Initialize the detections table by
    deleting all entries.

    It is used at the beginning and the end
    when detected sources are inserted.
    """

    try:
        cursor = conn.cursor()
        query = """\
        DELETE FROM detections
        """
        cursor.execute(query)
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _insert_into_detections(conn, results):
    """Insert all detections

    Insert all detections, as they are,
    straight into the detection table.

    """

    # TODO: COPY INTO is faster.
    if not results:
        return
    try:
        query = [str(det.serialize()) if isinstance(det, Detection) else
                 str(tuple(det)) for det in results]
        query = "INSERT INTO detections VALUES " + ",".join(query)
        conn.cursor().execute(query)
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        conn.cursor().close()


def _insert_extractedsources(conn, image_id):
    """Insert all extracted sources with their properties

    Insert all detected sources and some derived properties into the
    extractedsources table.

    """

    cursor = conn.cursor()
    try:
        query = """\
        INSERT INTO extractedsources
          (image_id
          ,zone
          ,ra
          ,decl
          ,ra_err
          ,decl_err
          ,x
          ,y
          ,z
          ,det_sigma
          ,I_peak
          ,I_peak_err
          ,I_int
          ,I_int_err
          )
          SELECT image_id
                ,t0.zone
                ,ra
                ,decl
                ,ra_err
                ,decl_err
                ,x
                ,y
                ,z
                ,det_sigma
                ,I_peak
                ,I_peak_err
                ,I_int
                ,I_int_err
           FROM (SELECT %s AS image_id
                       ,CAST(FLOOR(ldecl) AS INTEGER) AS zone
                       ,lra AS ra
                       ,ldecl AS decl
                       ,lra_err * 3600 AS ra_err
                       ,ldecl_err * 3600 AS decl_err
                       ,COS(RADIANS(ldecl)) * COS(RADIANS(lra)) AS x
                       ,COS(RADIANS(ldecl)) * SIN(RADIANS(lra)) AS y
                       ,SIN(RADIANS(ldecl)) AS z
                       ,ldet_sigma AS det_sigma
                       ,lI_peak AS I_peak 
                       ,lI_peak_err AS I_peak_err
                       ,lI_int AS I_int
                       ,lI_int_err AS I_int_err
                   FROM detections
                ) t0
               /*,node n
          WHERE n.zone = t0.zone */
        """
        cursor.execute(query, (image_id,))
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def insert_extracted_sources(conn, image_id, results):
    """Insert all extracted sources

    Insert the sources that were detected by the Source Extraction
    procedures into the extractedsources table.

    Therefore, we use a temporary table containing the "raw" detections,
    from which the sources will then be inserted into extractedsourtces.
    """

    _empty_detections(conn)
    _insert_into_detections(conn, results)
    _insert_extractedsources(conn, image_id)
    _empty_detections(conn)


# The following set of functions are private to the module;
# these are called by associate_extracted_sources, and should
# only be used in that way
def _empty_temprunningcatalog(conn):
    """Initialize the temporary storage table

    Initialize the temporary table temprunningcatalog which contains
    the current observed sources.
    """

    try:
        cursor = conn.cursor()
        query = """DELETE FROM temprunningcatalog"""
        cursor.execute(query)
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _insert_temprunningcatalog_by_bmaj(conn, image_id):
    """Select matched sources

    Here we select the extractedsources that have a positional match
    with the sources in the running catalogue table (runningcatalog)
    and those who have will be inserted into the temporary running
    catalogue table (temprunningcatalog).
    Matching criteria are the that counterparts do not lie further apart
    than the semi-major axis of synthesized beam.

    Explanation of some columns used in the SQL query:

    - avg_I_peak := average of I_peak
    - avg_I_peak_sq := average of I_peak^2
    - avg_weight_I_peak := average of weight of I_peak, i.e. 1/error^2
    - avg_weighted_I_peak := average of weighted i_peak,
         i.e. average of I_peak/error^2
    - avg_weighted_I_peak_sq := average of weighted i_peak^2,
         i.e. average of I_peak^2/error^2

    This result set might contain multiple associations (1-n,n-1)
    for a single known source in runningcatalog.

    The n-1 assocs will be treated similar as the 1-1 assocs.
    """

    try:
        cursor = conn.cursor()
        query = """\
        SELECT COS(RADIANS(bmaj))
          FROM images
         WHERE imageid = %s
        """
        cursor.execute(query, (image_id,))
        results = zip(*cursor.fetchall())
        if len(results) != 0:
            cos_rad_bmaj = results[0]
        print "cos_rad_bmaj = ", cos_rad_bmaj[0]
        # !!TODO!!: Add columns for previous weighted averaged values,
        # otherwise the assoc_r will be biased.
        query = """\
        INSERT INTO temprunningcatalog
          (xtrsrc_id
          ,assoc_xtrsrc_id
          ,ds_id
          ,band
          ,datapoints
          ,zone
          ,wm_ra
          ,wm_decl
          ,wm_ra_err
          ,wm_decl_err
          ,avg_wra
          ,avg_wdecl
          ,avg_weight_ra
          ,avg_weight_decl
          ,x
          ,y
          ,z
          ,avg_I_peak
          ,avg_I_peak_sq
          ,avg_weight_peak
          ,avg_weighted_I_peak
          ,avg_weighted_I_peak_sq
          )
          SELECT t0.xtrsrc_id
                ,t0.assoc_xtrsrc_id
                ,t0.ds_id
                ,t0.band
                ,t0.datapoints
                ,CAST(FLOOR(t0.wm_decl/1.0) AS INTEGER)
                ,t0.wm_ra
                ,t0.wm_decl
                ,t0.wm_ra_err
                ,t0.wm_decl_err
                ,t0.avg_wra
                ,t0.avg_wdecl
                ,t0.avg_weight_ra
                ,t0.avg_weight_decl
                ,COS(RADIANS(t0.wm_decl)) * COS(RADIANS(t0.wm_ra))
                ,COS(RADIANS(t0.wm_decl)) * SIN(RADIANS(t0.wm_ra))
                ,SIN(RADIANS(t0.wm_decl))
                ,t0.avg_I_peak
                ,t0.avg_I_peak_sq
                ,t0.avg_weight_peak
                ,t0.avg_weighted_I_peak
                ,t0.avg_weighted_I_peak_sq
            FROM (SELECT rc.xtrsrc_id as xtrsrc_id
                        ,x0.xtrsrcid as assoc_xtrsrc_id
                        ,im0.ds_id
                        ,im0.band
                        ,rc.datapoints + 1 AS datapoints
                        ,((datapoints * rc.avg_wra + x0.ra /
                          (x0.ra_err * x0.ra_err)) / (datapoints + 1))
                         /
                         ((datapoints * rc.avg_weight_ra + 1 /
                           (x0.ra_err * x0.ra_err)) / (datapoints + 1))
                         AS wm_ra
                        ,((datapoints * rc.avg_wdecl + x0.decl /
                          (x0.decl_err * x0.decl_err)) / (datapoints + 1))
                         /
                         ((datapoints * rc.avg_weight_decl + 1 /
                           (x0.decl_err * x0.decl_err)) / (datapoints + 1))
                         AS wm_decl
                        ,SQRT(1 / ((datapoints + 1) *
                          ((datapoints * rc.avg_weight_ra +
                            1 / (x0.ra_err * x0.ra_err)) / (datapoints + 1))
                                  )
                             ) AS wm_ra_err
                        ,SQRT(1 / ((datapoints + 1) *
                          ((datapoints * rc.avg_weight_decl +
                            1 / (x0.decl_err * x0.decl_err)) / (datapoints + 1))
                                  )
                             ) AS wm_decl_err
                        ,(datapoints * rc.avg_wra + x0.ra / (x0.ra_err * x0.ra_err))
                         / (datapoints + 1) AS avg_wra
                        ,(datapoints * rc.avg_wdecl + x0.decl /
                          (x0.decl_err * x0.decl_err))
                         / (datapoints + 1) AS avg_wdecl
                        ,(datapoints * rc.avg_weight_ra + 1 /
                          (x0.ra_err * x0.ra_err))
                         / (datapoints + 1) AS avg_weight_ra
                        ,(datapoints * rc.avg_weight_decl + 1 /
                          (x0.decl_err * x0.decl_err))
                         / (datapoints + 1) AS avg_weight_decl
                        ,(datapoints * rc.avg_I_peak + x0.I_peak)
                         / (datapoints + 1)
                         AS avg_I_peak
                        ,(datapoints * rc.avg_I_peak_sq +
                          x0.I_peak * x0.I_peak)
                         / (datapoints + 1)
                         AS avg_I_peak_sq
                        ,(datapoints * rc.avg_weight_peak + 1 /
                          (x0.I_peak_err * x0.I_peak_err))
                         / (datapoints + 1)
                         AS avg_weight_peak
                        ,(datapoints * rc.avg_weighted_I_peak + x0.I_peak /
                          (x0.I_peak_err * x0.I_peak_err))
                         / (datapoints + 1)
                         AS avg_weighted_I_peak
                        ,(datapoints * rc.avg_weighted_I_peak_sq
                          + (x0.I_peak * x0.I_peak) /
                             (x0.I_peak_err * x0.I_peak_err))
                         / (datapoints + 1) AS avg_weighted_I_peak_sq
                    FROM runningcatalog rc
                        ,extractedsources x0
                        ,images im0
                   WHERE x0.image_id = %s
                     AND x0.image_id = im0.imageid
                     AND im0.ds_id = rc.ds_id
                     AND rc.zone BETWEEN CAST(FLOOR(x0.decl - im0.bmaj) AS INTEGER)
                                     AND CAST(FLOOR(x0.decl + im0.bmaj) AS INTEGER)
                     AND rc.wm_decl BETWEEN x0.decl - im0.bmaj
                                        AND x0.decl + im0.bmaj
                     AND rc.wm_ra BETWEEN x0.ra - alpha(im0.bmaj, x0.decl)
                                      AND x0.ra + alpha(im0.bmaj, x0.decl)
                     /*AND rc.x * x0.x + rc.y * x0.y + rc.z * x0.z > COS(RADIANS(im0.bmaj))*/
                     AND rc.x * x0.x + rc.y * x0.y + rc.z * x0.z > %s
                 ) t0
        """
        cursor.execute(query, (image_id,cos_rad_bmaj[0]))
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()

def _insert_temprunningcatalog(conn, image_id, deRuiter_r):
    """Select matched sources

    Here we select the extractedsources that have a positional match
    with the sources in the running catalogue table (runningcatalog)
    and those who have will be inserted into the temporary running
    catalogue table (temprunningcatalog).

    Explanation of some columns used in the SQL query:

    - avg_I_peak := average of I_peak
    - avg_I_peak_sq := average of I_peak^2
    - avg_weight_I_peak := average of weight of I_peak, i.e. 1/error^2
    - avg_weighted_I_peak := average of weighted i_peak,
         i.e. average of I_peak/error^2
    - avg_weighted_I_peak_sq := average of weighted i_peak^2,
         i.e. average of I_peak^2/error^2

    This result set might contain multiple associations (1-n,n-1)
    for a single known source in runningcatalog.

    The n-1 assocs will be treated similar as the 1-1 assocs.
    """

    try:
        cursor = conn.cursor()
        # !!TODO!!: Add columns for previous weighted averaged values,
        # otherwise the assoc_r will be biased.
        query = """\
INSERT INTO temprunningcatalog
  (xtrsrc_id
  ,assoc_xtrsrc_id
  ,ds_id
  ,band
  ,datapoints
  ,zone
  ,wm_ra
  ,wm_decl
  ,wm_ra_err
  ,wm_decl_err
  ,avg_wra
  ,avg_wdecl
  ,avg_weight_ra
  ,avg_weight_decl
  ,x
  ,y
  ,z
  ,avg_I_peak
  ,avg_I_peak_sq
  ,avg_weight_peak
  ,avg_weighted_I_peak
  ,avg_weighted_I_peak_sq
  )
  SELECT t0.xtrsrc_id
        ,t0.assoc_xtrsrc_id
        ,t0.ds_id
        ,t0.band
        ,t0.datapoints
        ,CAST(FLOOR(t0.wm_decl/1.0) AS INTEGER)
        ,t0.wm_ra
        ,t0.wm_decl
        ,t0.wm_ra_err
        ,t0.wm_decl_err
        ,t0.avg_wra
        ,t0.avg_wdecl
        ,t0.avg_weight_ra
        ,t0.avg_weight_decl
        ,COS(RADIANS(t0.wm_decl)) * COS(RADIANS(t0.wm_ra))
        ,COS(RADIANS(t0.wm_decl)) * SIN(RADIANS(t0.wm_ra))
        ,SIN(RADIANS(t0.wm_decl))
        ,t0.avg_I_peak
        ,t0.avg_I_peak_sq
        ,t0.avg_weight_peak
        ,t0.avg_weighted_I_peak
        ,t0.avg_weighted_I_peak_sq
    FROM (SELECT rc.xtrsrc_id as xtrsrc_id
                ,x0.xtrsrcid as assoc_xtrsrc_id
                ,im0.ds_id
                ,im0.band
                ,rc.datapoints + 1 AS datapoints
                ,((datapoints * rc.avg_wra + x0.ra /
                  (x0.ra_err * x0.ra_err)) / (datapoints + 1))
                 /
                 ((datapoints * rc.avg_weight_ra + 1 /
                   (x0.ra_err * x0.ra_err)) / (datapoints + 1))
                 AS wm_ra
                ,((datapoints * rc.avg_wdecl + x0.decl /
                  (x0.decl_err * x0.decl_err)) / (datapoints + 1))
                 /
                 ((datapoints * rc.avg_weight_decl + 1 /
                   (x0.decl_err * x0.decl_err)) / (datapoints + 1))
                 AS wm_decl
                ,SQRT(1 / ((datapoints + 1) *
                  ((datapoints * rc.avg_weight_ra +
                    1 / (x0.ra_err * x0.ra_err)) / (datapoints + 1))
                          )
                     ) AS wm_ra_err
                ,SQRT(1 / ((datapoints + 1) *
                  ((datapoints * rc.avg_weight_decl +
                    1 / (x0.decl_err * x0.decl_err)) / (datapoints + 1))
                          )
                     ) AS wm_decl_err
                ,(datapoints * rc.avg_wra + x0.ra / (x0.ra_err * x0.ra_err))
                 / (datapoints + 1) AS avg_wra
                ,(datapoints * rc.avg_wdecl + x0.decl /
                  (x0.decl_err * x0.decl_err))
                 / (datapoints + 1) AS avg_wdecl
                ,(datapoints * rc.avg_weight_ra + 1 /
                  (x0.ra_err * x0.ra_err))
                 / (datapoints + 1) AS avg_weight_ra
                ,(datapoints * rc.avg_weight_decl + 1 /
                  (x0.decl_err * x0.decl_err))
                 / (datapoints + 1) AS avg_weight_decl
                ,(datapoints * rc.avg_I_peak + x0.I_peak)
                 / (datapoints + 1)
                 AS avg_I_peak
                ,(datapoints * rc.avg_I_peak_sq +
                  x0.I_peak * x0.I_peak)
                 / (datapoints + 1)
                 AS avg_I_peak_sq
                ,(datapoints * rc.avg_weight_peak + 1 /
                  (x0.I_peak_err * x0.I_peak_err))
                 / (datapoints + 1)
                 AS avg_weight_peak
                ,(datapoints * rc.avg_weighted_I_peak + x0.I_peak /
                  (x0.I_peak_err * x0.I_peak_err))
                 / (datapoints + 1)
                 AS avg_weighted_I_peak
                ,(datapoints * rc.avg_weighted_I_peak_sq
                  + (x0.I_peak * x0.I_peak) /
                     (x0.I_peak_err * x0.I_peak_err))
                 / (datapoints + 1) AS avg_weighted_I_peak_sq
            FROM runningcatalog rc
                ,extractedsources x0
                ,images im0
           WHERE x0.image_id = %s
             AND x0.image_id = im0.imageid
             AND im0.ds_id = rc.ds_id
             AND rc.zone BETWEEN CAST(FLOOR(x0.decl - 0.025) as INTEGER)
                             AND CAST(FLOOR(x0.decl + 0.025) as INTEGER)
             AND rc.wm_decl BETWEEN x0.decl - 0.025
                                AND x0.decl + 0.025
             AND rc.wm_ra BETWEEN x0.ra - alpha(0.025,x0.decl)
                              AND x0.ra + alpha(0.025,x0.decl)
             AND SQRT(  (x0.ra * COS(RADIANS(x0.decl)) - rc.wm_ra * COS(RADIANS(rc.wm_decl)))
                      * (x0.ra * COS(RADIANS(x0.decl)) - rc.wm_ra * COS(RADIANS(rc.wm_decl)))
                      / (x0.ra_err * x0.ra_err + rc.wm_ra_err * rc.wm_ra_err)
                     + (x0.decl - rc.wm_decl) * (x0.decl - rc.wm_decl)
                      / (x0.decl_err * x0.decl_err + rc.wm_decl_err * rc.wm_decl_err)
                     ) < %s
         ) t0
"""
        cursor.execute(query, (image_id, deRuiter_r/3600.))
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _flag_multiple_counterparts_in_runningcatalog_by_dist(conn):
    """Flag source with multiple associations

    Before we continue, we first take care of the sources that have
    multiple associations in both directions.

    -1- running-catalogue sources  <- extracted source

    An extracted source has multiple counterparts in the running
    catalogue.  We only keep the ones with the lowest distance_arcsec
    value, the rest we throw away. The query below selects the rest.

    NOTE:

    It is worth considering whether this might be changed to selecting
    the brightest neighbour source, instead of just the closest
    neighbour.

    (There are cases [when flux_lim > 10Jy] that the nearest source has
    a lower flux level, causing unexpected spectral indices)
    """
    
    # TODO: change r into distance
    try:
        cursor = conn.cursor()
        query = """\
        SELECT t1.xtrsrc_id
              ,t1.assoc_xtrsrc_id
          FROM (SELECT trc0.assoc_xtrsrc_id
                      ,MIN(3600*DEGREES(2*ASIN(SQRT( (rc0.x - x0.x) * (rc0.x - x0.x)
                                    + (rc0.y - x0.y) * (rc0.y - x0.y)
                                    + (rc0.z - x0.z) * (rc0.z - x0.z)
                                    ) / 2) 
                          )) AS min_dist_param
                  FROM temprunningcatalog trc0
                      ,runningcatalog rc0
                      ,extractedsources x0
                 WHERE trc0.xtrsrc_id = rc0.xtrsrc_id
                   AND trc0.assoc_xtrsrc_id = x0.xtrsrcid
                GROUP BY trc0.assoc_xtrsrc_id
                HAVING COUNT(*) > 1
               ) t0
              ,(SELECT trc1.xtrsrc_id
                      ,trc1.assoc_xtrsrc_id
                      ,3600*DEGREES(2*ASIN(SQRT( (rc1.x - x1.x) * (rc1.x - x1.x)
                                + (rc1.y - x1.y) * (rc1.y - x1.y)
                                + (rc1.z - x1.z) * (rc1.z - x1.z)
                                ) / 2
                           )) AS dist_param
                  FROM temprunningcatalog trc1
                      ,runningcatalog rc1
                      ,extractedsources x1
                 WHERE trc1.xtrsrc_id = rc1.xtrsrc_id
                   AND trc1.assoc_xtrsrc_id = x1.xtrsrcid
               ) t1
         WHERE t1.assoc_xtrsrc_id = t0.assoc_xtrsrc_id
           AND t1.dist_param > t0.min_dist_param
        """
        cursor.execute(query)
        results = zip(*cursor.fetchall())
        if len(results) != 0:
            xtrsrc_id = results[0]
            assoc_xtrsrc_id = results[1]
            # TODO: Consider setting row to inactive instead of deleting
            query = """\
            DELETE
              FROM temprunningcatalog
             WHERE xtrsrc_id = %s
               AND assoc_xtrsrc_id = %s
            """
            for j in range(len(xtrsrc_id)):
                cursor.execute(query, (xtrsrc_id[j], assoc_xtrsrc_id[j]))
                if not AUTOCOMMIT:
                    conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()

def _flag_multiple_counterparts_in_runningcatalog(conn):
    """Flag source with multiple associations

    Before we continue, we first take care of the sources that have
    multiple associations in both directions.

    -1- running-catalogue sources  <- extracted source

    An extracted source has multiple counterparts in the running
    catalogue.  We only keep the ones with the lowest deRuiter_r
    value, the rest we throw away.

    NOTE:

    It is worth considering whether this might be changed to selecting
    the brightest neighbour source, instead of just the closest
    neighbour.

    (There are cases [when flux_lim > 10Jy] that the nearest source has
    a lower flux level, causing unexpected spectral indices)
    """
    
    # TODO: change r into distance
    try:
        cursor = conn.cursor()
        query = """\
        SELECT t1.xtrsrc_id
              ,t1.assoc_xtrsrc_id
          FROM (SELECT trc0.assoc_xtrsrc_id
                      ,MIN(SQRT((x0.ra * COS(RADIANS(x0.decl)) - rc0.wm_ra * COS(RADIANS(rc0.wm_decl)))
                                * (x0.ra * COS(RADIANS(x0.decl))- rc0.wm_ra * COS(RADIANS(rc0.wm_decl)))
                                / (x0.ra_err * x0.ra_err + rc0.wm_ra_err * rc0.wm_ra_err)
                               + (x0.decl - rc0.wm_decl) * (x0.decl - rc0.wm_decl)
                                / (x0.decl_err * x0.decl_err + rc0.wm_decl_err * rc0.wm_decl_err)
                               )
                          ) AS min_r1
                  FROM temprunningcatalog trc0
                      ,runningcatalog rc0
                      ,extractedsources x0
                 WHERE trc0.assoc_xtrsrc_id IN (SELECT assoc_xtrsrc_id
                                                  FROM temprunningcatalog
                                                GROUP BY assoc_xtrsrc_id
                                                HAVING COUNT(*) > 1
                                               )
                   AND trc0.xtrsrc_id = rc0.xtrsrc_id
                   AND trc0.assoc_xtrsrc_id = x0.xtrsrcid
                GROUP BY trc0.assoc_xtrsrc_id
               ) t0
              ,(SELECT trc1.xtrsrc_id
                      ,trc1.assoc_xtrsrc_id
                      ,SQRT( (x1.ra * COS(RADIANS(x1.decl)) - rc1.wm_ra * COS(RADIANS(rc1.wm_decl)))
                            *(x1.ra * COS(RADIANS(x1.decl)) - rc1.wm_ra * COS(RADIANS(rc1.wm_decl)))
                            / (x1.ra_err * x1.ra_err + rc1.wm_ra_err * rc1.wm_ra_err)
                           + (x1.decl - rc1.wm_decl) * (x1.decl - rc1.wm_decl)
                             / (x1.decl_err * x1.decl_err + rc1.wm_decl_err * rc1.wm_decl_err)
                           ) AS r1
                  FROM temprunningcatalog trc1
                      ,runningcatalog rc1
                      ,extractedsources x1
                 WHERE trc1.assoc_xtrsrc_id IN (SELECT assoc_xtrsrc_id
                                                 FROM temprunningcatalog
                                               GROUP BY assoc_xtrsrc_id
                                               HAVING COUNT(*) > 1
                                              )
                   AND trc1.xtrsrc_id = rc1.xtrsrc_id
                   AND trc1.assoc_xtrsrc_id = x1.xtrsrcid
               ) t1
         WHERE t1.assoc_xtrsrc_id = t0.assoc_xtrsrc_id
           AND t1.r1 > t0.min_r1
        """
        cursor.execute(query)
        results = zip(*cursor.fetchall())
        if len(results) != 0:
            xtrsrc_id = results[0]
            assoc_xtrsrc_id = results[1]
            # TODO: Consider setting row to inactive instead of deleting
            query = """\
            DELETE
              FROM temprunningcatalog
             WHERE xtrsrc_id = %s
               AND assoc_xtrsrc_id = %s
            """
            for j in range(len(xtrsrc_id)):
                cursor.execute(query, (xtrsrc_id[j], assoc_xtrsrc_id[j]))
                if not AUTOCOMMIT:
                    conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _insert_multiple_assocs(conn):
    """Insert sources with multiple associations

    -2- Now, we take care of the sources in the running catalogue that
    have more than one counterpart among the extracted sources.

    We now make two entries in the running catalogue, in stead of the
    one we had before. Therefore, we 'swap' the ids.
    """

    try:
        cursor = conn.cursor()
        query = """\
        INSERT INTO assocxtrsources
          (xtrsrc_id
          ,assoc_xtrsrc_id
          ,assoc_distance_arcsec
          ,assoc_r
          ,assoc_lr_method
          )
          SELECT t.assoc_xtrsrc_id
                ,t.xtrsrc_id
                ,3600 * DEGREES(2 * ASIN(SQRT( (r.x - x.x) * (r.x - x.x)
                                             + (r.y - x.y) * (r.y - x.y)
                                             + (r.z - x.z) * (r.z - x.z)
                                             ) / 2) ) AS assoc_distance_arcsec
                ,3600 * sqrt(
                    ( (r.wm_ra * cos(RADIANS(r.wm_decl)) - x.ra * cos(RADIANS(x.decl)))
                     *(r.wm_ra * cos(RADIANS(r.wm_decl)) - x.ra * cos(RADIANS(x.decl)))
                    ) 
                    / (r.wm_ra_err * r.wm_ra_err + x.ra_err * x.ra_err)
                    + ((r.wm_decl - x.decl) * (r.wm_decl - x.decl)) 
                    / (r.wm_decl_err * r.wm_decl_err + x.decl_err * x.decl_err)
                            ) as assoc_r
                ,1
            FROM temprunningcatalog t
                ,runningcatalog r
                ,extractedsources x
           WHERE t.xtrsrc_id = r.xtrsrc_id
             AND t.xtrsrc_id = x.xtrsrcid
             AND t.xtrsrc_id IN (SELECT xtrsrc_id
                                   FROM temprunningcatalog
                                 GROUP BY xtrsrc_id
                                 HAVING COUNT(*) > 1
                                )
        """
        cursor.execute(query)
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _insert_first_of_assocs(conn):
    """Insert identical ids

    -3- And, we have to insert identical ids to identify a light-curve
    starting point.
    """

    try:
        cursor = conn.cursor()
        query = """\
        INSERT INTO assocxtrsources
          (xtrsrc_id
          ,assoc_xtrsrc_id
          ,assoc_distance_arcsec
          ,assoc_r
          ,assoc_lr_method
          )
          SELECT assoc_xtrsrc_id
                ,assoc_xtrsrc_id
                ,0
                ,0
                ,2
            FROM temprunningcatalog
           WHERE xtrsrc_id IN (SELECT xtrsrc_id
                                 FROM temprunningcatalog
                               GROUP BY xtrsrc_id
                               HAVING COUNT(*) > 1
                              )
        """
        cursor.execute(query)
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _flag_swapped_assocs(conn):
    """Throw away swapped ids

    -4- And, we throw away the swapped id.

    It might be better to flag this record: consider setting rows to
    inactive instead of deleting
    """
    try:
        cursor = conn.cursor()
        query = """\
        DELETE
          FROM assocxtrsources
         WHERE xtrsrc_id IN (SELECT xtrsrc_id
                               FROM temprunningcatalog
                             GROUP BY xtrsrc_id
                             HAVING COUNT(*) > 1
                            )
        """
        cursor.execute(query)
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _insert_multiple_assocs_runcat(conn):
    """Insert new ids of the sources in the running catalogue"""

    try:
        cursor = conn.cursor()
        query = """\
        INSERT INTO runningcatalog
          (xtrsrc_id
          ,ds_id
          ,band
          ,datapoints
          ,zone
          ,wm_ra
          ,wm_decl
          ,wm_ra_err
          ,wm_decl_err
          ,avg_wra
          ,avg_wdecl
          ,avg_weight_ra
          ,avg_weight_decl
          ,x
          ,y
          ,z
          ,avg_I_peak
          ,avg_I_peak_sq
          ,avg_weight_peak
          ,avg_weighted_I_peak
          ,avg_weighted_I_peak_sq
          )
          SELECT assoc_xtrsrc_id
                ,ds_id
                ,band
                ,datapoints
                ,zone
                ,wm_ra
                ,wm_decl
                ,wm_ra_err
                ,wm_decl_err
                ,avg_wra
                ,avg_wdecl
                ,avg_weight_ra
                ,avg_weight_decl
                ,x
                ,y
                ,z
                ,avg_I_peak
                ,avg_I_peak_sq
                ,avg_weight_peak
                ,avg_weighted_I_peak
                ,avg_weighted_I_peak_sq
            FROM temprunningcatalog
           WHERE xtrsrc_id IN (SELECT xtrsrc_id
                                 FROM temprunningcatalog
                               GROUP BY xtrsrc_id
                               HAVING COUNT(*) > 1
                              )
        """
        cursor.execute(query)
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _flag_old_assocs_runcat(conn):
    """Here the old assocs in runcat will be deleted."""

    # TODO: Consider setting row to inactive instead of deleting
    try:
        cursor = conn.cursor()
        query = """\
        DELETE
          FROM runningcatalog
         WHERE xtrsrc_id IN (SELECT xtrsrc_id
                               FROM temprunningcatalog
                             GROUP BY xtrsrc_id
                             HAVING COUNT(*) > 1
                            )
        """
        cursor.execute(query)
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _flag_multiple_assocs(conn):
    """Delete the multiple assocs from the temporary running catalogue table"""

    try:
        cursor = conn.cursor()
        query = """\
        DELETE
          FROM temprunningcatalog
         WHERE xtrsrc_id IN (SELECT xtrsrc_id
                               FROM temprunningcatalog
                             GROUP BY xtrsrc_id
                             HAVING COUNT(*) > 1
                            )
        """
        cursor.execute(query)
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _insert_single_assocs(conn):
    """Insert remaining 1-1 associations into assocxtrsources table"""
    try:
        cursor = conn.cursor()
        query = """\
        INSERT INTO assocxtrsources
          (xtrsrc_id
          ,assoc_xtrsrc_id
          ,assoc_distance_arcsec
          ,assoc_r
          ,assoc_lr_method
          )
          SELECT t.xtrsrc_id
                ,t.assoc_xtrsrc_id
                ,3600 * DEGREES(2 * ASIN(SQRT( (r.x - x.x) * (r.x - x.x)
                                             + (r.y - x.y) * (r.y - x.y)
                                             + (r.z - x.z) * (r.z - x.z)
                                             ) / 2) ) AS assoc_distance_arcsec
                ,3600 * SQRT( (r.wm_ra * COS(RADIANS(r.wm_decl)) - x.ra * COS(RADIANS(x.decl)))
                            * (r.wm_ra * COS(RADIANS(r.wm_decl)) - x.ra * COS(RADIANS(x.decl)))
                            / (r.wm_ra_err * r.wm_ra_err + x.ra_err * x.ra_err)
                            + ((r.wm_decl - x.decl) * (r.wm_decl - x.decl)) 
                            / (r.wm_decl_err * r.wm_decl_err + x.decl_err * x.decl_err)
                            ) AS assoc_r
                ,3
            FROM temprunningcatalog t
                ,runningcatalog r
                ,extractedsources x
           WHERE t.xtrsrc_id = r.xtrsrc_id
             AND t.assoc_xtrsrc_id = x.xtrsrcid
        """
        cursor.execute(query)
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _update_runningcatalog(conn):
    """Update the running catalog"""

    try:
        cursor = conn.cursor()
        query = """\
        SELECT datapoints
              ,zone
              ,wm_ra
              ,wm_decl
              ,wm_ra_err
              ,wm_decl_err
              ,avg_wra
              ,avg_wdecl
              ,avg_weight_ra
              ,avg_weight_decl
              ,x
              ,y
              ,z
              ,avg_I_peak
              ,avg_I_peak_sq
              ,avg_weight_peak
              ,avg_weighted_I_peak
              ,avg_weighted_I_peak_sq
              ,xtrsrc_id
          FROM temprunningcatalog
        """
        cursor.execute(query)
        results = cursor.fetchall()
        query = """\
        UPDATE runningcatalog
          SET datapoints = %s
             ,zone = %s
             ,wm_ra = %s
             ,wm_decl = %s
             ,wm_ra_err = %s
             ,wm_decl_err = %s
             ,avg_wra = %s
             ,avg_wdecl = %s
             ,avg_weight_ra = %s
             ,avg_weight_decl = %s
             ,x = %s
             ,y = %s
             ,z = %s
             ,avg_I_peak = %s
             ,avg_I_peak_sq = %s
             ,avg_weight_peak = %s
             ,avg_weighted_I_peak = %s
             ,avg_weighted_I_peak_sq = %s
        WHERE xtrsrc_id = %s
        """
        for result in results:
            cursor.execute(query, tuple(result))
            if not AUTOCOMMIT:
                conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _count_known_sources_by_bmaj(conn, image_id):
    """
    Count number of extracted sources that are known 
    in the running catalog
    """

    cursor = conn.cursor()
    try:
        query = """\
        SELECT COUNT(*)
          FROM extractedsources x0
              ,images im0
              ,runningcatalog r0
         WHERE x0.image_id = %s
           AND x0.image_id = im0.imageid
           AND im0.ds_id = r0.ds_id
           AND r0.zone BETWEEN CAST(FLOOR(x0.decl - im0.bmaj) AS INTEGER)
                           AND CAST(FLOOR(x0.decl + im0.bmaj) AS INTEGER)
           AND r0.wm_decl BETWEEN x0.decl - im0.bmaj
                              AND x0.decl + im0.bmaj
           AND r0.wm_ra BETWEEN x0.ra - alpha(im0.bmaj, x0.decl)
                            AND x0.ra + alpha(im0.bmaj, x0.decl)
           AND r0.x * x0.x + r0.y * x0.y + r0.z * r0.z > COS(RADIANS(im0.bmaj))
        """
        curisor.execute(query, (image_id,))
        y = cursor.fetchall()
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()

def _count_known_sources(conn, image_id, deRuiter_r):
    """
    Count number of extracted sources that are known 
    in the running catalog
    """
    
    cursor = conn.cursor()
    try:
        query = """\
        SELECT COUNT(*)
          FROM extractedsources x0
              ,images im0
              ,runningcatalog b0
         WHERE x0.image_id = %s
           AND x0.image_id = im0.imageid
           AND im0.ds_id = b0.ds_id
           AND b0.zone BETWEEN x0.zone - cast(0.025 as integer)
                           AND x0.zone + cast(0.025 as integer)
           AND b0.wm_decl BETWEEN x0.decl - 0.025
                              AND x0.decl + 0.025
           AND b0.wm_ra BETWEEN x0.ra - alpha(0.025,x0.decl)
                            AND x0.ra + alpha(0.025,x0.decl)
           AND SQRT(  (x0.ra * COS(RADIANS(x0.decl)) - b0.wm_ra * COS(RADIANS(b0.wm_decl)))
                    * (x0.ra * COS(RADIANS(x0.decl)) - b0.wm_ra * COS(RADIANS(b0.wm_decl)))
                    / (x0.ra_err * x0.ra_err + b0.wm_ra_err * b0.wm_ra_err)
                    + (x0.decl - b0.wm_decl) * (x0.decl - b0.wm_decl)
                    / (x0.decl_err * x0.decl_err + b0.wm_decl_err * b0.wm_decl_err)
                   ) < %s
        """
        cursor.execute(query, (image_id, deRuiter_r/3600.))
        y = cursor.fetchall()
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()

def _insert_new_assocs_by_bmaj(conn, image_id):
    """Insert new associations for unknown sources

    This inserts new associations for the sources that were not known
    in the running catalogue (i.e. they did not have an entry in the
    runningcatalog table).
    """

    cursor = conn.cursor()
    try:
        query = """\
        SELECT COS(RADIANS(bmaj))
          FROM images
         WHERE imageid = %s
        """
        cursor.execute(query, (image_id,))
        results = zip(*cursor.fetchall())
        if len(results) != 0:
            cos_rad_bmaj = results[0]
        
        query = """\
        INSERT INTO assocxtrsources
          (xtrsrc_id
          ,assoc_xtrsrc_id
          ,assoc_distance_arcsec
          ,assoc_r
          ,assoc_lr_method
          )
          SELECT x1.xtrsrcid as xtrsrc_id
                ,x1.xtrsrcid as assoc_xtrsrc_id
                ,0
                ,0
                ,4
            FROM extractedsources x1
           WHERE x1.image_id = %s
             AND x1.xtrsrcid NOT IN (SELECT x0.xtrsrcid
                                       FROM extractedsources x0
                                           ,runningcatalog r0
                                           ,images im0
                                      WHERE x0.image_id = %s
                                        AND x0.image_id = im0.imageid
                                        AND im0.ds_id = r0.ds_id
                                        AND r0.zone BETWEEN CAST(FLOOR(x0.decl - im0.bmaj) AS INTEGER)
                                                        AND CAST(FLOOR(x0.decl + im0.bmaj) AS INTEGER)
                                        AND r0.wm_decl BETWEEN x0.decl - im0.bmaj
                                                           AND x0.decl + im0.bmaj
                                        AND r0.wm_ra BETWEEN x0.ra - alpha(im0.bmaj, x0.decl)
                                                         AND x0.ra + alpha(im0.bmaj, x0.decl)
                                        /*AND r0.x * x0.x + r0.y * x0.y + r0.z * r0.z > COS(RADIANS(im0.bmaj))*/
                                        AND r0.x * x0.x + r0.y * x0.y + r0.z * r0.z > %s
                                    )
        """
        cursor.execute(query, (image_id, image_id, cos_rad_bmaj[0]))
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()

def _insert_new_assocs(conn, image_id, deRuiter_r):
    """Insert new associations for unknown sources

    This inserts new associations for the sources that were not known
    in the running catalogue (i.e. they did not have an entry in the
    runningcatalog table).
    """

    cursor = conn.cursor()
    try:
        query = """\
        INSERT INTO assocxtrsources
          (xtrsrc_id
          ,assoc_xtrsrc_id
          ,assoc_distance_arcsec
          ,assoc_r
          ,assoc_lr_method
          )
          SELECT x1.xtrsrcid as xtrsrc_id
                ,x1.xtrsrcid as assoc_xtrsrc_id
                ,0
                ,0
                ,4
            FROM extractedsources x1
           WHERE x1.image_id = %s
             AND x1.xtrsrcid NOT IN (SELECT x0.xtrsrcid
                                       FROM extractedsources x0
                                           ,runningcatalog b0
                                           ,images im0
                                      WHERE x0.image_id = %s
                                        AND x0.image_id = im0.imageid
                                        AND im0.ds_id = b0.ds_id
                                        AND b0.zone BETWEEN x0.zone - cast(0.025 as integer)
                                                        AND x0.zone + cast(0.025 as integer)
                                        AND b0.wm_decl BETWEEN x0.decl - 0.025
                                                           AND x0.decl + 0.025
                                        AND b0.wm_ra BETWEEN x0.ra - alpha(0.025,x0.decl)
                                                         AND x0.ra + alpha(0.025,x0.decl)
                                        AND SQRT(  (x0.ra * COS(RADIANS(x0.decl)) - b0.wm_ra * COS(RADIANS(b0.wm_decl)))
                                                 * (x0.ra * COS(RADIANS(x0.decl)) - b0.wm_ra * COS(RADIANS(b0.wm_decl)))
                                                 / (x0.ra_err * x0.ra_err + b0.wm_ra_err * b0.wm_ra_err)
                                                 + (x0.decl - b0.wm_decl) * (x0.decl - b0.wm_decl)
                                                 / (x0.decl_err * x0.decl_err + b0.wm_decl_err * b0.wm_decl_err)
                                                ) < %s
                                    )
        """
        cursor.execute(query, (image_id, image_id, deRuiter_r/3600.))
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _insert_new_source_runcat_by_bmaj(conn, image_id):
    """Insert new sources into the running catalog"""
    # TODO: check zone cast in search radius!
    try:
        cursor = conn.cursor()
        query = """\
        SELECT COS(RADIANS(bmaj))
          FROM images
         WHERE imageid = %s
        """
        cursor.execute(query, (image_id,))
        results = zip(*cursor.fetchall())
        if len(results) != 0:
            cos_rad_bmaj = results[0]
        query = """\
        INSERT INTO runningcatalog
          (xtrsrc_id
          ,ds_id
          ,band
          ,datapoints
          ,zone
          ,wm_ra
          ,wm_decl
          ,wm_ra_err
          ,wm_decl_err
          ,avg_wra
          ,avg_wdecl
          ,avg_weight_ra
          ,avg_weight_decl
          ,x
          ,y
          ,z
          ,avg_I_peak
          ,avg_I_peak_sq
          ,avg_weight_peak
          ,avg_weighted_I_peak
          ,avg_weighted_I_peak_sq
          )
          SELECT x1.xtrsrcid
                ,im1.ds_id
                ,band
                ,1
                ,x1.zone
                ,x1.ra
                ,x1.decl
                ,x1.ra_err
                ,x1.decl_err
                ,x1.ra / (x1.ra_err * x1.ra_err)
                ,x1.decl / (x1.decl_err * x1.decl_err)
                ,1 / (x1.ra_err * x1.ra_err)
                ,1 / (x1.decl_err * x1.decl_err)
                ,x1.x
                ,x1.y
                ,x1.z
                ,I_peak
                ,I_peak * I_peak
                ,1 / (I_peak_err * I_peak_err)
                ,I_peak / (I_peak_err * I_peak_err)
                ,I_peak * I_peak / (I_peak_err * I_peak_err)
            FROM extractedsources x1
                ,images im1
           WHERE x1.image_id = %s
             AND x1.image_id = im1.imageid
             AND x1.xtrsrcid NOT IN (SELECT x0.xtrsrcid
                                       FROM extractedsources x0
                                           ,runningcatalog r0
                                           ,images im0
                                      WHERE x0.image_id = %s
                                        AND x0.image_id = im0.imageid
                                        AND im0.ds_id = r0.ds_id
                                        AND r0.zone BETWEEN CAST(FLOOR(x0.decl - im0.bmaj) AS INTEGER)
                                                        AND CAST(FLOOR(x0.decl + im0.bmaj) AS INTEGER) 
                                        AND r0.wm_decl BETWEEN x0.decl - im0.bmaj
                                                           AND x0.decl + im0.bmaj
                                        AND r0.wm_ra BETWEEN x0.ra - alpha(im0.bmaj, x0.decl)
                                                         AND x0.ra + alpha(im0.bmaj, x0.decl)
                                        /*AND r0.x * x0.x + r0.y * x0.y + r0.z * x0.z > COS(RADIANS(im0.bmaj))*/
                                        AND r0.x * x0.x + r0.y * x0.y + r0.z * x0.z > %s
                                    )
        """
        cursor.execute(query, (image_id, image_id, cos_rad_bmaj[0]))
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()

def _insert_new_source_runcat(conn, image_id, deRuiter_r):
    """Insert new sources into the running catalog"""
    # TODO: check zone cast in search radius!
    try:
        cursor = conn.cursor()
        query = """\
        INSERT INTO runningcatalog
          (xtrsrc_id
          ,ds_id
          ,band
          ,datapoints
          ,zone
          ,wm_ra
          ,wm_decl
          ,wm_ra_err
          ,wm_decl_err
          ,avg_wra
          ,avg_wdecl
          ,avg_weight_ra
          ,avg_weight_decl
          ,x
          ,y
          ,z
          ,avg_I_peak
          ,avg_I_peak_sq
          ,avg_weight_peak
          ,avg_weighted_I_peak
          ,avg_weighted_I_peak_sq
          )
          SELECT x1.xtrsrcid
                ,im1.ds_id
                ,band
                ,1
                ,x1.zone
                ,x1.ra
                ,x1.decl
                ,x1.ra_err
                ,x1.decl_err
                ,x1.ra / (x1.ra_err * x1.ra_err)
                ,x1.decl / (x1.decl_err * x1.decl_err)
                ,1 / (x1.ra_err * x1.ra_err)
                ,1 / (x1.decl_err * x1.decl_err)
                ,x1.x
                ,x1.y
                ,x1.z
                ,I_peak
                ,I_peak * I_peak
                ,1 / (I_peak_err * I_peak_err)
                ,I_peak / (I_peak_err * I_peak_err)
                ,I_peak * I_peak / (I_peak_err * I_peak_err)
            FROM extractedsources x1
                ,images im1
           WHERE x1.image_id = %s
             AND x1.image_id = im1.imageid
             AND x1.xtrsrcid NOT IN (SELECT x0.xtrsrcid
                                       FROM extractedsources x0
                                           ,runningcatalog b0
                                           ,images im0
                                      WHERE x0.image_id = %s
                                        AND x0.image_id = im0.imageid
                                        AND im0.ds_id = b0.ds_id
                                        AND b0.zone BETWEEN x0.zone - cast(0.025 as integer)
                                                        AND x0.zone + cast(0.025 as integer)
                                        AND b0.wm_decl BETWEEN x0.decl - 0.025
                                                           AND x0.decl + 0.025
                                        AND b0.wm_ra BETWEEN x0.ra - alpha(0.025,x0.decl)
                                                         AND x0.ra + alpha(0.025,x0.decl)
                                        AND b0.x * x0.x + b0.y * x0.y + b0.z * x0.z > COS(RADIANS(0.025))
                                        AND SQRT(  (x0.ra * COS(RADIANS(x0.decl)) - b0.wm_ra * COS(RADIANS(b0.wm_decl)))
                                                 * (x0.ra * COS(RADIANS(x0.decl)) - b0.wm_ra * COS(RADIANS(b0.wm_decl)))
                                                 / (x0.ra_err * x0.ra_err + b0.wm_ra_err * b0.wm_ra_err)
                                                 + (x0.decl - b0.wm_decl) * (x0.decl - b0.wm_decl)
                                                 / (x0.decl_err * x0.decl_err + b0.wm_decl_err * b0.wm_decl_err)
                                                ) < %s
                                    )
        """
        cursor.execute(query, (image_id, image_id, deRuiter_r/3600.))
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def _associate_across_frequencies(conn, ds_id, image_id, deRuiter_r=DERUITER_R):
    """Associate sources in running catalog across frequency bands

    The dimensionless distance between two sources is given by the
    "De Ruiter radius", see Ch2&3 of thesis Scheers.

    Here we use a default value of deRuiter_r = 3.717/3600. for a
    reliable association.
    """
    return
    try:
        cursor = conn.cursor()
        query = """\
        SELECT COUNT(*)
              /*r1.xtrsrc_id AS runcat_id
              ,r1.band AS band
              ,r2.xtrsrc_id AS assoc_runcat_id
              ,r2.band AS assoc_band*/
          FROM runningcatalog r1
              ,runningcatalog r2
              ,images im1
         WHERE r1.ds_id = %s
           AND im1.imageid = %s
           AND r1.band = im1.band
           AND r2.ds_id = r1.ds_id
           AND r2.band <> r1.band
           AND r2.zone BETWEEN CAST(FLOOR(r1.decl - 0.025) AS INTEGER)
                           AND CAST(FLOOR(r1.decl + 0.025) AS INTEGER)
           AND r2.decl BETWEEN r1.decl - 0.025
                           AND r1.decl + 0.025
           AND r2.ra BETWEEN r1.ra - alpha(0.025, r1.decl)
                         AND r1.ra + alpha(0.025, r1.decl)
           AND SQRT( ((r1.ra * COS(RADIANS(r1.decl)) - r2.ra * COS(RADIANS(r2.decl))) 
                      * (r1.ra * COS(RADIANS(r1.decl)) - r2.ra * COS(RADIANS(r2.decl))))
                     / (r1.ra_err * r1.ra_err + r2.ra_err * r2.ra_err)
                   + ((r1.decl - r2.decl) * (r1.decl - r2.decl))
                    / (r1.decl_err * r1.decl_err + r2.decl_err * r2.decl_err)
                   ) < %s
        """
        cursor.execute(query, (ds_id, image_id, deRuiter_r/3600.))
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s; for reason %s" % (query, e))
        raise
    finally:
        cursor.close()

def associate_extracted_sources(conn, image_id, deRuiter_r=DERUITER_R):
    """Associate extracted sources with sources detected in the running
    catalog

    The dimensionless distance between two sources is given by the
    "De Ruiter radius", see Ch2&3 of thesis Scheers.

    Here we use a default value of deRuiter_r = 3.717/3600. for a
    reliable association.
    """

    _empty_temprunningcatalog(conn)
    #_insert_temprunningcatalog(conn, image_id, deRuiter_r)
    _insert_temprunningcatalog_by_bmaj(conn, image_id)
    #+----------------------------------------------------------------+
    #| Below, we take care of the extractedsources (i.e. more than 1) |
    #| that were associated to the same runningcatalog source.        |
    #+----------------------------------------------------------------+
    #_flag_multiple_counterparts_in_runningcatalog(conn)
    _flag_multiple_counterparts_in_runningcatalog_by_dist(conn)
    _insert_multiple_assocs(conn)
    _insert_first_of_assocs(conn)
    _flag_swapped_assocs(conn)
    _insert_multiple_assocs_runcat(conn)
    _flag_old_assocs_runcat(conn)
    _flag_multiple_assocs(conn)
    #if image_id > 1:
    #    sys.exit(1)
    #+-----------------------------------------------------+
    #| After all this, we are now left with the 1-1 assocs |
    #+-----------------------------------------------------+
    _insert_single_assocs(conn)
    _update_runningcatalog(conn)
    _empty_temprunningcatalog(conn)
    #+-----------------------------------------------------------+
    #| We end with extractedsources that could not be associated |
    #+-----------------------------------------------------------+
    #_count_known_sources(conn, image_id, deRuiter_r)
    #_insert_new_assocs(conn, image_id, deRuiter_r)
    _insert_new_assocs_by_bmaj(conn, image_id)
    #_insert_new_source_runcat(conn, image_id, deRuiter_r)
    _insert_new_source_runcat_by_bmaj(conn, image_id)
    #_associate_across_frequencies(conn, ds_id, image_id, deRuiter_r)
    #+----------------------- TODO --------------------------------+
    #| We have to include the monitorlist: source taht are in the  |
    #| runningcatalog, but were not extracted from the image ->    |
    #| force a fit to the runningcatalog position in current image |
    #+-------------------------------------------------------------+

def _insert_cat_assocs(conn, image_id, radius, deRuiter_r):
    """Insert found xtrsrc--catsrc associations into assoccatsources table.

    The search for cataloged counterpart sources is done in the catalogedsources
    table, which should have been preloaded with a selection of 
    the catalogedsources, depending on the expected field of view.
    
    """
    # TODO: change strict limits of assoc_r
    try:
        cursor = conn.cursor()
        query = """\
        INSERT INTO assoccatsources
          (xtrsrc_id
          ,assoc_catsrc_id
          ,assoc_distance_arcsec
          ,assoc_lr_method
          ,assoc_r
          ,assoc_loglr
          )
          SELECT xtrsrcid AS xtrsrc_id
                ,catsrcid AS assoc_catsrc_id
                ,3600 * DEGREES(2 * ASIN(SQRT((x0.x - c0.x) * (x0.x - c0.x)
                                          + (x0.y - c0.y) * (x0.y - c0.y)
                                          + (x0.z - c0.z) * (x0.z - c0.z)
                                          ) / 2) ) AS assoc_distance_arcsec
                ,3
                ,3600 * sqrt( ((x0.ra * cos(RADIANS(x0.decl)) - c0.ra * cos(RADIANS(c0.decl))) 
                             * (x0.ra * cos(RADIANS(x0.decl)) - c0.ra * cos(RADIANS(c0.decl)))) 
                             / (x0.ra_err * x0.ra_err + c0.ra_err*c0.ra_err)
                            +
                              ((x0.decl - c0.decl) * (x0.decl - c0.decl)) 
                             / (x0.decl_err * x0.decl_err + c0.decl_err*c0.decl_err)
                            ) as assoc_r
                ,LOG10(EXP((   (x0.ra * COS(RADIANS(x0.decl)) - c0.ra * COS(RADIANS(c0.decl)))
                             * (x0.ra * COS(RADIANS(x0.decl)) - c0.ra * COS(RADIANS(x0.decl)))
                             / (x0.ra_err * x0.ra_err + c0.ra_err * c0.ra_err)
                            +  (x0.decl - c0.decl) * (x0.decl - c0.decl) 
                             / (x0.decl_err * x0.decl_err + c0.decl_err * c0.decl_err)
                           ) / 2
                          )
                      /
                      (2 * PI() * SQRT(x0.ra_err * x0.ra_err + c0.ra_err * c0.ra_err)
                                * SQRT(x0.decl_err * x0.decl_err + c0.decl_err * c0.decl_err) * %s)
                      ) AS assoc_loglr
            FROM (select xtrsrcid
                        ,ra
                        ,decl
                        ,ra_err
                        ,decl_err
                        ,cast(floor(decl - %s) as integer) as zone_min
                        ,cast(floor(decl + %s) as integer) as zone_max
                        ,ra + alpha(%s, decl) as ra_max
                        ,ra - alpha(%s, decl) as ra_min
                        ,decl - %s as decl_min
                        ,decl + %s as decl_max
                        ,x
                        ,y
                        ,z
                    from extractedsources
                   where image_id = %s
                 ) x0
                ,catalogedsources c0
           WHERE c0.zone BETWEEN zone_min AND zone_max
             AND c0.decl BETWEEN decl_min AND decl_max
             AND c0.ra BETWEEN ra_min AND ra_max
             and x0.x*c0.x + x0.y*c0.y + x0.z*c0.z > %s
             AND SQRT(  (x0.ra * COS(RADIANS(x0.decl)) - c0.ra * COS(RADIANS(c0.decl)))
                      * (x0.ra * COS(RADIANS(x0.decl)) - c0.ra * COS(RADIANS(c0.decl)))
                      / (x0.ra_err * x0.ra_err + c0.ra_err * c0.ra_err)
                     + (x0.decl - c0.decl) * (x0.decl - c0.decl)
                      / (x0.decl_err * x0.decl_err + c0.decl_err * c0.decl_err)
                     ) < %s
        """
        cursor.execute(query, (BG_DENSITY, radius, radius, radius, radius, radius, radius, 
                               image_id,math.cos(math.pi*radius/180.), deRuiter_r/3600.))
        if not AUTOCOMMIT:
            conn.commit()
    except db.Error, e:
        logging.warn("Failed on query nr %s." % query)
        raise
    finally:
        cursor.close()


def associate_with_catalogedsources(conn, image_id, radius=0.025, deRuiter_r=DERUITER_R):
    """Associate extracted sources in specified image with known sources 
    in the external catalogues

    radius (typical 90arcsec=0.025deg), is the radius of the area centered
    at the extracted source which are searched for counterparts in the catalogues.
    
    The dimensionless distance between two sources is given by the
    "De Ruiter radius", see Ch2&3 of thesis Scheers.

    Here we use a default value of deRuiter_r = 3.717/3600. for a
    reliable association.

    Every found candidate is added to the assoccatsources table.
    """

    _insert_cat_assocs(conn, image_id, radius, deRuiter_r)
    #_insert_cat_assocs_by_bmaj(conn, image_id, radius, deRuiter_r)



