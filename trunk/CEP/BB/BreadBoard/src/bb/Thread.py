#!/usr/bin/env python

"""
The Thread class

$Id$

"""

class Thread:
  """a series of actions hopefully leading to a satisfying result"""
  
  root = 0;
  current = 0;

  def __init__(self):
    wl = Workload();
    self.root = wl;
    self.current = wl;

