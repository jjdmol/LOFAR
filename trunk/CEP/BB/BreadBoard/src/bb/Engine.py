#!/usr/bin/env python

"""
The Engine class

$Id$

"""

import pg;

db = pg.DB(dbname="bb");

import string

from selfcal import selfcal;
from Dataclass import Dataclass;
from Workload import Workload;
from KnowledgeSource import KnowledgeSource;
from selfcal.paramset import list2ParamSet;
from selfcal.paramset import ParamSet2list;
from selfcal.paramset import list2JobAssignment;
from util.pglist import pgArray2list;
from util.pglist import pgArray2booleanList;

debug = False;

class Engine(Dataclass,KnowledgeSource):
  """a predict/solver"""

  def __init__(self):
    self.tablename = "engines";
    Dataclass.__init__(self);
    KnowledgeSource.__init__(self);
    self.engineStub = selfcal.SelfcalEngineStub()

  def action(self):
    """ find an assigned action and perform """
    debug = True
    result = db.get("workloads", self.id , "engine_id");
    if(debug):
      print "results: " , result.keys();
    if(string.find(result["status"],'new') > -1):
      if(debug):
        print result["parameterset"], result["jobassignment"];
      ps = pgArray2list(result["parameterset"])[0];
      ja = pgArray2booleanList(result["jobassignment"]);
      if(debug):
        print ps, ja;
      params = list2ParamSet(ps);
      self.engineStub.init(params["length"],params["data"]);
      job = list2JobAssignment(ja);
      outp = self.engineStub.Solve(job["data"],params["data"]);
      if debug:
        print "-- " , outp, " --";
      wl = Workload(id = result["oid"]);
      wl.status("done");
      wl.parameters(ParamSet2list(outp));
      if debug:
        print "done";
