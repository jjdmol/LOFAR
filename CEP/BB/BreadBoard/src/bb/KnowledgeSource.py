#!/usr/bin/env python

"""
A KnowledgeSource

$Id$

"""

import BlackBoard;
import threading;
import thread;

debug = False;

class KnowledgeSource(threading.Thread):
  """ place holder for utility functions """

  def __init__(self):
    self.stopevent = threading.Event()
    self.keepsOnRunning = True;
    threading.Thread.__init__(self, name=str(self.id))

  def register(self,obj):
    """ register obj as Blackboard for this KnowledgeSource """
    self.bb = obj;

  def run(self):
    while not self.stopevent.isSet():
      if(debug):
        print "action by " , self.id;
      self.action();
      self.stopevent.wait(1);

  def keepRunning(keepsOnRunning = None):
    if keepsOnRunning :
      self.keepsOnRunning = keepsOnRunning;
      print self.id, " ordered to stop";
    return self.keepsOnRunning;

  def join(self,timeout = None):
    if not self.keepsOnRunning :
      self.stopevent.set();
      print self.id, " stopping"
    threading.Thread.join(self,timeout);

  def exit(self):
    debug = True;
    if debug:
      print "exiting ", self.getName();
    thread.exit();
