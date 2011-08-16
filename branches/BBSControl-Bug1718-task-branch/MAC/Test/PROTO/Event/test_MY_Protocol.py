#!/usr/bin/python

from MY_Protocol import *

if __name__ == "__main__":

    ba = ABSBeamAllocEvent()
    print "length=", ba.length

    ba.param1 = [10,20]
    print ba.param1
    ba.param2 = 20

    print ba.param10
    ba.param10 = [1,2,3,4]
    print ba.param10
    
    ba.ext1Dim = 100
    ext1 = int_array(ba.ext1Dim);

    for i in range(ba.ext1Dim):
        ext1[i] = i

    ba.ext1 = ext1

    ba.ext2 = "test_string_ext2"
    ba.ext2Dim = len(ba.ext2)

    ba.ext3 = "test_string_ext3"
    print ba.ext3

    transObj = TransObject(10, 20.0, "test_string_obj1")
    ba.pObj1 = transObj

    ba.onTheDouble = [10.0,2.0,4,5,6.5]
    ba.onTheFloat = [1.0,3*2.4/3.0,2.0/3]

    print ba.onTheDouble
    print ba.onTheFloat

    ba.bounded_string = "test2310923090923-"

    print ba.bounded_string

    print ba.pObj1.value3

    del ext1
    del transObj
    
