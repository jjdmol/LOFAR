#!/usr/bin/env python

"""
The blackboard-control class

$Id$

"""

import pg;
db = pg.DB(dbname="bb");

## from BlackBoard import BlackBoard;
from Thread import Thread;

class BlackBoardController:
  """a runtime object creation factory"""

  def __init__(self, bb):
    self.bb = bb;

  def spawn(self,newThread):
    newThread = Thread();

##   def fork(self, newBB):
##     newBB = BlackBoard(self.bb);
##     newBB.parent(self.bb.id);
