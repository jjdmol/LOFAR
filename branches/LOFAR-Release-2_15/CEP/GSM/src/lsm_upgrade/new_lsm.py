#!/usr/bin/python
import sys
import string
import pylab
import numpy as np
import monetdb.sql as db

db_host = "localhost"
db_dbase = "barts"
db_user = "barts"
db_passwd = "barts"
db_port = 50000
db_autocommit = True

def alpha(theta, decl):
    """
    """
    if abs(decl) + theta > 89.9:
        return 180.0
    else:
        return np.degrees(abs(np.arctan(np.sin(np.radians(theta)) /
                       np.sqrt(abs(np.cos(np.radians(decl - theta)) *
                                   np.cos(np.radians(decl + theta)))))))

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


try:
    conn = db.connect(hostname=db_host, database=db_dbase, username=db_user, password=db_passwd, port=db_port, autocommit = db_autocommit)
except db.Error, e:
    raise

ra_c = 0.0
decl_c = -34.0
fov_radius = 10.0

sql = """
select catsrcid, ra, decl, pa, minor, major,
       spectral_index_power,
       spectral_index_0, spectral_index_1, spectral_index_2
  from bbs_parameters
 where ra between %(ra)s - %(fova)s and %(ra)s + %(fova)s
   and decl between %(decl)s - %(fov)s and %(decl)s + %(fov)s
   and x * COS(RADIANS(%(decl)s)) * COS(RADIANS(%(ra)s))
     + y * COS(RADIANS(%(decl)s)) * SIN(RADIANS(%(ra)s))
     + z * SIN(RADIANS(%(decl)s)) > COS(RADIANS(%(fov)s))
 order by catsrcid;
""" % { 'ra': ra_c, 'decl': decl_c,
        'fov': fov_radius, 'fova': alpha(fov_radius, decl_c) }
cur = conn.cursor()
cur.execute(sql)
skymodel = open('bbs_new', 'w')
header = "# (Name, Type, Ra, Dec, I, Q, U, V, ReferenceFrequency='60e6',  SpectralIndex='[0.0]', MajorAxis, MinorAxis, Orientation) = format\n\n"
skymodel.write(header)
for xdata in iter(cur.fetchone, None):
    iid, ra, decl, pa, minor, major, spi, sp0, sp1, sp2 = xdata
    if pa is not None and major is not None and minor is not None:
        shape = "GAUSSIAN, "
        addon = ", %.1f, %.1f, %.1f" % (major, minor, pa)
    else:
        shape = "POINT, "
        addon = ""
    bbsrow = "%s, %s, %s, %s, " % (iid, shape, ra2bbshms(ra), decl2bbsdms(decl))
    if spi == 1:
        sps = str(round(sp1,4))
    else:
        sps = "%.4f, %.4f" % (sp1, sp2)
    bbsrow += "%.4f, , , , , [%s]%s" % (sp0, sps, addon)
    skymodel.write(bbsrow + '\n')
cur.close()
skymodel.close()
