#!/usr/bin/env python

"""
The blackboard class

$Id$

"""

import pg;
db = pg.DB(dbname="bb");

import util.pgdate;

import BlackBoardController;

class BlackBoard:
  """something containing information about a problem to be solved."""

  def __init__(self, copy = None):
    self.threads = {};
    self.workload = {};
    self.knowledgeSources = {};
    self.controller =  BlackBoardController.BlackBoardController(self);
    if copy == None:
      self.id = db.query("INSERT INTO blackboards DEFAULT VALUES");
    else:
      rc = db.insert("blackboards",copy.record);
      self.id = rc["oid_blackboards"];
    self.refresh_data();

  def refresh_data(self):
    self.record = db.get("blackboards", self.id, "oid");

  def time(self, start, end):
    self.record["starttime"] = start;
    self.record["endtime"] = end;
    qstr ="UPDATE blackboards SET starttime = '"+ str(start) +"' , endtime = '" + str(end) + "' WHERE oid = " + str(self.id)
    data = db.query(qstr);

##  def split_over_time(self, early, late):
    
  def split_over_time(self):
    early = BlackBoard(self);
    late = BlackBoard(self);
    start = util.pgdate.string2date(self.record["starttime"]);
    end = util.pgdate.string2date(self.record["endtime"]);
    early.time(start, start + (end - start) /2);
    late.time(start + (end - start) /2, end);
    lst = {};
    lst["early"] = early;
    lst["late"] = late;
    return lst;

  def split_frequency(self):
    low = controller.fork();
    high = controller.fork();
    middel = (self.record["low"] + self.record["high"])/2;
    low.frequency(self.record["low"], middel);
    high.frequency(middel, self.record["high"]);
    lst = {};
    lst["early"] = early;
    lst["late"] = late;
    return lst;

  def parent(self, parent_id):
    db.query("UPDATE blackboards SET parent_id = " + str(parent_id) + " WHERE oid = " + str(self.id) );

  def frequency(self, low, high):
    db.query("UPDATE blackboards set low = '"+ str(low) +"' , high = '" + str(high) + "' WHERE oid = " + str(self.id) );

  def range(self, low, high):
    db.query("UPDATE blackboards set low = '"+ str(low) +"' , high = '" + str(high) + "' WHERE oid = " + str(self.id) );

  def register(self,obj):
    self.knowledgeSources[id(obj)] = obj;

  def say(self,args):
    print "BlackBoard: " + repr(self) + " " + args.__str__();

  def __repr__(self):
    representation = "";
    representation += "<" + self.__class__.__name__ + " instance at " + repr(id(self))  + ">\n";
    representation += str(self.record) + "\n";
    for kskey in self.knowledgeSources.keys():
      representation += "\tks: " + repr(self.knowledgeSources[kskey]) + "\n";
    return representation;
