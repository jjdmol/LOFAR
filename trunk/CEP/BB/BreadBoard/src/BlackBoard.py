#!/usr/bin/env python

"""
The blackboard class

$Id$

"""

class BlackBoard:
  """something"""
  def __init__(self):
    self.workload_count = 0;
    self.workload = {};
    self.knowledgeSources = {};
  def addWorkload(self):
    self.workload_count = self.workload_count + 1;
  def register(self,obj):
    self.knowledgeSources[id(obj)] = obj;

  def say(self,args):
    print "BlackBoard: " + repr(self) + " " + args.__str__();

  def __repr__(self):
    representation = "";
    representation += "<" + self.__class__.__name__ + " instance at " + repr(id(self))  + ">\n";
    for kskey in self.knowledgeSources.keys():
      representation += "\tks: " + repr(self.knowledgeSources[kskey]) + "\n";
    return representation;


class Workload:
    """ something to do """

    workload_count = 0;

    def __init__(self):
        self.id = Workload.workload_count = Workload.workload_count + 1;

class WatchJob:
  """ an observation job """

  watchedThreads = {};

  def __init__(self,watcher):
    self.watcher = watcher;
    
  def addThread(self, watcher, thread):
    if not self.watchedThreads.has_key(id(watcher)):
      self.watchedThreads[id(watcher)] = [];
    self.watchedThreads[id(watcher)].add(thread);

wl = Workload();
bb = BlackBoard();
bb.register(wl);
wl = Workload();
bb.register(wl);
bb.say(bb.workload_count);
print repr(bb);
