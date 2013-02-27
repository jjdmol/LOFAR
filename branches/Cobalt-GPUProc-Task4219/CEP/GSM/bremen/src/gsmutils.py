#                                                         LOFAR IMAGING PIPELINE
#
#                                                      BBS Source Catalogue List
#                                                             Bart Scheers, 2011
#                                                           L.H.A.Scheers@uva.nl
# ------------------------------------------------------------------------------

import sys
import string
import numpy as np
import monetdb.sql as db
import logging

V_FREQ = np.log10(74.0/60.0)
WM_FREQ = np.log10(325.0/60.0)
WP_FREQ = np.log10(352.0/60.0)
N_FREQ = np.log10(1400.0/60.0)

def subquery_catalog(cat_id, ra_central, decl_central, fov_radius,
                     limit_src_type=False):
    """
    Retrieve data for a field of view from a given catalog.
    """
    if limit_src_type:
        src_type = "  AND (src_type = 'S' OR src_type = 'C')"
    else:
        src_type = ""
    return """SELECT catsrcid
      ,catsrcname
      ,ra
      ,decl
      ,ra_err
      ,decl_err
      ,pa
      ,major
      ,minor
      ,x
      ,y
      ,z
      ,i_int_avg
      ,i_int_avg_err
  FROM catalogedsources
 WHERE cat_id = %s %s
   AND zone BETWEEN CAST(FLOOR(CAST(%s AS DOUBLE) - %s) AS INTEGER)
                AND CAST(FLOOR(CAST(%s AS DOUBLE) + %s) AS INTEGER)
   AND decl BETWEEN CAST(%s AS DOUBLE) - %s
                AND CAST(%s AS DOUBLE) + %s
   AND ra BETWEEN CAST(%s AS DOUBLE) - alpha(%s, %s)
              AND CAST(%s AS DOUBLE) + alpha(%s, %s)
   AND x * COS(RADIANS(%s)) * COS(RADIANS(%s))
     + y * COS(RADIANS(%s)) * SIN(RADIANS(%s))
     + z * SIN(RADIANS(%s)) > COS(RADIANS(%s))""" % (
  cat_id, src_type,
  decl_central, fov_radius, decl_central, fov_radius,
  decl_central, fov_radius, decl_central, fov_radius,
  ra_central, fov_radius, decl_central, ra_central, fov_radius, decl_central,
  decl_central, ra_central, decl_central, ra_central, decl_central, fov_radius)


def subquery_catalog_association(cat_id, ra_central, decl_central, fov_radius,
                                 assoc_theta, deRuiter_reduced,
                                 limit_src_type=False):
    """
    Get association information for two catalogs.
    """
    return """SELECT c1.catsrcid AS v_catsrcid
              ,c2.catsrcid AS catsrcid
              ,c2.i_int_avg
              ,c2.i_int_avg_err
              ,3600 * DEGREES(2 * ASIN(SQRT( (c1.x - c2.x) * (c1.x - c2.x)
                                           + (c1.y - c2.y) * (c1.y - c2.y)
                                           + (c1.z - c2.z) * (c1.z - c2.z)
                                           ) / 2)
                             ) AS assoc_distance_arcsec
              ,SQRT(((c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                    * (c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                    / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                    + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                    / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))
                    ) AS assoc_r
          FROM (%s) c1
              ,(%s) c2
         WHERE c1.x * c2.x + c1.y * c2.y + c1.z * c2.z > COS(RADIANS(%s))
           AND (((c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                    * (c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                    / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err))
                    + ((c1.decl - c2.decl) * (c1.decl - c2.decl)
                    / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err))) < %s""" % (
        subquery_catalog(4, ra_central, decl_central, fov_radius),
        subquery_catalog(cat_id, ra_central, decl_central, fov_radius, limit_src_type),
        assoc_theta, deRuiter_reduced
                    )


