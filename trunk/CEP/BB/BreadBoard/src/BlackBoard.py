#!/usr/bin/env python

"""
The blackboard package

$Id$

"""
from bb.BlackBoard import BlackBoard;
from bb.Workload import Workload;
from bb.Thread import Thread;
from bb.Controller import Controller;
from bb.Engine import Engine;
from util import paramset;

#import bb;


blb = BlackBoard();
blbchld1 = None;
blbchld2 = None;
## blb.split_over_time(blbchld1, blbchld2);
smllst = blb.split_over_time(2);
blbchld1 = smllst[0];
blbchld2 = smllst[1];
numOfParams = 10;
ps = paramset.mkParamSet(numOfParams);
wl1 = wl2 = {};
wl1["parameterset"] = ps
paramset.unsetRandom();
wl1["jobassignment"] = paramset.mkJobAssignment(numOfParams)
wl1["status"] = "new";
wl2["parameterset"] = ps 
wl2["jobassignment"] = paramset.mkJobAssignment(numOfParams)
wl2["status"] = "new";
thr1 = Thread(wl1);
thr2 = Thread(wl2);
thrcntrl1 = Controller(thr1);
thrcntrl2 = Controller(thr2);
blbchld1.register(thrcntrl1);
blbchld1.register(thrcntrl2);
eng1 = Engine();
eng2 = Engine();
blbchld1.register(eng1);
blbchld1.register(eng2);
thrcntrl1.addEngine(eng1);
thrcntrl2.addEngine(eng2);
blbchld1.start();
blbchld2.frequency(20,200);
blbchld2chld1 = None;
blbchld2chld2 = None;
blbchld2chld3 = None;
smllst = [blbchld2chld1, blbchld2chld2, blbchld2chld3];
smllst = blbchld2.split_frequency(3);
##print repr(blb);


