#!/usr/bin/env python

"""
The blackboard class

$Id$

"""

import pg;
db = pg.DB(dbname="bb");

import util.pgdate;
import util.pglist;

import BlackBoardController;

class BlackBoard:
  """something containing information about a problem to be solved."""

  def __init__(self, copy = None):
    self.threads = {};
    self.workload = {};
    self.knowledgeSources = {};
    self.controller =  BlackBoardController.BlackBoardController(self);
    self.id = db.query("INSERT INTO blackboards DEFAULT VALUES");
    self.refresh_data();
    if copy != None:
      self.parent(copy.record["parent"]);
      """ do not copy children """
      self.time(copy.record["starttime"], copy.record["endtime"]);
      self.frequency(copy.record["low"], copy.record["high"]);
    self.refresh_data();

  def refresh_data(self):
    self.record = db.get("blackboards", self.id, "oid");

  def time(self, start, end):
    self.record["starttime"] = start;
    self.record["endtime"] = end;
    qstr ="UPDATE blackboards SET starttime = '"+ str(start) +"' , endtime = '" + str(end) + "' WHERE oid = " + str(self.id)
    data = db.query(qstr);

##  def split_over_time(self, early, late):
    
  def split_over_time(self,num_of_parts):
    start = util.pgdate.string2date(self.record["starttime"]);
    end = util.pgdate.string2date(self.record["endtime"]);
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
    lower = self.record["low"];
    higher = self.record["high"];
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
    db.query("UPDATE blackboards SET parent = " + str(parent_id) + " WHERE oid = " + str(self.id) );

  def add_child(self, child_id):
    print  "children: " + self.record["children"];
    s = self.record["children"];
    s = s[1:(len(s)-1)];
    if (len(s) == 0):
      chldlst = [];
    else:
      chldlst = util.pglist.pgArray2list(s);
    print "child list: " + str(chldlst);
    if(child_id not in chldlst):
      chldlst.append(child_id);
      print "child list: " + str(chldlst);
      self.record["children"] = util.pglist.list2pgArray(chldlst);
      print "children: " + str(self.record["children"]);
      db.query("UPDATE blackboards SET children = '" + self.record["children"] + "' WHERE oid = " + str(self.id) );

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
