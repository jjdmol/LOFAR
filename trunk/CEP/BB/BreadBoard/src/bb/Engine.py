#!/usr/bin/env python

"""
The Engine class

$Id$

"""

import pg;

db = pg.DB(dbname="bb");

from selfcal import selfcal;
from Dataclass import Dataclass;
from KnowledgeSource import KnowledgeSource;
from selfcal.paramset import list2ParamSet;
from util.pglist import pgArray2list;

class Engine(selfcal.SelfcalEngineStub,Dataclass,KnowledgeSource):
  """a predict/solver"""

  def __init__(self):
    self.tablename = "engines";
    Dataclass.__init__(self);
    KnowledgeSource.__init__(self);

  def action(self):
    """ find an assigned action and perform """
    result = db.get("workloads", self.id , "engine_id");
    print "results: " , result.keys();
    if(result["status"] == 'new'):
      print result["parameterset"], result["jobassignment"];
      ps = pgArray2list(result["parameterset"])[0];
      ja = pgArray2list(result["jobassignment"])[0];
      print ps, ja;
      params = list2ParamSet(ps);
      self.init(params["length"],params["data"]);
      job = list2ParamSet(ja);
      self.Solve(job["data"],params["data"]);
      wl = Workload(result["oid"]);
      wl.status("done");

