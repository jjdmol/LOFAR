#!/usr/bin/env python

"""
The Controller class

$Id$

"""

import pg;
db = pg.DB(dbname="bb");

import string

from Dataclass import Dataclass;
from KnowledgeSource import KnowledgeSource;
from Workload import Workload;

debug = False;
class Controller(Dataclass,KnowledgeSource):
  """a thread coNtroller"""

  def __init__(self,thread):
    self.tablename = "controllers";
    Dataclass.__init__(self);
    self.thread(thread);
    self.engines = [];
    KnowledgeSource.__init__(self)

  def addEngine(self,eng):
    self.engines.append(eng);
    self.thread().addEngine(eng);

  def action(self):
    debug = True;
    """look for the last job and see if it is done"""
    wl = self.thread().getCurrent();
    wl.refresh_data();
    if(debug):
      print "status: ", wl.status();
    if string.find(wl.status(),"done") > -1:
      if debug:
        print "creating new workload ",;
      wl2 = Workload(workload = wl.getWl(), id=None);
      wl2.status("new");
      self.thread().addWorkload(wl2);
      if debug:
        print "id:   ", wl2.id;
        print "stat: ", wl2.status();
        print "job : ", wl2.job();
        print "pars: ", wl2.parameters();
    else:
      if debug:
        print "current job not done ", wl.status(),
        print "id: " , wl.id,;
        print "engine " , wl.engine();
  
  def thread(self,t=None):
    debug = True;
    if(t):
      self._thread = t
      self.record["thread"] = self._thread.id;
      qstr ="UPDATE " + self.tablename + " SET thread = " + str(self.record["thread"]) +  " WHERE oid = " + str(self.id)
      if debug:
        print qstr;
      data = db.query(qstr);
    return self._thread;
