#!/usr/bin/env python

"""
The Workload class

$Id$

"""

import pg;
db = pg.DB(dbname="bb");

class Workload:
  """ something to do """

  workload_count = 0;

  def __init__(self):
    self.id = Workload.workload_count = Workload.workload_count + 1;
    self.id = db.query("INSERT INTO workloads DEFAULT VALUES");
    self.data = db.get("workloads", self.id, 'oid');
    print db.query("select oid,* from workloads where oid = '" + str(self.id) + "'");
