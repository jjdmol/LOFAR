#!/usr/bin/env python

"""
The Controller class

$Id$

"""

## import pg;
## db = pg.DB(dbname="bb");

import string

from Dataclass import Dataclass;
from KnowledgeSource import KnowledgeSource;
from Workload import Workload;

from Engine import Engine;

debug = False;
class Controller(Dataclass,KnowledgeSource):
  """a thread controller"""

  jobsToDo = 10;
  jobsDone = 0;
  convergenceCriterium = 10E-2;
  
  def __init__(self,thread):
    debug = True;
    if debug:
      print "creating Controller with thread ", thread;
      
    self.tablename = "controllers";
    Dataclass.__init__(self);
    self.thread(thread);
    self.engines = [];
    KnowledgeSource.__init__(self)

  def addEngine(self,eng):
    """

     <todo>

      here a more proper administration is in place. is there an
      engine per thread? do threads/controllers hold some kind of
      exclusiveness on engines?

     </todo>

    """
    self.engines.append(eng);
    self.thread().addEngine(eng);

  def action(self):
    """look for the last job and see if it is done"""
    debug = True;
    wl = self.thread().getCurrent();
    wl.refresh_data();
    if(debug):
      print "status: ", wl.status();
    if string.find(wl.status(),"done") > -1:
      """
      
       Check the strategy for action to take after the work is
       done. We could just count the workloads done. We could compare
       the parameters with the last set and see is we are satisfied
       with the adjustment.
      
      """

      self.oneDone();

      if self.jobsToDo > 0:
      
        if debug:
          print "jobs to do: ", self.jobsToDo;
          print "creating new workload ",;
        wl2 = Workload(workload = wl.getWl(), id=None, parent=wl.id);
        wl2.status("new");

        """there is a lot of confusion even with the writer of this
        about when to use a real Workload, and when to use a hash
        representation of one."""

        self.thread().addWorkload(wl2);
        self.thread().deleteWorkload(wl);
        j = rotate(wl.job());
        wl2.job(j);
        if debug:
          print "j:    ", j;
          print "id:   ", wl.id;
          print "stat: ", wl.status();
          print "job : ", wl.job();
          print "pars: ", wl.parameters();
          print "id:   ", wl2.id;
          print "stat: ", wl2.status();
          print "job : ", wl2.job();
          print "pars: ", wl2.parameters();
      else:
      ## thread is not a thread!!!!?????!!!!!
##      self.thread().exit();
        print self.engines;
        for i in self.engines:
          i.keepRunning(False);
          i.join();
      ## touch yourself
        self.keepRunning(False);
        print self,".exit()";
        self.exit();
        

    else:
      if debug:
        print "current job not done ", wl.status(),
        print "id: " , wl.id,;
        print "engine " , wl.engine();
          
      
  def thread(self,t=None):
    if(t):
      self._thread = t
      self.record["thread"] = self._thread.id;
      qstr ="UPDATE " + self.tablename + " SET thread = " + str(self.record["thread"]) +  " WHERE oid = " + str(self.id)
      if debug:
        print qstr;
      data = self.db.query(qstr);
    return self._thread;

  def yetToDo(self,jobs):
    if(jobs):
      self.jobsToDo = jobs;
    return self.jobsToDo;

  def oneDone(self):
    self.jobsToDo = self.jobsToDo - 1;
    self.jobsDone = self.jobsDone + 1;

  def countdown(self):
    if self.jobsToDo > 0:
      return True;
    else:
      return False;

  def converge(self,wl):
    
    """

    find out if all deltas for parameters in our job are still bigger
    in absolute value then our convergenceCriterium. A better one
    would be to compare the last delta with the one before and se if
    it is smaller in absolute value.

    """

    done = True;
    dees = wl.deltas();
    job = wl.job();
    for i in range(len(job)):
      if job[i]:
        if fabs(dees[i]) > self.convergenceCriterium:
          done = False;
    return done;

  def currentState(self):
    wl = self.thread().getCurrent();
    wl.refresh_data();
    return wl;


def rotate(wl):
  rc = [];
  l = len(wl);
  for i in range(l/2):
    rc.append(wl[i]);

  # rotate only the second half
  for i in range(l/2):
    rc.append(wl[l/2 + (i+1)%(l/2)]);
  return rc;
