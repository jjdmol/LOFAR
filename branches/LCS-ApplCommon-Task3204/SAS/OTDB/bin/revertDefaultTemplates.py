#!/usr/bin/env python
#coding: iso-8859-15
import os,sys,time,pg
from database import *

# get info from database.py
dbName=getDBname()
dbHost=getDBhost()

# calling stored procedures only works from the pg module for some reason.
otdb = pg.connect(user="postgres", host=dbHost, dbname=dbName)

#
# MAIN
#
if __name__ == '__main__':
    """
    revertDefaultTemplates reverts each default template in OTDB the has a matching older one.
    Two templates match when the templatename, the processType, the processSubtype and the Strategy values
    only differ in a leading '#'
    """

    # Give user escape possibility
    print "About to REVERT the default templates in database %s on host %s. Starting in 5 seconds..." % (dbName, dbHost)
    time.sleep(5)
    
    print "=> Collecting info about default templates..."
    # built dictionary with componentID, nodeID, nodeName, version and treeName of the default templates like:
    # {6171: (412, 2589, 'LOFAR', 40506, 'master template 4.5.6'), 
    #  6121: (203,  426, 'LOFAR', 40000, 'test template')}
    oldTrees = {}
    newTrees = {}
    dfltTmplInfo = {}
    dfltTemplateIDs = otdb.query("select * from getDefaultTemplates()").dictresult()
    for dfltTemplate in dfltTemplateIDs:
        state     = otdb.query("select state from getTreeInfo(%s, 'false')" % dfltTemplate['treeid']).getresult()[0][0]
        if state == 1200:
            oldTrees[dfltTemplate['name']] = {  \
                     'processType'    : dfltTemplate['processtype'], \
                     'processSubtype' : dfltTemplate['processsubtype'], \
                     'strategy'       : dfltTemplate['strategy'], \
                     'treeID'         : dfltTemplate['treeid'] }
        else:
            newTrees[dfltTemplate['name']] = {  \
                     'processType'    : dfltTemplate['processtype'], \
                     'processSubtype' : dfltTemplate['processsubtype'], \
                     'strategy'       : dfltTemplate['strategy'], \
                     'treeID'         : dfltTemplate['treeid']}
       
    # for each old default template make a new template
    for treeName in newTrees:
        if '#'+treeName in oldTrees:
            oTreeName = '#'+treeName
            if oldTrees[oTreeName]['processType']    == '#'+newTrees[treeName]['processType'] and \
               oldTrees[oTreeName]['processSubtype'] == '#'+newTrees[treeName]['processSubtype'] and \
               oldTrees[oTreeName]['strategy']       == '#'+newTrees[treeName]['strategy']:
               print newTrees[treeName]['treeID'],": ",treeName, newTrees[treeName]['processSubtype'], " <==> ", \
                     oldTrees[oTreeName]['treeID'],": ",oTreeName, oldTrees[oTreeName]['processSubtype']

        # delete new tree
        #print ("select * from deleteTree(1, %s)" % newTrees[treeName]['treeID'])
        otdb.query("select * from deleteTree(1, %s)" % newTrees[treeName]['treeID'])
        # set the old default template state to described (1200)
        oldTreeID = oldTrees[oTreeName]['treeID']
        #print ("select * from settreestate(1, %s, '100')" % (oldTreeID))
        otdb.query("select * from settreestate(1, %s, '100')" % (oldTreeID))
        # rename the old template with a '# ' before its original name
        #print ("select * from assignTemplateName(1, %s, '%s')" % (oldTreeID, oTreeName[1:]))
        otdb.query("select * from assignTemplateName(1, %s, '%s')" % (oldTreeID, oTreeName[1:]))
        #print ("select * from assignProcessType (1, %s, '%s', '%s', '%s')" % (oldTreeID, oldTrees[oTreeName]['processType'][1:], oldTrees[oTreeName]['processSubtype'][1:], oldTrees[oTreeName]['strategy'][1:]))
        otdb.query("select * from assignProcessType (1, %s, '%s', '%s', '%s')" % (oldTreeID, oldTrees[oTreeName]['processType'][1:], oldTrees[oTreeName]['processSubtype'][1:], oldTrees[oTreeName]['strategy'][1:]))

    otdb.close()
    sys.exit(0)
