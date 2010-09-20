#!/usr/bin/env python
#coding: iso-8859-15
import re,sys,pgdb,pg

dataBase = 'donker'

#
# MAIN
#
if __name__ == '__main__':

    db = pg.DB(user="postgres", host="dop50", dbname=dataBase)
    print db.query("select * from reference_coord")
    db.close()

