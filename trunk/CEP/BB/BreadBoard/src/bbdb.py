#!/usr/bin/env python
#
# $Id$
#
import pg

db = pg.DB(dbname="bb");

tables = ["blackboards", "watchers", "controllers", "threads", "engines", "comparisons", "forks", "workloads"];

for table in tables:
  try:
   db.query("DROP TABLE " + table)
  except:
   print table + " didn't exsist"

db.query("CREATE TABLE blackboards ( children oid[] default '{}', \
                                     starttime TIMESTAMP DEFAULT now() - interval '6 hours', \
                                     endtime TIMESTAMP DEFAULT CURRENT_TIMESTAMP, \
                                     low FLOAT default '10', \
                                     high FLOAT default '100', \
                                     baseline INTEGER[2] default '{1, 4950}', \
                                     north FLOAT, \
                                     east FLOAT, \
                                     south FLOAT, \
                                     west FLOAT, \
                                     objects INTEGER[2] \
                                    ) WITH OIDS")

## db.query("CREATE FUNCTION bla (VARCHAR,INTEGER) RETURNS INTEGER AS '\
##             BEGIN \
##               RETURN;\
##             END;\
##           ' LANGUAGE 'sql';\
##         ");

## db.query("CREATE FUNCTION no_child (blackboards) RETURNS VOID AS '\
##             DECLARE var  mybb ALIAS FOR $1;\
##             BEGIN\
##               UPDATE blackboards SET children[1] = oid WHERE OID = obj_id ;\
##               RETURN;\
##             END;\
##           ' LANGUAGE 'sql';\
##         ");

## db.query("CREATE TRIGGER no_child BEFORE INSERT ON blackboards FOR EACH ROW EXECUTE PROCEDURE no_child();");

db.query("CREATE TABLE watchers ( ) WITH OIDS")

db.query("CREATE TABLE controllers ( strategy text ) WITH OIDS")

db.query("CREATE TABLE threads ( ) WITH OIDS")

db.query("CREATE TABLE engines (  ) WITH OIDS")

db.query("CREATE TABLE comparisons ( watcher_id OID , controller_id OID) WITH OIDS")

db.query("CREATE TABLE forks ( controller_id OID, child_id OID ) WITH OIDS")

db.query("CREATE TABLE workloads ( controller_id OID,\
                                   engine_id OID,\
                                   parent_id OID,\
                                   start_time TIMESTAMP,\
                                   end_time TIMESTAMP,\
                                   start_freq FLOAT,\
                                   end_freq FLOAT\
                                  ) WITH OIDS")


# block of test code:

p = {}
p["watcher_id"] = db.query("INSERT INTO watchers DEFAULT VALUES");
p["controller_id"] =  db.query("INSERT INTO controllers DEFAULT VALUES");
result = db.insert("comparisons",p)

print result;
print result["oid_comparisons"];

# end of test code

db.query("TRUNCATE TABLE blackboards");
db.query("TRUNCATE TABLE watchers");
db.query("TRUNCATE TABLE controllers");
db.query("TRUNCATE TABLE threads");
db.query("TRUNCATE TABLE engines");
db.query("TRUNCATE TABLE comparisons");
db.query("TRUNCATE TABLE forks");
db.query("TRUNCATE TABLE workloads");
