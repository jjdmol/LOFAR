#!/usr/bin/env python

"""
The Controller class

$Id$

"""

from Dataclass import Dataclass;
from KnowledgeSource import KnowledgeSource;

class Controller(Dataclass,KnowledgeSource):
  """a thread coNtroller"""

  def __init__(self,thread):
    self.tablename = "controllers";
    Dataclass.__init__(self);
    self.thread = thread;
    self.engines = [];
    KnowledgeSource.__init__(self)

  def addEngine(self,eng):
    self.engines.append(eng);
    self.thread.addEngine(eng);

  def action(self):
    """look for the last job and see if it is done"""
    wl = self.thread.getCurrent();
    if wl["status"] == "done":
      wl2 = copy(wl);
      wl2["status"] = "new";
      self.thread.addWorkload(wl2);
          
