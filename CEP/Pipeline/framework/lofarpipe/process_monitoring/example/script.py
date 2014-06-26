#! /usr/bin/env python
""" This script will do some memory, CPU and disk usage to illustrate the monitor tool"""

import time
import os

time.sleep(20)

# I/O and some mem:
#print "IO"

for i in xrange(100):
    f1 = open("/dev/urandom")
    f2 = open("./testfile","w")
    var = f1.read(4024)
    time.sleep(1)
    f2.write(var)
    time.sleep(1)
    f1.close()
    f2.close()

#print "MEM"
#mem
b=list()
for i in range(100):
    b.append(range(100000))
    time.sleep(1)

del(b)

#print "CPU"

#CPU 
for i in xrange(100000000):
    a = 1093398476662. * 1093398476661.
