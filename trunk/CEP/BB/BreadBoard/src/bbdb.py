#!/usr/bin/env python
#
# $Id$
#
import pg

db = pg.DB(dbname="bb");

try:
 db.query("DROP TABLE blackboards")
except:
 print "blackboards didn't exsist"
db.query("CREATE TABLE blackboards ( starttime DATE, \
                                     endtime DATE, \
                                     low FLOAT default '10', \
                                     high FLOAT default '100', \
                                     north FLOAT, \
                                     east FLOAT, \
                                     south FLOAT, \
                                     west FLOAT, \
                                     first INTEGER, \
                                     last INTEGER\
                                    ) WITH OIDS")

try:
 db.query("DROP TABLE watchers")
except:
 print "watchers didn't exsist"
db.query("CREATE TABLE watchers ( ) WITH OIDS")

try:
 db.query("DROP TABLE controllers")
except:
 print "controllers didn't exsist"
db.query("CREATE TABLE controllers ( strategy text ) WITH OIDS")

try:
 db.query("DROP TABLE threads")
except:
 print "threads didn't exsist"
db.query("CREATE TABLE threads ( ) WITH OIDS")

try:
 db.query("DROP TABLE engines")
except:
 print "engines didn't exsist"
db.query("CREATE TABLE engines (  ) WITH OIDS")

try:
 db.query("DROP TABLE comparisons")
except:
 print "comparisons didn't exsist"
db.query("CREATE TABLE comparisons ( watcher_id oid , controller_id oid) WITH OIDS")

try:
 db.query("DROP TABLE forks")
except:
 print "forks didn't exsist"
db.query("CREATE TABLE forks ( controller_id oid, child_id oid ) WITH OIDS")

try:
 db.query("DROP TABLE workloads")
except:
 print "workloads didn't exsist"
db.query("CREATE TABLE workloads ( controller_id oid,\
                                   engine_id oid,\
                                   parent_id oid,\
                                   start_time timestamp,\
                                   end_time timestamp,\
                                   start_freq float,\
                                   end_freq float\
                                  ) WITH OIDS")

p = {}
p["watcher_id"] = db.query("INSERT INTO watchers DEFAULT VALUES");
p["controller_id"] =  db.query("INSERT INTO controllers DEFAULT VALUES");
result = db.insert("comparisons",p)

print result;
print result["oid_comparisons"];
