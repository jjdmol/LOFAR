#!/usr/bin/env python

"""
The Dataclass

$Id$

"""

import pg;
<<<<<<< variant A
import threading;
>>>>>>> variant B
======= end

debug = False;
class Dataclass:
  """ something to do """

  tablename = None;
  id = None;
  username="bb";
  dbname="bb";

  def mkConnection(self):
    self.db = pg.DB(dbname=self.dbname, user=self.username);
    

  def __init__(self):
    connectionCreated = False;
    debug = True;
    while not connectionCreated:
      try:
        self.mkConnection();
        connectionCreated = True;
      except:
        if debug:
          print "connection creation failed: " , self.id;
      #try again

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


