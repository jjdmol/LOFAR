#!/usr/bin/env python

"""
The Engine class

$Id$

"""

import pg;

#db = pg.DB(dbname="bb");

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

  def __init__(self, id = None):
    self.tablename = "engines";
    self.id = id;
    Dataclass.__init__(self);
    KnowledgeSource.__init__(self);
    self.engineStub = selfcal.SelfcalEngineStub()

  def action(self):
    debug = True;
    """ find an assigned action and perform """
    import pgdb
    """pgdb must be patched not to search for typprtlen in pg_type"""
    connectionCreated = False;
    newdb = None; # is for local use only
    while not connectionCreated:
      try:
        newdb = pgdb.connect(database=Dataclass.dbname, user=Dataclass.username)
        connectionCreated = True;
      except:
        if debug:
          print "connection creation failed: " , self.id;
      #try again

    cursor = newdb.cursor();
    cursor.execute("SELECT oid FROM workloads WHERE engine_id = " + str(self.id) + " AND status = 'new'")
    record = cursor.fetchone();
##    result = self.db.get("workloads", self.id , "engine_id");
    cursor.close();
    newdb.close();

    if record != None:
      record_id = record[0];
      result = self.db.get("workloads", record_id, "oid" );
    
      if(debug):
        print "parameterset: " , result["parameterset"], " -- assignment: ", result["jobassignment"];
      old_ps = pgArray2list(result["parameterset"])[0];
      ja = pgArray2booleanList(result["jobassignment"]);
      if(debug):
        print old_ps, ja;
      params = list2ParamSet(old_ps);
      self.engineStub.init(params["length"],params["data"]);
      job = list2JobAssignment(ja);
      outp = {};
      outp["length"] = params["length"]
      
      outp["data"] = selfcal.pars_frompointer(self.engineStub.Solve(job["data"],params["data"]));
      wl = Workload(id = record_id);
      wl.status("done");
      # fill wl["deltas"]
      new_ps = ParamSet2list(outp) ;
      wl.parameters(new_ps);
      ## loop over params to calc delta values
      deltas = [];
      for i  in range(len(old_ps)):
        deltas.append(new_ps[i] - old_ps[i]);
      wl.deltas(deltas);
      
      if debug:
        print "old set : ", old_ps ;
        print "new set : ", new_ps;
        print "new par : ", wl.parameters() ;
        print "deltas  : ", deltas;
        print "new dees: ", deltas;
        print "done";
      debug = False;
