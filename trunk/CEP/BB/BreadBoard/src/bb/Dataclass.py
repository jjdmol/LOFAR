#!/usr/bin/env python

"""
The Dataclass

$Id$

"""

import pg;

debug = False;
class Dataclass:
  """ something to do """

  tablename = None;
  id = None;

  def __init__(self):
    self.db = pg.DB(dbname="bb");

    if(self.tablename):
      if(self.id == None):
        self.id = self.db.query("INSERT INTO " + self.tablename + " DEFAULT VALUES");
      self.refresh_data();

  def __del__(self):
    self.db.close();

  def refresh_data(self):
    debug = True;
    if debug:
      print self.tablename, " - ", self.id;
    self.record = self.db.get(self.tablename, self.id, "oid");


