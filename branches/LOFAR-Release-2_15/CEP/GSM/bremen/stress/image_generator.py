#!/usr/bin/python
import sys
from os import path
from numpy import random
from tests.testlib import write_parset
from stress.generator import FREQUENCY

ERROR = 0.000001


def generate_image(filename, sourcename, band, size):
    f = open(sourcename, 'r')
    fo = open(filename, 'w')
    fo.write(f.readline())
    #fo.write(f.readline())
    for _ in xrange(size):
        z = f.readline().split()
        for ik in xrange(2):
            z[ik] = random.normal(z[ik], ERROR)
        fo.write('%s %s %s %s\n' % (z[0], z[1], z[2], 0.01))
    parsetname = path.basename(filename)
    parsetname = parsetname[:parsetname.index('.')] + '.parset'
    write_parset(parsetname, filename, FREQUENCY[band], 180.0, 0.0, 5.0)

    f.close()
    fo.close()


if __name__ == '__main__':
    if sys.argv[1].isdigit():
        for k in xrange(int(sys.argv[1])):
            generate_image('image%s.dat' % k, 'field.dat',
                           int(10 * random.random()) + 1, 100)
    else:
        generate_image(sys.argv[1], 'field.dat',
                       int(10 * random.random()) + 1, 100)
