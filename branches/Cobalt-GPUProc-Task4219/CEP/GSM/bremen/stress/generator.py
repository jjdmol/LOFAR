#!/usr/bin/python
import math
import sys
from os import path
from numpy import random
from tests.testlib import write_parset

FREQUENCY = {
  1:   30000000,
  2:   34000000,
  3:   38000000,
  4:   42000000,
  5:  120000000,
  6:  130000000,
  7:  140000000,
  8:  150000000,
  9:  160000000,
 10:  170000000,
 11:  325000000,
 12:  352000000,
 13:  640000000,
 14:  850000000,
 15: 1400000000,
 16: 2300000000,
 17: 4800000000,
 18: 8500000000,
 19:   33000000,
 20:   39000000,
 21:   45000000,
 22:   51000000,
 23:   57000000,
 24:   63000000,
 25:   69000000,
 26:   75000000,
}

def generate_field(ra, decl, radius, size):
    for _ in xrange(size):
        rr = radius * math.sqrt(random.random())
        alpha = math.pi * 2 * random.random()
        ra_ = rr * math.cos(alpha) + ra
        decl_ = rr * math.sin(alpha) + decl
        yield ra_, decl_, random.random()


def generate_field_file(filename, ra, decl, radius, size):
    f = open(filename, 'w')
    f.write('# RA DEC Total_flux e_Total_flux\n\n')
    for z in generate_field(ra, decl, radius, size):
        f.write('%s %s %s %s\n' % (z[0], z[1], z[2], 0.01))
    f.close()


def generate_field_parset(filename, ra, decl, radius, size,
                          frequency=8):
    generate_field_file(filename, ra, decl, radius, size)
    parsetname = path.basename(filename)
    parsetname = parsetname[:parsetname.index('.')] + '.parset'
    write_parset(parsetname, filename, FREQUENCY[frequency], ra, decl, radius)

if __name__ == '__main__':
    generate_field_parset(sys.argv[1],
                          float(sys.argv[2]),
                          float(sys.argv[3]),
                          float(sys.argv[4]),
                          int(sys.argv[5]))
