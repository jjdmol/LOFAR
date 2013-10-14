#!/usr/bin/python

#
# Script used for testing the bbs skymodel files 
#
import sys, os, time

import monetdb
import monetdb.sql as db
import gsmutils as gsm 

db_host = "ldb002"
db_dbase = "gsm"
db_user = "gsm"
db_passwd = "msss"
db_port = 51000
db_autocommit = True

try:
    conn = db.connect(hostname=db_host, database=db_dbase, username=db_user, password=db_passwd, port=db_port, autocommit = db_autocommit)
except db.Error, e:
    raise

#ra_c = 289.89258333333333
#decl_c = 0.0017444444444444445

#ra_c = 287.80216666666666 
#decl_c = 9.096861111111112

#ra_c = 0.2307692222*15.
#decl_c = 90.

#Test Wouter Klijn
#ra_c=2.15373629697 
#decl_c=0.841248699461 
#fov_radius=4.42394511011 
#assoc_theta=0.025


#ra_c = 362 
#decl_c = 10

#ra_c = 286.87425
#decl_c = 7.1466111111111115

ra_c = 70.0
decl_c = 33.0
fov_radius = 5.0
assoc_theta = 0.025
#fov_radius = 15.0
#assoc_theta = 30./3600.

# Test Adam Stewart, Tim Staley
#ra_c = (3.0*15.)
#decl_c = 90.0
#fov_radius = 15
#assoc_theta = (30./3600.)

gsm.expected_fluxes_in_fov(conn, ra_c, decl_c, fov_radius, assoc_theta, 'bbs.skymodel.test', storespectraplots=False, deruiter_radius=3.717, vlss_flux_cutoff=4.)

conn.close()

