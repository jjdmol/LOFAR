#!/usr/bin/env python

"""
The blackboard class

$Id$

"""

import pg;
db = pg.DB(dbname="bb");

class BlackBoard:
  """something containing information about a problem to be solved."""

  def __init__(self):
    self.threads = {};
    self.workload = {};
    self.knowledgeSources = {};
    self.id = db.query("INSERT INTO blackboards DEFAULT VALUES");
    print self.id;

  def time(self, start, end):
    db.query("UPDATE blackboards set starttime = '"+ str(start) +"' , endtime = '" + str(end) + "' WHERE oid = " + str(self.id) );

  def range(self, low, high):
    db.query("UPDATE blackboards set low = '"+ str(low) +"' , high = '" + str(high) + "' WHERE oid = " + str(self.id) );

  def register(self,obj):
    self.knowledgeSources[id(obj)] = obj;

  def say(self,args):
    print "BlackBoard: " + repr(self) + " " + args.__str__();

  def __repr__(self):
    representation = "";
    representation += "<" + self.__class__.__name__ + " instance at " + repr(id(self))  + ">\n";
    for kskey in self.knowledgeSources.keys():
      representation += "\tks: " + repr(self.knowledgeSources[kskey]) + "\n";
    return representation;
