#!/usr/bin/env python

"""
The blackboard package

$Id$

"""
from bb.BlackBoard import BlackBoard;
from bb.Workload import Workload;
import bb.Thread;

#import bb;


blb = BlackBoard();
wl = Workload();
blb.register(wl);
blb.time("2003/09/05T00:00:00","2003/09/05T06:00:00");
blb.say(blb.workload_count);
print repr(bb);
