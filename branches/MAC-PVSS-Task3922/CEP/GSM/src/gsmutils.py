#                                                         LOFAR IMAGING PIPELINE
#
#                                                      BBS Source Catalogue List 
#                                                             Bart Scheers, 2011
#                                                           L.H.A.Scheers@uva.nl
# ------------------------------------------------------------------------------

import sys, string
import numpy as np
import monetdb.sql as db
import logging
from gsm_exceptions import GSMException

def expected_fluxes_in_fov(conn, ra_central, decl_central, fov_radius, assoc_theta, bbsfile, 
                                 storespectraplots=False, deruiter_radius=0.,
                                 vlss_flux_cutoff=None):
    """Search for VLSS, WENSS and NVSS sources that
    are in the given FoV. The FoV is set by its central position
    (ra_central, decl_central) out to a radius of fov_radius.
    The query looks for cross-matches around the sources, out
    to a radius of assoc_theta.

    All units are in degrees.

    deruiter_radius is a measure for the association uncertainty that takes
    position errors into account (see thesis Bart Scheers). If not given
    as a positive value, it is read from the TKP config file. If not
    available, it defaults to 3.717.

    The query returns all vlss sources (id) that are in the FoV.
    If so, the counterparts from other catalogues are returned as well 
    (also their ids).
    """
    
    DERUITER_R = deruiter_radius
    if DERUITER_R <= 0:
        try:
            from tkp.config import config
            DERUITER_R = config['source_association']['deruiter_radius']
            ##print "DERUITER_R =",DERUITER_R
        except:
            DERUITER_R=3.717
    
    #TODO: Check what happens at high decl when alpha goes to 180 degrees
    if ra_central - alpha(fov_radius, decl_central) < 0:
        ra_min1 = np.float(ra_central - alpha(fov_radius, decl_central) + 360.0)
        ra_max1 = np.float(360.0)
        ra_min2 = np.float(0.0)
        ra_max2 = np.float(ra_central + alpha(fov_radius, decl_central))
        q = "q_across_ra0"
    elif ra_central + alpha(fov_radius, decl_central) > 360:
        ra_min1 = np.float(ra_central - alpha(fov_radius, decl_central))
        ra_max1 = np.float(360.0)
        ra_min2 = np.float(0.0)
        ra_max2 = np.float(ra_central + alpha(fov_radius, decl_central) - 360)
        q = "q_across_ra0"
    elif ra_central - alpha(fov_radius, decl_central) < 0 and ra_central + alpha(fov_radius, decl_central) > 360:
        raise BaseException("ra = %s > 360 degrees, not implemented yet" % str(ra_central + alpha(fov_radius, decl_central))) 
    else:
        ra_min = np.float(ra_central - alpha(fov_radius, decl_central))
        ra_max = np.float(ra_central + alpha(fov_radius, decl_central))
        q = "q0"
    
    if vlss_flux_cutoff is None:
        vlss_flux_cutoff = 0.
    skymodel = open(bbsfile, 'w')
    header = "# (Name, Type, Ra, Dec, I, Q, U, V, ReferenceFrequency='60e6',  SpectralIndex='[0.0]', MajorAxis, MinorAxis, Orientation) = format\n\n"
    skymodel.write(header)
    status = True

    # This is dimensionless search radius that takes into account 
    # the ra and decl difference between two sources weighted by 
    # their positional errors.
    deRuiter_reduced = DERUITER_R/3600.
    q_across_ra0 = """\
    SELECT t0.v_catsrcid
          ,t0.catsrcname
          ,t1.wm_catsrcid
          ,t2.wp_catsrcid
          ,t3.n_catsrcid
          ,t0.v_flux
          ,t1.wm_flux
          ,t2.wp_flux
          ,t3.n_flux
          ,t0.v_flux_err
          ,t1.wm_flux_err
          ,t2.wp_flux_err
          ,t3.n_flux_err
          ,t1.wm_assoc_distance_arcsec
          ,t1.wm_assoc_r
          ,t2.wp_assoc_distance_arcsec
          ,t2.wp_assoc_r
          ,t3.n_assoc_distance_arcsec
          ,t3.n_assoc_r
          ,t0.pa
          ,t0.major
          ,t0.minor
          ,t0.ra
          ,t0.decl
      FROM (SELECT c1.catsrcid AS v_catsrcid
                  ,c1.catsrcname
                  ,c1.ra
                  ,c1.decl
                  ,c1.i_int_avg AS v_flux
                  ,c1.i_int_avg_err AS v_flux_err
                  ,c1.pa
                  ,c1.major
                  ,c1.minor
              FROM (SELECT catsrcid
                          ,catsrcname
                          ,ra
                          ,decl
                          ,pa
                          ,major
                          ,minor
                          ,i_int_avg
                          ,i_int_avg_err
                      FROM catalogedsources 
                     WHERE cat_id = 4
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND (ra BETWEEN %s
                                   AND %s
                            OR ra BETWEEN %s
                                      AND %s
                           )
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c1
           ) t0
           FULL OUTER JOIN 
           (SELECT c1.catsrcid AS v_catsrcid
                  ,c2.catsrcid AS wm_catsrcid
                  ,c2.i_int_avg AS wm_flux
                  ,c2.i_int_avg_err AS wm_flux_err
                  ,3600 * DEGREES(2 * ASIN(SQRT( (c1.x - c2.x) * (c1.x - c2.x)
                                               + (c1.y - c2.y) * (c1.y - c2.y)
                                               + (c1.z - c2.z) * (c1.z - c2.z)
                                               ) / 2)
                                 ) AS wm_assoc_distance_arcsec
                  ,3600 * SQRT(((c1.ra_mod * COS(RADIANS(c1.decl)) - c2.ra_mod * COS(RADIANS(c2.decl)))
                               * (c1.ra_mod * COS(RADIANS(c1.decl)) - c2.ra_mod * COS(RADIANS(c2.decl)))
                               / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                              + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                               / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))
                              ) AS wm_assoc_r
              FROM (SELECT catsrcid
                          ,MOD(ra + 180, 360) AS ra_mod
                          ,decl
                          ,ra_err
                          ,decl_err
                          ,x
                          ,y
                          ,z
                      FROM catalogedsources 
                     WHERE cat_id = 4
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND (ra BETWEEN %s
                                   AND %s
                            OR ra BETWEEN %s
                                      AND %s
                           )
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c1
                  ,(SELECT catsrcid
                          ,MOD(ra + 180, 360) AS ra_mod
                          ,decl
                          ,ra_err
                          ,decl_err
                          ,x
                          ,y
                          ,z
                          ,i_int_avg
                          ,i_int_avg_err
                      FROM catalogedsources 
                     WHERE cat_id = 5
                       AND (src_type = 'S' OR src_type = 'M')
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND (ra BETWEEN %s
                                   AND %s
                            OR ra BETWEEN %s
                                      AND %s
                           )
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c2
             WHERE c1.x * c2.x + c1.y * c2.y + c1.z * c2.z > COS(RADIANS(%s))
               AND SQRT(((c1.ra_mod * COS(RADIANS(c1.decl)) - c2.ra_mod * COS(RADIANS(c2.decl)))
                        * (c1.ra_mod * COS(RADIANS(c1.decl)) - c2.ra_mod * COS(RADIANS(c2.decl)))
                        / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                        + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                        / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))) < %s
           ) t1
        ON t0.v_catsrcid = t1.v_catsrcid
           FULL OUTER JOIN 
           (SELECT c1.catsrcid AS v_catsrcid
                  ,c2.catsrcid AS wp_catsrcid
                  ,c2.i_int_avg AS wp_flux
                  ,c2.i_int_avg_err AS wp_flux_err
                  ,3600 * DEGREES(2 * ASIN(SQRT( (c1.x - c2.x) * (c1.x - c2.x)
                                               + (c1.y - c2.y) * (c1.y - c2.y)
                                               + (c1.z - c2.z) * (c1.z - c2.z)
                                               ) / 2)
                                 ) AS wp_assoc_distance_arcsec
                  ,3600 * SQRT(( (c1.ra_mod * COS(RADIANS(c1.decl)) - c2.ra_mod * COS(RADIANS(c2.decl)))
                               * (c1.ra_mod * COS(RADIANS(c1.decl)) - c2.ra_mod * COS(RADIANS(c2.decl)))
                               / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                              + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                               / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))
                              ) AS wp_assoc_r
              FROM (SELECT catsrcid
                          ,MOD(ra + 180, 360) AS ra_mod
                          ,decl
                          ,ra_err
                          ,decl_err
                          ,x
                          ,y
                          ,z
                      FROM catalogedsources 
                     WHERE cat_id = 4
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND (ra BETWEEN %s
                                   AND %s
                            OR ra BETWEEN %s
                                      AND %s
                           )
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c1
                  ,(SELECT catsrcid
                          ,MOD(ra + 180, 360) AS ra_mod
                          ,decl
                          ,ra_err
                          ,decl_err
                          ,x
                          ,y
                          ,z
                          ,i_int_avg
                          ,i_int_avg_err
                      FROM catalogedsources 
                     WHERE cat_id = 6
                       AND (src_type = 'S' OR src_type = 'M')
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND (ra BETWEEN %s
                                   AND %s
                            OR ra BETWEEN %s
                                      AND %s
                           )
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c2
             WHERE c1.x * c2.x + c1.y * c2.y + c1.z * c2.z > COS(RADIANS(%s))
               AND SQRT(( (c1.ra_mod * COS(RADIANS(c1.decl)) - c2.ra_mod * COS(RADIANS(c2.decl)))
                        * (c1.ra_mod * COS(RADIANS(c1.decl)) - c2.ra_mod * COS(RADIANS(c2.decl)))
                        / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                        + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                        / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))) < %s
           ) t2
        ON t0.v_catsrcid = t2.v_catsrcid
           FULL OUTER JOIN 
           (SELECT c1.catsrcid AS v_catsrcid
                  ,c2.catsrcid AS n_catsrcid
                  ,c2.i_int_avg AS n_flux
                  ,c2.i_int_avg_err AS n_flux_err
                  ,3600 * DEGREES(2 * ASIN(SQRT( (c1.x - c2.x) * (c1.x - c2.x)
                                               + (c1.y - c2.y) * (c1.y - c2.y)
                                               + (c1.z - c2.z) * (c1.z - c2.z)
                                               ) / 2)
                                 ) AS n_assoc_distance_arcsec
                  ,3600 * SQRT(((c1.ra_mod * COS(RADIANS(c1.decl)) - c2.ra_mod * COS(RADIANS(c2.decl)))
                        * (c1.ra_mod * COS(RADIANS(c1.decl)) - c2.ra_mod * COS(RADIANS(c2.decl)))
                        / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                        + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                        / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))
                        ) AS n_assoc_r
              FROM (SELECT catsrcid
                          ,MOD(ra + 180, 360) AS ra_mod
                          ,decl
                          ,ra_err
                          ,decl_err
                          ,x
                          ,y
                          ,z
                      FROM catalogedsources 
                     WHERE cat_id = 4
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND (ra BETWEEN %s
                                   AND %s
                            OR ra BETWEEN %s
                                      AND %s
                           )
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c1
                  ,(SELECT catsrcid
                          ,MOD(ra + 180, 360) AS ra_mod
                          ,decl
                          ,ra_err
                          ,decl_err
                          ,x
                          ,y
                          ,z
                          ,i_int_avg
                          ,i_int_avg_err
                      FROM catalogedsources 
                     WHERE cat_id = 3
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND (ra BETWEEN %s
                                   AND %s
                            OR ra BETWEEN %s
                                      AND %s
                           )
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c2
             WHERE c1.x * c2.x + c1.y * c2.y + c1.z * c2.z > COS(RADIANS(%s))
               AND SQRT(((c1.ra_mod * COS(RADIANS(c1.decl)) - c2.ra_mod * COS(RADIANS(c2.decl)))
                        * (c1.ra_mod * COS(RADIANS(c1.decl)) - c2.ra_mod * COS(RADIANS(c2.decl)))
                        / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                        + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                        / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))) < %s
           ) t3
        ON t0.v_catsrcid = t3.v_catsrcid
     WHERE t0.v_flux >= %s
    ORDER BY t0.v_catsrcid
    """
    q0 = """\
    SELECT t0.v_catsrcid
          ,t0.catsrcname
          ,t1.wm_catsrcid
          ,t2.wp_catsrcid
          ,t3.n_catsrcid
          ,t0.v_flux
          ,t1.wm_flux
          ,t2.wp_flux
          ,t3.n_flux
          ,t0.v_flux_err
          ,t1.wm_flux_err
          ,t2.wp_flux_err
          ,t3.n_flux_err
          ,t1.wm_assoc_distance_arcsec
          ,t1.wm_assoc_r
          ,t2.wp_assoc_distance_arcsec
          ,t2.wp_assoc_r
          ,t3.n_assoc_distance_arcsec
          ,t3.n_assoc_r
          ,t0.pa
          ,t0.major
          ,t0.minor
          ,t0.ra
          ,t0.decl
      FROM (SELECT c1.catsrcid AS v_catsrcid
                  ,c1.catsrcname
                  ,c1.ra
                  ,c1.decl
                  ,c1.i_int_avg AS v_flux
                  ,c1.i_int_avg_err AS v_flux_err
                  ,c1.pa
                  ,c1.major
                  ,c1.minor
              FROM (SELECT catsrcid
                          ,catsrcname
                          ,ra
                          ,decl
                          ,pa
                          ,major
                          ,minor
                          ,i_int_avg
                          ,i_int_avg_err
                      FROM catalogedsources 
                     WHERE cat_id = 4
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND ra BETWEEN CAST(%s AS DOUBLE) - alpha(%s, %s)
                                  AND CAST(%s AS DOUBLE) + alpha(%s, %s)
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c1
           ) t0
           FULL OUTER JOIN 
           (SELECT c1.catsrcid AS v_catsrcid
                  ,c2.catsrcid AS wm_catsrcid
                  ,c2.i_int_avg AS wm_flux
                  ,c2.i_int_avg_err AS wm_flux_err
                  ,3600 * DEGREES(2 * ASIN(SQRT( (c1.x - c2.x) * (c1.x - c2.x)
                                               + (c1.y - c2.y) * (c1.y - c2.y)
                                               + (c1.z - c2.z) * (c1.z - c2.z)
                                               ) / 2)
                                 ) AS wm_assoc_distance_arcsec
                  ,SQRT(((c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                        * (c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                        / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                        + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                        / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))
                        ) AS wm_assoc_r
              FROM (SELECT catsrcid
                          ,ra
                          ,decl
                          ,ra_err
                          ,decl_err
                          ,x
                          ,y
                          ,z
                      FROM catalogedsources 
                     WHERE cat_id = 4
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND ra BETWEEN CAST(%s AS DOUBLE) - alpha(%s, %s)
                                  AND CAST(%s AS DOUBLE) + alpha(%s, %s)
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c1
                  ,(SELECT catsrcid
                          ,ra
                          ,decl
                          ,ra_err
                          ,decl_err
                          ,x
                          ,y
                          ,z
                          ,i_int_avg
                          ,i_int_avg_err
                      FROM catalogedsources 
                     WHERE cat_id = 5
                       AND (src_type = 'S' OR src_type = 'M')
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND ra BETWEEN CAST(%s AS DOUBLE) - alpha(%s, %s)
                                  AND CAST(%s AS DOUBLE) + alpha(%s, %s)
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c2
             WHERE c1.x * c2.x + c1.y * c2.y + c1.z * c2.z > COS(RADIANS(%s))
               AND SQRT(((c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                        * (c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                        / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                        + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                        / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))) < %s
           ) t1
        ON t0.v_catsrcid = t1.v_catsrcid
           FULL OUTER JOIN 
           (SELECT c1.catsrcid AS v_catsrcid
                  ,c2.catsrcid AS wp_catsrcid
                  ,c2.i_int_avg AS wp_flux
                  ,c2.i_int_avg_err AS wp_flux_err
                  ,3600 * DEGREES(2 * ASIN(SQRT( (c1.x - c2.x) * (c1.x - c2.x)
                                               + (c1.y - c2.y) * (c1.y - c2.y)
                                               + (c1.z - c2.z) * (c1.z - c2.z)
                                               ) / 2)
                                 ) AS wp_assoc_distance_arcsec
                  ,SQRT(((c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                        * (c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                        / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                        + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                        / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))
                        ) AS wp_assoc_r
              FROM (SELECT catsrcid
                          ,ra
                          ,decl
                          ,ra_err
                          ,decl_err
                          ,x
                          ,y
                          ,z
                      FROM catalogedsources 
                     WHERE cat_id = 4
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND ra BETWEEN CAST(%s AS DOUBLE) - alpha(%s, %s)
                                  AND CAST(%s AS DOUBLE) + alpha(%s, %s)
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c1
                  ,(SELECT catsrcid
                          ,ra
                          ,decl
                          ,ra_err
                          ,decl_err
                          ,x
                          ,y
                          ,z
                          ,i_int_avg
                          ,i_int_avg_err
                      FROM catalogedsources 
                     WHERE cat_id = 6
                       AND (src_type = 'S' OR src_type = 'M')
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND ra BETWEEN CAST(%s AS DOUBLE) - alpha(%s, %s)
                                  AND CAST(%s AS DOUBLE) + alpha(%s, %s)
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c2
             WHERE c1.x * c2.x + c1.y * c2.y + c1.z * c2.z > COS(RADIANS(%s))
               AND SQRT(((c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                        * (c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                        / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                        + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                        / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))) < %s
           ) t2
        ON t0.v_catsrcid = t2.v_catsrcid
           FULL OUTER JOIN 
           (SELECT c1.catsrcid AS v_catsrcid
                  ,c2.catsrcid AS n_catsrcid
                  ,c2.i_int_avg AS n_flux
                  ,c2.i_int_avg_err AS n_flux_err
                  ,3600 * DEGREES(2 * ASIN(SQRT( (c1.x - c2.x) * (c1.x - c2.x)
                                               + (c1.y - c2.y) * (c1.y - c2.y)
                                               + (c1.z - c2.z) * (c1.z - c2.z)
                                               ) / 2)
                                 ) AS n_assoc_distance_arcsec
                  ,SQRT(((c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                        * (c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                        / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                        + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                        / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))
                        ) AS n_assoc_r
              FROM (SELECT catsrcid
                          ,ra
                          ,decl
                          ,ra_err
                          ,decl_err
                          ,x
                          ,y
                          ,z
                      FROM catalogedsources 
                     WHERE cat_id = 4
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND ra BETWEEN CAST(%s AS DOUBLE) - alpha(%s, %s)
                                  AND CAST(%s AS DOUBLE) + alpha(%s, %s)
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c1
                  ,(SELECT catsrcid
                          ,ra
                          ,decl
                          ,ra_err
                          ,decl_err
                          ,x
                          ,y
                          ,z
                          ,i_int_avg
                          ,i_int_avg_err
                      FROM catalogedsources 
                     WHERE cat_id = 3
                       AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                                    AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
                       AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                                    AND CAST(%s AS DOUBLE) + %s
                       AND ra BETWEEN CAST(%s AS DOUBLE) - alpha(%s, %s)
                                  AND CAST(%s AS DOUBLE) + alpha(%s, %s)
                       AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
                          + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
                          + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))
                   ) c2
             WHERE c1.x * c2.x + c1.y * c2.y + c1.z * c2.z > COS(RADIANS(%s))
               AND SQRT(((c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                        * (c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                        / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                        + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                        / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))) < %s
           ) t3
        ON t0.v_catsrcid = t3.v_catsrcid
     WHERE t0.v_flux >= %s
    ORDER BY t0.v_catsrcid
    """
    try:
        cursor = conn.cursor()
        if q == "q0":
            query = q0
            cursor.execute(query, (
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_central, fov_radius, decl_central, ra_central, fov_radius, decl_central, 
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_central, fov_radius, decl_central,ra_central, fov_radius, decl_central, 
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_central, fov_radius, decl_central,ra_central, fov_radius, decl_central, 
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     assoc_theta, deRuiter_reduced,
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_central, fov_radius, decl_central,ra_central, fov_radius, decl_central, 
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_central, fov_radius, decl_central,ra_central, fov_radius, decl_central, 
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     assoc_theta, deRuiter_reduced,
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_central, fov_radius, decl_central,ra_central, fov_radius, decl_central, 
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_central, fov_radius, decl_central,ra_central, fov_radius, decl_central, 
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     assoc_theta, deRuiter_reduced,
                     vlss_flux_cutoff
                              ))
        elif q == "q_across_ra0":
            query = q_across_ra0
            cursor.execute(query, (
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_min1, ra_max1, ra_min2, ra_max2,
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_min1, ra_max1, ra_min2, ra_max2,
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_min1, ra_max1, ra_min2, ra_max2,
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     assoc_theta, deRuiter_reduced,
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_min1, ra_max1, ra_min2, ra_max2,
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_min1, ra_max1, ra_min2, ra_max2,
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     assoc_theta, deRuiter_reduced,
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_min1, ra_max1, ra_min2, ra_max2,
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius, decl_central, fov_radius,
                     ra_min1, ra_max1, ra_min2, ra_max2,
                     decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius,
                     assoc_theta, deRuiter_reduced,
                     vlss_flux_cutoff
                              ))
        else:    
            raise BaseException("ra = %s > 360 degrees, not implemented yet" % str(ra_central + alpha(fov_radius, decl_central))) 
        results = zip(*cursor.fetchall())
        cursor.close()
        if len(results) != 0:
            vlss_catsrcid = results[0]
            vlss_name = results[1]
            wenssm_catsrcid = results[2]
            wenssp_catsrcid = results[3]
            nvss_catsrcid = results[4]
            v_flux = results[5]
            wm_flux = results[6]
            wp_flux = results[7]
            n_flux = results[8]
            v_flux_err = results[9]
            wm_flux_err = results[10]
            wp_flux_err = results[11]
            n_flux_err = results[12]
            wm_assoc_distance_arcsec = results[13]
            wm_assoc_r = results[14]
            wp_assoc_distance_arcsec = results[15]
            wp_assoc_r = results[16]
            n_assoc_distance_arcsec = results[17]
            n_assoc_r = results[18]
            pa = results[19]
            major = results[20]
            minor = results[21]
            ra = results[22]
            decl = results[23]
        else:
            status = False
        spectrumfiles = []
        # Check for duplicate vlss_names. This may arise when a VLSS source 
        # is associated with one or more (genuine) counterparts.
        # Eg., if two NVSS sources are seen as counterparts
        # VLSS - WENSS - NVSS_1
        # VLSS - WENSS - NVSS_2
        # two rows will be added to the sky model, where the VLSS name 
        # is postfixed with _0 and _1, resp.
        import collections
        items = collections.defaultdict(list)
        src_name = list(vlss_name)
        for i, item in enumerate(src_name):
            items[item].append(i)
        for item, locs in items.iteritems():
            if len(locs) > 1:
                #print "duplicates of", item, "at", locs
                for j in range(len(locs)):
                    src_name[locs[j]] = src_name[locs[j]] + "_" + str(j)
        if len(results) != 0:
            for i in range(len(vlss_catsrcid)):
                ##print "\ni = ", i
                bbsrow = ""
                # Here we check the cases for the degree of the polynomial spectral index fit
                #print i, vlss_name[i],vlss_catsrcid[i], wenssm_catsrcid[i], wenssp_catsrcid[i], nvss_catsrcid[i]
                # Write the vlss name of the source (either postfixed or not)
                bbsrow += src_name[i] + ", "
                # According to Jess, only sources that have values for all
                # three are considered as GAUSSIAN
                if pa[i] is not None and major[i] is not None and minor[i] is not None:
                    #print "Gaussian:", pa[i], major[i], minor[i]
                    bbsrow += "GAUSSIAN, "
                else:
                    #print "POINT"
                    bbsrow += "POINT, "
                #print "ra = ", ra[i], "; decl = ", decl[i]
                #print "BBS ra = ", ra2bbshms(ra[i]), "; BBS decl = ", decl2bbsdms(decl[i])
                bbsrow += ra2bbshms(ra[i]) + ", " + decl2bbsdms(decl[i]) + ", "
                # Stokes I id default, so filed is empty
                #bbsrow += ", "
                lognu = []
                logflux = []
                lognu.append(np.log10(74.0/60.0))
                logflux.append(np.log10(v_flux[i]))
                if wenssm_catsrcid[i] is not None:
                    lognu.append(np.log10(325.0/60.0))
                    logflux.append(np.log10(wm_flux[i]))
                if wenssp_catsrcid[i] is not None:
                    lognu.append(np.log10(352.0/60.0))
                    logflux.append(np.log10(wp_flux[i]))
                if nvss_catsrcid[i] is not None:
                    lognu.append(np.log10(1400.0/60.0))
                    logflux.append(np.log10(n_flux[i]))
                f = ""
                for j in range(len(logflux)):
                    f += str(10**logflux[j]) + "; "
                ##print f
                #print "len(lognu) = ",len(lognu), "nvss_catsrcid[",i,"] =", nvss_catsrcid[i]
                # Here we write the expected flux values at 60 MHz, and the fitted spectral index and
                # and curvature term
                if len(lognu) == 1:
                    #print "Exp. flux:", 10**(np.log10(v_flux[i]) + 0.7 * np.log10(74.0/60.0))
                    #print "Default -0.7"
                    bbsrow += str(round(10**(np.log10(v_flux[i]) + 0.7 * np.log10(74.0/60.0)), 2)) + ", , , , , "
                    bbsrow += "[-0.7]"
                elif len(lognu) == 2 or (len(lognu) == 3 and nvss_catsrcid[i] is None):
                    #print "Do a 1-degree polynomial fit"
                    # p has form : p(x) = p[0] + p[1]*x
                    p = np.poly1d(np.polyfit(np.array(lognu), np.array(logflux), 1))
                    #print p
                    if storespectraplots:
                        spectrumfile = plotSpectrum(np.array(lognu), np.array(logflux), p, "spectrum_%s.eps" % vlss_name[i])
                        spectrumfiles.append(spectrumfile)
                    # Default reference frequency is reported, so we leave it empty here;
                    # Catalogues just report on Stokes I, so others are empty.
                    bbsrow += str(round(10**p[0], 4)) + ", , , , , "
                    bbsrow += "[" + str(round(p[1], 4)) + "]"
                elif (len(lognu) == 3 and nvss_catsrcid[i] is not None) or len(lognu) == 4:
                    #print "Do a 2-degree polynomial fit"
                    # p has form : p(x) = p[0] + p[1]*x + p[2]*x**2
                    p = np.poly1d(np.polyfit(np.array(lognu), np.array(logflux), 2))
                    #print p
                    if storespectraplots:
                        spectrumfile = plotSpectrum(np.array(lognu), np.array(logflux), p, "spectrum_%s.eps" % vlss_name[i])
                        spectrumfiles.append(spectrumfile)
                    # Default reference frequency is reported, so we leave it empty here
                    bbsrow += str(round(10**p[0], 4)) + ", , , , , "
                    bbsrow += "[" + str(round(p[1],4)) + ", " + str(round(p[2],4)) + "]"
                if pa[i] is not None and major[i] is not None and minor[i] is not None:
                    # Gaussian source:
                    bbsrow += ", " + str(round(major[i], 2)) + ", " + str(round(minor[i], 2)) + ", " + str(round(pa[i], 2))
                #print bbsrow
                skymodel.write(bbsrow + '\n')
            
            if storespectraplots:
                print "Spectra available in:", spectrumfiles
        
        skymodel.close()
        print "Sky model stored in source table:", bbsfile
        
        if not status:
            raise GSMException("Sky Model File %s is empty" % (bbsfile,))

    except db.Error, e:
        logging.warn("Failed on query nr %s; for reason %s" % (query, e))
        raise

