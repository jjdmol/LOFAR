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
## blb.split_over_time(blbchld1, blbchld2);
smllst = blb.split_over_time(2);
blbchld1 = smllst[0];
blbchld2 = smllst[1];
thr = Thread();
blbchld1.register(thr);
blbchld1.time("2003/09/05T00:00:00","2003/09/05T06:00:00");
blbchld2.frequency(20,200);
blbchld1chld1 = None;
blbchld1chld2 = None;
blbchld1chld3 = None;
smllst = [blbchld1chld1, blbchld1chld2, blbchld1chld3];
smllst = blbchld1.split_frequency(3);
##print repr(blb);
