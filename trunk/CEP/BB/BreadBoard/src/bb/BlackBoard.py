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
##      print "self.id:= " + str(self.id);
    else:
      rc = db.insert("blackboards",copy.record);
      self.id = rc["oid_blackboards"];
##      print "self.id:= " + str(self.id);
    self.refresh_data();

  def refresh_data(self):
    self.record = db.get("blackboards", self.id, "oid");

  def time(self, start, end):
    self.record["starttime"] = start;
    self.record["endtime"] = end;
##    print "self.record := " + str(self.record);
##    print "self.id:= " + str(self.id);
    qstr ="UPDATE blackboards SET starttime = '"+ str(start) +"' , endtime = '" + str(end) + "' WHERE oid = " + str(self.id)
##    print qstr;
    data = db.query(qstr);
##    print "time updated: " + str(data);

  def split_over_time(self, early, late):
    early = BlackBoard(self);
    late = BlackBoard(self);
##    nu = datetime.now()
    start = util.pgdate.string2date(self.record["starttime"]);
##    end = util.pgdate.pgDate();
    end = util.pgdate.string2date(self.record["endtime"]);
##    print str(start) + " ---- " + str(start + (end - start) /2);
    early.time(start, start + (end - start) /2);
    late.time(start + (end - start) /2, end);


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
