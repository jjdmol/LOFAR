#!/usr/bin/env python

"""
The blackboard package

$Id$

"""
from bb.BlackBoard import BlackBoard;
from bb.Thread import Thread;
from bb.Controller import Controller;
from bb.Engine import Engine;
from util import paramset;
import util.pglist;

#import bb;


blb = BlackBoard();
blbEarlyChild = None;
blbLateChild = None;

# first get the list of children and the assign them, as can be seen
# at the sublevel this order can be reversed.

topLevelChildren = blb.split_over_time(2);
blbEarlyChild = topLevelChildren[0];
blbLateChild = topLevelChildren[1];

# Adjust the range for the second half of the observation.

blbLateChild.frequency(20,200);

# Initialize the children at the sublevel.

blbLateChildLowChild = None;
blbLateChildMiddleChild = None;
blbLateChildHighChild = None;

# Here we first put the children in the list and then assign them all
# at once. The catch is that you might assign (to) a list with the wrong length.

subLevelChildren = [blbLateChildLowChild, blbLateChildMiddleChild, blbLateChildHighChild];
subLevelChildren = blbLateChild.split_frequency(3);

# Now forget about all of this and work with one of the BlackBoards.
# make a set of assumptions for the parameters. These can be different
# in size for both engines.

numOfParams = 4;

# still fiddling with random in pyton.

paramset.unsetRandom();

ps = util.pglist.list2pgArray(paramset.mkParamSet(numOfParams));

# make the assignments:

##ja1 = util.pglist.list2pgArray(paramset.mkJobAssignment(numOfParams))
##ja1 = util.pglist.list2pgArray([False, True, False, False, True, True, True, False, False, False])
ja1 = util.pglist.list2pgArray([True, False, False, False]);
##ja2 = util.pglist.list2pgArray(paramset.mkJobAssignment(numOfParams))
##ja2 = util.pglist.list2pgArray([False, True, True, False, False, True, False, False, False, False])
ja2 = util.pglist.list2pgArray([False, True, True, False]);
##ja3 = util.pglist.list2pgArray(paramset.mkJobAssignment(numOfParams))
##ja3 = util.pglist.list2pgArray([False, True, True, False, False, True, False, False, False, False])
ja3 = util.pglist.list2pgArray([False, False, True, True]);

# initialize the workloads

wl1 = {};
wl2 = {};
wl3 = {};

wl1["parameterset"] = ps
wl1["jobassignment"] = ja1;
wl1["status"] = "new";

wl2["parameterset"] = ps 
wl2["jobassignment"] = ja2;
wl2["status"] = "new";

wl3["parameterset"] = ps 
wl3["jobassignment"] = ja3;
wl3["status"] = "new";

# create thread (starts)

thr1 = Thread(wl1);
thr2 = Thread(wl2);
thr3 = Thread(wl3);

# create controllers for those threads:

thr1cntrl = Controller(thr1);
thr2cntrl = Controller(thr2);
thr3cntrl = Controller(thr3);

# create the engines to work with:

eng1 = Engine();
eng2 = Engine();
eng3 = Engine();

# assign the engines to the threads:

thr1cntrl.addEngine(eng1);
thr2cntrl.addEngine(eng2);
thr3cntrl.addEngine(eng3);

# register the knowledge sources for (execution)thread control.

blbEarlyChild.register(thr1cntrl);
blbEarlyChild.register(thr2cntrl);
blbEarlyChild.register(thr3cntrl);
blbEarlyChild.register(eng1);
blbEarlyChild.register(eng2);
blbEarlyChild.register(eng3);

# all done start the calibration

blbEarlyChild.start();

# the program may not be ended as this gets to the screen. Then again
# it might.
import time;
time.sleep(10)

print "this is the end"
