#!/usr/bin/env python

"""
The Thread class

$Id$

"""

from CompositeAssignment import CompositeAssignment;
from Assignment import Assignment;

class Thread(CompositeAssignment):
  """a series of actions hopefully leading to a satisfying result"""
  
  root = 0;
  current = 0;

  def addAssignment(self, wl):
    self.subAssignments.append(wl);

  def __init__(self):
    wl = Assignment();
    self.addAssignment(wl);
    self.root = wl;
    self.current = wl;

