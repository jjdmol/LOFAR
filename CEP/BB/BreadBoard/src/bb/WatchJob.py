#!/usr/bin/env python

"""
The WatchJob class

$Id$

"""

class WatchJob:
  """ an observation job """

  watchedThreads = {};

  def __init__(self,watcher):
    self.watcher = watcher;
    
  def addThread(self, watcher, thread):
    if not self.watchedThreads.has_key(id(watcher)):
      self.watchedThreads[id(watcher)] = [];
    self.watchedThreads[id(watcher)].add(thread);