def expected_fluxes_in_fov(conn, ra_central, decl_central, fov_radius,
                           assoc_theta, bbsfile,
                           storespectraplots=False, deruiter_radius=0.):
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
            print "DERUITER_R =",DERUITER_R
        except:
            DERUITER_R=3.717

    if ra_central + alpha(fov_radius, decl_central) > 360:
        "This will be implemented soon"
        raise BaseException("ra = %s > 360 degrees, not implemented yet" % str(ra_central + alpha(fov_radius, decl_central)))

    skymodel = open(bbsfile, 'w')
    header = "# (Name, Type, Ra, Dec, I, Q, U, V, ReferenceFrequency='60e6',  SpectralIndex='[0.0]', MajorAxis, MinorAxis, Orientation) = format\n\n"
    skymodel.write(header)

    # This is dimensionless search radius that takes into account
    # the ra and decl difference between two sources weighted by
    # their positional errors.
    deRuiter_reduced = (DERUITER_R/3600)**2.
    try:
        cursor = conn.cursor()
        query = """
SELECT t0.v_catsrcid
      ,t0.catsrcname
      ,t1.catsrcid as wm_catsrcid
      ,t2.catsrcid as wp_catsrcid
      ,t3.catsrcid as n_catsrcid
      ,t0.i_int_avg as v_flux
      ,t1.i_int_avg as wm_flux
      ,t2.i_int_avg as wp_flux
      ,t3.i_int_avg as n_flux
      ,t0.i_int_avg_err AS v_flux_err
      ,t1.i_int_avg_err AS wm_flux_err
      ,t2.i_int_avg_err AS wp_flux_err
      ,t3.i_int_avg_err AS n_flux_err
      ,t1.assoc_distance_arcsec as wm_assoc_distance_arcsec
      ,t1.assoc_r as wm_assoc_r
      ,t2.assoc_distance_arcsec as wp_assoc_distance_arcsec
      ,t2.assoc_r as wp_assoc_r
      ,t3.assoc_distance_arcsec as n_assoc_distance_arcsec
      ,t3.assoc_r as n_assoc_r
      ,t0.pa
      ,t0.major
      ,t0.minor
      ,t0.ra
      ,t0.decl
  FROM (SELECT c1.catsrcid AS v_catsrcid
              ,c1.catsrcname
              ,c1.ra
              ,c1.decl
              ,c1.i_int_avg
              ,c1.i_int_avg_err
              ,c1.pa
              ,c1.major
              ,c1.minor
          FROM (%s) c1
       ) t0
       FULL OUTER JOIN
       (%s) t1
    ON t0.v_catsrcid = t1.v_catsrcid
       FULL OUTER JOIN
       (%s) t2
    ON t0.v_catsrcid = t2.v_catsrcid
       FULL OUTER JOIN
       (%s) t3
    ON t0.v_catsrcid = t3.v_catsrcid
        """
        q1 = query % (
                     subquery_catalog(4, ra_central, decl_central, fov_radius),
             subquery_catalog_association(5, ra_central, decl_central,
                                          fov_radius, assoc_theta, deRuiter_reduced,
                                          True),
             subquery_catalog_association(6, ra_central, decl_central,
                                          fov_radius, assoc_theta, deRuiter_reduced,
                                          True),
             subquery_catalog_association(3, ra_central, decl_central,
                                          fov_radius, assoc_theta, deRuiter_reduced,
                                          False)
                     )
        print q1
        #cursor.execute(q1)
        results = None #cursor.fetchone()
        i = 0
        while results:
            vlss_catsrcid, vlss_name, wenssm_catsrcid, wenssp_catsrcid, nvss_catsrcid, \
            v_flux, wm_flux, wp_flux, n_flux, v_flux_err, wm_flux_err, wp_flux_err, n_flux_err, \
            wm_assoc_distance_arcsec, wm_assoc_r, \
            wp_assoc_distance_arcsec, wp_assoc_r, \
            n_assoc_distance_arcsec, n_assoc_r, \
            pa, major, minor, ra, decl = results
            i = i + 1
            spectrumfiles = []
            print "\ni = ", i
            # Here we check the cases for the degree of the polynomial spectral index fit
            print vlss_catsrcid, wenssm_catsrcid, wenssp_catsrcid, nvss_catsrcid
            bbsrow = "%s, %s, %s, %s, " % (vlss_catsrcid, wenssm_catsrcid, wenssp_catsrcid, nvss_catsrcid)
            # According to Jess, only sources that have values for all
            # three are considered as GAUSSIAN
            if pa is not None and major is not None and minor is not None:
                #print "Gaussian:", pa, major, minor
                shape = "GAUSSIAN, "
            else:
                #print "POINT"
                shape = "POINT, "
            bbsrow += "%s, %s, %s, " % (shape, ra2bbshms(ra), decl2bbsdms(decl))
            # Stokes I id default, so filed is empty
            lognu = []
            logflux = []
            lognu.append(V_FREQ)
            logflux.append(np.log10(v_flux))
            if wenssm_catsrcid is not None:
                lognu.append(WM_FREQ)
                logflux.append(np.log10(wm_flux))
            if wenssp_catsrcid is not None:
                lognu.append(WP_FREQ)
                logflux.append(np.log10(wp_flux))
            if nvss_catsrcid is not None:
                lognu.append(N_FREQ)
                logflux.append(np.log10(n_flux))
            # Here we write the expected flux values at 60 MHz, and the fitted spectral index and
            # and curvature term
            if len(lognu) == 1:
                bbsrow += str(round(10**(np.log10(v_flux) + 0.7 * V_FREQ), 2)) + ", , , , , [-0.7]"
            elif len(lognu) == 2 or (len(lognu) == 3 and nvss_catsrcid is None):
                p = np.poly1d(np.polyfit(np.array(lognu), np.array(logflux), 1))
                # Default reference frequency is reported, so we leave it empty here;
                # Catalogues just report on Stokes I, so others are empty.
                bbsrow += "%s, , , , , [%s]" % (str(round(10**p[0], 4)), str(round(p[1],4)))
            elif (len(lognu) == 3 and nvss_catsrcid is not None) or len(lognu) == 4:
                p = np.poly1d(np.polyfit(np.array(lognu), np.array(logflux), 2))
                # Default reference frequency is reported, so we leave it empty here
                bbsrow += "%s, , , , , [%s, %s]" % (str(round(10**p[0], 4)), str(round(p[1],4)), str(round(p[2],4)))
            if storespectraplots:
                spectrumfile = plotSpectrum(np.array(lognu), np.array(logflux), p, "spectrum_%s.eps" % vlss_name)
                spectrumfiles.append(spectrumfile)
            if pa is not None and major is not None and minor is not None:
                # Gaussian source:
                bbsrow += ", %s, %s, %s" % (str(round(major, 2)), str(round(minor, 2)), str(round(pa, 2)))
            #print bbsrow
            skymodel.write(bbsrow + '\n')
            results = cursor.fetchone()

        if storespectraplots:
            print "Spectra available in:", spectrumfiles

        skymodel.close()
        print "Sky model stored in source table:", bbsfile

    except db.Error, e:
        logging.warn("Failed on query nr %s; for reason %s" % (query, e))
        raise
    finally:
        cursor.close()

