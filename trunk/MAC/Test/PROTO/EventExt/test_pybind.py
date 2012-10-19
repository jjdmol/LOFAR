#!/usr/bin/python

from pybind import *

if __name__ == "__main__":

    ba = ABSBeamAllocEvent()
    print "length=", ba.length
    bae = ABSBeamAllocEventExt(ba)
    print "length=", ba.length

    ba.param1 = 10
    ba.param2 = 100

    print ba.param1
    print ba.param2
    print ba.length
    print ba.signal

    bae.ext1Dim = 100
    ext1 = int_array(bae.ext1Dim)
    for i in range(bae.ext1Dim):
        ext1[i] = i

    bae.ext1 = ext1
    del ext1

    bae.ext2 = "test"
    bae.ext2Dim = len(bae.ext2)

    bae.obj1 = TransObject()
    bae.obj1.value1 = 10
    bae.obj1.value2 = 32.1
   # bae.obj1.value3 = string("klaas jan")

    print bae

    
