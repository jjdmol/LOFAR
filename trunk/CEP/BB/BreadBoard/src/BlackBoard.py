#!/usr/bin/env python

"""
The blackboard package

$Id$

"""
from bb.BlackBoard import BlackBoard;
from bb.Thread import Thread;
from bb.Controller import Controller;
from bb.Engine import Engine;
from bb.Workload import Workload;
from util import paramset;
import util.pglist;
import threading;

#import bb;


blb = BlackBoard();
blbEarlyChild = None;
blbLateChild = None;

# first get the list of children and the assign them, as can be seen
# at the sublevel this order can be reversed.

topLevelChildren = blb.split_over_time(2);
blbEarlyChild = topLevelChildren[0];
blbLateChild = topLevelChildren[1];

# for later use:
newEarlyChild = BlackBoard(copy=blbEarlyChild);

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

numOfParams = 6;

# still fiddling with random in pyton.

paramset.unsetRandom();

ps = util.pglist.list2pgArray(paramset.mkParamSet(numOfParams));

# make the assignments:

##ja1 = util.pglist.list2pgArray(paramset.mkJobAssignment(numOfParams))
##ja1 = util.pglist.list2pgArray([False, True, False, False, True, True, True, False, False, False])
ja1 = util.pglist.list2pgArray([True, False, False, True, True, False]);
##ja2 = util.pglist.list2pgArray(paramset.mkJobAssignment(numOfParams))
##ja2 = util.pglist.list2pgArray([False, True, True, False, False, True, False, False, False, False])
ja2 = util.pglist.list2pgArray([False, True, False, True, False, True]);
##ja3 = util.pglist.list2pgArray(paramset.mkJobAssignment(numOfParams))
##ja3 = util.pglist.list2pgArray([False, True, True, False, False, True, False, False, False, False])
ja3 = util.pglist.list2pgArray([False, False, True, False, True, True]);

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

# give em strategies

thr1cntrl.yetToDo(10);
thr2cntrl.yetToDo(12);
thr3cntrl.yetToDo(14);

## thr1cntrl.yetToDo(1);
## thr2cntrl.yetToDo(2);
## thr3cntrl.yetToDo(4);

# register the knowledge sources for (execution)thread control.

blbEarlyChild.register(thr1cntrl);
blbEarlyChild.register(thr2cntrl);
blbEarlyChild.register(thr3cntrl);
blbEarlyChild.register(eng1);
blbEarlyChild.register(eng2);
blbEarlyChild.register(eng3);

# All done, start the calibration. Actially the master blackboard
# should be started. It should then start all its children and monitor
# their save return.

blbEarlyChild.start();

# the program may not be ended as this gets to the screen. Then again
# it might.
import time;
time.sleep(1)

# wait till its done

print "stage one almost done"

blbEarlyChild.join();

print "stage one done"
print "remaining threads: ", threading.enumerate();

result1 = thr1cntrl.currentState();
result2 = thr2cntrl.currentState();
result3 = thr3cntrl.currentState();

#decide on further work:

wl1 = result1.getWl();
wl2 = result2.getWl();
wl3 = result3.getWl();

print  wl1;
print  wl2;
print  wl3;

p1 = util.pglist.pgArray2list(wl1["parameterset"])[0];
p2 = util.pglist.pgArray2list(wl2["parameterset"])[0];
p3 = util.pglist.pgArray2list(wl3["parameterset"])[0];

print p1
print p2
print p3

p2[0] = p1[0];
p3[0] = p1[0];
p1[1] = p2[1];
p3[1] = p2[1];
p1[2] = p3[2];
p2[2] = p3[2];

print p1
print p2
print p3

wl1["status"] = "new"
wl2["status"] = "new"
wl3["status"] = "new"

wl1["parameterset"] = util.pglist.list2pgArray(p1)
wl2["parameterset"] = util.pglist.list2pgArray(p2)
wl3["parameterset"] = util.pglist.list2pgArray(p3)

print  wl1;
print  wl2;
print  wl3;

# create thread (starts)

## thr4 = Thread(wl1);
## thr5 = Thread(wl2);
## thr6 = Thread(wl3);

thr1.addWorkload(Workload(workload=wl1));
thr2.addWorkload(Workload(workload=wl2));
thr3.addWorkload(Workload(workload=wl3));
## thr4.addWorkload(Workload(workload=wl1));
## thr5.addWorkload(Workload(workload=wl2));
## thr6.addWorkload(Workload(workload=wl3));

## # create controllers for those threads:

thr4cntrl = Controller(thr1);
thr5cntrl = Controller(thr2);
thr6cntrl = Controller(thr3);

## # create the engines to work with:

eng4 = Engine(eng1.id);
eng5 = Engine(eng2.id);
eng6 = Engine(eng3.id);

del(eng1)
del(eng2)
del(eng3)

## # assign the engines to the threads:

thr4cntrl.addEngine(eng4);
thr5cntrl.addEngine(eng5);
thr6cntrl.addEngine(eng6);

thr1cntrl.yetToDo(14);
thr2cntrl.yetToDo(12);
thr3cntrl.yetToDo(10);

## # register the knowledge sources for (execution)thread control.

newEarlyChild.register(thr4cntrl);
newEarlyChild.register(thr5cntrl);
newEarlyChild.register(thr6cntrl);
newEarlyChild.register(eng4);
newEarlyChild.register(eng5);
newEarlyChild.register(eng6);

newEarlyChild.start();

time.sleep(1)

# wait till its done

newEarlyChild.join();

result1 = thr4cntrl.currentState();
result2 = thr5cntrl.currentState();
result3 = thr6cntrl.currentState();

#decide on further work:

wl1 = result1.getWl();
wl2 = result2.getWl();
wl3 = result3.getWl();

print  wl1;
print  wl2;
print  wl3;

print "this is the end"
