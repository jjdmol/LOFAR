#!/usr/bin/env python
#
# $Id$
#
import pg

db = pg.DB(dbname="bb",user="bb");

tables = ["blackboards", "watchers", "controllers", "threads", "engines", "comparisons", "forks", "workloads", "range", "parameterlist"];

for table in tables:
  try:
   db.query("TRUNCATE TABLE " + table)
  except:
   print table + " didn't exsist"

