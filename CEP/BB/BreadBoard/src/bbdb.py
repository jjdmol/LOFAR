#!/usr/bin/env python
#
# $Id$
#
import pg

db = pg.DB(dbname="bb");
try:
 db.query("drop table watchers")
except:
 print "watchers didn't exsist"
db.query("CREATE TABLE watchers ( ) WITH OIDS")
try:
 db.query("drop table controllers")
except:
 print "controllers didn't exsist"
db.query("create table controllers ( controller_id oid, strategy text )")
try:
 db.query("drop table threads")
except:
 print "threads didn't exsist"
db.query("create table threads ( thread_id oid )")
try:
 db.query("drop table engines")
except:
 print "engines didn't exsist"
db.query("create table engines ( engine_id oid )")
try:
 db.query("drop table comparisons")
except:
 print "comparisons didn't exsist"
db.query("create table comparisons ( watcher_id oid , controller_id oid)")
try:
 db.query("drop table forks")
except:
 print "forks didn't exsist"
db.query("create table forks ( controller_id oid, child_id oid )")
try:
 db.query("drop table workloads")
except:
 print "workloads didn't exsist"
db.query("create table workloads ( workload_id oid ,\
                                   controller_id oid,\
                                   engine_id oid,\
                                   parent_id oid,\
                                   start_time time,\
                                   end_time time,\
                                   start_freq float,\
                                   end_freq float\
                                  )")
p = {}
p["watcher_id"] = 1
p["controller_id"] = 2
db.insert("comparisons",p)
