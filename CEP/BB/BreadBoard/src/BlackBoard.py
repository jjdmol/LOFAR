#!/usr/bin/env python

"""
The blackboard package

$Id$

"""
from bb.BlackBoard import BlackBoard;
from bb.Workload import Workload;
from bb.Thread import Thread;

#import bb;


blb = BlackBoard();
blbchld1 = None;
blbchld2 = None;
blb.split_over_time(blbchld1, blbchld2);
thr = Thread();
blbchld1.register(thr);
#wl = Workload();
#blb.register(wl);
blb.time("2003/09/05T00:00:00","2003/09/05T06:00:00");
blb.frequency(20,200);
print repr(blb);