def plotSpectrum(x, y, p, f):
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
    """
    Based on function deg2dec Written by Enno Middelberg 2001
    http://www.atnf.csiro.au/people/Enno.Middelberg/python/python.html

    >>> decl2bbsdms(1.0)
    '+01.00.00.00000000'
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

    return "%s%02d.%02d.%s" % (sign, hh, mm, string.zfill(ss, 11))

def ra2bbshms(a):
    """
    Convert right ascension from float to hms format.

    >>> ra2bbshms(1.0)
    '00:04:00.00000000'
    """
    deg=float(a)

    # test whether the input numbers are sane:
    if deg < 0 or deg > 360:
        logging.warn("%s: inputs may not exceed 90 degrees!" % deg)

    hh = int(deg / 15)
    mm = int((deg - 15 * hh) * 4)
    ss = '%10.8f' % ((4 * deg - 60 * hh - mm) * 60)

    return "%02d:%02d:%s" % (hh, mm, string.zfill(ss, 11))

def alpha(theta, decl):
    """
    """
    if abs(decl) + theta > 89.9:
        return 180.0
    else:
        return np.degrees(abs(np.arctan(np.sin(np.radians(theta)) /
                       np.sqrt(abs(np.cos(np.radians(decl - theta)) *
                                   np.cos(np.radians(decl + theta)))))))

#def degrees(r):
#    return r * RADIANS_TO_DEGREES
#
#def radians(d):
#    return d * DEGREES_TO_RADIANS
