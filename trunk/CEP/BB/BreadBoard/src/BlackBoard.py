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
## blb.examine_time();
blb.split_over_time(blbchld1, blbchld2);
## thr = Thread();
## blbchld1.register(thr);
## blbchld1.time("2003/09/05T00:00:00","2003/09/05T06:00:00");
## blbchld2.frequency(20,200);
print repr(blb);
