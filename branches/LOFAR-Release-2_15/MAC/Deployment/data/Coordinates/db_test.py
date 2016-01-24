#!/usr/bin/env python
#coding: iso-8859-15
import re,sys,pgdb,pg
import database

aDataBase = database.getDBname()
aHost = database.getDBhost()

#
# MAIN
#
if __name__ == '__main__':

    db = pg.DB(user="postgres", host=aHost, dbname=aDataBase)
    print db.query("select * from reference_coord")
    db.close()


