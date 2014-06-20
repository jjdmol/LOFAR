#!/usr/bin/env python
from _tConvert import *

def dotest(t):

    print ''
    print 'begin dotest'
    print t.testbool (True);
    print t.testbool (False);
    print t.testint (-1);
    print t.testint (10L);
    print t.testint64 (-123456789013L);
    print t.testint64 (123456789014L);
    print t.testssize (-2);
    print t.testssize (11);
    print t.testfloat (3.14);
    print t.testfloat (12);
    print t.teststring ("this is a string");

    print t.testvecint ([1,2,3,4]);
    print t.testvecint ([]);
    print t.testvecint ((-1,-2,-3,-4));
    print t.testvecint (-10);
    print t.testveccomplex ([1+2j, -1-3j, -1.5+2.5j]);
    print t.testvecstr (["a1","a2","b1","b2"])
    print t.testvecstr (())
    print t.testvecstr ("sc1")


t = tConvert();

dotest(t)