def plotSpectrum(x, y, p, f):
    import pylab
    expflux = "Exp. flux: " + str(round(10**p(0),3)) + " Jy"
    fig = pylab.figure()
    ax = fig.add_subplot(111)
    for i in range(len(ax.get_xticklabels())):
        ax.get_xticklabels()[i].set_size('x-large')
    for i in range(len(ax.get_yticklabels())):
        ax.get_yticklabels()[i].set_size('x-large')
    ax.set_xlabel(r'$\log \nu/\nu_0$', size='x-large')
    ax.set_ylabel('$\log S$', size='x-large')
    # Roughly between log10(30/60) and log10(1500/60)
    xp = np.linspace(-0.3, 1.5, 100)
    ax.plot(x, y, 'o', label='cat fluxes')
    ax.plot(0.0, p(0), 'o', color='k', label=expflux )
    ax.plot(xp, p(xp), linestyle='--', linewidth=2, label='fit')
    pylab.legend(numpoints=1, loc='best')
    pylab.grid(True)
    pylab.savefig(f, dpi=600)
    return f


def decl2bbsdms(d):
    """Based on function deg2dec Written by Enno Middelberg 2001
    http://www.atnf.csiro.au/people/Enno.Middelberg/python/python.html
    """
    deg = float(d)
    sign = "+"
    
    # test whether the input numbers are sane:
    
    # if negative, store "-" in sign and continue calulation
    # with positive value
    
    if deg < 0:
        sign = "-"
        deg = deg * (-1)
    
    #if deg > 180:
    #    logging.warn("%s: inputs may not exceed 180!" % deg)
    #    raise
    
    #if deg > 90:
    #    print `deg`+" exceeds 90, will convert it to negative dec\n"
    #    deg=deg-90
    #    sign="-"
    
    if deg < -90 or deg > 90:
        logging.warn("%s: inputs may not exceed 90 degrees!" % deg)
    
    hh = int(deg)
    mm = int((deg - int(deg)) * 60)
    ss = '%10.8f' % (((deg - int(deg)) * 60 - mm) * 60)
    
    #print '\t'+sign+string.zfill(`hh`,2)+':'+string.zfill(`mm`,2)+':'+'%10.8f' % ss
    #print '\t'+sign+string.zfill(`hh`,2)+' '+string.zfill(`mm`,2)+' '+'%10.8f' % ss
    #print '\t'+sign+string.zfill(`hh`,2)+'h'+string.zfill(`mm`,2)+'m'+'%10.8fs\n' % ss
    return sign + string.zfill(`hh`, 2) + '.' + string.zfill(`mm`, 2) + '.' + string.zfill(ss, 11)

