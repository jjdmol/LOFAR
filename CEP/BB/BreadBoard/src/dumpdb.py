#!/usr/bin/env python
#
# $Id$
#
import pg

db = pg.DB(dbname="bb");

print db.query("select oid,* from  blackboards");
print db.query("select oid,* from watchers");
print db.query("select oid,* from controllers");
print db.query("select oid,* from threads");
print db.query("select oid,* from engines");
print db.query("select oid,* from comparisons");
print db.query("select oid,* from forks");
print db.query("select oid,* from workloads");
