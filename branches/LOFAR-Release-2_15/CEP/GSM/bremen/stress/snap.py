#!/usr/bin/python
import sys
from os import path
from math import sin, cos, radians, degrees, pi, acos
from numpy import random
from tests.testlib import write_parset
from src.gsmconnectionmanager import GSMConnectionManager
from stress.generator import FREQUENCY

ERROR = 0.00001
FLUX_ERROR = 0.1


def get_field(ra, decl, radius, band, min_flux=None):
    """
    Create a query to get sources for a given fov in a given band.
    """
    decl = radians(decl)
    ra = radians(ra)
    x = cos(decl) * cos(ra)
    y = cos(decl) * sin(ra)
    z = sin(decl)
    r = sin(radians(radius))
    sql = """select r.ra, r.decl, r.i_int_avg, r.i_int_avg_err
  from catalogedsources r
 where r.x * {0} + r.y * {1} + r.z * {2} > {3}
   and r.x between {0} - {3} and {0} + {3}
   and r.y between {1} - {3} and {1} + {3}
   and r.z between {2} - {3} and {2} + {3}
   and r.band = {4};""".format(x, y, z, r, band)
    if min_flux:
        sql = "%s\n and r.i_int_avg > %s" % (sql, min_flux)
    print sql
    return sql


def generate_snapshot(filename):
    conn = GSMConnectionManager(database='stress',
                                use_monet=False).get_connection()
    decl_center = -50
    while decl_center < -30:
        decl_center = degrees(acos(2 * random.random() - 1) - 0.5 * pi)
    ra_center = degrees(2 * pi * random.random())
    band = random.random_integers(1, 3)
    sql = get_field(ra_center, decl_center, 3.0, 2)
    cur = conn.get_cursor(sql)
    f = open(filename, 'w')
    f.write('# RA DEC Total_flux e_Total_flux\n\n')
    for data in iter(cur.fetchone, None):
        f.write('%s %s %s %s\n' % (random.normal(data[0], ERROR),
                                   random.normal(data[1], ERROR),
                                   random.normal(data[2], FLUX_ERROR),
                                   data[3]))
    f.close()
    parsetname = path.basename(filename)
    parsetname = parsetname[:parsetname.index('.')] + '.parset'
    write_parset(parsetname, filename, FREQUENCY[band], ra_center, decl_center)


if __name__ == '__main__':
    if sys.argv[1].isdigit():
        for k in xrange(int(sys.argv[1])):
            generate_snapshot('image%s.dat' % k)
    else:
        generate_snapshot(sys.argv[1])
