#!/usr/bin/env python

"""
The Workload class

$Id$

"""

#import pg;
#db = pg.DB(dbname="bb");

import util.pgdate;
import util.pglist;

from Assignment import Assignment;
from Dataclass import Dataclass;

debug = False;

class Workload(Assignment,Dataclass):
  """ something to do """

  def __init__(self,workload=None,id = None , parent = None):
    self.id = id;
    Dataclass.__init__(self);
    self.tablename = "workloads"
    if not id:
      self.record = self.db.insert(self.tablename,{"status":"new"});
      self.id = self.record["oid_workloads"];
      ## calling the base add nothing in this case
    Dataclass.__init__(self);
    if(workload):
      if debug :
        print "jobassignment: " , workload["jobassignment"];
        print "parameterset: " , workload["parameterset"];
      self.record["jobassignment"] = workload["jobassignment"];
      self.record["parameterset"] = workload["parameterset"];
      if( debug ):
        print self.record["parameterset"];
      qstr ="UPDATE " + self.tablename + " SET jobassignment = '"+ str(self.record["jobassignment"]) +"' , parameterset = '" + str(self.record["parameterset"]) + "' WHERE oid = " + str(self.id)
      data = self.db.query(qstr);
    if parent:
      self.parent(parent);

  def getWl(self):
    wl = {};
    wl["jobassignment"] = self.record["jobassignment"];
    wl["parameterset"] = self.record["parameterset"];
    wl["deltas"] = self.record["deltas"];
    wl["status"] = self.record["status"];    
    return wl;

  def engine(self,eng=None):
    if(eng):
      if eng.__class__() == 0:
        self.record["engine_id"] = eng;
      else:
        self.record["engine_id"] = eng.id;
      qstr ="UPDATE " + self.tablename + " SET engine_id = "+ str(self.record["engine_id"]) + " WHERE oid = " + str(self.id)
      data = self.db.query(qstr);
    return self.record["engine_id"];

  def parent(self, par=None):
    if(par):
      if par.__class__() == 0:
        self.record["parent_id"] = par;
      else:
        self.record["parent"] = par.id;
      qstr ="UPDATE " + self.tablename + " SET parent_id = "+ str(self.record["parent_id"]) + " WHERE oid = " + str(self.id)
      data = self.db.query(qstr);
    return self.record["parent_id"];

  def job(self,j=None):
    if(j):
      if debug:
        print "job to set: ", j;
      self.record["jobassignment"] = util.pglist.list2pgArray(j);
      if debug:
        print "pg repres.: ", self.record["jobassignment"];
      qstr ="UPDATE " + self.tablename + " SET jobassignment = '"+ str(self.record["jobassignment"]) + "' WHERE oid = " + str(self.id)
      if debug:
        print qstr;
      data = self.db.query(qstr);
    # this should really return a python list, not a postgresql one.
    return util.pglist.pgArray2booleanList(self.record["jobassignment"]);

  def parameters(self,p=None):
    if(p):
      parr = util.pglist.list2pgArray(p);
      if debug:
        print "p: " , parr;
      self.record["parameterset"] = parr;
      qstr ="UPDATE " + self.tablename + " SET parameterset = '" + str(self.record["parameterset"]) + "' WHERE oid = " + str(self.id)
      data = self.db.query(qstr);
    # this should really return a python list, not a postgresql one.
      if debug:
        print "p: " , self.record["parameterset"];
    return util.pglist.pgArray2list(self.record["parameterset"])[0];

  def deltas(self,d=None):
    debug = False
    if(d):
      dees = util.pglist.list2pgArray(d);
      if debug:
        print "d before: " , dees;
      self.record["deltas"] = dees;
      qstr ="UPDATE " + self.tablename + " SET deltas = '" + str(self.record["deltas"]) + "' WHERE oid = " + str(self.id)
      if debug:
        print qstr;
      data = self.db.query(qstr);
      debug = False
    # this should really return a python list, not a postgresql one.
      if debug:
        print "d after : " , self.record["deltas"];
    return util.pglist.pgArray2list(self.record["deltas"])[0];

  def status(self,s=None):
    if(s):
      self.record["status"] = s;
      qstr ="UPDATE " + self.tablename + " SET status = '" + self.record["status"] + "' WHERE oid = " + str(self.id)
      data = self.db.query(qstr);
    return self.record["status"];
    
