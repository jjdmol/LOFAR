#!/usr/bin/env python

"""
The Workload class

$Id$

"""

import pg;
db = pg.DB(dbname="bb");

import util.pgdate;
import util.pglist;

from Assignment import Assignment;
from Dataclass import Dataclass;

debug = False;

class Workload(Assignment,Dataclass):
  """ something to do """

  def __init__(self,workload=None,id = None):
    self.tablename = "workloads"
    self.id = id;
    if not id:
      self.record = db.insert(self.tablename,{"status":"new"});
      self.id = self.record["oid_workloads"];
      ## calling the base add nothing in this case
    Dataclass.__init__(self);
    if(workload):
      self.record["jobassignment"] = util.pglist.list2pgArray(workload["jobassignment"]);
      self.record["parameterset"] = util.pglist.list2pgArray(workload["parameterset"]);
      if( debug ):
        print self.record["parameterset"];
      qstr ="UPDATE " + self.tablename + " SET jobassignment = '"+ str(self.record["jobassignment"]) +"' , parameterset = '" + str(self.record["parameterset"]) + "' WHERE oid = " + str(self.id)
      data = db.query(qstr);

  def getWl(self):
    wl = {};
    wl["jobassignment"] = self.record["jobassignment"];
    wl["parameterset"] = self.record["parameterset"];
    wl["status"] = self.record["status"];    
    return wl;

  def engine(self,eng=None):
    if(eng):
      self.record["engine_id"] = eng.id;
      qstr ="UPDATE " + self.tablename + " SET engine_id = "+ str(self.record["engine_id"]) + " WHERE oid = " + str(self.id)
      data = db.query(qstr);
    return self.record["engine_id"];

  def job(self,j=None):
    if(j):
      self.record["jobassignment"] = util.pglist.list2pgArray(j);
      qstr ="UPDATE " + self.tablename + " SET jobassignment = '"+ str(self.record["jobassignment"]) + "' WHERE oid = " + str(self.id)
      data = db.query(qstr);
    return self.record["jobassignment"];

  def parameters(self,p=None):
    if(p):
      self.record["parameterset"] = util.pglist.list2pgArray(p);
      qstr ="UPDATE " + self.tablename + " SET parameterset = '" + str(self.record["parameterset"]) + "' WHERE oid = " + str(self.id)
      data = db.query(qstr);
    return self.record["parameterset"];

  def status(self,s=None):
    if(s):
      self.record["status"] = s;
      qstr ="UPDATE " + self.tablename + " SET status = '" + self.record["status"] + "' WHERE oid = " + str(self.id)
      data = db.query(qstr);
    return self.record["status"];
    
