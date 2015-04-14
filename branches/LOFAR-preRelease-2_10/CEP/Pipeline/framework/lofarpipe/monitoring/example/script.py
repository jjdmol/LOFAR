#! /usr/bin/env python
""" This script will do some memory, CPU and disk usage to illustrate the monitor tool"""

import time
import os


# I/O and some mem:
print "IO operations"
for i in xrange(100):
    print "repeat {0}".format(i)
    f1 = open("/dev/urandom")
    f2 = open("./testfile","w")
    var = f1.read(4024)
    time.sleep(0.01)
    f2.write(var)
    time.sleep(0.01)
    f1.close()
    f2.close()

time.sleep(2)

print "MEM operations"
#mem

b=list()
for i in range(200):
    b.append(range(i, 1000000))
    time.sleep(0.01)

del(b)

time.sleep(2)

#CPU 
print "CPU intensive"
for i in xrange(300000000):
    a = 1093398476662. * 1093398476661.
