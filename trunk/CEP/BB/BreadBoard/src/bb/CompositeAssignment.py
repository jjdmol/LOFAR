#!/usr/bin/env python

"""
The CompositeAssignment class

$Id$

"""

from Assignment import Assignment;

class CompositeAssignment(Assignment):
  """
    The composite part of a composite pattern
    containing the work to be performed by this BB system.
  """

  subAssignments = [];

  def __init__(self):
    """
      Don't do a thing for now
    """

  def getQuality(self):
    
    """
    
      Do something with the quality of the children either take the
      last, or compare the last to the first or take the average.

    """

    return quality
