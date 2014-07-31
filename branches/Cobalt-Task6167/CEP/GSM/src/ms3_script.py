#!/usr/bin/python

import sys, os, time
from itertools import count
import logging
import tkp.database.database as database
import tkp.database.dataset as ds
import tkp.database.dbregion as reg
import tkp.database.utils as dbu
import monetdb.sql
from tkp.sourcefinder import image
from tkp.config import config
from tkp.utility import accessors, containers

db_enabled = config['database']['enabled']
db_host = config['database']['host']
db_user = config['database']['user']
db_passwd = config['database']['password']
db_dbase = config['database']['name']
db_port = config['database']['port']
db_autocommit = config['database']['autocommit']

basedir = config['test']['datapath']
imagesdir = basedir + '/fits'
regionfilesdir = basedir + '/regions'

if db_enabled:
    db = database.DataBase(host=db_host, name=db_dbase, user=db_user, password=db_passwd, port=db_port, autocommit=db_autocommit)

try:
    iter_start = time.time()
    
    if db_enabled:
        description = 'TRAPPED: LOFAR flare stars'
        dataset = ds.DataSet(data={'dsinname': description}, database=db)
        print "dataset.id:", dataset.id

    i = 0
    files = os.listdir(imagesdir)
    files.sort()
    for file in files:
        my_fitsfile = accessors.FitsFile(imagesdir + '/' + file)
        my_image = accessors.sourcefinder_image_from_accessor(my_fitsfile)
        #print "type(my_image):",type(my_image)
        print "\ni: ", i, "\nfile: ", file
        if db_enabled:
            dbimg = accessors.dbimage_from_accessor(dataset, my_fitsfile)
            print "dbimg.id: ", dbimg.id
        results = my_image.extract()
        print results
        if db_enabled:
            dbu.insert_extracted_sources(db.connection, dbimg.id, results)
            dbu.associate_extracted_sources(db.connection, dbimg.id)
            dbu.associate_with_catalogedsources(db.connection, dbimg.id)
        my_image.clearcache()
        i += 1

    db.close()
except db.Error, e:
    print "Failed for reason: %s " % (e,)
    raise