def ra2bbshms(a):
    deg=float(a)
    
    # test whether the input numbers are sane:
    
    if deg < 0 or deg > 360:
        logging.warn("%s: inputs may not exceed 90 degrees!" % deg)
    
    hh = int(deg / 15)
    mm = int((deg - 15 * hh) * 4)
    ss = '%10.8f' % ((4 * deg - 60 * hh - mm) * 60)
    
    #print '\t'+string.zfill(`hh`,2)+':'+string.zfill(`mm`,2)+':'+'%10.8f' % ss
    #print '\t'+string.zfill(`hh`,2)+' '+string.zfill(`mm`,2)+' '+'%10.8f' % ss
    #print '\t'+string.zfill(`hh`,2)+'h'+string.zfill(`mm`,2)+'m'+'%10.8fs\n' % ss
    return string.zfill(`hh`, 2) + ':' + string.zfill(`mm`, 2) + ':' + string.zfill(ss, 11)

def alpha(theta, decl):
    if abs(decl) + theta > 89.9:
        return 180.0
    else:
        return degrees(abs(np.arctan(np.sin(radians(theta)) / np.sqrt(abs(np.cos(radians(decl - theta)) * np.cos(radians(decl + theta)))))))

def degrees(r):
    return r * 180 / np.pi

def radians(d):
    return d * np.pi / 180
