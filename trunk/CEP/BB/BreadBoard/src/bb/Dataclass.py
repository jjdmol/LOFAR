#!/usr/bin/env python

"""
The Dataclass

$Id$

"""

import pg;

db = pg.DB(dbname="bb");

debug = False;
class Dataclass:
  """ something to do """

  tablename = None;
  id = None;

  def __init__(self):
    if(self.tablename):
      if(self.id == None):
        self.id = db.query("INSERT INTO " + self.tablename + " DEFAULT VALUES");
      self.refresh_data();

  def refresh_data(self):
    debug = True;
    if debug:
      print self.tablename, " - ", self.id;
    self.record = db.get(self.tablename, self.id, "oid");


