#!/usr/bin/env python

"""
A KnowledgeSource

$Id$

"""

import BlackBoard;
import threading;

debug = False;

class KnowledgeSource(threading.Thread):
  """ place holder for utility functions """

  def __init__(self):
    self.stopevent = threading.Event()
    threading.Thread.__init__(self, name=str(self.id))

  def register(self,obj):
    """ register obj as Blackboard for this KnowledgeSource """
    self.bb = obj;

  def run(self):
    debug = True;
    while not self.stopevent.isSet():
      if(debug):
        print "action by " , self.id;
      self.action();
      self.stopevent.wait(1);

  def join(self,timeout = None):
    self.stopevent.set();
    print self.id, " stopping"
    threading.Thread.join(self,timeout);
