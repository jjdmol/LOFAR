#!/usr/bin/python

#
# Script used for testing the bbs skymodel files 
#
import sys, os, time

import monetdb
import monetdb.sql as db
import gsmutils as gsm 

db_host = "ldb001"
db_dbase = "gsm"
db_user = "gsm"
db_passwd = "msss"
db_port = 50000

try:
    conn = monetdb.sql.connect(hostname=db_host, database=db_dbase, username=db_user, password=db_passwd, port=db_port)
except db.Error, e:
    raise

#ra_c = 289.89258333333333
#decl_c = 0.0017444444444444445

ra_c = 287.80216666666666 
decl_c = 9.096861111111112

#ra_c = 362 
#decl_c = 10

#ra_c = 286.87425
#decl_c = 7.1466111111111115

#ra_c = 70.0
#decl_c = 33.0
fov_radius = 5.0
assoc_theta = 0.025

gsm.expected_fluxes_in_fov(conn, ra_c, decl_c, fov_radius, assoc_theta, 'bbs.skymodel.test', storespectraplots=False)

conn.close()

