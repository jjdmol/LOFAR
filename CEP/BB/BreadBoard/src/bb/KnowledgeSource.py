#!/usr/bin/env python

"""
A KnowledgeSource

$Id$

"""

import BlackBoard;
import threading;

class KnowledgeSource(threading.Thread):
  """ place holder for utility functions """

  def __init__(self):
    self.stopevent = threading.Event()
    threading.Thread.__init__(self, name=str(self.id))

  def register(self,obj):
    """ register obj as Blackboard for this KnowledgeSource """
    self.bb = obj;

  def run(self):
    while not self.stopevent.isSet():
      self.action();
      self.stopevent.wait(0);

  def join(self,timeout = None):
    self.stopevent.set();
    threading.Thread.join(self,timeout);
