#!/usr/bin/env python
#coding: iso-8859-15
import os,sys,time,pg
from database import *


#
# MAIN
#
if __name__ == '__main__':
    """
    repairTree is a temporarely script that adds an 'identifications' field to a given tree
    """

    # check syntax of invocation
    # Expected syntax: copyTree momID database
    if (len(sys.argv) != 3):
        print "Syntax: %s MoMID database" % sys.argv[0]
        sys.exit(1)
    momID  = int(sys.argv[1])
    DBname = sys.argv[2]
    
    # calling stored procedures only works from the pg module for some reason.
    database = pg.connect(user="postgres", host="localhost", dbname=DBname)
    print "Connected to database", DBname

    # Check for tree-existance in both databases.
    DBtree = database.query("select * from gettreelist(0::int2,3::int2,0,'','','') where momid=%d" % momID).dictresult()
    if len(DBtree) == 0:
        print "Tree with MoMId %d not found in database %s" % (momID, DBname)
        sys.exit(1)
    if DBtree[0]['type'] == 10:	# PIC tree?
        print "PIC trees cannot be copied"
        sys.exit(1)

    # What's the version of this tree?
    treeID    = DBtree[0]['treeid']
    nodeDefID = database.query("select * from getTopNode(%d)" % treeID).dictresult()[0]
    nodeInfo  = database.query("select * from getVICnodedef(%s)" % nodeDefID['paramdefid']).dictresult()[0]
    version   = nodeInfo['version']
    print "Tree %d was built with components of version %d" % (treeID, version)
    parentNodes = database.query("select * from VICnodedef where version=%d and name like 'Output_%%'" % version).dictresult()
    for node in parentNodes:
        print DBtree[0]['momid'], treeID, node['nodeid'], node['name'],
        paramid = 0
        idnode = database.query("select * from vicparamdef where nodeid=%d and name='identifications'" % node['nodeid']).dictresult()
        if len(idnode):
            paramid = idnode[0]['paramid']
            print "No need to insert the parameter, paramid=%d" % paramid
        else:
            print "Adding parameter to the component",
            paramid = database.query("select * from savevicparamdef(1,%d,'identifications',212::int2,0::int2,10::int2,100::int2,true,'[]','identifications and topology of the output data products')" % node['nodeid']).getresult()[0]
            print ", paramid=%d" % paramid;

        vicrecs = database.query("select * from vichierarchy where treeid=%d and paramrefid=%d" % (treeID, node['nodeid'])).dictresult()
        if len(vicrecs):
            print "parent node found in victree",
            found = database.query("select * from vichierarchy where treeid=%d and parentid='%d' and name like '%%identifications'" % (treeID, vicrecs[0]['nodeid'])).dictresult()
            if len(found):
                print ", parameter already added, id=%d" % found[0]['nodeid']
            else:
                print ", parameter not in tree, adding it"
                newid = database.query("insert into VIChierarchy(treeID, parentID, paramrefID, name, value) values (%d, %d, %d, '%s.identifications','[]')" % (treeID, vicrecs[0]['nodeid'], paramid, vicrecs[0]['name']))
        else:
            print "parent node NOT in victree, ready"

    database.close()
    sys.exit(0)

