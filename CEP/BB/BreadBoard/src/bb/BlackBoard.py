#!/usr/bin/env python

"""
The blackboard class

$Id$

"""

import pg;

import util.pgdate;
import util.pglist;

import BlackBoardController;
from Dataclass import Dataclass;
import threading;

debug = False;

class BlackBoard(threading.Thread,Dataclass):
  """something containing information about a problem to be solved."""

  def __init__(self, copy = None):
    self.tablename = "blackboards";
    Dataclass.__init__(self);
    self.threads = {};
    self.workload = {};
    self.knowledgeSources = {};
    self.controller =  BlackBoardController.BlackBoardController(self);
    if copy != None:
      self.parent(copy.record["parent"]);
      """ do not copy children """
      self.time(copy.record["start_time"], copy.record["end_time"]);
      self.frequency(copy.record["low_freq"], copy.record["high_freq"]);
    self.refresh_data();
    self.stopevent = threading.Event()
    threading.Thread.__init__(self, name=str(self.id));

  def run(self):
    self.startChildren();
    while not self.stopevent.isSet():
      self.action();
      self.stopevent.wait(0.1);

  def startChildren(self):
    for i in self.knowledgeSources.values():
      i.start();

  def join(self,timeout = None):
    for i in self.knowledgeSources.values():
      print "stopping ", i;
      i.join();
    print "about to stop bb: " , self.id;
    threading.Thread.join(self,timeout)
    

  def refresh_data(self):
    self.record = self.db.get(self.tablename, self.id, "oid");

  def time(self, start, end):
    self.record["start_time"] = start;
    self.record["end_time"] = end;
    qstr ="UPDATE blackboards SET start_time = '"+ str(start) +"' , end_time = '" + str(end) + "' WHERE oid = " + str(self.id)
    data = self.db.query(qstr);

##  def split_over_time(self, early, late):
    
  def split_over_time(self,num_of_parts):
    start = util.pgdate.string2date(self.record["start_time"]);
    end = util.pgdate.string2date(self.record["end_time"]);
    step = ( end - start ) / num_of_parts;
    i = 0;
    lst = [];
    while ( i < num_of_parts ):
      sub = self.controller.fork();
      sub.time(start + i * step, start + ( i + 1 ) * step);
      lst.append(sub);
      i = i+1;
    return lst;

  def split_frequency(self,num_of_parts):
    lower = self.record["low_freq"];
    higher = self.record["high_freq"];
    step = ( higher - lower ) / num_of_parts;
    i = 0;
    lst = [];
    while ( i < num_of_parts ):
      sub = self.controller.fork();
      sub.frequency(lower + i * step, lower + ( i + 1 ) * step);
      lst.append(sub);
      i = i+1;
    return lst;

  def parent(self, parent_id):
    if(parent_id == None):
      parent_id = self.id;
    self.db.query("UPDATE blackboards SET parent = " + str(parent_id) + " WHERE oid = " + str(self.id) );

  def add_child(self, child_id):
    s = self.record["children"];
    s = s[1:(len(s)-1)];
    if (len(s) == 0):
      chldlst = [];
    else:
      chldlst = util.pglist.pgArray2list(s);
    if(child_id not in chldlst):
      chldlst.append(child_id);
      self.record["children"] = util.pglist.list2pgArray(chldlst);
      self.db.query("UPDATE blackboards SET children = '" + self.record["children"] + "' WHERE oid = " + str(self.id) );

  def frequency(self, low, high):
    self.db.query("UPDATE blackboards set low_freq = '"+ str(low) +"' , high_freq = '" + str(high) + "' WHERE oid = " + str(self.id) );

  def range(self, low, high):
    self.db.query("UPDATE blackboards set low_freq = '"+ str(low) +"' , high_freq = '" + str(high) + "' WHERE oid = " + str(self.id) );

  def register(self,obj):
    """not usefull if obj is not a KnowledgeSource"""
    self.knowledgeSources[id(obj)] = obj;
    obj.register(self);

  def say(self,args):
    print "BlackBoard: " + repr(self) + " " + args.__str__();

  def __repr__(self):
    representation = "";
    representation += "<" + self.__class__.__name__ + " instance at " + repr(id(self))  + ">\n";
    representation += str(self.record) + "\n";
    for kskey in self.knowledgeSources.keys():
      representation += "\tks: " + repr(self.knowledgeSources[kskey]) + "\n";
    return representation;

  def action(self):
    debug = True;
    if debug:
      print self.id , " performing action";
    self.stopevent.wait(1);
    done = True;
    for i in self.knowledgeSources.values():
      if i.isAlive():
        print i, " is alive";
        done = False;
    if done:
      if debug:
        print "stopevent set for ", self
      self.stopevent.set();
    if debug:
      print "thread count  : ", threading.activeCount();
##      print "active threads: ", threading.enumerate();
